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

#include "GlobalStateImpl.h"
#include <fmt/format.h>

namespace OpenVDS
{
  GlobalLogInterface::GlobalLogInterface()
    : callback(nullptr)
    , userHandle(nullptr)
  {}
  void GlobalLogInterface::SetLoggCallback(LogCallback callback, void* userHandle)
  {
    std::unique_lock<std::mutex> lock(mutex);
    this->callback = callback;
    this->userHandle = userHandle;
  }
  template<size_t SIZE>
  static fmt::string_view create_fmt_string_view(const char(&a)[SIZE])
  {
    return fmt::string_view(a, SIZE - 1);
  }

  static fmt::string_view GetLogLevelString(LogLevel loglevel)
  {
    const fmt::string_view tagNames[] =
    {
      create_fmt_string_view("None"),
      create_fmt_string_view("Error"),
      create_fmt_string_view("Warning"),
      create_fmt_string_view("Info"),
      create_fmt_string_view("Trace")
    };
    static_assert(sizeof(tagNames) / sizeof(*tagNames) == int(LogLevel::Trace) + 1, "LogLevel names does not match LogLevel enum");
    return tagNames[int(loglevel)];
  }

  void GlobalLogInterface::SetDefaultLogCallback()
  {
    std::unique_lock<std::mutex> lock(mutex);
    callback = [](LogLevel level, const char* message, size_t messageSize, void*)
    {
      auto stream = (int(level) < int(LogLevel::Warning)) ? stdout : stderr;
      auto str = fmt::format("{}: {}\n", GetLogLevelString(level), fmt::string_view(message, messageSize));
      fwrite(str.c_str(), 1, str.size(), stream);
    };
    userHandle = nullptr;
  }
}
