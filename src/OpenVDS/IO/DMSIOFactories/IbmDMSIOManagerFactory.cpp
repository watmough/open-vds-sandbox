#include "IbmDMSIOManagerFactory.h"

#include "IO/IOManagerAWSCurl.h"

#include "VDS/Env.h"

#include <string>
#include <fmt/format.h>

namespace OpenVDS
{

IbmDMSIOManagerFactory::IbmDMSIOManagerFactory(DMSDataset& dataset, Logger &logger)
  : AwsDMSIOManagerFactory(dataset, logger)
{
  m_endpointOverride = getStringEnvironmentVariable("IBM_COS_URL");
}

void IbmDMSIOManagerFactory::getComponentsFromGCSUrl(const std::string& gcsUrl, std::string& bucket, std::string& prefixPath) const 
{
  auto pos = gcsUrl.find("/");
  if (pos == std::string::npos)
  {
    bucket = gcsUrl;
  }
  else
  {
    bucket = std::string(gcsUrl.begin(), gcsUrl.begin() + pos);
    auto prefixBegin = gcsUrl.begin() + pos + 2;
    if (prefixBegin != gcsUrl.end())
    {
      prefixPath = std::string(prefixBegin, gcsUrl.end());
    }
  }
}

}
