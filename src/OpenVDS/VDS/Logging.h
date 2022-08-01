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

#include <OpenVDS/OpenVDSInterface.h>

namespace OpenVDS
{
  inline void LogTrace(OpenVDSLogging& handler, const char* message, size_t size)
  {
    if (int(handler.level) >= int(OpenVDSLogging::Trace))
      handler.callback(OpenVDSLogging::Trace, message, size, handler.userHandle);
  }
  inline void LogTrace(OpenVDSLogging& handler, const std::string& message) { LogTrace(handler, message.c_str(), message.size()); }
  template<size_t SIZE>
  inline void LogTrace(OpenVDSLogging& handler, const char(&message)[SIZE]) { LogTrace(handler, message, SIZE - 1); }

  inline void LogInfo(OpenVDSLogging& handler, const char* message, size_t size)
  {
    if (int(handler.level) >= int(OpenVDSLogging::Info))
      handler.callback(OpenVDSLogging::Info, message, size, handler.userHandle);
  }
  inline void LogInfo(OpenVDSLogging& handler, const std::string& message) { LogInfo(handler, message.c_str(), message.size()); }
  template<size_t SIZE>
  inline void LogInfo(OpenVDSLogging& handler, const char(&message)[SIZE]) { LogInfo(handler, message, SIZE - 1); }

  inline void LogWarning(OpenVDSLogging& handler, const char* message, size_t size)
  {
    if (int(handler.level) >= int(OpenVDSLogging::Warning))
      handler.callback(OpenVDSLogging::Warning, message, size, handler.userHandle);
  }
  inline void LogWarning(OpenVDSLogging& handler, const std::string& message) { LogWarning(handler, message.c_str(), message.size()); }
  template<size_t SIZE>
  inline void LogWarning(OpenVDSLogging& handler, const char(&message)[SIZE]) { LogWarning(handler, message, SIZE - 1); }

  inline void LogError(OpenVDSLogging& handler, const char* message, size_t size)
  {
    if (int(handler.level) >= int(OpenVDSLogging::Error))
      handler.callback(OpenVDSLogging::Error, message, size, handler.userHandle);
  }
  inline void LogError(OpenVDSLogging& handler, const std::string& message) { LogError(handler, message.c_str(), message.size()); }
  template<size_t SIZE>
  inline void LogError(OpenVDSLogging& handler, const char(&message)[SIZE]) { LogError(handler, message, SIZE - 1); }
}

#endif
