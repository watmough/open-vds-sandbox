#include "IOManagerDmsProxy.h"

#include "IORefreshToken.h"
#include <chrono>
#include <random>
#include "json_cpp_include.h"
#include "Base64/Base64.h"
#include <fmt/chrono.h>

#include "SDPath.h"
#include "VDS/Url.h"

#include "ErrorRequest.h"

#include "IOManagerAzure.h"
#include "IOManagerAzurePresigned.h"

namespace OpenVDS
{

static std::vector<std::string> split(const std::string& text, char sep) 
{
  std::vector<std::string> tokens;
  std::size_t start = 0, end = 0;
  while ((end = text.find(sep, start)) != std::string::npos) {
    tokens.push_back(text.substr(start, end - start));
    start = end + 1;
  }
  tokens.push_back(text.substr(start));
  return tokens;
}

static bool ParseJSONFromBuffer(const std::vector<unsigned char> &json, Json::Value &root, Error &error)
{
  try
  {
    Json::CharReaderBuilder rbuilder;
    rbuilder["collectComments"] = false;

    std::unique_ptr<Json::CharReader> reader(rbuilder.newCharReader());
    const char *json_begin = reinterpret_cast<const char *>(json.data());
    reader->parse(json_begin, json_begin + json.size(), &root, &error.string);

    return true;
  }
  catch(Json::Exception &e)
  {
    error.code = -1;
    error.string = e.what() + std::string(" : ") + error.string;
  }

  return false;
}

struct DMSManager
{
  DMSManager(const std::string& authorityUrl, const std::string& appKey, CurlHandler &curlHandler, Logger& logger)
    : m_authorityUrl(authorityUrl)
    , m_appKey(appKey)
    , m_curlHandler(curlHandler)
    , m_logger(logger)
    , m_authProviderCallback(nullptr)
    , m_authProviderCallbackData(nullptr)
  {}

  void setSdTokenDefaultExpiry()
  {
    m_logger.LogWarning("Failed to parse sd token to find expiry, assuming one hour.");
    m_sdTokenExpiry = std::chrono::system_clock::now() + std::chrono::hours(1);
  }
  bool ensureSdToken(Error &error)
  {
    if (m_sdTokenExpiry < std::chrono::system_clock::now() + std::chrono::minutes(1))
    {
      try
      {
        m_sdToken = m_authProviderCallback(m_authProviderCallbackData);
      }
      catch (const std::runtime_error& ex)
      {
        error.code = -1;
        error.string = ex.what();
        return false;
      }

      if (m_sdToken.empty())
      {
        error.code = -1;
        error.string = "Got empty sdToken from AuthProviderCallback.";
        return false;
      }

      auto tokens = split(m_sdToken, '.');
      if (tokens.size() < 3)
      {
        setSdTokenDefaultExpiry();
        return true;
      }

      auto payload = tokens[1];
      std::vector<unsigned char> decodedData;
      if (!Base64Decode(payload.c_str(), payload.size(), decodedData))
      {
        setSdTokenDefaultExpiry();
        return true;
      }
      Json::Value root;
      if (!ParseJSONFromBuffer(decodedData, root, error))
      {
        error = OpenVDS::Error();
        setSdTokenDefaultExpiry();
        return true;
      }
      try
      {
        if (!root.isMember("exp"))
        {
          setSdTokenDefaultExpiry();
          return true;
        }
        m_sdTokenExpiry = std::chrono::system_clock::from_time_t(time_t(root["exp"].asUInt64()));
      }
      catch (Json::Exception& e)
      {
        (void)e;
        setSdTokenDefaultExpiry();
        return true;
      }
    }
    return true;
  }

  const std::string m_authorityUrl;
  const std::string m_appKey;
  CurlHandler& m_curlHandler;
  std::string m_sdToken;
  std::chrono::time_point<std::chrono::system_clock> m_sdTokenExpiry;
  Logger& m_logger;
  std::string (*m_authProviderCallback)(const void*);
  const void *m_authProviderCallbackData;
};

static std::string gen_lock_id(IOManager::AccessPattern accessPattern)
{
    const char hex_characters[]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0, 15);

    std::string ret;
    ret.reserve(16);
    ret.push_back(accessPattern == IOManager::ReadOnly ? 'R' : 'W');
    for (int i = 0; i < 15; i++)
    {
      ret.push_back(hex_characters[dist(mt)]);
    }
    return ret;
}

