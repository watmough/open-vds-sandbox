#ifndef OPENVDS_AWSDMSIOMANAGER_FACTORY_H
#define OPENVDS_AWSDMSIOMANAGER_FACTORY_H

#include <IO/IOManagerDmsProxy.h>

namespace OpenVDS
{

struct AwsDMSIOManagerFactory : public DMSIOManagerFactory
{
  AwsDMSIOManagerFactory(DMSDataset& dataset, Logger &logger);

  bool ensureIOManager(std::unique_ptr<IOManager>& ioManager, Error& error) override;
  void invalidate() override;
  std::chrono::time_point<std::chrono::steady_clock> m_expire;
  Logger& m_logger;
  std::string m_region;
  std::string m_endpointOverride;
};

}

#endif
