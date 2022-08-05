#include "IOManagerDmsFactory.h"

#include "IOManagerDms.h"
namespace OpenVDS
{
  IOManager* CreateDMSIOManager(const DMSOpenOptions& openOptions, IOManager::AccessPattern accessPattern, Logger &logger, Error& error)
  {
    return new IOManagerDms(openOptions, accessPattern, logger, error);
  }
}