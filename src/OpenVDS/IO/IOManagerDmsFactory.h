#ifndef IOMANAGERDMSFACTORY_H
#define IOMANAGERDMSFACTORY_H

#include "IOManager.h"

namespace OpenVDS
{
  IOManager* CreateDMSIOManager(const DMSOpenOptions& openOptions, IOManager::AccessPattern accessPattern, Error& error);
}
#endif
