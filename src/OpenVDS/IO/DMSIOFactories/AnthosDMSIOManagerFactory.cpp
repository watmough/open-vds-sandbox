#include "AnthosDMSIOManagerFactory.h"

#include "VDS/Env.h"

namespace OpenVDS
{
AnthosDMSIOManagerFactory::AnthosDMSIOManagerFactory(DMSDataset& dataset, Logger& logger)
  : AwsDMSIOManagerFactory(dataset,logger)
{
  m_endpointOverride = getStringEnvironmentVariable("S3_ENDPOINT_OVERRIDE");
}
}
