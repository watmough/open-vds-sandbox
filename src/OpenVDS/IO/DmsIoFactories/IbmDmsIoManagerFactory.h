#ifndef OPENVDS_IBMDmsIoMANAGER_FACTORY_H
#define OPENVDS_IBMDmsIoMANAGER_FACTORY_H

#include "AwsDmsIoManagerFactory.h"

namespace OpenVDS
{

struct IbmDmsIoManagerFactory : public AwsDmsIoManagerFactory
{
  IbmDmsIoManagerFactory(DmsDataset& dataset, Logger &logger);
  
  void getComponentsFromGCSUrl(const std::string& gcsUrl, std::string& bucket, std::string& prefixPath) const override;
};

}

#endif
