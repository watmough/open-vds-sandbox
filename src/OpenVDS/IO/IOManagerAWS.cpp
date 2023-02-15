/****************************************************************************
** Copyright 2019 The Open Group
** Copyright 2019 Bluware, Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
****************************************************************************/

#define USE_IMPORT_EXPORT 1
#include "IOManagerAWS.h"

#include <aws/core/Aws.h>
#include <aws/s3/model/Bucket.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/GetBucketLocationRequest.h>
#include <aws/s3/model/GetBucketLocationResult.h>
#include <aws/s3/model/BucketLocationConstraint.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/core/utils/memory/AWSMemory.h>
#include <aws/core/utils/memory/stl/AWSString.h>
#include <aws/core/utils/logging/DefaultLogSystem.h>
#include <aws/core/utils/logging/ConsoleLogSystem.h>
#include <aws/core/utils/logging/AWSLogging.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/auth/AWSCredentialsProviderChain.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/sts/STSClient.h>
#include <aws/sts/model/AssumeRoleRequest.h>
#include <aws/transfer/TransferManager.h>
#include <aws/core/utils/Array.h>
#include <mutex>
#include <functional>

#include "SslVerifyPeerEnv.h"

#include <fmt/format.h>

#include <VDS/Logging.h>

#include <cstdarg>

namespace OpenVDS
{

  static int initialize_sdk = 0;
  static std::mutex initialize_sdk_mutex;
  static Aws::SDKOptions initialize_sdk_options;

  static Aws::String convertStdString(const std::string& s)
  {
    return Aws::String(s.begin(), s.end());
  }

  static std::string convertAwsString(const Aws::String& s)
  {
    return std::string(s.begin(), s.end());
  }

  inline char asciitolower(char in)
  {
    if (in <= 'Z' && in >= 'A')
      return in - ('Z' - 'z');
    return in;
  }

  static Aws::Utils::Logging::LogLevel resolveLoglevel(LogLevel loglevel)
  {
    static Aws::Utils::Logging::LogLevel awsLogLevels[] = {
      Aws::Utils::Logging::LogLevel::Off,   //OpenVDSLogging::None:
      Aws::Utils::Logging::LogLevel::Error, //OpenVDSLogging::Error:
      Aws::Utils::Logging::LogLevel::Warn,  //OpenVDSLogging::Warning:
      Aws::Utils::Logging::LogLevel::Info,  //OpenVDSLogging::Info:
      Aws::Utils::Logging::LogLevel::Trace  //OpenVDSLogging::Trace:
    };
    static_assert(sizeof(awsLogLevels) / sizeof(*awsLogLevels) == int(LogLevel::Trace) + 1, "The loglevel conversion table is wrong, has the enums changed");
    return awsLogLevels[int(loglevel)];
  }

  static LogLevel resolveAwsLogLevel(Aws::Utils::Logging::LogLevel logLevel)
  {
    static LogLevel openVdsLogLevels[] = {
      LogLevel::None,   //Off = 0,
      LogLevel::Error,  //Fatal = 1,
      LogLevel::Error,  //Error = 2,
      LogLevel::Warning,//Warn = 3,
      LogLevel::Info,   //Info = 4,
      LogLevel::Info,   //Debug = 5,
      LogLevel::Trace   //Trace = 6
    };
    static_assert(sizeof(openVdsLogLevels) / sizeof(*openVdsLogLevels) == int(Aws::Utils::Logging::LogLevel::Trace) + 1, "The loglevel conversion table is wrong, has the enums changed");
    return openVdsLogLevels[int(logLevel)];
  }

  class OpenVDSAwsLogger : public Aws::Utils::Logging::LogSystemInterface
  {
  public:
    OpenVDSAwsLogger(const Logger &logger)
      : awsLogLevel(resolveLoglevel(logger.level))
      , logger(logger)
    {
    }

    Aws::Utils::Logging::LogLevel GetLogLevel() const override final { return awsLogLevel; }
                
