/****************************************************************************
** Copyright 2021 The Open Group
** Copyright 2021 Bluware, Inc.
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
#include <OpenVDS/VolumeDataLayout.h>
#include <OpenVDS/VolumeDataAccess.h>
#include <OpenVDS/ValueConversion.h>
#include <OpenVDS/GlobalMetadataCommon.h>
#include <OpenVDS/MetadataContainer.h>

#include <fmt/format.h>
#include <gtest/gtest.h>

TEST(OpenVDS_integration, MultiComponent)
{
  int32_t samplesX = 100;
  int32_t samplesY = 100;
  int32_t samplesZ = 100;
  auto format = OpenVDS::VolumeDataFormat::Format_R32;
  auto brickSize = OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32;
  int negativeMargin = 4;
  int positiveMargin = 4;
  int brickSize2DMultiplier = 4;
  auto lodLevels = OpenVDS::VolumeDataLayoutDescriptor::LODLevels_None;
  auto layoutOptions = OpenVDS::VolumeDataLayoutDescriptor::Options_None;
  OpenVDS::VolumeDataLayoutDescriptor layoutDescriptor(brickSize, negativeMargin, positiveMargin, brickSize2DMultiplier, lodLevels, layoutOptions);

  std::vector<OpenVDS::VolumeDataAxisDescriptor> axisDescriptors;
  axisDescriptors.emplace_back(samplesX, KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_SAMPLE, "ms", 0.0f, 4.f);
  axisDescriptors.emplace_back(samplesY, KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_CROSSLINE, "", 1932.f, 2536.f);
  axisDescriptors.emplace_back(samplesZ, KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_INLINE,    "", 9985.f, 10369.f);

  std::vector<OpenVDS::VolumeDataChannelDescriptor> channelDescriptors;
  float rangeMin = -0.1234f;
  float rangeMax = 0.1234f;
  float intScale = 1.0f;
  float intOffset = 0.0f;
  channelDescriptors.emplace_back(format, OpenVDS::VolumeDataComponents::Components_2, AMPLITUDE_ATTRIBUTE_NAME, "", rangeMin, rangeMax, OpenVDS::VolumeDataMapping::Direct, 1, OpenVDS::VolumeDataChannelDescriptor::Default, 0.f, intScale, intOffset);

  OpenVDS::MetadataContainer metadataContainer;
  OpenVDS::Error error;
  auto handle = OpenVDS::Create("inmemory://MultiComponent", "", layoutDescriptor, axisDescriptors, channelDescriptors, metadataContainer, error);
  ASSERT_TRUE(handle);
  {
    OpenVDS::VolumeDataLayout* layout = OpenVDS::GetLayout(handle);
    ASSERT_TRUE(layout);
    OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);

    OpenVDS::VolumeDataPageAccessor* pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 100, OpenVDS::VolumeDataAccessManager::AccessMode_Create);
    //ASSERT_TRUE(pageAccessor);

    int32_t chunkCount = int32_t(pageAccessor->GetChunkCount());

    OpenVDS::VolumeDataChannelDescriptor::Format outformat = layout->GetChannelFormat(0);
    ASSERT_EQ(format, outformat);

    for (int i = 0; i < chunkCount; i++)
    {
      OpenVDS::VolumeDataPage* page = pageAccessor->CreatePage(i);
      int pitch[OpenVDS::Dimensionality_Max];
      void* buffer = page->GetWritableBuffer(pitch);
      (void) buffer;
      page->Release();
    }
    pageAccessor->Commit();
    accessManager.Flush(error);
    ASSERT_EQ(error.code, 0);
    accessManager.DestroyVolumeDataPageAccessor(pageAccessor);
  }
  OpenVDS::Close(handle);
  handle = OpenVDS::Open("inmemory://MultiComponent", "", error);
  auto requestManager = OpenVDS::GetAccessManager(handle);
  int min[] = {0,0,0,0,0,0};
  int max[] = {100,100,100, 0, 0, 0};
  auto request = requestManager.RequestVolumeSubset(OpenVDS::Dimensions_012, 0, 0, min, max, format); 
  request->WaitForCompletion();
  
  ASSERT_TRUE(handle);
}
