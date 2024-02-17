#define USE_IMPORT_EXPORT 1
#include "IOManagerAWSCurl.h"

#include "ErrorRequest.h"

#include <fmt/format.h>

#include <aws/crt/Api.h>
#include <aws/crt/auth/Credentials.h>
#include <aws/crt/auth/Sigv4Signing.h>
#include <aws/crt/http/HttpRequestResponse.h>
#include <aws/crt/StringUtils.h>
#include <aws/crt/crypto/Hash.h>
#include <aws/common/encoding.h>

namespace OpenVDS
{

inline char asciitolower(char in)
{
  if (in <= 'Z' && in >= 'A')
    return in - ('Z' - 'z');
  return in;
}

static aws_byte_cursor createByteCursor(const std::string& a)
{
  return { a.size(),(uint8_t*) a.c_str() };
}
template<int SIZE>
static aws_byte_cursor createByteCursor(const char(&a)[SIZE])
{
  return { SIZE - 1, (uint8_t*)a };
}

template<typename A, typename B>
static Aws::Crt::Http::HttpHeader createHttpHeader(const A& key, const B& value)
{
  Aws::Crt::Http::HttpHeader header;
  header.name = createByteCursor(key);
  header.value = createByteCursor(value);
  return header;
}

class BucketLocationTransferDownloadHandler : public TransferDownloadHandler
{
public:
  BucketLocationTransferDownloadHandler()
    : TransferDownloadHandler()
  {}
  void HandleObjectSize(int64_t size) override {}
  void HandleObjectLastWriteTime(const std::string& lastWriteTimeISO8601) override {}
  void HandleMetadata(const std::string& key, const std::string& header) override { meta[key] = header; };
  void HandleData(std::vector<uint8_t>&& data) override { this->data = std::move(data); }
  void Completed(const Request& request, const Error& error) override {};

  std::vector<uint8_t> data;
  std::map<std::string, std::string> meta;
};

static const std::string& empty_sha256()
{
  static std::string empty("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
  return empty;
}

static bool signRequest(Aws::Crt::Auth::Sigv4HttpRequestSigner &requestSign, Aws::Crt::Auth::AwsSigningConfig &signingConfig, std::shared_ptr<Aws::Crt::Http::HttpRequest> &request)
{
  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);
  std::condition_variable condition;
  bool done = false;
  int errorCode = 0;

  bool success = requestSign.SignRequest(request, signingConfig, [&errorCode, &done, &condition](const std::shared_ptr<Aws::Crt::Http::HttpRequest>& request, int code)
    {
      errorCode = code;
      done = true;
      condition.notify_one();
    });
  if (!success)
    return false;
  condition.wait(lock, [&done] { return done; });

  return errorCode == AWS_ERROR_SUCCESS;
}

static std::string GetBucketLocation(const std::shared_ptr<Aws::Crt::Auth::ICredentialsProvider> &credsProvider, const std::string &bucket, CurlHandler &curlHandler)
{
  Aws::Crt::Auth::AwsSigningConfig signingConfig;
  signingConfig.SetCredentialsProvider(credsProvider);
  signingConfig.SetService("s3");
  signingConfig.SetSignatureType(Aws::Crt::Auth::SignatureType::HttpRequestViaHeaders);
  signingConfig.SetSignedBodyHeader(Aws::Crt::Auth::SignedBodyHeaderType::None);
  signingConfig.SetRegion("us-west-1");
  std::string host = fmt::format("{}.s3.us-west-1.amazonaws.com", bucket);
  std::string url = "https://" + host + "/?location";
  auto crtrequest = std::make_shared<Aws::Crt::Http::HttpRequest>();
  crtrequest->SetPath(createByteCursor(url));
  crtrequest->SetMethod(createByteCursor("GET"));
  crtrequest->AddHeader(createHttpHeader("Host", host));

  Aws::Crt::Auth::Sigv4HttpRequestSigner requestSign;
  if (!signRequest(requestSign, signingConfig, crtrequest))
    return "";

  std::vector<std::string> headers;
  int headerCount = (int)crtrequest->GetHeaderCount();
  for (int i = 0; i < headerCount; i++)
  {
    auto h = crtrequest->GetHeader(i);
    std::string name((const char*)h->name.ptr, h->name.len);
    std::string value((const char*)h->value.ptr, h->value.len);
    std::string header = name + ": " + value;
    headers.emplace_back(std::move(header));
  }
  headers.emplace_back("x-amz-content-sha256: " + empty_sha256());

  auto handler = std::make_shared<BucketLocationTransferDownloadHandler>();
  std::shared_ptr<DownloadRequestCurl> request = std::make_shared<DownloadRequestCurl>("", handler);
  curlHandler.addDownloadRequest(request, url, headers, convertToISO8601, CurlVerb::GET);

  Error error; //don't propogate error
  request->WaitForFinish(error);
  if (error.code)
    return "";
  std::string xmlContent((const char*)handler->data.data(), handler->data.size());
  auto it = xmlContent.find("LocationConstraint");
  if (it == std::string::npos)
    return "";
  auto start = xmlContent.find('>', it);
  if (start == std::string::npos)
    return "";
  start++;
  auto end = xmlContent.find('<', start + 1);
  if (end == std::string::npos)
    return "";
  return xmlContent.substr(start, end - start);
}

void assignByteCursorFromString(Aws::Crt::ByteCursor& cursor, const std::string& source)
{
  cursor.ptr = (uint8_t*)source.data();
  cursor.len = source.size();
}

std::shared_ptr<AwsCrtApiHandle> AwsCrtApiHandle::GetAwsCrtApiHandle(bool disableInitAPI)
{
  static std::weak_ptr<AwsCrtApiHandle> sharedApiHandle;
  static std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);
  auto awsCrtAPIHandle = sharedApiHandle.lock();
  if(!awsCrtAPIHandle)
  {
    auto apiHandle = disableInitAPI ? nullptr : new Aws::Crt::ApiHandle();
    auto eventLoopGroup = new Aws::Crt::Io::EventLoopGroup(1);
    auto hostResolver = new Aws::Crt::Io::DefaultHostResolver(*eventLoopGroup, 1000, 1000);
    auto clientBootstrap = new Aws::Crt::Io::ClientBootstrap(*eventLoopGroup, *hostResolver);
    awsCrtAPIHandle.reset(new AwsCrtApiHandle(apiHandle, eventLoopGroup, hostResolver, clientBootstrap));
    sharedApiHandle = awsCrtAPIHandle;
  }
  return awsCrtAPIHandle;
}

