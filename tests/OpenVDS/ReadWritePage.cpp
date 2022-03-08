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

#include <fmt/format.h>
#include <gtest/gtest.h>

#include "../utils/GenerateVDS.h"
#include "../utils/SlowIOManager.h"
#include "../utils/FacadeIOManager.h"

#include <OpenVDS/IO/IOManager.h>
#include <OpenVDS/IO/IOManagerInMemory.h>

enum WriteBufferAccessor
{
  Create,
  Read
};
template<typename T>
void writeBuffer(OpenVDS::VolumeDataPageAccessor* pageAccessor, WriteBufferAccessor accessor, int64_t chunk, T value)
{
  OpenVDS::VolumeDataPage* page = nullptr;
  if (accessor == Create)
    page = pageAccessor->CreatePage(chunk);
  else
    page = pageAccessor->ReadPage(chunk);

  int pitch[OpenVDS::Dimensionality_Max];
  T* buffer = static_cast<T *>(page->GetWritableBuffer(pitch));
  int min[OpenVDS::VolumeDataLayout::Dimensionality_Max];
  int max[OpenVDS::VolumeDataLayout::Dimensionality_Max];
  page->GetMinMax(min, max);
  for (int i = min[2]; i < max[2]; i++)
  {
    for (int j = min[1]; j < max[1]; j++)
    {
      for (int k = min[0]; k < max[0]; k++)
      {
        size_t offset = (i - min[2]) * pitch[2] + (j - min[1]) * pitch[1] + (k - min[0]) * pitch[0];
        buffer[offset] = value;
      }
    }
  }
  page->UpdateWrittenRegion(min, max);
  page->Release();
}

template<typename T>
bool compareBuffer(OpenVDS::VolumeDataPageAccessor* pageAccessor, int64_t chunk, T value)
{
  auto page = pageAccessor->ReadPage(chunk);
  int pitch[OpenVDS::Dimensionality_Max];
  const T* buffer = static_cast<const T *>(page->GetBuffer(pitch));
  int min[OpenVDS::VolumeDataLayout::Dimensionality_Max];
  int max[OpenVDS::VolumeDataLayout::Dimensionality_Max];
  page->GetMinMax(min, max);
  bool all_equal = true;
  for (int i = min[2]; i < max[2]; i++)
    for (int j = min[1]; j < max[1]; j++)
      for (int k = min[0]; k < max[0]; k++)
      {
        size_t offset = (i - min[2]) * pitch[2] + (j - min[1]) * pitch[1] + (k - min[0]) * pitch[0];
        if (buffer[offset] != value)
          all_equal = false;
      }
  page->Release();
  return all_equal;
}

TEST(OpenVDS_integration, WriteReadPage)
{
  OpenVDS::InMemoryOpenOptions options;
  OpenVDS::Error error;
  std::unique_ptr<OpenVDS::IOManager> inMemory(OpenVDS::IOManagerInMemory::CreateIOManager(options, OpenVDS::IOManager::AccessPattern::ReadWrite, error));
  auto ioCreate = new IOManagerFacadeLight(inMemory.get());
  {
    OpenVDS::ScopedVDSHandle handle(generateSimpleInMemory3DVDS(60, 60, 60, OpenVDS::VolumeDataChannelDescriptor::Format_R32, OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, ioCreate));
    fill3DVDSWithBitNoise(handle);
  }
  SlowIOManager* slowIOManager = new SlowIOManager(50, inMemory.get());
  OpenVDS::ScopedVDSHandle handle(OpenVDS::Open(slowIOManager, error));
  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);
  
  auto pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 1024, OpenVDS::VolumeDataAccessManager::AccessMode_ReadWrite);
  writeBuffer(pageAccessor, Read, 2, 1234.5f);
  pageAccessor->Commit();

  ASSERT_TRUE(compareBuffer(pageAccessor, 2, 1234.5f));
}

