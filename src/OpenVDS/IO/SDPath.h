#ifndef OPENVDS_SDPATH_H
#define OPENVDS_SDPATH_H

#include <string>
#include <OpenVDS/Error.h>

#include <assert.h>
#include <fmt/format.h>

namespace OpenVDS
{
  // Get path sections confirming to sd://<tenant>/<subproject>/<path>*/<dataset>
  inline bool getSdPath(const std::string& sdPath, std::string& tenant, std::string& subproject, std::string& path, std::string& dataset, Error &error)
  {
    const char sd_protocol[] = "sd://";
    if (sdPath.size() < sizeof(sd_protocol)) //including zero termination
    {
      error.code = -1;
      error.string = fmt::format("sd path {} is empty. Expected format: sd://<tenant>/<subproject>/<path>*/<dataset>", sdPath);
      return false;
    }
    assert(tenant.empty());
    assert(subproject.empty());
    assert(path.empty());
    assert(dataset.empty());

    auto current = sdPath.begin() + sizeof(sd_protocol) - 1;
    auto end = sdPath.end();
    while (end != current && end - 1 != current && *(end - 1) == '/')
      end--;

    {
      auto tenant_end = std::find(current, end, '/');
      tenant = std::string(current, tenant_end);
      while (tenant_end != end && *tenant_end == '/')
        tenant_end++;
      current = tenant_end;
      if (tenant_end == end)
      {
        error.code = -1;
        error.string = fmt::format("sd path: {} is missing subproject. Expected format:  sd://<tenant>/<subproject>/<path>*/<dataset>", sdPath);
        return false;
      }
    }
    {
      auto subproject_end = std::find(current, end, '/');
      subproject = std::string(current, subproject_end);

      while (subproject_end != end && *subproject_end == '/')
        subproject_end++;
      current = subproject_end;
      if (subproject_end == end || subproject_end + 1 == end)
      {
        error.code = -1;
        error.string = fmt::format("sd path: {} is missing dataset. Expected format:  sd://<tenant>/<subproject>/<path>*//<dataset>", sdPath);
        return false;
      }
    }
    {
      auto rcurrent = std::make_reverse_iterator(end);
      auto rend = std::make_reverse_iterator(current);
      auto last_slash = std::find(rcurrent, rend, '/');
      while (last_slash != rend && *last_slash == '/')
        last_slash++;
      auto base_last_slash = last_slash.base();
      if (base_last_slash == current)
      {
        dataset = std::string(current, end);
      }
      else
      {
        path = std::string(current, base_last_slash);
        while (base_last_slash != end && *base_last_slash == '/')
          base_last_slash++;
        dataset = std::string(base_last_slash, end);
      }
    }
    return true;
  }
}

#endif