IOManagerAWSCurl::IOManagerAWSCurl(const AWSOpenOptions& openOptions, std::shared_ptr<CurlHandler> curlHandler, Error& error)
  : IOManager(OpenOptions::AWS)
  , m_curlHandler(curlHandler)
  , m_awsCrtApiHandle(AwsCrtApiHandle::GetAwsCrtApiHandle(openOptions.disableInitApi))
  , m_useVirtualAddressing(true)
  , m_secureSocket(true)
  , m_region(openOptions.region)
  , m_bucket(openOptions.bucket)
  , m_path(openOptions.key)
  , m_protocol("https")

{
  if (error.code)
    return;

  if (openOptions.accessKeyId.size())
  {
    m_accessKeyId = openOptions.accessKeyId;
    m_secretAccessKey = openOptions.secretKey;
    m_sessionToken = openOptions.sessionToken;
    Aws::Crt::Auth::CredentialsProviderStaticConfig config;
    assignByteCursorFromString(config.AccessKeyId, m_accessKeyId);
    assignByteCursorFromString(config.SecretAccessKey, m_secretAccessKey);
    if (m_sessionToken.size())
      assignByteCursorFromString(config.SessionToken, m_sessionToken);
    m_credentialsProvider = Aws::Crt::Auth::CredentialsProvider::CreateCredentialsProviderStatic(config);
  }
  else
  {
    Aws::Crt::Auth::CredentialsProviderChainDefaultConfig config;
    config.Bootstrap = m_awsCrtApiHandle->GetClientBootstrap();
    m_credentialsProvider = Aws::Crt::Auth::CredentialsProvider::CreateCredentialsProviderChainDefault(config);
  }

  if (m_region.empty() && openOptions.endpointOverride.empty())
  {
    m_region = GetBucketLocation(m_credentialsProvider, m_bucket, *m_curlHandler);
  }

  if (openOptions.endpointOverride.size())
  {
    if (openOptions.endpointOverride.rfind("https://", 0) == 0)
    {
      m_host = openOptions.endpointOverride.substr(8);
    }
    else if (openOptions.endpointOverride.rfind("http://", 0) == 0)
    {
      m_host = openOptions.endpointOverride.substr(7);
      m_protocol = "http";
      m_secureSocket = false;
    }
    else
    {
      m_host = openOptions.endpointOverride;
    }
    m_region = "us-east-1";
    m_useVirtualAddressing = false;
  }
  else
  {
    m_host = fmt::format("{}.s3.{}.amazonaws.com", m_bucket, m_region);
  }

  if (m_region.empty())
  {
    error.string = fmt::format("Could not resolve region for s3://{}", m_bucket);
    error.code = -1;
  }
}

