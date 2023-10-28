#include "GcpDmsIoManagerFactory.h"

#include <fmt/format.h>
#include "VDS/Env.h"

#include <IO/IOManagerGoogle.h>

namespace OpenVDS
{
GcpDmsIoManagerFactory::GcpDmsIoManagerFactory(DmsDataset& dataset, Logger& logger)
  : DmsIoManagerFactory(dataset)
  , m_logger(logger)
{
}

static void getComponentsFromGCSUrl(const std::string& gcsUrl, std::string& bucket, std::string& prefixPath)
{
  auto pos = gcsUrl.find("/");
  if (pos == std::string::npos)
  {
    bucket = gcsUrl;
  }
  else
  {
    bucket = std::string(gcsUrl.begin(), gcsUrl.begin() + pos);
    auto prefixBegin = gcsUrl.begin() + pos + 1;
    if (prefixBegin != gcsUrl.end())
    {
      prefixPath = std::string(prefixBegin, gcsUrl.end());
    }
  }
}

std::unique_ptr<IOManager> GcpDmsIoManagerFactory::createIOManager(std::shared_ptr<CurlHandler> curlHandler, std::chrono::time_point<std::chrono::steady_clock> &expirationTime, Error& error)
{
  auto gcs_access_token = gcsAccessToken(error);
  if (error.code)
    return nullptr;

  std::string access_token;
  Json::Value root;
  if (!ParseJSONFromBuffer(gcs_access_token.data, root, error))
    return nullptr;
  try
  {
    access_token = root["access_token"].asString();
    int expires_in = root["expires_in"].asInt();
    expirationTime = std::chrono::steady_clock::now() + std::chrono::seconds(expires_in);
  }
  catch (Json::Exception& ex)
  {
    error.code = -1;
    error.string = fmt::format("Seismic DMS: gcs-access-token json error: {}", ex.what());
    return nullptr;
  }

  std::string bucket;
  std::string pathPrefix;
  getComponentsFromGCSUrl(m_dataset.m_gc_url, bucket, pathPrefix);
  GoogleCredentialsToken token(access_token);
  GoogleOpenOptions openOptions(bucket, pathPrefix, token);
  return std::unique_ptr<IOManager>(IOManagerGoogle::CreateIOManagerGoogle(openOptions, curlHandler, error));
}

}