TEST(OpenVDS_integration, ReadWritePage)
{
  OpenVDS::InMemoryOpenOptions options;
  OpenVDS::Error error;
  std::unique_ptr<OpenVDS::IOManager> inMemory(OpenVDS::IOManagerInMemory::CreateIOManager(options, OpenVDS::IOManager::AccessPattern::ReadWrite, error));
  auto ioCreate = new IOManagerFacadeLight(inMemory.get());
  {
    OpenVDS::ScopedVDSHandle handle(generateSimpleInMemory3DVDS(60, 60, 60, OpenVDS::VolumeDataChannelDescriptor::Format_R32, OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, ioCreate));
    fill3DVDSWithBitNoise(handle);
  }
  SlowIOManager* slowIOManager = new SlowIOManager(50, inMemory.get());
  OpenVDS::ScopedVDSHandle handle(OpenVDS::Open(slowIOManager, error));
  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);

  auto pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 1024, OpenVDS::VolumeDataAccessManager::AccessMode_ReadWrite);

  {
    auto page = pageAccessor->ReadPage(2);
    int pitch[6];
    auto buffer = page->GetBuffer(pitch);
    (void)buffer;
    page->Release();
  }

  writeBuffer(pageAccessor, Read, 2, 1234.5f);
  pageAccessor->Commit();

  ASSERT_TRUE(compareBuffer(pageAccessor, 2, 1234.5f));
}

TEST(OpenVDS_integration, CopyPage)
{
  OpenVDS::InMemoryOpenOptions options;
  OpenVDS::Error error;
  std::unique_ptr<OpenVDS::IOManager> inMemory(OpenVDS::IOManagerInMemory::CreateIOManager(options, OpenVDS::IOManager::AccessPattern::ReadWrite, error));
  auto ioCreate = new IOManagerFacadeLight(inMemory.get());
  {
    OpenVDS::ScopedVDSHandle handle(generateSimpleInMemory3DVDS(60, 60, 60, OpenVDS::VolumeDataChannelDescriptor::Format_R32, OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, ioCreate));
    fill3DVDSWithBitNoise(handle);
  }
  SlowIOManager* slowIOManager = new SlowIOManager(50, inMemory.get());
  OpenVDS::ScopedVDSHandle handle(OpenVDS::Open(slowIOManager, error));
  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);

  auto pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 1024, OpenVDS::VolumeDataAccessManager::AccessMode_ReadWrite);

  OpenVDS::ScopedVDSHandle copyTo;

  {
    auto layout = OpenVDS::GetLayout(handle);
    int dimensionCount = layout->GetDimensionality();
    std::vector<OpenVDS::VolumeDataAxisDescriptor> axisDescriptors;
    axisDescriptors.reserve(dimensionCount);
    for (int i = 0; i < dimensionCount; i++)
    {
      axisDescriptors.emplace_back(layout->GetAxisDescriptor(i));
    }

    int channelCount = layout->GetChannelCount();
    std::vector<OpenVDS::VolumeDataChannelDescriptor> channelDescriptors;
    channelDescriptors.reserve(channelCount);
    for (int i = 0; i < channelCount; i++)
    {
      channelDescriptors.emplace_back(layout->GetChannelDescriptor(i));
    }
    OpenVDS::VolumeDataLayoutDescriptor layoutDescriptor = layout->GetLayoutDescriptor();
    auto compressionMethod = OpenVDS::GetCompressionMethod(handle);
    auto compressionTolerance = OpenVDS::GetCompressionTolerance(handle);
    copyTo = OpenVDS::Create("inmemory://copy_to", "", layoutDescriptor, axisDescriptors, channelDescriptors, *layout, compressionMethod, compressionTolerance, error);
  }

  auto copyToAccessManager = OpenVDS::GetAccessManager(copyTo);
  auto copyToPageAccessor = copyToAccessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 1024, OpenVDS::VolumeDataAccessManager::AccessMode_Create);

  {
    copyToPageAccessor->CopyPage(2, *pageAccessor);
    copyToPageAccessor->Commit();
  }

  {
    int pitch[6];

    auto sourcePage = pageAccessor->ReadPage(2);
    auto sourceBuffer = sourcePage->GetBuffer(pitch);
    auto sourceBufferSize = pitch[0] * pitch[1] * pitch[2] * 4;

    auto targetPage = copyToPageAccessor->ReadPage(2);
    auto targetBuffer = targetPage->GetBuffer(pitch);
    auto targetBufferSize = pitch[0] * pitch[1] * pitch[2] * 4;
    ASSERT_EQ(sourceBufferSize, targetBufferSize);
    ASSERT_TRUE(memcmp(sourceBuffer, targetBuffer, sourceBufferSize) == 0);
  }
}

