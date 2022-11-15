#ifndef OPENVDS_SDPATH_H
#define OPENVDS_SDPATH_H

#include <string>
#include <OpenVDS/Error.h>


namespace OpenVDS
{
  // Get path sections confirming to sd://<tenant>/<subproject>/<path>*/<dataset>
  bool getSdPath(const std::string& sdPath, std::string& tenant, std::string& subproject, std::string& path, std::string& dataset, Error &error);
}

#endif
