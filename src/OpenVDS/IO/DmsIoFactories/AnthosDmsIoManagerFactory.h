#ifndef OPENVDS_ANHTOSDmsIoMANAGER_FACTORY_H
#define OPENVDS_ANHTOSDmsIoMANAGER_FACTORY_H

#include "AwsDmsIoManagerFactory.h"

namespace OpenVDS
{

struct AnthosDmsIoManagerFactory : public AwsDmsIoManagerFactory
{
  AnthosDmsIoManagerFactory(DmsDataset& dataset, Logger &logger);
};

}

#endif
