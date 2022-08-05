#ifndef IOMANAGERDMSFACTORY_H
#define IOMANAGERDMSFACTORY_H

#include "IOManager.h"
#include <VDS/Logging.h>

namespace OpenVDS
{
  IOManager* CreateDMSIOManager(const DMSOpenOptions& openOptions, IOManager::AccessPattern accessPattern, Logger &logger, Error& error);
}
#endif
