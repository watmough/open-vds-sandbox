#ifndef OPENVDS_STRING_TO_DOUBLE_H
#define OPENVDS_STRING_TO_DOUBLE_H
#include <string>
#include <OpenVDS/OpenVDS.h>
namespace OpenVDS
{
  double StringToDouble(const std::string& number, Error& error);
}
#endif