    void Log(Aws::Utils::Logging::LogLevel logLevel, const char* tag, const char* formatStr, ...) override final
    {
      Aws::StringStream ss;

      std::va_list args;
      va_start(args, formatStr);

      va_list tmp_args; //unfortunately you cannot consume a va_list twice
      va_copy(tmp_args, args); //so we have to copy it
#ifdef _WIN32
      const int requiredLength = _vscprintf(formatStr, tmp_args) + 1;
#else
      const int requiredLength = vsnprintf(nullptr, 0, formatStr, tmp_args) + 1;
#endif
      va_end(tmp_args);

      Aws::Utils::Array<char> outputBuff(requiredLength);
#ifdef _WIN32
      vsnprintf_s(outputBuff.GetUnderlyingData(), requiredLength, _TRUNCATE, formatStr, args);
#else
      vsnprintf(outputBuff.GetUnderlyingData(), requiredLength, formatStr, args);
#endif // _WIN32

      auto str = std::string(tag) + ": " + ss.str();
      logger.logInterface.Log(resolveAwsLogLevel(logLevel), str.c_str(), str.size());
      va_end(args);
    }
    void LogStream(Aws::Utils::Logging::LogLevel logLevel, const char* tag, const Aws::OStringStream& messageStream) override final
    {
      auto str = std::string(tag) + ": " + messageStream.str();
      logger.logInterface.Log(resolveAwsLogLevel(logLevel), str.c_str(), str.size());
    }
    void Flush() override final {}
  private:
    Aws::Utils::Logging::LogLevel awsLogLevel;
    Logger logger;
  };

  static void initializeAWSSDK(const Logger &logger)
  {
    std::unique_lock<std::mutex> lock(initialize_sdk_mutex);
    initialize_sdk++;
    if (initialize_sdk == 1)
    {

      Aws::Utils::Logging::InitializeAWSLogging(Aws::MakeShared<OpenVDSAwsLogger>("OpenVDS-S3 Integration", logger));
      Aws::InitAPI(initialize_sdk_options);
    }
  }

  static void deinitializeAWSSDK()
  {
    std::unique_lock<std::mutex> lock(initialize_sdk_mutex);
    initialize_sdk--;
    if (!initialize_sdk)
    {
      Aws::Utils::Logging::ShutdownAWSLogging();
      Aws::ShutdownAPI(initialize_sdk_options);
    }

  }

  template<typename T>
  struct AsyncContext
  {
    AsyncContext(T *back)
      : back(back)
    {}
    T *back;
    std::mutex mutex;
  };

  GetOrHeadRequestAWS::GetOrHeadRequestAWS(const std::string& id, const std::shared_ptr<TransferDownloadHandler>& handler)
    : RequestImpl(id)
    , m_handler(handler)
  {
  }

  GetOrHeadRequestAWS::~GetOrHeadRequestAWS()
  {
  }

  static void readobjectinfo_callback(const Aws::S3::S3Client *client, const Aws::S3::Model::HeadObjectRequest& objreq, const Aws::S3::Model::HeadObjectOutcome &getObjectOutcome, std::weak_ptr<ReadObjectInfoRequestAWS> weak_request)
  {
    auto objReq =  weak_request.lock();

    if (!objReq)
      return;

    RequestStateHandler requestStateHandler(*objReq);
    if (requestStateHandler.isCancelledRequested())
    {
      return;
    }

    if (getObjectOutcome.IsSuccess())
    {
      Aws::S3::Model::HeadObjectResult result = const_cast<Aws::S3::Model::HeadObjectOutcome&>(getObjectOutcome).GetResultWithOwnership();

      int64_t content_length = int64_t(result.GetContentLength());
      objReq->m_handler->HandleObjectSize(content_length);

      auto lastModified = result.GetLastModified();
      objReq->m_handler->HandleObjectLastWriteTime(convertAwsString(lastModified.ToGmtString(Aws::Utils::DateFormat::ISO_8601)));

      for (auto it : result.GetMetadata())
      {
        objReq->m_handler->HandleMetadata(convertAwsString(it.first), convertAwsString(it.second));
      }
    }
    else
    {
      auto s3error = getObjectOutcome.GetError();
      objReq->m_error.code = int(s3error.GetResponseCode());
      objReq->m_error.string = (s3error.GetExceptionName() + " : " + s3error.GetMessage()).c_str();
    }

    objReq->m_handler->Completed(*objReq, objReq->m_error);
  }

  ReadObjectInfoRequestAWS::ReadObjectInfoRequestAWS(const std::string &id, const std::shared_ptr<TransferDownloadHandler>& handler)
    : GetOrHeadRequestAWS(id, handler)
  {
  }

  void ReadObjectInfoRequestAWS::run(Aws::S3::S3Client& client, const std::string& bucket, std::weak_ptr<ReadObjectInfoRequestAWS> readObjectInfoRequest)
  {
    Aws::S3::Model::HeadObjectRequest object_request;
    object_request.SetBucket(convertStdString(bucket));
    object_request.SetKey(convertStdString(GetObjectName()));

    Aws::S3::HeadObjectResponseReceivedHandler bounded_callback = [readObjectInfoRequest](const Aws::S3::S3Client *client, const Aws::S3::Model::HeadObjectRequest &request, const Aws::S3::Model::HeadObjectOutcome &outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&)
    {
      readobjectinfo_callback(client, request, outcome, readObjectInfoRequest);
    };
    client.HeadObjectAsync(object_request, bounded_callback);
  }