TEST(OpenVDS_integration, ReadWriteVDSFile)
{
  const OpenVDS::VDSFileOpenOptions openOptions("readwritevdsfile.vds");
  remove(openOptions.fileName.c_str());

  OpenVDS::Error error;

  OpenVDS::VolumeDataLayoutDescriptor
    layoutDescriptor = OpenVDS::VolumeDataLayoutDescriptor(OpenVDS::VolumeDataLayoutDescriptor::BrickSize_64, 0, 0, 4,
      OpenVDS::VolumeDataLayoutDescriptor::LODLevels_None,
      OpenVDS::VolumeDataLayoutDescriptor::Options_None);

  std::vector<OpenVDS::VolumeDataAxisDescriptor>
    axisDescriptors = { OpenVDS::VolumeDataAxisDescriptor(100, "X", "m", 0.0, 2000.0),
                        OpenVDS::VolumeDataAxisDescriptor(100, "Y", "m", 0.0, 2000.0),
                        OpenVDS::VolumeDataAxisDescriptor(100, "Z", "m", 0.0, 2000.0) };

  std::vector<OpenVDS::VolumeDataChannelDescriptor>
    channelDescriptors = { OpenVDS::VolumeDataChannelDescriptor(OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataComponents::Components_1, "Value", "", 0.0, 300.0) };

  OpenVDS::MetadataContainer
    metadataContainer;

  OpenVDS::ScopedVDSHandle handle = OpenVDS::Create(openOptions, layoutDescriptor, axisDescriptors, channelDescriptors, metadataContainer, error);
  ASSERT_EQ(error.code, 0);

  auto accessManager = OpenVDS::GetAccessManager(handle);
  auto layout = accessManager.GetVolumeDataLayout();
  const int channel = 0;

  OpenVDS::VolumeDataPageAccessor* pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, channel, 0, 100, OpenVDS::VolumeDataAccessManager::AccessMode_Create);
  OpenVDS::VolumeDataFormat expectedFormat = OpenVDS::VolumeDataFormat::Format_R32;
  EXPECT_EQ(layout->GetChannelFormat(channel), expectedFormat);
  EXPECT_EQ(layout->GetChannelFormat(channel), pageAccessor->GetChannelDescriptor().GetFormat());

  int32_t chunkCount = int32_t(pageAccessor->GetChunkCount());

  for (int i = 0; i < chunkCount; i++)
  {
    OpenVDS::VolumeDataPage* page = pageAccessor->CreatePage(i);
    int pitch[OpenVDS::VolumeDataLayout::Dimensionality_Max];
    auto buffer = reinterpret_cast<float*>(page->GetWritableBuffer(pitch));
    int min[OpenVDS::VolumeDataLayout::Dimensionality_Max], max[OpenVDS::VolumeDataLayout::Dimensionality_Max];
    page->GetMinMax(min, max);
    for (int i = min[2]; i < max[2]; i++)
      for (int j = min[1]; j < max[1]; j++)
        for (int k = min[0]; k < max[0]; k++)
        {
          size_t offset = (i - min[2]) * pitch[2] + (j - min[1]) * pitch[1] + (k - min[0]) * pitch[0];
          buffer[offset] = float(i + j + k);
        }
    page->Release();
  }
  pageAccessor->Commit();
  accessManager.DestroyVolumeDataPageAccessor(pageAccessor);

  const char* object = "";
  int         errorCode = 0;
  const char* errorString = "";

  accessManager.GetCurrentUploadError(&object, &errorCode, &errorString);
  EXPECT_EQ(errorCode, 0) << "Error " << errorCode << " writing " << object << ": " << errorString;

  handle.Close(error);
  EXPECT_EQ(error.code, 0);

  // Modify dataset
  handle = OpenVDS::Open(openOptions, error);
  ASSERT_EQ(error.code, 0);
  accessManager = OpenVDS::GetAccessManager(handle);
  layout = accessManager.GetVolumeDataLayout();

  {
    auto accessor = accessManager.CreateVolumeData3DReadWriteAccessorR32(OpenVDS::DimensionsND::Dimensions_012, 0, channel, 100);
    for (int i = 0; i < 16; i++)
      for (int j = 0; j < 16; j++)
        for (int k = 0; k < 16; k++)
        {
          accessor.SetValue(OpenVDS::IntVector3(i, j, k), 13);
        }
  }

  accessManager.GetCurrentUploadError(&object, &errorCode, &errorString);
  EXPECT_EQ(errorCode, 0) << "Error " << errorCode << " writing " << object << ": " << errorString;

  // Close VDS and remove file
  handle.Close(error);
  EXPECT_EQ(error.code, 0);
  remove(openOptions.fileName.c_str());
}

