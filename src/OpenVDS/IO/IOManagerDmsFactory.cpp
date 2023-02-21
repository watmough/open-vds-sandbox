#include "IOManagerDmsFactory.h"

#include "IOManagerDms.h"
#include "IOManagerDmsProxy.h"
#include "VDS/Env.h"
namespace OpenVDS
{
  IOManager* CreateDMSIOManager(const DMSOpenOptions& openOptions, IOManager::AccessPattern accessPattern, Logger &logger, Error& error)
  {
    bool useSDAPI= getBooleanEnvironmentVariable("OPENVDS_DMS_SDAPI");
    if (useSDAPI)
      return new IOManagerDms(openOptions, accessPattern, logger, error);
    return new IOManagerDMSProxy(openOptions, accessPattern, logger, error);
  }
}