#include "AwsDMSIOManagerFactory.h"

#include "IO/IOManagerAWSCurl.h"

#include "VDS/Env.h"

#include <string>
#include <fmt/format.h>

namespace OpenVDS
{

bool AwsDMSIOManagerFactory::getComponentsFromAccessToken(const std::string& accessToken, std::string& key, std::string& secret, std::string& session, Error& error) const
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

void AwsDMSIOManagerFactory::getComponentsFromGCSUrl(const std::string& gcsUrl, std::string& bucket, std::string& prefixPath) const
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

AwsDMSIOManagerFactory::AwsDMSIOManagerFactory(DMSDataset& dataset, Logger &logger)
  : DMSIOManagerFactory(dataset)
  , m_logger(logger)
  , m_region(getStringEnvironmentVariable("AWS_REGION"))
{
  if (m_region.empty())
  {
    //we have to skip automatic location detection
    m_region = "us-east-1";
  }
}

bool AwsDMSIOManagerFactory::ensureIOManager(std::unique_ptr<IOManager>& ioManager, Error& error)
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

  AWSOpenOptions openOptions;
  if (!getComponentsFromAccessToken(access_token, openOptions.accessKeyId, openOptions.secretKey, openOptions.sessionToken, error))
  {
    return false;
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

  ioManager.reset(new IOManagerAWSCurl(openOptions, m_logger, error));
  if (error.code)
  {
    ioManager.reset();
    return false;
  }
  return true;
}

void AwsDMSIOManagerFactory::invalidate()
{
  m_expire = {};
}

}