  static void download_callback(const Aws::S3::S3Client *client, const Aws::S3::Model::GetObjectRequest& objreq, const Aws::S3::Model::GetObjectOutcome &getObjectOutcome, std::weak_ptr<DownloadRequestAWS> weak_request)
  {
    auto objReq =  weak_request.lock();

    if (!objReq)
      return;

    RequestStateHandler requestStateHandler(*objReq);
    if (requestStateHandler.isCancelledRequested())
    {
      return;
    }

    if (getObjectOutcome.IsSuccess())
    {
      Aws::S3::Model::GetObjectResult result = const_cast<Aws::S3::Model::GetObjectOutcome&>(getObjectOutcome).GetResultWithOwnership();

      int64_t content_length = int64_t(result.GetContentLength());
      objReq->m_handler->HandleObjectSize(content_length);

      auto lastModified = result.GetLastModified();
      objReq->m_handler->HandleObjectLastWriteTime(convertAwsString(lastModified.ToGmtString(Aws::Utils::DateFormat::ISO_8601)));

      for (auto it : result.GetMetadata())
      {
        objReq->m_handler->HandleMetadata(convertAwsString(it.first), convertAwsString(it.second));
      }

      auto& retrieved_object = result.GetBody();
      std::vector<uint8_t> data;

      if (content_length > 0)
      {
        data.resize(content_length);
        retrieved_object.read((char*)&data[0], content_length);
        objReq->m_handler->HandleData(std::move(data));
      }
    }
    else
    {
      auto s3error = getObjectOutcome.GetError();
      objReq->m_error.code = int(s3error.GetResponseCode());
      objReq->m_error.string = (s3error.GetExceptionName() + " : " + s3error.GetMessage()).c_str();
    }

    objReq->m_handler->Completed(*objReq, objReq->m_error);
  }

  DownloadRequestAWS::DownloadRequestAWS(const std::string &id, const std::shared_ptr<TransferDownloadHandler>& handler)
    : GetOrHeadRequestAWS(id, handler)
  {
  }

  void DownloadRequestAWS::run(Aws::S3::S3Client& client, const std::string& bucket, const IORange& range, std::weak_ptr<DownloadRequestAWS> downloadRequest)
  {
    Aws::S3::Model::GetObjectRequest object_request;
    object_request.SetBucket(convertStdString(bucket));
    object_request.SetKey(convertStdString(GetObjectName()));
    if (range.end)
    {
      object_request.SetRange(convertStdString(fmt::format("bytes={}-{}", range.start, range.end)));
    }
    Aws::S3::GetObjectResponseReceivedHandler bounded_callback = [downloadRequest](const Aws::S3::S3Client *client, const Aws::S3::Model::GetObjectRequest &request, const Aws::S3::Model::GetObjectOutcome &outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&)
    {
      download_callback(client, request, outcome, downloadRequest);
    };
    client.GetObjectAsync(object_request, bounded_callback);
  }

  static void upload_callback(const Aws::S3::S3Client* client, const Aws::S3::Model::PutObjectRequest&putRequest, const Aws::S3::Model::PutObjectOutcome &outcome, std::weak_ptr<UploadRequestAWS> weak_upload)
  {
    auto objReq =  weak_upload.lock();
    if (!objReq)
      return;

    RequestStateHandler requestStateHandler(*objReq);
    if (requestStateHandler.isCancelledRequested())
    {
      return;
    }
    if (!outcome.IsSuccess())
    {
      auto s3error = outcome.GetError();
      objReq->m_error.code = int(s3error.GetResponseCode());
      objReq->m_error.string = (s3error.GetExceptionName() + " : " + s3error.GetMessage()).c_str();
    }

    if (objReq->m_completedCallback)
      objReq->m_completedCallback(*objReq, objReq->m_error);
  }

  UploadRequestAWS::UploadRequestAWS(const std::string& id, std::function<void(const Request & request, const Error & error)> completedCallback)
    : RequestImpl(id)
    , m_completedCallback(completedCallback)
  {
  }
 
