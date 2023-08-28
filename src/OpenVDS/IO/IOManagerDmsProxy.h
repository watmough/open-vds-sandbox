#ifndef IOMANAGERDMSPROXY_H
#define IOMANAGERDMSPROXY_H

#include <OpenVDS/OpenVDS.h>

#include "IOManager.h"
#include "IOManagerCurl.h"
#include "DmsIoFactories/DmsIoManagerFactory.h"
#include "json_cpp_include.h"

#include <memory>

namespace OpenVDS
{
  class TokenRefresher;
  struct DmsDataset;
  struct DmsIoManagerFactory;


  class IOManagerDMSProxy : public IOManager
  {
    public:
      IOManagerDMSProxy(const DMSOpenOptions &openOptions, IOManager::AccessPattern accessPattern, const Logger &logger, Error &error);
      ~IOManagerDMSProxy() override;

      std::shared_ptr<Request> ReadObjectInfo(const std::string &objectName, std::shared_ptr<TransferDownloadHandler> handler) override;
      std::shared_ptr<Request> ReadObject(const std::string &objectName, std::shared_ptr<TransferDownloadHandler> handler, const IORange& range = IORange()) override;
      std::shared_ptr<Request> WriteObject(const std::string &objectName, const std::string& contentDispostionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::function<void(const Request & request, const Error & error)> completedCallback = nullptr) override;
      bool Close(uint64_t serializedSize, uint64_t chunkCount, Error& error) override;
      bool EnableWriting(Error& error) override;
      std::string GetLegalTag() const override;
    private:
      static std::string AuthProviderCallback(const void* data);
      IOManager::AccessPattern m_accessPattern;
      Logger m_logger;
      CurlHandler m_curlHandler;
    
      std::unique_ptr<TokenRefresher> m_tokenRefresher;
      std::unique_ptr<DmsManager> m_dmsManager;
      std::unique_ptr<DmsDataset> m_dmsDataset;
      std::unique_ptr<DmsIoManagerFactory> m_ioManagerFactory;
      std::unique_ptr<IOManager> m_proxy;
    
      bool m_useFileNameForSingleFileDatasets;
      std::string m_filename;
  };
}

#endif