TEST(OpenVDS_integration, WriteReadMultiPageAccessorPage)
{
  OpenVDS::InMemoryOpenOptions options;
  OpenVDS::Error error;
  std::unique_ptr<OpenVDS::IOManager> inMemory(OpenVDS::IOManagerInMemory::CreateIOManager(options, OpenVDS::IOManager::AccessPattern::ReadWrite, error));
  auto ioCreate = new IOManagerFacadeLight(inMemory.get());
  {
    OpenVDS::ScopedVDSHandle handle(generateSimpleInMemory3DVDS(60, 60, 60, OpenVDS::VolumeDataChannelDescriptor::Format_R32, OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, ioCreate));
    fill3DVDSWithBitNoise(handle);
  }
  SlowIOManager* slowIOManager = new SlowIOManager(150, inMemory.get(), SlowIOManager::Write);
  OpenVDS::ScopedVDSHandle handle(OpenVDS::Open(slowIOManager, error));
  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);
  
  auto writePageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 1024, OpenVDS::VolumeDataAccessManager::AccessMode_ReadWrite);
  writeBuffer(writePageAccessor, Read, 2, 1234.5f);
  writePageAccessor->SetMaxPages(0);
  
  auto readPageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 1024, OpenVDS::VolumeDataAccessManager::AccessMode_ReadOnly);
  ASSERT_TRUE(compareBuffer(readPageAccessor, 2, 1234.5f));

}

TEST(OpenVDS_integration, ReadWriteReadMultiPageAccessorPage)
{
  OpenVDS::InMemoryOpenOptions options;
  OpenVDS::Error error;
  std::unique_ptr<OpenVDS::IOManager> inMemory(OpenVDS::IOManagerInMemory::CreateIOManager(options, OpenVDS::IOManager::AccessPattern::ReadWrite, error));
  auto ioCreate = new IOManagerFacadeLight(inMemory.get());
  {
    OpenVDS::ScopedVDSHandle handle(generateSimpleInMemory3DVDS(60, 60, 60, OpenVDS::VolumeDataChannelDescriptor::Format_R32, OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, ioCreate));
    fill3DVDSWithBitNoise(handle);
  }
  SlowIOManager* slowIOManager = new SlowIOManager(150, inMemory.get(), SlowIOManager::Read);
  OpenVDS::ScopedVDSHandle handle(OpenVDS::Open(slowIOManager, error));
  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);

  auto readPageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 1024, OpenVDS::VolumeDataAccessManager::AccessMode_ReadOnly);
  auto writePageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 1024, OpenVDS::VolumeDataAccessManager::AccessMode_ReadWrite);
  {
    std::thread threadRead([&readPageAccessor]
      {
        auto page = readPageAccessor->ReadPage(2);
        int pitch[6];
        auto buffer = page->GetBuffer(pitch);
        (void)buffer;
        page->Release();

      });
  
    std::thread threadWrite([&writePageAccessor]
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        writeBuffer(writePageAccessor, Create, 2, 1234.5f);
        writePageAccessor->SetMaxPages(0);
      });
    threadRead.join();
    threadWrite.join();
  }

  
  auto readPageAccessor2 = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 1024, OpenVDS::VolumeDataAccessManager::AccessMode_ReadOnly);
  ASSERT_TRUE(compareBuffer(readPageAccessor2, 2, 1234.5f));

}

