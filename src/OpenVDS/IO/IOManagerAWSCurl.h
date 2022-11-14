#ifndef IOMANAGERAWSCURL_H
#define IOMANAGERAWSCURL_H

#include <OpenVDS/OpenVDS.h>

#include "IOManager.h"
#include "IOManagerCurl.h"

#define AWS_CRT_CPP_USE_IMPORT_EXPORT
#include <aws/crt/auth/Credentials.h>
#include <aws/crt/auth/Sigv4Signing.h>
#include <aws/crt/Api.h>

#include <memory>

namespace OpenVDS
{
  class AwsCrtApiHandle
  {
    std::unique_ptr<Aws::Crt::ApiHandle> apiHandle;
    std::unique_ptr<Aws::Crt::Io::EventLoopGroup> eventLoopGroup;
    std::unique_ptr<Aws::Crt::Io::DefaultHostResolver> hostResolver;
    std::unique_ptr<Aws::Crt::Io::ClientBootstrap> clientBootstrap;

    AwsCrtApiHandle(Aws::Crt::ApiHandle *apiHandle, Aws::Crt::Io::EventLoopGroup *eventLoopGroup, Aws::Crt::Io::DefaultHostResolver *hostResolver, Aws::Crt::Io::ClientBootstrap *clientBootstrap) : apiHandle(apiHandle), eventLoopGroup(eventLoopGroup), hostResolver(hostResolver), clientBootstrap(clientBootstrap) {}

  public:
    Aws::Crt::Io::ClientBootstrap* GetClientBootstrap() { return clientBootstrap.get(); }

    static std::shared_ptr<AwsCrtApiHandle> GetAwsCrtApiHandle(bool disableInitAPI);
  };

  class IOManagerAWSCurl : public IOManager
  {
    public:
      IOManagerAWSCurl(const AWSOpenOptions &openOptions, const Logger &logger, Error &error);
      ~IOManagerAWSCurl() override;

      std::shared_ptr<Request> ReadObjectInfo(const std::string &objectName, std::shared_ptr<TransferDownloadHandler> handler) override;
      std::shared_ptr<Request> ReadObject(const std::string &objectName, std::shared_ptr<TransferDownloadHandler> handler, const IORange& range = IORange()) override;
      std::shared_ptr<Request> WriteObject(const std::string &objectName, const std::string& contentDispostionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::function<void(const Request & request, const Error & error)> completedCallback = nullptr) override;
      bool Close(uint64_t serializedSize, uint64_t chunkCount, Error &error) override { return true; }
    private:
      CurlHandler m_curlHandler;
      std::shared_ptr<AwsCrtApiHandle> m_awsCrtApiHandle;
      std::shared_ptr<Aws::Crt::Auth::ICredentialsProvider> m_credentialsProvider;
      bool m_useVirtualAddressing;
      bool m_secureSocket;
      std::string m_region;
      std::string m_bucket;
      std::string m_path;
      std::string m_protocol;
      std::string m_host;
      std::string m_accessKeyId;
      std::string m_secretAccessKey;
      std::string m_sessionToken;
  };
}

#endif //IOMANAGERAWSCURL_H