struct DMSDataset
{
  DMSDataset(DMSManager& manager, const std::string url, Error &error)
    : m_manager(manager)
    , m_url(url)
    , m_accessPattern(IOManager::ReadOnly)
    , m_opened(false)
  {
    if (!getSdPath(url, m_tenant, m_subproject, m_path, m_dataset, error))
      return;

    if (m_path.empty())
      m_path = '/';
  }

  bool open(IOManager::AccessPattern accessPattern, Error &error)
  {
    if (m_opened)
    {
      error.code = -1;
      error.string = "Seismic DMS: Opening an allready opened DatasetInstance.";
      return false;
    }
    if (!m_manager.ensureSdToken(error))
      return false;

    {
      std::string url;
      if (accessPattern == IOManager::Create)
        url = fmt::format("{}/dataset/tenant/{}/subproject/{}/dataset/{}?path={}", m_manager.m_authorityUrl, URLEncode(m_tenant), URLEncode(m_subproject), URLEncode(m_dataset), URLEncode(m_path));
      else
        url = fmt::format("{}/dataset/tenant/{}/subproject/{}/dataset/{}/lock?openmode={}&path={}", m_manager.m_authorityUrl, URLEncode(m_tenant), URLEncode(m_subproject), URLEncode(m_dataset), accessPattern == IOManager::ReadOnly ? "read" : "write", URLEncode(m_path));
      std::shared_ptr<UploadRequestCurl> request = std::make_shared<UploadRequestCurl>("lock_dataset", std::function<void(const Request& request, const Error& error)>());
      std::vector<std::string> headers;
      headers.emplace_back("Content-Type: application/json");
      m_lock_id = gen_lock_id(accessPattern);
      headers.emplace_back(fmt::format("x-seismic-dms-lockid: {}", m_lock_id));
      headers.emplace_back(fmt::format("appkey: {}", m_manager.m_appKey));
      headers.emplace_back(fmt::format("authorization: Bearer {}", m_manager.m_sdToken));
      m_manager.m_curlHandler.addUploadRequest(request, url, headers, accessPattern == IOManager::Create ? CurlVerb::POST : CurlVerb::PUT, {}, 0);
      request->WaitForFinish(error);
      std::vector<unsigned char> respons_data;
      respons_data.insert(respons_data.end(), request->m_uploadHandler->responsData.begin(), request->m_uploadHandler->responsData.end());
      if (error.code || !request->m_uploadHandler)
      {
        std::string respons_str;
        respons_str.insert(respons_str.end(), respons_data.begin(), respons_data.end());
        error.string = fmt::format("Seismic dms lock failed: {} - {}", error.string, respons_str);
        return false;
      }
      m_accessPattern = accessPattern;
      m_opened = true;
      Json::Value root;
      if (!ParseJSONFromBuffer(respons_data, root, error))
      {
        return false;
      }
      try
      {
        m_gc_url = root["gcsurl"].asString();
      }
      catch (const Json::Exception& ex)
      {
        error.code = -1;
        error.string = fmt::format("Seismic dms lock failed: {}", ex.what());
        return false;
      }

      for (auto& header : request->m_uploadHandler->responsHeaders)
      {
        if (header.first == "service-provider")
          m_service_provider = header.second;
      }

      if (m_service_provider.empty())
      {
        error.code = -1;
        error.string = "Seismic dms lock failed: Missing service-provider header";
        return false;
      }
    }
    return true;
  }

  bool close(Error& error)
  {
    if (!m_opened)
    {
      error.code = -1;
      error.string = "Seismic DMS: Closing an already closed DatasetInstance.";
      return false;
    }
    if (!m_manager.ensureSdToken(error))
      return false;

    std::string url = fmt::format("{}/dataset/tenant/{}/subproject/{}/dataset/{}?path={}&close={}", m_manager.m_authorityUrl, URLEncode(m_tenant), URLEncode(m_subproject), URLEncode(m_dataset), URLEncode(m_path), m_lock_id);
    std::shared_ptr<UploadRequestCurl> request = std::make_shared<UploadRequestCurl>("lock_dataset", std::function<void(const Request& request, const Error& error)>());
    std::vector<std::string> headers;
    headers.emplace_back("Content-Type: application/json");
    headers.emplace_back(fmt::format("x-seismic-dms-lockid: {}", m_lock_id));
    headers.emplace_back(fmt::format("appkey: {}", m_manager.m_appKey));
    headers.emplace_back(fmt::format("authorization: Bearer {}", m_manager.m_sdToken));
    m_manager.m_curlHandler.addUploadRequest(request, url, headers, CurlVerb::PATCH, {}, 0);
    request->WaitForFinish(error);
    std::vector<unsigned char> respons_data;
    respons_data.insert(respons_data.end(), request->m_uploadHandler->responsData.begin(), request->m_uploadHandler->responsData.end());
    if (error.code || !request->m_uploadHandler)
    {
      std::string respons_str;
      respons_str.insert(respons_str.end(), respons_data.begin(), respons_data.end());
      error.string = fmt::format("Seismic dms lock failed: {} - {}", error.string, respons_str);
      return false;
    }
    Json::Value root;
    if (!ParseJSONFromBuffer(respons_data, root, error))
    {
      return false;
    }
    try
    {
      m_gc_url = root["gcsurl"].asString();
    }
    catch (const Json::Exception& ex)
    {
      error.code = -1;
      error.string = fmt::format("Seismic dms lock failed: {}", ex.what());
      return false;
    }

    for (auto& header : request->m_uploadHandler->responsHeaders)
    {
      if (header.first == "service-provider")
        m_service_provider = header.second;
    }

    if (m_service_provider.empty())
    {
      error.code = -1;
      error.string = "Seismic dms lock failed: Missing service-provider header";
      return false;
    }

    m_opened = false;
    return true;

  }

