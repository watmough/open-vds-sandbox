#ifndef OPENVDS_GCPDMSIOMANAGER_FACTORY_H
#define OPENVDS_GCPDMSIOMANAGER_FACTORY_H

#include "DMSIOManagerFactory.h"

namespace OpenVDS
{

struct GcpDMSIOManagerFactory : public DMSIOManagerFactory
{
  GcpDMSIOManagerFactory(DMSDataset& dataset, Logger &logger);

  bool ensureIOManager(std::unique_ptr<IOManager>& iomanager, Error& error) override;
  void invalidate() override;

  Logger& m_logger;
  std::chrono::time_point<std::chrono::steady_clock> m_expire;
};

}

#endif
