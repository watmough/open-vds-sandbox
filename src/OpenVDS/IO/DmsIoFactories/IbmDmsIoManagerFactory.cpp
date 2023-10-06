#include "IbmDmsIoManagerFactory.h"

#include "IO/IOManagerAWSCurl.h"

#include "VDS/Env.h"

#include <string>
#include <fmt/format.h>

namespace OpenVDS
{

IbmDmsIoManagerFactory::IbmDmsIoManagerFactory(DmsDataset& dataset, Logger &logger)
  : AwsDmsIoManagerFactory(dataset, logger)
{
  m_endpointOverride = getStringEnvironmentVariable("IBM_COS_URL");
}

void IbmDmsIoManagerFactory::getComponentsFromGCSUrl(const std::string& gcsUrl, std::string& bucket, std::string& prefixPath) const 
{
  auto pos = gcsUrl.find("/");
  if (pos == std::string::npos)
  {
    bucket = gcsUrl;
  }
  else
  {
    bucket = std::string(gcsUrl.begin(), gcsUrl.begin() + pos);
    auto prefixBegin = gcsUrl.begin() + pos + 1;
    if (prefixBegin != gcsUrl.end())
    {
      prefixPath = std::string(prefixBegin, gcsUrl.end());
    }
  }
}

}
