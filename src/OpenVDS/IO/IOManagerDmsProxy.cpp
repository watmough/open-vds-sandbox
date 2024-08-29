#include "IOManagerDmsProxy.h"

#include "ErrorRequest.h"
#include "IO/IORefreshToken.h"

namespace OpenVDS
{

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
  , m_curlHandler(error, m_logger, CurlHandler::defaultMaxHostConnections, openOptions.httpProxy)
  , m_ioManagerCurlHandler(std::make_shared<CurlHandler>(error, m_logger, CurlHandler::defaultMaxHostConnections))
  , m_dmsManager(new DmsManager(openOptions.sdAuthorityUrl, openOptions.sdApiKey, m_curlHandler, m_logger))
{
  if (m_dmsManager->m_authorityUrl.empty())
  {
    error.code = -1;
    error.string = "DMS OpenOptions sdAuthorityUrl is empty.";
    return;
  }
  if (openOptions.authProviderCallback)
  {
    m_dmsManager->m_authProviderCallback = openOptions.authProviderCallback;
    m_dmsManager->m_authProviderCallbackData = openOptions.authProviderCallbackData;
  }
  else if (openOptions.authTokenUrl.size() && openOptions.clientId.size() && (openOptions.refreshToken.size() || openOptions.clientSecret.size()))
  {
    m_tokenRefresher.reset(new TokenRefresher(openOptions.authTokenUrl, openOptions.clientId, openOptions.clientSecret, openOptions.scopes, openOptions.refreshToken, m_curlHandler, std::function<void(std::string&& new_token)>()));
    m_dmsManager->m_authProviderCallback = AuthProviderCallback;
    m_dmsManager->m_authProviderCallbackData = this;
  }
  else
  {
    m_dmsManager->m_sdToken = openOptions.sdToken;
    m_dmsManager->m_sdTokenExpiry = std::chrono::system_clock::now() + std::chrono::hours(168 * 32);
  }

  m_dmsDataset.reset(new DmsDataset(*m_dmsManager.get(), openOptions.datasetPath, error, openOptions.legalTag));
  if (error.code)
    return;
  if (!m_dmsDataset->open(accessPattern, error))
    return;

  m_ioManagerFactory.reset(DmsIoManagerFactory::createDmsIoManagerFactory(m_dmsDataset->m_service_provider, *m_dmsDataset, m_logger, error));
}

IOManagerDMSProxy::~IOManagerDMSProxy()
{
}

std::shared_ptr<IOManager> IOManagerDMSProxy::ensureIOManager(Error& error)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  if (m_proxy && m_expirationTime > std::chrono::steady_clock::now() + std::chrono::minutes(1))
    return m_proxy;
  if (!m_dmsManager->ensureSdToken(error))
    return nullptr;
  m_proxy = m_ioManagerFactory->createIOManager(m_ioManagerCurlHandler, m_expirationTime, error);
  return m_proxy;
}

void IOManagerDMSProxy::invalidate()
{
  m_expirationTime = {};
}

bool IOManagerDMSProxy::Close(uint64_t serializedSize, uint64_t chunkCount, Error& error)
{
  if (m_dmsDataset)
  {
    bool ret = m_dmsDataset->close(serializedSize, chunkCount, error);
    if (ret)
      invalidate();
    return ret;
  }
  return true;
}

bool IOManagerDMSProxy::EnableWriting(Error& error)
{
  if (m_accessPattern == IOManager::ReadOnly)
  {
    if (!m_dmsDataset->close(0, 0, error))
      return false;
    if (!m_dmsDataset->open(IOManager::ReadWrite, error))
      return false;
    invalidate();
    m_accessPattern = IOManager::ReadWrite;
  }
  return true;
}

std::shared_ptr<Request> IOManagerDMSProxy::ReadObjectInfo(const std::string& objectName, std::shared_ptr<TransferDownloadHandler> handler)
{
  Error error;
  auto ioManager = ensureIOManager(error);
  if (!ioManager)
  {
    return std::make_shared<ErrorRequest>(objectName, std::move(error.string));
  }
  return ioManager->ReadObjectInfo(objectName, handler);
}

std::shared_ptr<Request> IOManagerDMSProxy::ReadObject(const std::string& objectName, std::shared_ptr<TransferDownloadHandler> handler, const IORange& range)
{
  Error error;
  auto ioManager = ensureIOManager(error);
  if (!ioManager)
  {
    return std::make_shared<ErrorRequest>(objectName, std::move(error.string));
  }
  return ioManager->ReadObject(objectName, handler, range);
}

std::shared_ptr<Request> IOManagerDMSProxy::WriteObject(const std::string& objectName, const std::string& contentDispostionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::function<void(const Request& request, const Error& error)> completedCallback)
{
  Error error;
  auto ioManager = ensureIOManager(error);
  if (!ioManager)
  {
    return std::make_shared<ErrorRequest>(objectName, std::move(error.string));
  }
  return ioManager->WriteObject(objectName, contentDispostionFilename, contentType, metadataHeader, data, completedCallback);
}

std::string IOManagerDMSProxy::GetLegalTag() const {
  return m_dmsDataset->m_legalTag;
}

int IOManagerDMSProxy::GetObjectCount() const {
  return m_dmsDataset->m_nobjects;
}
}
