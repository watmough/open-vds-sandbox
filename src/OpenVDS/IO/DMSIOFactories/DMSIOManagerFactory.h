#ifndef OPENVDS_DMSIOMANAGER_FACTORY_H
#define OPENVDS_DMSIOMANAGER_FACTORY_H

#include <OpenVDS/OpenVDS.h>
#include <IO/IOManager.h>
#include <IO/IOManagerCurl.h>
#include <chrono>
#include <json_cpp_include.h>

namespace OpenVDS
{
bool ParseJSONFromBuffer(const std::vector<unsigned char>& json, Json::Value& root, Error& error);

struct DMSManager
{
  DMSManager(const std::string& authorityUrl, const std::string& appKey, CurlHandler& curlHandler, Logger& logger);
  void setSdTokenDefaultExpiry();
  bool ensureSdToken(Error& error);
  void addHeaders(std::vector<std::string>& headers);

  const std::string m_authorityUrl;
  const std::string m_appKey;
  CurlHandler& m_curlHandler;
  std::string m_sdToken;
  std::chrono::time_point<std::chrono::system_clock> m_sdTokenExpiry;
  Logger& m_logger;
  std::string (*m_authProviderCallback)(const void*);
  const void *m_authProviderCallbackData;
};

struct DMSDataset
{
  DMSDataset(DMSManager& manager, const std::string url, Error& error);

  bool open(IOManager::AccessPattern accessPattern, Error& error);
  bool close(Error& error);

  DMSManager& m_manager;
  const std::string m_url;
  std::string m_tenant;
  std::string m_subproject;
  std::string m_path;
  std::string m_dataset;
  std::string m_lock_id;
  std::string m_service_provider;
  std::string m_gc_url;
  IOManager::AccessPattern m_accessPattern;
  bool m_opened;
  std::chrono::time_point<std::chrono::steady_clock> m_azure_sas_token_expires;
};

struct DMSIOManagerFactory
{
  static DMSIOManagerFactory* createDMSIOManagerFactory(const std::string& serviceProvider, DMSDataset &dataset, Error &error);

  virtual ~DMSIOManagerFactory();
  virtual bool ensureIOManager(std::unique_ptr<IOManager>& iomanager, Error& error) = 0;
  virtual void invalidate() = 0;

  struct GcsAccessToken
  {
    std::vector<std::pair<std::string, std::string>> headers;
    std::vector<uint8_t> data;
  };
  GcsAccessToken gcsAccessToken(Error& error);

protected:
  DMSIOManagerFactory(DMSDataset& dataset);

  DMSDataset& m_dataset;
};
}
#endif