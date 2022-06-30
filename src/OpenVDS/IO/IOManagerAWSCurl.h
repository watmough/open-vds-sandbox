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

  struct InitAws
  {
    InitAws()
    {
      std::unique_lock<std::mutex> lock(mutex);
      if (!apiHandle)
        apiHandle.reset(new Aws::Crt::ApiHandle());
      count++;
    }
    ~InitAws()
    {
      std::unique_lock<std::mutex> lock(mutex);
      count--;
      if (count == 0)
        apiHandle.reset();
    }

    static std::mutex mutex;
    static int count;
    static std::unique_ptr<Aws::Crt::ApiHandle> apiHandle;
  };
  class IOManagerAWSCurl : public IOManager
  {
    public:
      IOManagerAWSCurl(const AWSOpenOptions &openOptions, Error &error);
      ~IOManagerAWSCurl() override;

      std::shared_ptr<Request> ReadObjectInfo(const std::string &objectName, std::shared_ptr<TransferDownloadHandler> handler) override;
      std::shared_ptr<Request> ReadObject(const std::string &objectName, std::shared_ptr<TransferDownloadHandler> handler, const IORange& range = IORange()) override;
      std::shared_ptr<Request> WriteObject(const std::string &objectName, const std::string& contentDispostionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::function<void(const Request & request, const Error & error)> completedCallback = nullptr) override;
      bool Close(Error &error) override { return true; }
    private:
      CurlHandler m_curlHandler;
      std::unique_ptr<InitAws> m_awsInitDeinit;
      Aws::Crt::Io::EventLoopGroup m_eventLoopGroup;
      Aws::Crt::Io::DefaultHostResolver m_hostResolver;
      Aws::Crt::Io::ClientBootstrap m_clientBootstrap;
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