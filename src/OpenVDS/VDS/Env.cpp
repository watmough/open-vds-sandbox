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

#ifdef WIN32
#undef WIN32_LEAN_AND_MEAN // avoid warnings if defined on command line
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
static void s2ws(const std::string& source, std::wstring& target)
{
  int len;
  int slength = (int)source.length() + 1;
  len = MultiByteToWideChar(CP_UTF8, 0, source.c_str(), slength, 0, 0);
  target.resize(len);
  MultiByteToWideChar(CP_UTF8, 0, source.c_str(), slength, &target[0], len);
}
#endif

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

std::string getStringEnvironmentVariable(const char* name)
{
  const char *c_var = getenv(name);
  if (c_var == nullptr)
    return std::string();
  return std::string(c_var);
}

bool isEnvironmentVariableSet(const char* name)
{
  const char *c_var = getenv(name);
  return c_var != nullptr;
}

void setEnvironmentVariable(const char* name, const std::string& value)
{
#ifdef WIN32
  std::wstring name_w;
  s2ws(name, name_w);
  std::wstring value_w;
  s2ws(value, value_w);
  SetEnvironmentVariableW(name_w.c_str(), value_w.c_str());
#else
  setenv(name, value.c_str(), true);
#endif
}

}
