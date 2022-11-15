#include "IOManagerDmsProxy.h"

#include "ErrorRequest.h"
#include "IO/IORefreshToken.h"

#include "IOManagerAzure.h"
#include "IOManagerAzurePresigned.h"

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

  m_ioManagerFactory.reset(DMSIOManagerFactory::createDMSIOManagerFactory(m_dmsDataset->m_service_provider, *m_dmsDataset, m_logger, error));
}

IOManagerDMSProxy::~IOManagerDMSProxy()
{
}

bool IOManagerDMSProxy::Close(uint64_t serializedSize, uint64_t chunkCount, Error& error)
{
  if (m_dmsDataset)
  {
    bool ret = m_dmsDataset->close(serializedSize, chunkCount, error);
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
    if (!m_dmsDataset->close(0, 0, error))
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
  std::string toRead = objectName.empty() ? m_filename : objectName;
  return m_proxy->ReadObjectInfo(toRead, handler);
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
  std::string toRead = objectName.empty() ? m_filename : objectName;
  return m_proxy->ReadObject(toRead, handler, range);
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