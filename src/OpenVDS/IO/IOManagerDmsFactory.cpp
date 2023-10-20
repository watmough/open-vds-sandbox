#include "IOManagerDmsFactory.h"

#include "IOManagerDmsProxy.h"
#include "VDS/Env.h"
namespace OpenVDS
{
  IOManager* CreateDMSIOManager(const DMSOpenOptions& openOptions, IOManager::AccessPattern accessPattern, Logger &logger, Error& error)
  {
    std::unique_ptr<IOManager> ioManager(new IOManagerDMSProxy(openOptions, accessPattern, logger, error));
    return (error.code == 0) ? ioManager.release() : nullptr;
  }
}
