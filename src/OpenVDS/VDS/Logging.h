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
#include <string>

namespace OpenVDS
{
struct Logger
{
  Logger(LogLevel level, const LogHandler& handler)
    : level(level)
    , handler(handler)
  {}
  LogLevel level;
  LogHandler handler;

  inline void LogTrace(const char* message, size_t size)
  {
    if (int(level) >= int(LogLevel::Trace))
      handler.callback(LogLevel::Trace, message, size, handler.userHandle);
  }
  inline void LogTrace(const std::string& message) { LogTrace(message.c_str(), message.size()); }
  template<size_t SIZE>
  inline void LogTrace(const char(&message)[SIZE]) { LogTrace(message, SIZE - 1); }

  inline void LogInfo(const char* message, size_t size)
  {
    if (int(level) >= int(LogLevel::Info))
      handler.callback(LogLevel::Info, message, size, handler.userHandle);
  }
  inline void LogInfo(const std::string& message) { LogInfo(message.c_str(), message.size()); }
  template<size_t SIZE>
  inline void LogInfo(const char(&message)[SIZE]) { LogInfo(message, SIZE - 1); }

  inline void LogWarning(const char* message, size_t size)
  {
    if (int(level) >= int(LogLevel::Warning))
      handler.callback(LogLevel::Warning, message, size, handler.userHandle);
  }
  inline void LogWarning(const std::string& message) { LogWarning(message.c_str(), message.size()); }
  template<size_t SIZE>
  inline void LogWarning(const char(&message)[SIZE]) { LogWarning(message, SIZE - 1); }

  inline void LogError(const char* message, size_t size)
  {
    if (int(level) >= int(LogLevel::Error))
      handler.callback(LogLevel::Error, message, size, handler.userHandle);
  }
  inline void LogError(const std::string& message) { LogError(message.c_str(), message.size()); }
  template<size_t SIZE>
  inline void LogError(const char(&message)[SIZE]) { LogError(message, SIZE - 1); }
};
}

#endif