TEST(OpenVDS_integration, CreateMultiplePageAccessors)
{
  OpenVDS::Error error;
  int negativeMargin = 4;
  int positiveMargin = 4;
  int brickSize2DMultiplier = 4;
  auto lodLevels = OpenVDS::VolumeDataLayoutDescriptor::LODLevels_None;
  auto layoutOptions = OpenVDS::VolumeDataLayoutDescriptor::Options_None;
  OpenVDS::VolumeDataLayoutDescriptor layoutDescriptor(OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, negativeMargin, positiveMargin, brickSize2DMultiplier, lodLevels, layoutOptions);

  std::vector<OpenVDS::VolumeDataAxisDescriptor> axisDescriptors;
  axisDescriptors.emplace_back(63, KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_SAMPLE, "ms", 0.0f, 4.f);
  axisDescriptors.emplace_back(32, KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_CROSSLINE, "", 1932.f, 2536.f);
  axisDescriptors.emplace_back(32, KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_INLINE,    "", 9985.f, 10369.f);

  std::vector<OpenVDS::VolumeDataChannelDescriptor> channelDescriptors;
  float rangeMin = -0.1234f;
  float rangeMax = 0.1234f;
  float intScale;
  float intOffset;
  getScaleOffsetForFormat(rangeMin, rangeMax, true, OpenVDS::VolumeDataChannelDescriptor::Format_U32, intScale, intOffset);
  channelDescriptors.push_back(OpenVDS::VolumeDataChannelDescriptor(OpenVDS::VolumeDataChannelDescriptor::Format_U32, OpenVDS::VolumeDataChannelDescriptor::Components_1, AMPLITUDE_ATTRIBUTE_NAME, "", rangeMin, rangeMax, OpenVDS::VolumeDataMapping::Direct, 1, OpenVDS::VolumeDataChannelDescriptor::Default, 0.f, intScale, intOffset));

  OpenVDS::MetadataContainer metadataContainer;

  std::string in_memory_name = fmt::format("inmemory://{}", "CreateMultiplePageAccessors");
  auto handle = OpenVDS::Create(in_memory_name, std::string(""), layoutDescriptor, axisDescriptors, channelDescriptors, metadataContainer, error);
  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);
  
  {
    OpenVDS::VolumeDataPageAccessor* pageAccessor0 = accessManager.CreateVolumeDataPageAccessor(
      OpenVDS::Dimensions_012, 0, 0, 100, OpenVDS::VolumeDataAccessManager::AccessMode_Create);
    accessManager.DestroyVolumeDataPageAccessor(pageAccessor0);
  }

  {
    OpenVDS::VolumeDataPageAccessor* pageAccessor1 = accessManager.CreateVolumeDataPageAccessor(
      OpenVDS::Dimensions_012, 0, 0, 100, OpenVDS::VolumeDataAccessManager::AccessMode_Create);
    accessManager.DestroyVolumeDataPageAccessor(pageAccessor1);
  }

  OpenVDS::Close(handle);
}

