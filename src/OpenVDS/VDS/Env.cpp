#define _CRT_SECURE_NO_WARNINGS 1
#include "Env.h"

#include <stdlib.h>
#include <string.h>
#include <string>
#include <algorithm>

namespace OpenVDS
{

  inline char asciitolower(char in) {
  if (in <= 'Z' && in >= 'A')
    return in - ('Z' - 'z');
  return in;
}

bool getBooleanEnvironmentVariable(const char *name)
{
  const char *c_var = getenv(name);
  if (c_var == nullptr)
    return false;
  std::string var(c_var);
  std::transform(var.begin(), var.end(), var.begin(), asciitolower);
  if (var == "false")
    return false;
  if (var == "0")
    return false;
  if (var == "off")
    return false;
  return true;
}
}
