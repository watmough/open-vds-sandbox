#ifndef OPENVDS_AZUREDMSIOMANAGER_FACTORY_H
#define OPENVDS_AZUREDMSIOMANAGER_FACTORY_H

#include <IO/IOManagerDmsProxy.h>

namespace OpenVDS
{

struct AzureDMSIOManagerFactory : public DMSIOManagerFactory
{
  AzureDMSIOManagerFactory(DMSDataset& dataset);

  bool ensureIOManager(std::unique_ptr<IOManager>& ioManager, Error& error) override;
  void invalidate() override;
  std::chrono::time_point<std::chrono::steady_clock> m_expire;
};

}

#endif