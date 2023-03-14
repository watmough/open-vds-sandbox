#include "IOManagerDmsFactory.h"

#include "IOManagerDmsProxy.h"
#include "VDS/Env.h"
namespace OpenVDS
{
  IOManager* CreateDMSIOManager(const DMSOpenOptions& openOptions, IOManager::AccessPattern accessPattern, Logger &logger, Error& error)
  {
    return new IOManagerDMSProxy(openOptions, accessPattern, logger, error);
  }
}
