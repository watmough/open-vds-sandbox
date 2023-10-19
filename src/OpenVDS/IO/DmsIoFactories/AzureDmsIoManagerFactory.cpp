#include "AzureDmsIoManagerFactory.h"

#include <IO/IOManagerAzurePresigned.h>

#include <string>
#include <fmt/format.h>

namespace OpenVDS
{
template<size_t SIZE>
static bool startsWith(const std::string& source, const char(&starts_with)[SIZE])
{
  return source.rfind(starts_with, 0) == 0;
}

static bool getComponentsFromSAS(const std::string& sas, std::string& account, std::string& container, std::string& bearer, Error& error)
{
  auto current = sas.begin();
  auto end = sas.end();
  if (startsWith(sas, "https://"))
    current += 8;
  else if (startsWith(sas, "http://"))
    current += 7;
  if (current == end)
  {
    error.code = -1;
    error.string = "Empty sas token.";
    return false;
  }
  auto account_end = std::find(current, end, '.');
  if (account_end == end)
  {
    error.code = -1;
    error.string = "Missing account in sas token.";
    return false;
  }
  account = std::string(current, account_end);
  current = account_end++;
  auto container_start = std::find(current, end, '/');
  while (container_start != end && *container_start == '/')
    container_start++;
  auto container_end = std::find(container_start, end, '?');
  if (container_start == end || container_end == end)
  {
    error.code = -1;
    error.string = "Missing container in sas token.";
    return false;

  }
  container = std::string(container_start, container_end);
  bearer = std::string(container_end + 1, end);
  return true;
}

static void getContainerAndBlobPrefix(const std::string& gcsurl, std::string& container, std::string& blobPrefix)
{
  auto current = gcsurl.begin();
  auto end = gcsurl.end();
  auto container_end = std::find(current, end, '/');
  container = std::string(current, container_end);
  while (container_end != end && *container_end == '/')
    container_end++;
  blobPrefix = std::string(container_end, end);
}

static bool getPresignedUrl(const std::string& url, const std::string& blob_prefix, std::string& presignedUrl, std::string& presignedSuffix, Error& error)
{
  auto it = std::find(url.begin(), url.end(), '?');
  if (it == url.end())
  {
    error.code = -1;
    error.string = "Failed to generate presigned url";
    return false;
  }

  presignedUrl = std::string(url.begin(), it);
  if (blob_prefix.size())
  {
    presignedUrl += '/' + blob_prefix;
  }
  presignedSuffix = std::string(it, url.end());
  return true;
}

AzureDmsIoManagerFactory::AzureDmsIoManagerFactory(DmsDataset& dataset)
  : DmsIoManagerFactory(dataset)
{}

std::unique_ptr<IOManager> AzureDmsIoManagerFactory::createIOManager(std::chrono::time_point<std::chrono::steady_clock> &expirationTime, Error& error)
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

  std::string account;
  std::string container;
  std::string bearer;
  if (!getComponentsFromSAS(access_token, account, container, bearer, error))
  {
    return nullptr;
  }
  std::string container_2;
  std::string blob_prefix;
  getContainerAndBlobPrefix(m_dataset.m_gc_url, container_2, blob_prefix);
  AzureOpenOptions openOptions;
  openOptions.accountName = account;
  openOptions.container = container;
  openOptions.blob = blob_prefix;


  std::string presignedUrl;
  std::string presignedSuffix;
  if (!getPresignedUrl(access_token, blob_prefix, presignedUrl, presignedSuffix, error))
    return nullptr;

  auto ioManager = std::unique_ptr<IOManager>(new IOManagerAzurePresigned(presignedUrl, presignedSuffix, m_dataset.m_manager.m_logger, error));

  if (error.code)
  {
    return nullptr;
  }

  return ioManager;
}

}