TEST(OpenVDS_integration, CreateMultiplePageAccessorsAndWriteData)
{
  OpenVDS::Error error;
  int negativeMargin = 4;
  int positiveMargin = 4;
  int brickSize2DMultiplier = 4;
  auto lodLevels = OpenVDS::VolumeDataLayoutDescriptor::LODLevels_None;
  auto layoutOptions = OpenVDS::VolumeDataLayoutDescriptor::Options_None;
  OpenVDS::VolumeDataLayoutDescriptor layoutDescriptor(OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, negativeMargin, positiveMargin, brickSize2DMultiplier, lodLevels, layoutOptions);

  std::vector<OpenVDS::VolumeDataAxisDescriptor> axisDescriptors;
  axisDescriptors.emplace_back(63, KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_SAMPLE, "ms", 0.0f, 4.f);
  axisDescriptors.emplace_back(32, KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_CROSSLINE, "", 1932.f, 2536.f);
  axisDescriptors.emplace_back(32, KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_INLINE,    "", 9985.f, 10369.f);

  std::vector<OpenVDS::VolumeDataChannelDescriptor> channelDescriptors;
  float rangeMin = -0.1234f;
  float rangeMax = 0.1234f;
  float intScale;
  float intOffset;
  getScaleOffsetForFormat(rangeMin, rangeMax, true, OpenVDS::VolumeDataChannelDescriptor::Format_U32, intScale, intOffset);
  channelDescriptors.push_back(OpenVDS::VolumeDataChannelDescriptor(OpenVDS::VolumeDataChannelDescriptor::Format_U32, OpenVDS::VolumeDataChannelDescriptor::Components_1, AMPLITUDE_ATTRIBUTE_NAME, "", rangeMin, rangeMax, OpenVDS::VolumeDataMapping::Direct, 1, OpenVDS::VolumeDataChannelDescriptor::Default, 0.f, intScale, intOffset));

  OpenVDS::MetadataContainer metadataContainer;

  std::string in_memory_name = fmt::format("inmemory://{}", "CreateMultiplePageAccessorsAndWriteData");
  auto handle = OpenVDS::Create(in_memory_name, std::string(""), layoutDescriptor, axisDescriptors, channelDescriptors, metadataContainer, error);
  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);
  
  OpenVDS::VolumeDataPageAccessor* pageAccessor = accessManager.CreateVolumeDataPageAccessor(
    OpenVDS::Dimensions_012, 0, 0, 100, OpenVDS::VolumeDataAccessManager::AccessMode_Create);

  OpenVDS::VolumeDataPageAccessor* pageAccessor2 = accessManager.CreateVolumeDataPageAccessor(
    OpenVDS::Dimensions_012, 0, 0, 100, OpenVDS::VolumeDataAccessManager::AccessMode_Create);

  OpenVDS::VolumeDataPage *page =  pageAccessor->CreatePage(0);
  int pitch[OpenVDS::VolumeDataLayout::Dimensionality_Max];
  auto buffer = reinterpret_cast<float *>(page->GetWritableBuffer(pitch));
  int min[OpenVDS::VolumeDataLayout::Dimensionality_Max], max[OpenVDS::VolumeDataLayout::Dimensionality_Max];
  page->GetMinMax(min, max);
  for(int i = min[2]; i < max[2]; i++)
  for(int j = min[1]; j < max[1]; j++)
  for(int k = min[0]; k < max[0]; k++)
  {
    size_t offset = (i - min[2]) * pitch[2] + (j - min[1]) * pitch[1] + (k - min[0]) * pitch[0];
    buffer[offset] = float(i + j + k);
  }
  page->Release();

  EXPECT_THROW(page =  pageAccessor2->ReadPage(0), OpenVDS::InvalidOperation);
  pageAccessor->Commit();
  EXPECT_NO_THROW(page =  pageAccessor2->ReadPage(0));
  page->Release();

  accessManager.DestroyVolumeDataPageAccessor(pageAccessor);
  accessManager.DestroyVolumeDataPageAccessor(pageAccessor2);

  OpenVDS::Close(handle);
}
