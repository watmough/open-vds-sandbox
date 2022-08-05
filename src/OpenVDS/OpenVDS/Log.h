/****************************************************************************
** Copyright 2019 The Open Group
** Copyright 2019 Bluware, Inc.
** Copyright 2020 Microsoft Corp.
** Copyright 2020 Google, Inc.
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

#ifndef OPENVDS_LOG_H
#define OPENVDS_LOG_H

namespace OpenVDS
{
enum class LogLevel
{
  None = 0,
  Error = 1,
  Warning = 2,
  Info = 3,
  Trace = 4
};
/// <summary>
/// The OpenVDS Logging interface is used to provide a callback for
/// applications to get logging output from the library
/// </summary>
struct LogHandler
{
  typedef void (*LogCallback)(LogLevel level, const char* message, size_t messageSize, void* userHandle);

  LogHandler()
    : callback(nullptr)
    , userHandle(nullptr)
  {}

  LogHandler(LogCallback callback, void* userHandle = nullptr)
    : callback(callback)
    , userHandle(userHandle)
  {}

  LogCallback callback;
  void* userHandle;
};
}

#endif