  void UploadRequestAWS::run(Aws::S3::S3Client& client, const std::string& bucket, const std::string& contentDispostionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::weak_ptr<UploadRequestAWS> uploadRequest)
  {
    m_stream = std::make_shared<IOStream>(data);

    Aws::S3::Model::PutObjectRequest put;
    put.SetBucket(convertStdString(bucket));
    put.SetKey(convertStdString(GetObjectName()));
    put.SetBody(m_stream);
    put.SetContentType(convertStdString(contentType));
    put.SetContentLength(data->size());
    if (contentDispostionFilename.size())
      put.SetContentDisposition(Aws::String("attachment; filename=" + convertStdString(contentDispostionFilename)));
    for (auto &metaPair : metadataHeader)
    {
      put.AddMetadata(convertStdString(metaPair.first), convertStdString(metaPair.second.c_str()));
    }
    
    Aws::S3::PutObjectResponseReceivedHandler bounded_callback = [uploadRequest] (const Aws::S3::S3Client* client, const Aws::S3::Model::PutObjectRequest&putRequest, const Aws::S3::Model::PutObjectOutcome &outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>&) { upload_callback(client, putRequest, outcome, uploadRequest);};
    client.PutObjectAsync(put, bounded_callback);

  }

  IOManagerAWS::IOManagerAWS(const AWSOpenOptions& openOptions, const Logger &logger, Error &error)
    : IOManager(OpenOptions::AWS)
    , m_region(openOptions.region)
    , m_bucket(openOptions.bucket)
    , m_objectId(openOptions.key)
    , m_disableInitializeSdk(openOptions.disableInitApi)
    , m_logger(logger)
  {
    if (m_bucket.empty())
    {
      error.code = -1;
      error.string = "AWS Config error. Empty bucket";
      return;
    }

    if (m_objectId.size() && m_objectId[m_objectId.size() -1] == '/')
      m_objectId.resize(m_objectId.size() - 1);

    if (!m_disableInitializeSdk)
      initializeAWSSDK(m_logger);

    Aws::String profileName = "";

    Aws::Auth::AWSCredentials
      credentials;

    if (openOptions.accessKeyId.size())
    {
      credentials.SetAWSAccessKeyId(convertStdString(openOptions.accessKeyId));
      if (openOptions.secretKey.size())
      {
        credentials.SetAWSSecretKey(convertStdString(openOptions.secretKey));
      }
      if (openOptions.sessionToken.size())
      {
        credentials.SetSessionToken(convertStdString(openOptions.sessionToken));
      }
      if (openOptions.expiration.size())
      {
        Aws::Utils::DateTime expiration(convertStdString(openOptions.expiration), Aws::Utils::DateFormat::AutoDetect);
        if (!expiration.WasParseSuccessful())
        {
          error.code = -1;
          error.string = fmt::format("Failed to parse expiration parameter: {}.", openOptions.expiration);
          return;
        }
        credentials.SetExpiration(expiration);
      }
    }
    else
    {
      // If the default profile uses a role, we need to resolve the role ourselves
      if (profileName.empty() && !Aws::Config::GetCachedConfigProfile(Aws::Auth::GetConfigProfileName()).GetRoleArn().empty())
      {
        profileName = Aws::Auth::GetConfigProfileName();
      }

      // If there is no profile name set we use the default credentials provider chain
      if (profileName.empty())
      {
        Aws::Auth::DefaultAWSCredentialsProviderChain provider;
        credentials = provider.GetAWSCredentials();
      }
      else
      {
        auto profile = Aws::Config::GetCachedConfigProfile(profileName);

        // If the profile is using roles we need to resolve the role ourselves as the AWS C++ SDK doesn't do this correctly
        if (!profile.GetRoleArn().empty())
        {
          auto sourceProfileName = profile.GetSourceProfile();
          Aws::Auth::ProfileConfigFileAWSCredentialsProvider sourceCredentialsProvider(sourceProfileName.c_str());
          auto sourceProfileCredentials = sourceCredentialsProvider.GetAWSCredentials();

          Aws::STS::Model::AssumeRoleRequest request;
          request.SetRoleArn(profile.GetRoleArn());
          request.SetRoleSessionName("OpenVDS");

          Aws::Client::ClientConfiguration clientConfig;
          clientConfig.verifySSL = isDisableSSLVerificationEnvSet();
          clientConfig.scheme = Aws::Http::Scheme::HTTPS;
          Aws::STS::STSClient stsClient(sourceProfileCredentials, clientConfig);
          auto result = stsClient.AssumeRole(request);
          if (result.IsSuccess())
          {
            auto stsCredentials = result.GetResult().GetCredentials();
            credentials = Aws::Auth::AWSCredentials(stsCredentials.GetAccessKeyId(), stsCredentials.GetSecretAccessKey(), stsCredentials.GetSessionToken(), stsCredentials.GetExpiration());
          }
        }
        else
        {
          Aws::Auth::ProfileConfigFileAWSCredentialsProvider credentialsProvider(profileName.c_str());
          credentials = credentialsProvider.GetAWSCredentials();
        }
      }
    }

    Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy payloadSigningPolicy = Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never;
    Aws::Client::ClientConfiguration clientConfig;
    clientConfig.verifySSL = isDisableSSLVerificationEnvSet();
    clientConfig.scheme = Aws::Http::Scheme::HTTPS;
    if (m_region.size())
      clientConfig.region = convertStdString(m_region);
    clientConfig.connectTimeoutMs = openOptions.connectionTimeoutMs;
    clientConfig.requestTimeoutMs = openOptions.requestTimeoutMs;
    bool useVirtualAddressing = true;
    if (openOptions.endpointOverride.size())
    {
      clientConfig.endpointOverride = convertStdString(openOptions.endpointOverride);
      useVirtualAddressing = false;
      clientConfig.enableEndpointDiscovery = false;
      if (openOptions.endpointOverride.rfind("http://", 0) == 0)
      {
        clientConfig.scheme = Aws::Http::Scheme::HTTP;
        payloadSigningPolicy = Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Always;
      }
    }
    if (m_region.empty() && openOptions.endpointOverride.empty())
    {
      clientConfig.region = "us-west-1"; // workaround bug: https://github.com/aws/aws-sdk-cpp/issues/1339 should be fixed in 1.8
      m_s3Client.reset(new Aws::S3::S3Client(credentials, clientConfig, payloadSigningPolicy, useVirtualAddressing));
      Aws::S3::Model::GetBucketLocationRequest bucketLocationRequest;
      bucketLocationRequest.SetBucket(convertStdString(m_bucket));
      auto outcome = m_s3Client->GetBucketLocation(bucketLocationRequest);
      auto &result = outcome.GetResult();
      auto location = result.GetLocationConstraint();
      if (location != Aws::S3::Model::BucketLocationConstraint::NOT_SET)
      {
        m_region = convertAwsString(Aws::S3::Model::BucketLocationConstraintMapper::GetNameForBucketLocationConstraint(location));
        clientConfig.region = convertStdString(m_region);
        m_s3Client.reset(new Aws::S3::S3Client(credentials, clientConfig, payloadSigningPolicy, useVirtualAddressing));
      }
    }
    else
    {
      m_s3Client.reset(new Aws::S3::S3Client(credentials, clientConfig, payloadSigningPolicy, useVirtualAddressing));
    }

    //We do this to use a symbol from the transfermanager so we get the linker chain working on linux
    Aws::Utils::Threading::DefaultExecutor threadExecutor;
    Aws::Transfer::TransferManagerConfiguration transferConfig(&threadExecutor);
    std::shared_ptr<Aws::S3::S3Client> s3ClientSharedPtr(m_s3Client.get(), [](Aws::S3::S3Client*) {});
    transferConfig.s3Client = s3ClientSharedPtr;
    auto manager = Aws::Transfer::TransferManager::Create(transferConfig);
  }

