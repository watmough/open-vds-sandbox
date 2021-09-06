#ifndef ENV_H
#define ENV_H

#include <string>

namespace OpenVDS
{
bool getBooleanEnvironmentVariable(const char *name);

bool isEnvironmentVariableSet(const char* name);

void setEnvironmentVariable(const char *name, const std::string &value);
}

#endif
