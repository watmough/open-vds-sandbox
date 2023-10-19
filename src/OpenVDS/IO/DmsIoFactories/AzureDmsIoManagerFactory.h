#ifndef OPENVDS_AZUREDmsIoMANAGER_FACTORY_H
#define OPENVDS_AZUREDmsIoMANAGER_FACTORY_H

#include "DmsIoManagerFactory.h"

namespace OpenVDS
{

struct AzureDmsIoManagerFactory : public DmsIoManagerFactory
{
  AzureDmsIoManagerFactory(DmsDataset& dataset);

  std::unique_ptr<IOManager> createIOManager(std::chrono::time_point<std::chrono::steady_clock> &expirationTime, Error& error) override;
};

}

#endif