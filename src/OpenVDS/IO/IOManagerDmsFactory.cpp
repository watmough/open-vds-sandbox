#include "IOManagerDmsFactory.h"

#include "IOManagerDms.h"
namespace OpenVDS
{
  IOManager* CreateDMSIOManager(const DMSOpenOptions& openOptions, IOManager::AccessPattern accessPattern, OpenVDSLogging logHandler, Error& error)
  {
    return new IOManagerDms(openOptions, accessPattern, logHandler, error);
  }
}