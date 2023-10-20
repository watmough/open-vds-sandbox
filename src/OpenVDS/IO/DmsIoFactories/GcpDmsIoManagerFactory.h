#ifndef OPENVDS_GCPDmsIoMANAGER_FACTORY_H
#define OPENVDS_GCPDmsIoMANAGER_FACTORY_H

#include "DmsIoManagerFactory.h"

namespace OpenVDS
{

struct GcpDmsIoManagerFactory : public DmsIoManagerFactory
{
  GcpDmsIoManagerFactory(DmsDataset& dataset, Logger &logger);

  std::unique_ptr<IOManager> createIOManager(std::shared_ptr<CurlHandler> curlHandler, std::chrono::time_point<std::chrono::steady_clock> &expirationTime, Error& error) override;

  Logger& m_logger;
};

}

#endif
