#define USE_IMPORT_EXPORT 1
#include "AwsDmsIoManagerFactory.h"

#include "IO/IOManagerAWSCurl.h"

#include "VDS/Env.h"

#include <string>
#include <fmt/format.h>

namespace OpenVDS
{

bool AwsDmsIoManagerFactory::getComponentsFromAccessToken(const std::string& accessToken, std::string& key, std::string& secret, std::string& session, Error& error) const
{
  if (accessToken.empty())
  {
    error.code = -1;
    error.string = "Empty accessToken";
    return false;
  }

  auto it = std::find(accessToken.begin(), accessToken.end(), ':');
  if (it == accessToken.end())
  {
    error.code = -1;
    error.string = "Missing secret for Aws AccessToken";
    return false;
  }
  key = std::string(accessToken.begin(), it);

  auto secret_begin = it + 1;
  it = std::find(secret_begin, accessToken.end(), ':');
  if (it == accessToken.end() || it + 1 == accessToken.end())
  {
    error.code = -1;
    error.string = "Missing session token for Aws AccessToken";
    return false;
  }
  secret = std::string(secret_begin, it);
  session = std::string(it + 1, accessToken.end());

  return true;
}

void AwsDmsIoManagerFactory::getComponentsFromGCSUrl(const std::string& gcsUrl, std::string& bucket, std::string& prefixPath) const
{
  auto pos = gcsUrl.find("$$");
  if (pos == std::string::npos)
  {
    bucket = gcsUrl;
  }
  else
  {
    bucket = std::string(gcsUrl.begin(), gcsUrl.begin() + pos);
    auto prefixBegin = gcsUrl.begin() + pos + 2;
    if (prefixBegin != gcsUrl.end())
    {
      prefixPath = std::string(prefixBegin, gcsUrl.end());
    }
  }
}

AwsDmsIoManagerFactory::AwsDmsIoManagerFactory(DmsDataset& dataset, Logger &logger)
  : DmsIoManagerFactory(dataset)
  , m_logger(logger)
  , m_region(getStringEnvironmentVariable("AWS_REGION"))
{
  if (m_region.empty())
  {
    //we have to skip automatic location detection
    m_region = "us-east-1";
  }
}

std::unique_ptr<IOManager> AwsDmsIoManagerFactory::createIOManager(std::chrono::time_point<std::chrono::steady_clock> &expirationTime, Error& error)
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

  AWSOpenOptions openOptions;
  if (!getComponentsFromAccessToken(access_token, openOptions.accessKeyId, openOptions.secretKey, openOptions.sessionToken, error))
  {
    return nullptr;
  }

  getComponentsFromGCSUrl(m_dataset.m_gc_url, openOptions.bucket, openOptions.key);

  if (m_endpointOverride.size())
  {
    openOptions.endpointOverride = m_endpointOverride;
  }
  else
  {
    openOptions.region = m_region;
  }

  auto ioManager = std::unique_ptr<IOManager>(new IOManagerAWSCurl(openOptions, m_logger, error));
  if (error.code)
  {
    return nullptr;
  }

  return ioManager;
}

}
