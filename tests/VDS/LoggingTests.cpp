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
  int count = 0;
  OpenVDS::LogHandler logHandler;
  logHandler.userHandle = &count;
  logHandler.callback = [](OpenVDS::LogLevel level, const char* message, size_t messageSize, void* userHandle)
  {
    auto local_count = static_cast<int*>(userHandle);
    (*local_count)++;
  };
  OpenVDS::Logger logger(OpenVDS::LogLevel::None, logHandler);

  logger.level = OpenVDS::LogLevel::None;
  logger.LogError("hello world");
  EXPECT_EQ(0, count);
  logger.LogWarning("hello world");
  EXPECT_EQ(0, count);
  logger.LogInfo("hello world");
  EXPECT_EQ(0, count);
  logger.LogTrace("hello world");
  EXPECT_EQ(0, count);

  logger.level = OpenVDS::LogLevel::Error;
  logger.LogError("hello world");
  EXPECT_EQ(1, count);
  logger.LogWarning("hello world");
  EXPECT_EQ(1, count);
  logger.LogInfo("hello world");
  EXPECT_EQ(1, count);
  logger.LogTrace("hello world");
  EXPECT_EQ(1, count);

  logger.level = OpenVDS::LogLevel::Warning;
  logger.LogError("hello world");
  EXPECT_EQ(2, count);
  logger.LogWarning("hello world");
  EXPECT_EQ(3, count);
  logger.LogInfo("hello world");
  EXPECT_EQ(3, count);
  logger.LogTrace("hello world");
  EXPECT_EQ(3, count);

  logger.level = OpenVDS::LogLevel::Info;
  logger.LogError("hello world");
  EXPECT_EQ(4, count);
  logger.LogWarning("hello world");
  EXPECT_EQ(5, count);
  logger.LogInfo("hello world");
  EXPECT_EQ(6, count);
  logger.LogTrace("hello world");
  EXPECT_EQ(6, count);

  logger.level = OpenVDS::LogLevel::Trace;
  logger.LogError("hello world");
  EXPECT_EQ(7, count);
  logger.LogWarning("hello world");
  EXPECT_EQ(8, count);
  logger.LogInfo("hello world");
  EXPECT_EQ(9, count);
  logger.LogTrace("hello world");
  EXPECT_EQ(10, count);
}
