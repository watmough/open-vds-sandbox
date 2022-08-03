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

#include <VDS/Logging.h>
#include <gtest/gtest.h>

GTEST_TEST(VDS_integration, Logging)
{
  OpenVDS::LogHandler logHandler;
  int count = 0;
  logHandler.userHandle = &count;
  logHandler.callback = [](OpenVDS::LogLevel level, const char* message, size_t messageSize, void* userHandle)
  {
    auto local_count = static_cast<int*>(userHandle);
    (*local_count)++;
  };

  logHandler.level = OpenVDS::LogLevel::None;
  OpenVDS::LogError(logHandler, "hello world");
  EXPECT_EQ(0, count);
  OpenVDS::LogWarning(logHandler, "hello world");
  EXPECT_EQ(0, count);
  OpenVDS::LogInfo(logHandler, "hello world");
  EXPECT_EQ(0, count);
  OpenVDS::LogTrace(logHandler, "hello world");
  EXPECT_EQ(0, count);

  logHandler.level = OpenVDS::LogLevel::Error;
  OpenVDS::LogError(logHandler, "hello world");
  EXPECT_EQ(1, count);
  OpenVDS::LogWarning(logHandler, "hello world");
  EXPECT_EQ(1, count);
  OpenVDS::LogInfo(logHandler, "hello world");
  EXPECT_EQ(1, count);
  OpenVDS::LogTrace(logHandler, "hello world");
  EXPECT_EQ(1, count);

  logHandler.level = OpenVDS::LogLevel::Warning;
  OpenVDS::LogError(logHandler, "hello world");
  EXPECT_EQ(2, count);
  OpenVDS::LogWarning(logHandler, "hello world");
  EXPECT_EQ(3, count);
  OpenVDS::LogInfo(logHandler, "hello world");
  EXPECT_EQ(3, count);
  OpenVDS::LogTrace(logHandler, "hello world");
  EXPECT_EQ(3, count);

  logHandler.level = OpenVDS::LogLevel::Info;
  OpenVDS::LogError(logHandler, "hello world");
  EXPECT_EQ(4, count);
  OpenVDS::LogWarning(logHandler, "hello world");
  EXPECT_EQ(5, count);
  OpenVDS::LogInfo(logHandler, "hello world");
  EXPECT_EQ(6, count);
  OpenVDS::LogTrace(logHandler, "hello world");
  EXPECT_EQ(6, count);

  logHandler.level = OpenVDS::LogLevel::Trace;
  OpenVDS::LogError(logHandler, "hello world");
  EXPECT_EQ(7, count);
  OpenVDS::LogWarning(logHandler, "hello world");
  EXPECT_EQ(8, count);
  OpenVDS::LogInfo(logHandler, "hello world");
  EXPECT_EQ(9, count);
  OpenVDS::LogTrace(logHandler, "hello world");
  EXPECT_EQ(10, count);
}
