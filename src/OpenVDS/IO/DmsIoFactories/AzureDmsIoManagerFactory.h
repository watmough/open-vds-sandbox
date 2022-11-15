#ifndef OPENVDS_AZUREDmsIoMANAGER_FACTORY_H
#define OPENVDS_AZUREDmsIoMANAGER_FACTORY_H

#include "DmsIoManagerFactory.h"

namespace OpenVDS
{

struct AzureDmsIoManagerFactory : public DmsIoManagerFactory
{
  AzureDmsIoManagerFactory(DmsDataset& dataset);

  bool ensureIOManager(std::unique_ptr<IOManager>& ioManager, Error& error) override;
  void invalidate() override;
  std::chrono::time_point<std::chrono::steady_clock> m_expire;
};

}

#endif