/****************************************************************************
** Copyright 2022 The Open Group
** Copyright 2022 Bluware, Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
****************************************************************************/

#ifndef OPENVDSLOGGING_H_INCLUDE
#define OPENVDSLOGGING_H_INCLUDE

#include <OpenVDS/OpenVDS.h>
#include "GlobalStateImpl.h"
#include <string>

namespace OpenVDS
{
struct Logger
{
  Logger(GlobalLogInterface &logInterface, LogLevel level)
    : logInterface(logInterface)
    , level(level)
  {}
  GlobalLogInterface &logInterface;
  LogLevel level;

  inline void LogTrace(const char* message, size_t size) const
  {
    if (int(level) >= int(LogLevel::Trace))
      logInterface.Log(LogLevel::Trace, message, size);
  }
  inline void LogTrace(const std::string& message) const { LogTrace(message.c_str(), message.size()); }
  template<size_t SIZE>
  inline void LogTrace(const char(&message)[SIZE]) const { LogTrace(message, SIZE - 1); }

  inline void LogInfo(const char* message, size_t size) const
  {
    if (int(level) >= int(LogLevel::Info))
      logInterface.Log(LogLevel::Info, message, size);
  }
  inline void LogInfo(const std::string& message) const { LogInfo(message.c_str(), message.size()); }
  template<size_t SIZE>
  inline void LogInfo(const char(&message)[SIZE]) const { LogInfo(message, SIZE - 1); }

  inline void LogWarning(const char* message, size_t size) const
  {
    if (int(level) >= int(LogLevel::Warning))
      logInterface.Log(LogLevel::Warning, message, size);
  }
  inline void LogWarning(const std::string& message) const { LogWarning(message.c_str(), message.size()); }
  template<size_t SIZE>
  inline void LogWarning(const char(&message)[SIZE]) const { LogWarning(message, SIZE - 1); }

  inline void LogError(const char* message, size_t size) const
  {
    if (int(level) >= int(LogLevel::Error))
      logInterface.Log(LogLevel::Error, message, size);
  }
  inline void LogError(const std::string& message) const { LogError(message.c_str(), message.size()); }
  template<size_t SIZE>
  inline void LogError(const char(&message)[SIZE]) const { LogError(message, SIZE - 1); }
};
}

#endif