  DMSManager& m_manager;
  const std::string m_url;
  std::string m_tenant;
  std::string m_subproject;
  std::string m_path;
  std::string m_dataset;
  std::string m_lock_id;
  std::string m_service_provider;
  std::string m_gc_url;
  IOManager::AccessPattern m_accessPattern;
  bool m_opened;
  std::chrono::time_point<std::chrono::steady_clock> m_azure_sas_token_expires;
};

struct DMSIOManagerFactory
{
  DMSIOManagerFactory(DMSDataset& dataset)
    : m_dataset(dataset)
  {}

  virtual bool ensureIOManager(std::unique_ptr<IOManager>& iomanager, Error& error) = 0;
  virtual void invalidate() = 0;

  struct GcsAccessToken
  {
    std::vector<std::pair<std::string, std::string>> headers;
    std::vector<uint8_t> data;
  };
  GcsAccessToken gcsAccessToken(Error &error)
  {
      GcsAccessToken ret;
      std::string readonly = m_dataset.m_accessPattern == IOManager::ReadOnly ? "true" : "false";

      std::string sdPath = URLEncode(fmt::format("sd://{}/{}", m_dataset.m_tenant, m_dataset.m_subproject));
      std::string url = fmt::format("{}/utility/gcs-access-token?readonly={}&sdpath={}", m_dataset.m_manager.m_authorityUrl, readonly, sdPath);
      std::shared_ptr<DownloadRequestCurl> request = std::make_shared<DownloadRequestCurl>("gcs-access-token", nullptr);
      std::vector<std::string> headers;
      headers.emplace_back("Content-Type: application/json");
      headers.emplace_back(fmt::format("appkey: {}", m_dataset.m_manager.m_appKey));
      headers.emplace_back(fmt::format("authorization: Bearer {}", m_dataset.m_manager.m_sdToken));
      m_dataset.m_manager.m_curlHandler.addDownloadRequest(request, url, headers, {}, CurlVerb::GET);
      request->WaitForFinish(error);
      if (error.code || !request->m_downloadHandler)
      {
        std::string respons_str;
        respons_str.insert(respons_str.end(), request->m_downloadHandler->responseData.begin(), request->m_downloadHandler->responseData.end());
        error.string = fmt::format("Seismic DMS: gcs-access-token failed: {} - {}", error.string, respons_str);
        return ret;
      }
      ret.data = std::move(request->m_downloadHandler->responseData);
      ret.headers = std::move(request->m_downloadHandler->responseHeaders);
      return ret;
  }
  
  DMSDataset& m_dataset;
};

template<size_t SIZE>
static bool startsWith(const std::string& source, const char (&starts_with)[SIZE])
{
  return source.rfind(starts_with, 0) == 0;
}

static bool getComponentsFromSAS(const std::string& sas, std::string& account, std::string& container, std::string& bearer, Error &error)
{
  auto current = sas.begin();
  auto end = sas.end();
  if (startsWith(sas, "https://"))
    current += 8;
  else if (startsWith(sas, "http://"))
    current += 7;
  if (current == end)
  {
    error.code = -1;
    error.string = "Empty sas token.";
    return false;
  }
  auto account_end = std::find(current, end, '.');
  if (account_end == end)
  {
    error.code = -1;
    error.string = "Missing account in sas token.";
    return false;
  }
  account = std::string(current, account_end);
  current = account_end++;
  auto container_start = std::find(current, end, '/');
  while (container_start != end && *container_start == '/')
    container_start++;
  auto container_end = std::find(container_start, end, '?');
  if (container_start == end || container_end == end)
  {
    error.code = -1;
    error.string = "Missing container in sas token.";
    return false;

  }
  container = std::string(container_start, container_end);
  bearer = std::string(container_end + 1, end);
  return true;
}

