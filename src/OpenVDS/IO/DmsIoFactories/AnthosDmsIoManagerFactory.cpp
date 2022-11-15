#include "AnthosDmsIoManagerFactory.h"

#include "VDS/Env.h"

namespace OpenVDS
{
AnthosDmsIoManagerFactory::AnthosDmsIoManagerFactory(DmsDataset& dataset, Logger& logger)
  : AwsDmsIoManagerFactory(dataset,logger)
{
  m_endpointOverride = getStringEnvironmentVariable("S3_ENDPOINT_OVERRIDE");
}
}
