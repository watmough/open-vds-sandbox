#ifndef OPENVDS_GCPDmsIoMANAGER_FACTORY_H
#define OPENVDS_GCPDmsIoMANAGER_FACTORY_H

#include "DmsIoManagerFactory.h"

namespace OpenVDS
{

struct GcpDmsIoManagerFactory : public DmsIoManagerFactory
{
  GcpDmsIoManagerFactory(DmsDataset& dataset, Logger &logger);

  bool ensureIOManager(std::unique_ptr<IOManager>& iomanager, Error& error) override;
  void invalidate() override;

  Logger& m_logger;
  std::chrono::time_point<std::chrono::steady_clock> m_expire;
};

}

#endif
