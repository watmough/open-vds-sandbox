/****************************************************************************
** Copyright 2019 The Open Group
** Copyright 2019 Bluware, Inc.
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

#include <OpenVDS/OpenVDS.h>
#include <OpenVDS/MetadataContainer.h>

#include <cstdlib>

#include <gtest/gtest.h>

GTEST_TEST(OpenVDS_integration, OpenClose)
{
  OpenVDS::Error error;
  std::string url = TEST_URL;
  std::string connectionString = TEST_CONNECTION;
  if(url.empty())
  {
    GTEST_SKIP() << "Test Environment for connecting to VDS is not set";
  }

  OpenVDS::ScopedVDSHandle handle(OpenVDS::Open(url, connectionString, error));
  ASSERT_TRUE(handle);
}

GTEST_TEST(OpenVDS_integration, OpenCloseVDSFile)
{
  OpenVDS::Error error;
  std::string fileName = TEST_DATA_PATH "/subset.vds";

  OpenVDS::ScopedVDSHandle handle(OpenVDS::Open(OpenVDS::VDSFileOpenOptions(fileName), error));
  ASSERT_TRUE(handle);
}

TEST(OpenVDS_integration, CreateAndModifyMetadata)
{
  const std::string
    category = "Answers",
    key = "LifeUniverseAndEverything";

  const int
    initialValue = 42,
    modifiedValue = 2022;

  int negativeMargin = 4;
  int positiveMargin = 4;
  int brickSize2DMultiplier = 4;
  auto LODLevels = OpenVDS::VolumeDataLayoutDescriptor::LODLevels_1;
  auto layoutOptions = OpenVDS::VolumeDataLayoutDescriptor::Options_None;
  OpenVDS::VolumeDataLayoutDescriptor layoutDescriptor(OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, negativeMargin, positiveMargin, brickSize2DMultiplier, LODLevels, layoutOptions);

  std::vector<OpenVDS::VolumeDataAxisDescriptor> axisDescriptors;
  axisDescriptors.emplace_back(100, "X", "", 1.0f, 100.f);
  axisDescriptors.emplace_back(100, "Y", "", 1.f,  100.f);
  axisDescriptors.emplace_back(100, "Z", "", 1.f,  100.f);

  std::vector<OpenVDS::VolumeDataChannelDescriptor> channelDescriptors;
  float rangeMin = 0.0f;
  float rangeMax = 1.0f;
  float integerScale = 0;
  float integerOffset = 0;
  channelDescriptors.push_back(OpenVDS::VolumeDataChannelDescriptor(OpenVDS::VolumeDataChannelDescriptor::Format_R32, OpenVDS::VolumeDataChannelDescriptor::Components_1, "", "", rangeMin, rangeMax, OpenVDS::VolumeDataMapping::Direct, 1, OpenVDS::VolumeDataChannelDescriptor::Default, 0.f, integerScale, integerOffset));

  OpenVDS::MetadataContainer metadataContainer;

  metadataContainer.SetMetadataInt(category.c_str(), key.c_str(), initialValue);

  {
    OpenVDS::Error error;
    OpenVDS::ScopedVDSHandle handle = OpenVDS::Create("inmemory://CreateAndModifyMetadata", "", layoutDescriptor, axisDescriptors, channelDescriptors, metadataContainer, error);

    auto accessManager = OpenVDS::GetAccessManager(handle);
    auto layout = accessManager.GetVolumeDataLayout();
    EXPECT_EQ(layout->GetMetadataInt(category.c_str(), key.c_str()), initialValue);

    auto metadataWriteAccess = OpenVDS::GetMetadataWriteAccessInterface(handle);
    metadataWriteAccess->SetMetadataInt(category.c_str(), key.c_str(), modifiedValue);

    EXPECT_EQ(layout->GetMetadataInt(category.c_str(), key.c_str()), modifiedValue);

    accessManager.Flush(error);
  }

  {
    OpenVDS::Error error;
    OpenVDS::ScopedVDSHandle handle = OpenVDS::Open("inmemory://CreateAndModifyMetadata", "", error);

    auto accessManager = OpenVDS::GetAccessManager(handle);
    auto layout = accessManager.GetVolumeDataLayout();
    EXPECT_EQ(layout->GetMetadataInt(category.c_str(), key.c_str()), modifiedValue);
  }
}
