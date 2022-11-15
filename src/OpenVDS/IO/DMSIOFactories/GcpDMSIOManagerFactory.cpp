#include "GcpDMSIOManagerFactory.h"

#include <fmt/format.h>
#include "VDS/Env.h"

#include <IO/IOManagerGoogle.h>

namespace OpenVDS
{
GcpDMSIOManagerFactory::GcpDMSIOManagerFactory(DMSDataset& dataset, Logger& logger)
  : DMSIOManagerFactory(dataset)
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
    auto prefixBegin = gcsUrl.begin() + pos + 2;
    if (prefixBegin != gcsUrl.end())
    {
      prefixPath = std::string(prefixBegin, gcsUrl.end());
    }
  }
}

bool GcpDMSIOManagerFactory::ensureIOManager(std::unique_ptr<IOManager>& ioManager, Error& error)
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

  std::string bucket;
  std::string pathPrefix;
  getComponentsFromGCSUrl(m_dataset.m_gc_url, bucket, pathPrefix);
  GoogleCredentialsToken token(access_token);
  GoogleOpenOptions openOptions(bucket, pathPrefix, token);
  ioManager.reset(new IOManagerGoogle(openOptions, m_logger, error));

  if (error.code)
  {
    ioManager.reset();
    return false;
  }
  return true;
}

void GcpDMSIOManagerFactory::invalidate()
{
  m_expire = {};
}

}