static void getContainerAndBlobPrefix(const std::string& gcsurl, std::string& container, std::string& blobPrefix)
{
  auto current = gcsurl.begin();
  auto end = gcsurl.end();
  auto container_end = std::find(current, end, '/');
  container = std::string(current, container_end);
  while (container_end != end && *container_end == '/')
    container_end++;
  blobPrefix = std::string(container_end, end);
}

static bool getPresignedUrl(const std::string& url, const std::string& blob_prefix, std::string &presignedUrl, std::string &presignedSuffix, Error &error)
{
  auto it = std::find(url.begin(), url.end(), '?');
  if (it == url.end())
  {
    error.code = -1;
    error.string = "Failed to generate presigned url";
    return false;
  }

  presignedUrl = std::string(url.begin(), it);
  if (blob_prefix.size())
  {
    presignedUrl += '/' + blob_prefix;
  }
  presignedSuffix = std::string(it, url.end());
  return true;
}

struct AzureDMSIOManagerFactory : public DMSIOManagerFactory
{
  AzureDMSIOManagerFactory(DMSDataset &dataset)
    : DMSIOManagerFactory(dataset)
  {}

  bool ensureIOManager(std::unique_ptr<IOManager> &ioManager, Error& error) override
  {
    if (ioManager && m_expire > std::chrono::steady_clock::now() + std::chrono::minutes(1))
      return true;
    if (!m_dataset.m_manager.ensureSdToken(error))
      return false;

    auto gcs_access_token = gcsAccessToken(error);
    if (error.code)
      return false;

    std::string access_token;
    Json::Value root;
    if (!ParseJSONFromBuffer(gcs_access_token.data, root, error))
      return false;
    try
    {
      access_token = root["access_token"].asString();
      int expires_in = root["expires_in"].asInt();
      m_expire = std::chrono::steady_clock::now() + std::chrono::seconds(expires_in);
    }
    catch (Json::Exception& ex)
    {
      error.code = -1;
      error.string = fmt::format("Seismic DMS: gcs-access-token json error: {}", ex.what());
      return false;
    }

    std::string account;
    std::string container;
    std::string bearer;
    if (!getComponentsFromSAS(access_token, account, container, bearer, error))
    {
      return false;
    }
    std::string container_2;
    std::string blob_prefix;
    getContainerAndBlobPrefix(m_dataset.m_gc_url, container_2, blob_prefix);
    AzureOpenOptions openOptions;
    openOptions.accountName = account;
    openOptions.container = container;
    openOptions.blob = blob_prefix;
    

    std::string presignedUrl;
    std::string presignedSuffix;
    if (!getPresignedUrl(access_token, blob_prefix, presignedUrl, presignedSuffix, error))
      return false;
    ioManager.reset(new IOManagerAzurePresigned(presignedUrl, presignedSuffix, m_dataset.m_manager.m_logger, error));

    //ioManager.reset(new IOManagerAzure(openOptions, error));
    if (error.code)
    {
      ioManager.reset();
      return false;
    }
    return true;
  }

  void invalidate() override
  {
    m_expire = {};
  }
  std::chrono::time_point<std::chrono::steady_clock> m_expire;
};
class DMSProxyAuthProviderException : public std::runtime_error
{
public:
  DMSProxyAuthProviderException(const std::string& what)
    : std::runtime_error(what)
  {}
};
std::string IOManagerDMSProxy::AuthProviderCallback(const void* data)
{
  IOManagerDMSProxy* iomanager = static_cast<IOManagerDMSProxy*>(const_cast<void*>(data));
  Error error;
  std::string token = iomanager->m_tokenRefresher->newToken(error);
  if (error.code)
  {
    throw DMSProxyAuthProviderException(error.string);
  }
  return token;
}