  IOManagerAWS::~IOManagerAWS()
  {
    m_s3Client.reset();

    if (!m_disableInitializeSdk)
      deinitializeAWSSDK();
  }

  std::shared_ptr<Request> IOManagerAWS::ReadObjectInfo(const std::string &objectName, std::shared_ptr<TransferDownloadHandler> handler)
  {
    std::string id = objectName.empty()? m_objectId : m_objectId + "/" + objectName;
    auto ret = std::make_shared<ReadObjectInfoRequestAWS>(id, handler);
    ret->run(*m_s3Client.get(), m_bucket, ret);
    return ret;
  }

  std::shared_ptr<Request> IOManagerAWS::ReadObject(const std::string &objectName, std::shared_ptr<TransferDownloadHandler> handler, const IORange &range)
  {
    std::string id = objectName.empty()? m_objectId : m_objectId + "/" + objectName;
    auto ret = std::make_shared<DownloadRequestAWS>(id, handler);
    ret->run(*m_s3Client.get(), m_bucket, range, ret);
    return ret;
  }

  std::shared_ptr<Request> IOManagerAWS::WriteObject(const std::string &objectName, const std::string& contentDispositionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::function<void(const Request & request, const Error & error)> completedCallback)
  {
    std::string id = objectName.empty()? m_objectId : m_objectId + "/" + objectName;
    auto ret = std::make_shared<UploadRequestAWS>(id, completedCallback);
    ret->run(*m_s3Client.get(), m_bucket, contentDispositionFilename, contentType, metadataHeader, data, ret);
    return ret;
  }
}
