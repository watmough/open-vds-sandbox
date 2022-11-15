#ifndef OPENVDS_IBMDMSIOMANAGER_FACTORY_H
#define OPENVDS_IBMDMSIOMANAGER_FACTORY_H

#include "AwsDMSIOManagerFactory.h"

namespace OpenVDS
{

struct IbmDMSIOManagerFactory : public AwsDMSIOManagerFactory
{
  IbmDMSIOManagerFactory(DMSDataset& dataset, Logger &logger);
  
  void getComponentsFromGCSUrl(const std::string& gcsUrl, std::string& bucket, std::string& prefixPath) const override;
};

}

#endif