IOManagerDMSProxy::IOManagerDMSProxy(const DMSOpenOptions& openOptions, IOManager::AccessPattern accessPattern, const Logger& logger, Error& error)
  : IOManager(OpenOptions::DMS)
  , m_accessPattern(accessPattern)
  , m_logger(logger)
  , m_curlHandler(error, m_logger)
  , m_useFileNameForSingleFileDatasets(openOptions.useFileNameForSingleFileDatasets)
  , m_dmsManager(new DMSManager(openOptions.sdAuthorityUrl, openOptions.sdApiKey, m_curlHandler, m_logger))
{
  if (openOptions.datasetPath.size() && m_useFileNameForSingleFileDatasets)
  {
    auto it = openOptions.datasetPath.rfind('/');
    if (it == openOptions.datasetPath.size() - 1)
    {
      it = openOptions.datasetPath.rfind('/', 1);
    }
    if (it != std::string::npos)
    {
      m_filename = openOptions.datasetPath.substr(it + 1);
    }
  }
  else
  {
    m_filename = "0";
  }
  if (openOptions.authProviderCallback)
  {
    m_dmsManager->m_authProviderCallback = openOptions.authProviderCallback;
    m_dmsManager->m_authProviderCallbackData = openOptions.authProviderCallbackData;
  }
  else if (openOptions.authTokenUrl.size() && openOptions.clientId.size() && openOptions.refreshToken.size())
  {
    m_tokenRefresher.reset(new TokenRefresher(openOptions.authTokenUrl, openOptions.clientId, openOptions.clientSecret, openOptions.scopes, openOptions.refreshToken, m_curlHandler, std::function<void(std::string&& new_token)>()));
    m_dmsManager->m_authProviderCallback = AuthProviderCallback;
    m_dmsManager->m_authProviderCallbackData = this;
  }
  else
  {
    m_dmsManager->m_sdToken = openOptions.sdToken;
  }

  m_dmsDataset.reset(new DMSDataset(*m_dmsManager.get(), openOptions.datasetPath, error));
  if (error.code)
    return;
  if (!m_dmsDataset->open(accessPattern, error))
    return;

  if (m_dmsDataset->m_service_provider == "azure")
    m_ioManagerFactory.reset(new AzureDMSIOManagerFactory(*m_dmsDataset));
}

IOManagerDMSProxy::~IOManagerDMSProxy()
{
}

bool IOManagerDMSProxy::Close(Error& error)
{
  if (m_dmsDataset)
  {
    bool ret = m_dmsDataset->close(error);
    if (ret)
      m_ioManagerFactory->invalidate();
    return ret;
  }
  return true;
}
      
bool IOManagerDMSProxy::EnableWriting(Error& error)
{
  if (m_accessPattern == IOManager::ReadOnly)
  {
    if (!m_dmsDataset->close(error))
      return false;
    if (!m_dmsDataset->open(IOManager::ReadWrite, error))
      return false;
    m_ioManagerFactory->invalidate();
    m_accessPattern = IOManager::ReadWrite;
  }
  return true;
}

std::shared_ptr<Request> IOManagerDMSProxy::ReadObjectInfo(const std::string& objectName, std::shared_ptr<TransferDownloadHandler> handler)
{
  Error error;
  if (!m_ioManagerFactory->ensureIOManager(m_proxy, error))
  {
    return std::make_shared<ErrorRequest>(objectName, std::move(error.string));
  }
  if (!m_proxy)
  {
    return std::make_shared<ErrorRequest>(objectName, "Seismic DMS initialization not successful. No request created\n");
  }
  return m_proxy->ReadObjectInfo(objectName, handler);
}
std::shared_ptr<Request> IOManagerDMSProxy::ReadObject(const std::string& objectName, std::shared_ptr<TransferDownloadHandler> handler, const IORange& range)
{
  Error error;
  if (!m_ioManagerFactory->ensureIOManager(m_proxy, error))
  {
    return std::make_shared<ErrorRequest>(objectName, std::move(error.string));
  }
  if (!m_proxy)
  {
    return std::make_shared<ErrorRequest>(objectName, "Seismic DMS initialization not successful. No request created\n");
  }
  return m_proxy->ReadObject(objectName, handler, range);
}
std::shared_ptr<Request> IOManagerDMSProxy::WriteObject(const std::string& objectName, const std::string& contentDispostionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::function<void(const Request& request, const Error& error)> completedCallback)
{
  Error error;
  if (!m_ioManagerFactory->ensureIOManager(m_proxy, error))
  {
    return std::make_shared<ErrorRequest>(objectName, std::move(error.string));
  }
  if (!m_proxy)
  {
    return std::make_shared<ErrorRequest>(objectName, "Seismic DMS initialization not successful. No request created\n");
  }
  return m_proxy->WriteObject(objectName, contentDispostionFilename, contentType, metadataHeader, data, completedCallback);
}
}