IOManager *IOManagerAWSCurl::CreateIOManagerAWSCurl(const AWSOpenOptions &openOptions, std::shared_ptr<CurlHandler> curlHandler, Error& error)
{
  std::unique_ptr<IOManager> ioManager(new IOManagerAWSCurl(openOptions, curlHandler, error));
  return (error.code == 0) ? ioManager.release() : nullptr;
}

IOManager *IOManagerAWSCurl::CreateIOManagerAWSCurl(const AWSOpenOptions &openOptions, const Logger& logger, Error& error)
{
  auto curlHandler = std::make_shared<CurlHandler>(error, logger);
  return (error.code == 0) ? CreateIOManagerAWSCurl(openOptions, curlHandler, error) : nullptr;
}

static std::string getUrlInternal(bool useVirtualAdressing, const std::string &protocol, const std::string& host, const std::string &bucket, const std::string &path, const std::string& objectName)
{
  if (objectName.empty())
  {
    assert(path.size());
    if (useVirtualAdressing)
      return fmt::format("{}://{}/{}", protocol, host, path);
    else
      return fmt::format("{}://{}/{}/{}", protocol, host, bucket, path);
  }
  if (path.empty())
  {
    assert(objectName.size());
    if (useVirtualAdressing)
      return fmt::format("{}://{}/{}", protocol, host, objectName);
    else
      return fmt::format("{}://{}/{}/{}", protocol, host, bucket, objectName);

  }
  if (useVirtualAdressing)
    return fmt::format("{}://{}/{}/{}", protocol, host, path, objectName);
  return fmt::format("{}://{}/{}/{}/{}", protocol, host, bucket, path, objectName);
}

static std::string getUrl(bool useVirtualAdressing, const std::string &protocol, const std::string& host, const std::string &bucket, const std::string& path, const std::string& objectName)
{
  return getUrlInternal(useVirtualAdressing, protocol, host, bucket, path, objectName);
}

static std::vector<std::string> signRequest(const std::string& host, const std::string& url,  std::shared_ptr<Aws::Crt::Auth::ICredentialsProvider> &credentialsProvider, const std::string &region, const std::string& verb, const std::string& payloadHash, const std::map<std::string, std::string> headerMap, Error &error)
{
    auto crtrequest = std::make_shared<Aws::Crt::Http::HttpRequest>();
    crtrequest->SetPath(createByteCursor(url));
    crtrequest->SetMethod(createByteCursor(verb));
    crtrequest->AddHeader(createHttpHeader("Host", host));

    Aws::Crt::Auth::AwsSigningConfig signingConfig;
    signingConfig.SetCredentialsProvider(credentialsProvider);
    signingConfig.SetService("s3");
    signingConfig.SetSignatureType(Aws::Crt::Auth::SignatureType::HttpRequestViaHeaders);
    signingConfig.SetSignedBodyHeader(Aws::Crt::Auth::SignedBodyHeaderType::None);
    signingConfig.SetRegion(region.c_str());
    signingConfig.SetSignedBodyHeader(Aws::Crt::Auth::SignedBodyHeaderType::XAmzContentSha256);
    signingConfig.SetSignedBodyValue(payloadHash.c_str());

    for (auto h : headerMap)
      crtrequest->AddHeader(createHttpHeader(h.first, h.second));
 
    Aws::Crt::Auth::Sigv4HttpRequestSigner requestSign;
    if (!signRequest(requestSign, signingConfig, crtrequest))
    {
        error.string = "unexpected AWS signing failure";
        error.code = -1;
    }

    std::vector<std::string> headers;
    int headerCount = (int)crtrequest->GetHeaderCount();
    for (int i = 0; i < headerCount; i++)
    {
      auto h = crtrequest->GetHeader(i);
      std::string name((const char *)h->name.ptr, h->name.len);
      std::string value((const char *)h->value.ptr, h->value.len);
      std::string header = name + ": " + value;
      headers.emplace_back(std::move(header));
    }

    return headers;
}

