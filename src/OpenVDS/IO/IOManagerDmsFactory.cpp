#include "IOManagerDmsFactory.h"

#include "IOManagerDms.h"
namespace OpenVDS
{
  IOManager* CreateDMSIOManager(const DMSOpenOptions& openOptions, IOManager::AccessPattern accessPattern, Error& error)
  {
    return new IOManagerDms(openOptions, accessPattern, error);
  }
}