#include "IOManagerDmsFactory.h"

#include "IOManagerDms.h"
#include "IOManagerDmsProxy.h"
#include "VDS/Env.h"
namespace OpenVDS
{
  IOManager* CreateDMSIOManager(const DMSOpenOptions& openOptions, IOManager::AccessPattern accessPattern, Logger &logger, Error& error)
  {
    bool useCurl = getBooleanEnvironmentVariable("OPENVDS_DMS_CURL");
    if (useCurl)
      return new IOManagerDMSProxy(openOptions, accessPattern, logger, error);
    return new IOManagerDms(openOptions, accessPattern, logger, error);
  }
}