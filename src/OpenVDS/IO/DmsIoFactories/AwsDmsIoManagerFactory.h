#ifndef OPENVDS_AWSDmsIoMANAGER_FACTORY_H
#define OPENVDS_AWSDmsIoMANAGER_FACTORY_H

#include "DmsIoManagerFactory.h"

namespace OpenVDS
{

struct AwsDmsIoManagerFactory : public DmsIoManagerFactory
{
  AwsDmsIoManagerFactory(DmsDataset& dataset, Logger &logger);

  bool ensureIOManager(std::unique_ptr<IOManager>& ioManager, Error& error) override;
  void invalidate() override;

  virtual bool getComponentsFromAccessToken(const std::string& accessToken, std::string& key, std::string& secret, std::string& session, Error& error) const;
  virtual void getComponentsFromGCSUrl(const std::string& gcsUrl, std::string& bucket, std::string& prefixPath) const;

  std::chrono::time_point<std::chrono::steady_clock> m_expire;
  Logger& m_logger;
  std::string m_region;
  std::string m_endpointOverride;
};

}

#endif