std::shared_ptr<Request> IOManagerAWSCurl::ReadObjectInfo(const std::string& objectName, std::shared_ptr<TransferDownloadHandler> handler)
{
  std::string url = getUrl(m_useVirtualAddressing, m_protocol, m_host, m_bucket, m_path, objectName);
  Error error;
  std::vector<std::string> headers = signRequest(m_host, url, m_credentialsProvider, m_region, "HEAD", empty_sha256(), {}, error);
  if (error.code)
  {
    return std::make_shared<ErrorRequest>(objectName, error.string);
  }
  std::shared_ptr<DownloadRequestCurl> request = std::make_shared<DownloadRequestCurl>(objectName, handler);
  m_curlHandler->addDownloadRequest(request, url, headers, convertToISO8601, CurlVerb::HEADER);
  return request;
}

std::shared_ptr<Request> IOManagerAWSCurl::ReadObject(const std::string& objectName, std::shared_ptr<TransferDownloadHandler> handler, const IORange& range)
{
    std::string url = getUrl(m_useVirtualAddressing, m_protocol, m_host, m_bucket, m_path, objectName);
    std::map<std::string, std::string> headerMap;
    if (range.start != range.end)
    {
      headerMap["Range"] = fmt::format("bytes={}-{}", range.start, range.end - 1);
    }
    Error error;
    auto headers = signRequest(m_host, url, m_credentialsProvider, m_region, "GET", empty_sha256(), headerMap, error);
    if (error.code)
    {
      return std::make_shared<ErrorRequest>(objectName, error.string);
    }
    std::shared_ptr<DownloadRequestCurl> request = std::make_shared<DownloadRequestCurl>(objectName, handler);
    m_curlHandler->addDownloadRequest(request, url, headers, convertToISO8601, CurlVerb::GET);
    return request;
}

std::string hashData(const std::vector<uint8_t>& data, bool secureSocket)
{
  if (secureSocket)
    return "UNSIGNED-PAYLOAD";

  Aws::Crt::ByteCursor inputhash;
  inputhash.len = data.size();
  inputhash.ptr = const_cast<uint8_t*>(data.data());

  Aws::Crt::ByteBuf buf;
  aws_byte_buf_init(&buf, Aws::Crt::g_allocator, Aws::Crt::Crypto::SHA256_DIGEST_SIZE);
  Aws::Crt::Crypto::ComputeSHA256(inputhash, buf);

  Aws::Crt::ByteCursor inputencoded;
  inputencoded.len = buf.len;
  inputencoded.ptr = buf.buffer;

  Aws::Crt::ByteBuf buf2;
  aws_byte_buf_init(&buf2, Aws::Crt::g_allocator, Aws::Crt::Crypto::SHA256_DIGEST_SIZE * 16);

  aws_hex_encode(&inputencoded, &buf2);
  std::string sha256((const char*)buf2.buffer, buf2.len);
  aws_byte_buf_clean_up(&buf);
  aws_byte_buf_clean_up(&buf2);
  return sha256;

}

std::shared_ptr<Request> IOManagerAWSCurl::WriteObject(const std::string& objectName, const std::string& contentDispostionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::function<void(const Request& request, const Error& error)> completedCallback)
{
  std::string url = getUrl(m_useVirtualAddressing, m_protocol, m_host, m_bucket, m_path, objectName);
  std::shared_ptr<UploadRequestCurl> request = std::make_shared<UploadRequestCurl>(objectName, completedCallback);
  std::map<std::string, std::string> headerMap;
  for (auto& header : metadataHeader)
      headerMap[fmt::format("x-amz-meta-{}", header.first)] = header.second;
  Error error;
  auto headers = signRequest(m_host, url, m_credentialsProvider, m_region, "PUT", hashData(*data, m_secureSocket), headerMap, error);
  if (error.code)
      return std::make_shared<ErrorRequest>(objectName, error.string);
  if (contentDispostionFilename.size())
    headers.push_back(fmt::format("content-disposition: attachment; filename=\"{}\"", contentDispostionFilename));
  if (contentType.size())
    headers.push_back(fmt::format("content-type: {}", contentType));
  if (data->size())
    headers.push_back(fmt::format("content-length: {}", data->size()));
  m_curlHandler->addUploadRequest(request, url, headers, data);
  return request;
}
}
