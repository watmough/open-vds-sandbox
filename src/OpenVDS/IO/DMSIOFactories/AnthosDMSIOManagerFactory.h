#ifndef OPENVDS_ANHTOSDMSIOMANAGER_FACTORY_H
#define OPENVDS_ANHTOSDMSIOMANAGER_FACTORY_H

#include "AwsDMSIOManagerFactory.h"

namespace OpenVDS
{

struct AnthosDMSIOManagerFactory : public AwsDMSIOManagerFactory
{
  AnthosDMSIOManagerFactory(DMSDataset& dataset, Logger &logger);
};

}

#endif
