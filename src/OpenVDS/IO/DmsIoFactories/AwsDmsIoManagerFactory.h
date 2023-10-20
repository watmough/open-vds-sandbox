#ifndef OPENVDS_AWSDmsIoMANAGER_FACTORY_H
#define OPENVDS_AWSDmsIoMANAGER_FACTORY_H

#include "DmsIoManagerFactory.h"

namespace OpenVDS
{

struct AwsDmsIoManagerFactory : public DmsIoManagerFactory
{
  AwsDmsIoManagerFactory(DmsDataset& dataset, Logger &logger);

  std::unique_ptr<IOManager> createIOManager(std::shared_ptr<CurlHandler> curlHandler, std::chrono::time_point<std::chrono::steady_clock> &expirationTime, Error& error) override;

  virtual bool getComponentsFromAccessToken(const std::string& accessToken, std::string& key, std::string& secret, std::string& session, Error& error) const;
  virtual void getComponentsFromGCSUrl(const std::string& gcsUrl, std::string& bucket, std::string& prefixPath) const;

  Logger& m_logger;
  std::string m_region;
  std::string m_endpointOverride;
};

}

#endif
