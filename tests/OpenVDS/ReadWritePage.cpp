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

  auto exception = page->GetError();
  if(exception.GetErrorCode()) throw exception;

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

  auto exception = page->GetError();
  if(exception.GetErrorCode()) throw exception;

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
  
  auto readWritePageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 1024, OpenVDS::VolumeDataAccessManager::AccessMode_ReadWrite);
  writeBuffer(readWritePageAccessor, Read, 2, 1234.5f);
  readWritePageAccessor->SetMaxPages(0); // Make sure the buffer is flushed to the write queue, without committing

  ASSERT_TRUE(compareBuffer(readWritePageAccessor, 2, 1234.5f));
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
  auto readWritePageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 1024, OpenVDS::VolumeDataAccessManager::AccessMode_ReadWrite);
  {
    std::thread threadRead([&readPageAccessor]
      {
        auto page = readPageAccessor->ReadPage(2);
        int pitch[6];
        auto buffer = page->GetBuffer(pitch);
        (void)buffer;
        page->Release();

      });
  
    std::thread threadWrite([&readWritePageAccessor]
      {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        writeBuffer(readWritePageAccessor, Create, 2, 1234.5f);
        readWritePageAccessor->SetMaxPages(0); // Make sure the buffer is flushed to the write queue, without committing
      });
    threadRead.join();
    threadWrite.join();
  }

  ASSERT_TRUE(compareBuffer(readWritePageAccessor, 2, 1234.5f));
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

TEST(OpenVDS_integration, GenerateLOD)
{
  OpenVDS::Error error;
  int negativeMargin = 4;
  int positiveMargin = 4;
  int brickSize2DMultiplier = 4;
  auto lodLevels = OpenVDS::VolumeDataLayoutDescriptor::LODLevels_1;
  auto layoutOptions = OpenVDS::VolumeDataLayoutDescriptor::Options_None;
  OpenVDS::VolumeDataLayoutDescriptor layoutDescriptor(OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, negativeMargin, positiveMargin, brickSize2DMultiplier, lodLevels, layoutOptions);

  std::vector<OpenVDS::VolumeDataAxisDescriptor> axisDescriptors;
  axisDescriptors.emplace_back(100, "X", "", 1.0f, 100.f);
  axisDescriptors.emplace_back(100, "Y", "", 1.f,  100.f);
  axisDescriptors.emplace_back(100, "Z", "", 1.f,  100.f);

  std::vector<OpenVDS::VolumeDataChannelDescriptor> channelDescriptors;
  float rangeMin = 0.0f;
  float rangeMax = 1.0f;
  float integerScale = 0;
  float integerOffset = 0;
  channelDescriptors.push_back(OpenVDS::VolumeDataChannelDescriptor(OpenVDS::VolumeDataChannelDescriptor::Format_R32, OpenVDS::VolumeDataChannelDescriptor::Components_1, AMPLITUDE_ATTRIBUTE_NAME, "", rangeMin, rangeMax, OpenVDS::VolumeDataMapping::Direct, 1, OpenVDS::VolumeDataChannelDescriptor::Default, 0.f, integerScale, integerOffset));

  OpenVDS::MetadataContainer metadataContainer;

  std::string in_memory_name = fmt::format("inmemory://{}", "GenerateLOD");
  OpenVDS::ScopedVDSHandle handle(OpenVDS::Create(in_memory_name, std::string(""), layoutDescriptor, axisDescriptors, channelDescriptors, metadataContainer, error));
  fill3DVDSWithNoise(handle, 0, OpenVDS::FloatVector3(0.6f, 2.f, 4.f), true);

  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);

  auto accessor = accessManager.CreateVolumeData3DReadAccessorR32(OpenVDS::Dimensions_012, 0, 0);

  {
    const int LOD = 1;

    ASSERT_EQ(accessManager.GetVDSProduceStatus(OpenVDS::Dimensions_012, LOD, 0), OpenVDS::VDSProduceStatus::Normal);

    OpenVDS::VolumeDataPageAccessor* pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, LOD, 0, 100, OpenVDS::VolumeDataAccessManager::AccessMode_ReadOnly);

    for(int64_t chunk = 0, chunkCount = pageAccessor->GetChunkCount(); chunk < chunkCount; chunk++)
    {
      auto page = pageAccessor->ReadPage(chunk);

      int size[OpenVDS::Dimensionality_Max];
      int pitch[OpenVDS::Dimensionality_Max];
      int min[OpenVDS::Dimensionality_Max];
      int max[OpenVDS::Dimensionality_Max];
      page->GetMinMax(min, max);    
      const float* buffer = static_cast<const float *>(page->GetBuffer(size, pitch));
      for(int i = 0; i < size[2]; i++)
      for(int j = 0; j < size[1]; j++)
      for(int k = 0; k < size[0]; k++)
      {
        float valueLOD1 = buffer[i * pitch[2] + j * pitch[1] + k];
        float valueLOD0 = accessor.GetValue(OpenVDS::IntVector3((i << LOD) + min[2], (j << LOD) + min[1], (k << LOD) + min[0]));      
        if(valueLOD0 != valueLOD1)
        {
          EXPECT_EQ(valueLOD0, valueLOD1);
          break;
        }
      }

      (void)buffer;
      page->Release();
    }

    accessManager.DestroyVolumeDataPageAccessor(pageAccessor);
  }
}

TEST(OpenVDS_integration, RemapLOD)
{
  OpenVDS::Error error;
  int negativeMargin = 4;
  int positiveMargin = 4;
  int brickSize2DMultiplier = 4;
  auto lodLevels = OpenVDS::VolumeDataLayoutDescriptor::LODLevels_1;
  auto layoutOptions = OpenVDS::VolumeDataLayoutDescriptor::Options_None;
  OpenVDS::VolumeDataLayoutDescriptor layoutDescriptor(OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, negativeMargin, positiveMargin, brickSize2DMultiplier, lodLevels, layoutOptions);

  std::vector<OpenVDS::VolumeDataAxisDescriptor> axisDescriptors;
  axisDescriptors.emplace_back(100, "X", "", 1.0f, 100.f);
  axisDescriptors.emplace_back(100, "Y", "", 1.f,  100.f);
  axisDescriptors.emplace_back(100, "Z", "", 1.f,  100.f);

  std::vector<OpenVDS::VolumeDataChannelDescriptor> channelDescriptors;
  float rangeMin = 0.0f;
  float rangeMax = 1.0f;
  float integerScale = 0;
  float integerOffset = 0;
  channelDescriptors.push_back(OpenVDS::VolumeDataChannelDescriptor(OpenVDS::VolumeDataChannelDescriptor::Format_R32, OpenVDS::VolumeDataChannelDescriptor::Components_1, AMPLITUDE_ATTRIBUTE_NAME, "", rangeMin, rangeMax, OpenVDS::VolumeDataMapping::Direct, 1, OpenVDS::VolumeDataChannelDescriptor::Default, 0.f, integerScale, integerOffset));

  OpenVDS::MetadataContainer metadataContainer;

  std::string in_memory_name = fmt::format("inmemory://{}", "RemapLOD");
  OpenVDS::ScopedVDSHandle handle(OpenVDS::Create(in_memory_name, std::string(""), layoutDescriptor, axisDescriptors, channelDescriptors, metadataContainer, error));
  fill3DVDSWithNoise(handle, 0, OpenVDS::FloatVector3(0.6f, 2.f, 4.f), false);

  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);

  auto accessor = accessManager.CreateVolumeData3DReadAccessorR32(OpenVDS::Dimensions_012, 0, 0);

  {
    const int LOD = 1;

    ASSERT_EQ(accessManager.GetVDSProduceStatus(OpenVDS::Dimensions_012, LOD, 0), OpenVDS::VDSProduceStatus::Remapped);

    OpenVDS::VolumeDataPageAccessor* pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, LOD, 0, 100, OpenVDS::VolumeDataAccessManager::AccessMode_ReadOnly);

    for(int64_t chunk = 0, chunkCount = pageAccessor->GetChunkCount(); chunk < chunkCount; chunk++)
    {
      auto page = pageAccessor->ReadPage(chunk);
      
      int size[OpenVDS::Dimensionality_Max];
      int pitch[OpenVDS::Dimensionality_Max];
      int min[OpenVDS::Dimensionality_Max];
      int max[OpenVDS::Dimensionality_Max];
      page->GetMinMax(min, max);    
      const float* buffer = static_cast<const float *>(page->GetBuffer(size, pitch));
      for(int i = 0; i < size[2]; i++)
      for(int j = 0; j < size[1]; j++)
      for(int k = 0; k < size[0]; k++)
      {
        float valueLOD1 = buffer[i * pitch[2] + j * pitch[1] + k];
        float valueLOD0 = accessor.GetValue(OpenVDS::IntVector3((i << LOD) + min[2], (j << LOD) + min[1], (k << LOD) + min[0]));      
        if(valueLOD0 != valueLOD1)
        {
          EXPECT_EQ(valueLOD0, valueLOD1);
          break;
        }
      }

      (void)buffer;
      page->Release();
    }

    accessManager.DestroyVolumeDataPageAccessor(pageAccessor);
  }
}

/*
class VolumeDataAccessorLODTestPlugin : public Hue::HueSpaceLib::VDSFilterPlugin
{
private:
  int m_fullResolutionDimension;

public:
  static Hue::HueSpaceLib::PluginParameterDescriptor
  DescribePluginParameter(int parameter)
  {
    static Hue::HueSpaceLib::PluginParameterDescriptor
      descriptors[] = {
        Hue::HueSpaceLib::PluginParameterDescriptor::PluginParameterInt("SizeT", "Size T", "Size in T dimension (dimension 3)", 10),
        Hue::HueSpaceLib::PluginParameterDescriptor::PluginParameterInt("SizeI", "Size I", "Size in I dimension (dimension 2)", 10),
        Hue::HueSpaceLib::PluginParameterDescriptor::PluginParameterInt("SizeJ", "Size J", "Size in J dimension (dimension 1)", 10),
        Hue::HueSpaceLib::PluginParameterDescriptor::PluginParameterInt("SizeK", "Size K", "Size in K dimension (dimension 0)", 10),
        Hue::HueSpaceLib::PluginParameterDescriptor::PluginParameterInt("Dimensionality", "Dimensionality", "Number of dimensions", 4),
        Hue::HueSpaceLib::PluginParameterDescriptor::PluginParameterInt("FullResolutionDimension", "Full-resolution dimension", "Full-resolution dimension, if any", -1),
        Hue::HueSpaceLib::PluginParameterDescriptor::PluginParameterEnd()
      };

    return descriptors[parameter];
  }

  bool
  CanProduceLOD(int lod) override
  {
    return true;
  }

  int
  GetFullResolutionDimension() override
  {
    return m_fullResolutionDimension;
  }

  Hue::HueSpaceLib::InputChannelDescriptor
  DescribeInputChannel(int channel) override
  {
    return Hue::HueSpaceLib::InputChannelDescriptor::InputChannelEnd();
  }

  Hue::HueSpaceLib::IndexingDescriptor
  DescribeInputIndexingMethod(int inputVDS, int dimension) override
  {
    return IndexingDescriptorEnd();
  }

  int
  GetOutputDimensionality(Hue::HueSpaceLib::AccessVDSPluginParameterInterface &parameterInterface) override
  {
    return std::min(std::max(parameterInterface.GetValueInt("Dimensionality"), 2), 4);
  }

  Hue::HueSpaceLib::VolumeDataAxisDescriptor
  DescribeOutputAxis(int dimension, Hue::HueSpaceLib::AccessVDSPluginParameterInterface &parameterInterface) override
  {
    switch (dimension)
    {
    case 0: return Hue::HueSpaceLib::VolumeDataAxisDescriptor(parameterInterface.GetValueInt("SizeK"), "K", "", 1.0, float(parameterInterface.GetValueInt("SizeK")));
    case 1: return Hue::HueSpaceLib::VolumeDataAxisDescriptor(parameterInterface.GetValueInt("SizeJ"), "J", "", 1.0, float(parameterInterface.GetValueInt("SizeJ")));
    case 2: return Hue::HueSpaceLib::VolumeDataAxisDescriptor(parameterInterface.GetValueInt("SizeI"), "I", "", 1.0, float(parameterInterface.GetValueInt("SizeI")));
    case 3: return Hue::HueSpaceLib::VolumeDataAxisDescriptor(parameterInterface.GetValueInt("SizeT"), "T", "", 1.0, float(parameterInterface.GetValueInt("SizeT")));
    }

    return Hue::HueSpaceLib::VolumeDataAxisDescriptor();
  }

  int
  GetOutputChannelCount(Hue::HueSpaceLib::AccessVDSPluginParameterInterface &parameterInterface) override
  {
    return 1;
  }

  Hue::HueSpaceLib::ProcessorPreference
  GetOutputChannelProcessorPreference(Hue::HueSpaceLib::AccessVDSPluginParameterInterface &parameterInterface, int channel)
  {
    return Hue::HueSpaceLib::ProcessorPreferenceCPU;
  }

  static int
  VoxelValue(intVec<4> voxel)
  {
    return voxel[0] + 37 * (voxel[1] + 37 * (voxel[2] + 37 * voxel[3]));
  }

  Hue::HueSpaceLib::VolumeDataChannelDescriptor
  DescribeOutputValue(Hue::HueSpaceLib::AccessVDSPluginParameterInterface &parameterInterface, int channel) override
  {
    int
      dimensionality = GetOutputDimensionality(parameterInterface);

    intVec<4>
      voxelIndex(parameterInterface.GetValueInt("SizeK") - 1,
                 parameterInterface.GetValueInt("SizeJ") - 1,
                 dimensionality >= 3 ? parameterInterface.GetValueInt("SizeI") - 1 : 0,
                 dimensionality >= 4 ? parameterInterface.GetValueInt("SizeT") - 1 : 0);

    return Hue::HueSpaceLib::VolumeDataChannelDescriptor(Hue::HueSpaceLib::DataBlock::Format_R32, Hue::HueSpaceLib::DataBlock::Components_1, "Value", "", 0.0f, (float)VoxelValue(voxelIndex));
  }

  void
  OnPluginParametersCreated(Hue::HueSpaceLib::AccessVDSPluginParameterInterface &parameterInterface) override
  {
    m_fullResolutionDimension = parameterInterface.GetValueInt("FullResolutionDimension");
  }

  /// Called when host application has changed one or more of the plugin parameters
  /// Member variables of the plugin should be set here, since this function will be called for each instance of the plugin.
  /// \param parameterInterface The parameter interface for accessing plugin parameters and input metadata.
  /// \return The action to take after parameters have changed
  ParametersChangedAction
  OnPluginParametersChanged(Hue::HueSpaceLib::AccessVDSPluginParameterInterface &parameterInterface) override
  {
    OnPluginParametersCreated(parameterInterface);
    return Continue_Processing;
  }

  void
  ProcessFilter(Hue::HueSpaceLib::VolumeDataCacheItem **, int, const Hue::HueSpaceLib::VolumeDataCacheItem **, int, Context *context) override
  {
    Hue::HueSpaceLib::VolumeDataCacheItem
      *outputItem = context->GetOutputVolumeDataCacheItem(0);

    Hue::HueSpaceLib::DataBlock
      *valuesDataBlock  = outputItem->CreateDataBlock();

    VolumeIndexer4D
      valuesIndexer(outputItem, valuesDataBlock);

    float
      *values = valuesDataBlock->GetBufferR32CPU();

    int
      samples[4] = { valuesIndexer.GetDataBlockNumSamples(0),
                     valuesIndexer.GetDataBlockNumSamples(1),
                     valuesIndexer.GetDataBlockNumSamples(2),
                     valuesIndexer.GetDataBlockNumSamples(3) };

    for (int dim3 = 0; dim3 < samples[3]; ++dim3)
    for (int dim2 = 0; dim2 < samples[2]; ++dim2)
    for (int dim1 = 0; dim1 < samples[1]; ++dim1)
    for (int dim0 = 0; dim0 < samples[0]; ++dim0)
    {
      intVec<4>
        localIndex(dim0, dim1, dim2, dim3),
        voxelIndex = valuesIndexer.LocalIndexToVoxelIndex(localIndex);

      values[valuesIndexer.LocalIndexToDataIndex(localIndex)] = (float)VoxelValue(voxelIndex);
    }
  }
};

Hue::HueSpaceLib::PluginFactory &
GetVolumeDataAccessorLODTestPluginFactoryInstance()
{
  static Hue::HueSpaceLib::PluginFactoryTemplate<VolumeDataAccessorLODTestPlugin>
    instance("HueVolumeDataAccessorLODTestPlugin", "VolumeDataAccessorLODTestPlugin", "VolumeDataAccessorLODTestPlugin");

  return instance;
}
*/

void FillPages(OpenVDS::VolumeDataPageAccessor *pageAccessor, std::function<float(OpenVDS::IntVector4)> voxelValueFunction)
{
  int chunkCount = int(pageAccessor->GetChunkCount());
  auto channelDescriptor = pageAccessor->GetChannelDescriptor();
  if(channelDescriptor.GetFormat() != OpenVDS::VolumeDataFormat::Format_R32) throw std::runtime_error("FillVDS can only create R32 data for now");

  for (int i = 0; i < chunkCount; i++)
  {
    OpenVDS::VolumeDataPage *page =  pageAccessor->CreatePage(i);

    int min[OpenVDS::VolumeDataLayout::Dimensionality_Max];
    int max[OpenVDS::VolumeDataLayout::Dimensionality_Max];
    page->GetMinMax(min, max);

    int LOD = pageAccessor->GetLOD();

    int size[OpenVDS::Dimensionality_Max];
    int pitch[OpenVDS::Dimensionality_Max];
    void *buffer = page->GetWritableBuffer(size, pitch);

    auto layoutDescriptor = pageAccessor->GetLayout()->GetLayoutDescriptor();
    int fullResolutionDimension = layoutDescriptor.IsForceFullResolutionDimension() ? layoutDescriptor.GetFullResolutionDimension() : -1;
    int dimensionLOD[4] = { fullResolutionDimension != 0 ? LOD : 0,
                            fullResolutionDimension != 1 ? LOD : 0,
                            fullResolutionDimension != 2 ? LOD : 0,
                            fullResolutionDimension != 3 ? LOD : 0 };

    for (int t = 0; t < size[3]; t++)
    for (int i = 0; i < size[2]; i++)
    for (int j = 0; j < size[1]; j++)
    for (int k = 0; k < size[0]; k++)
    {
      OpenVDS::IntVector4 voxel(min[3] + (t << dimensionLOD[3]),
                                min[2] + (i << dimensionLOD[2]),
                                min[1] + (j << dimensionLOD[1]),
                                min[0] + (k << dimensionLOD[0]));

      int offset = k + j * pitch[1] + i * pitch[2] + t * pitch[3];

      OpenVDS::WriteElement((float *)buffer, offset, voxelValueFunction(voxel));
    }

    page->Release();
  }
  pageAccessor->Commit();
}

static float
LODTestVoxelValue(OpenVDS::IntVector4 voxel)
{
  return float(voxel[3] + 37 * (voxel[2] + 37 * (voxel[1] + 37 * voxel[0])));
}

OpenVDS::VDS *CreateVolumeDataAccessorLODTestVDS(int dimensionality, int fullResolutionDimension)
{
  const int
    SIZE_K = 79,
    SIZE_J = 69,
    SIZE_I = 37,
    SIZE_T = 7;

  OpenVDS::Error error;
  int negativeMargin = 4;
  int positiveMargin = 4;
  int brickSize2DMultiplier = 4;
  auto lodLevels = OpenVDS::VolumeDataLayoutDescriptor::LODLevels_4;
  auto layoutOptions = OpenVDS::VolumeDataLayoutDescriptor::Options_Create2DLODs;

  if(fullResolutionDimension >= 0)
  {
    layoutOptions = layoutOptions | OpenVDS::VolumeDataLayoutDescriptor::Options_ForceFullResolutionDimension;
  }

  OpenVDS::VolumeDataLayoutDescriptor layoutDescriptor(OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, negativeMargin, positiveMargin, brickSize2DMultiplier, lodLevels, layoutOptions, fullResolutionDimension);

  std::vector<OpenVDS::VolumeDataAxisDescriptor> axisDescriptors;
  axisDescriptors.emplace_back(SIZE_K, "X", "", 1.0f, float(SIZE_K));
  axisDescriptors.emplace_back(SIZE_J, "Y", "", 1.0f, float(SIZE_J));
  if(dimensionality > 2) axisDescriptors.emplace_back(SIZE_I, "Z", "", 1.0f, float(SIZE_I));
  if(dimensionality > 3) axisDescriptors.emplace_back(SIZE_T, "T", "", 1.0f, float(SIZE_T));

  std::vector<OpenVDS::VolumeDataChannelDescriptor> channelDescriptors;
  float rangeMin = 0.0f;
  float rangeMax = 1.0f;
  float integerScale = 0;
  float integerOffset = 0;
  channelDescriptors.push_back(OpenVDS::VolumeDataChannelDescriptor(OpenVDS::VolumeDataChannelDescriptor::Format_R32, OpenVDS::VolumeDataChannelDescriptor::Components_1, AMPLITUDE_ATTRIBUTE_NAME, "", rangeMin, rangeMax, OpenVDS::VolumeDataMapping::Direct, 1, OpenVDS::VolumeDataChannelDescriptor::Default, 0.f, integerScale, integerOffset));

  OpenVDS::MetadataContainer
    metadataContainer;

  OpenVDS::VDS *handle = OpenVDS::Create(OpenVDS::InMemoryOpenOptions(), layoutDescriptor, axisDescriptors, channelDescriptors, metadataContainer, error);

  auto accessManager = OpenVDS::GetAccessManager(handle);

  for(int LOD = 0; LOD <= int(lodLevels); LOD++)
  {
    const int channel = 0;
    OpenVDS::VolumeDataPageAccessor* pageAccessor = accessManager.CreateVolumeDataPageAccessor(dimensionality == 2 ? OpenVDS::DimensionsND::Dimensions_01 : OpenVDS::Dimensions_012, LOD, channel, 100, OpenVDS::VolumeDataAccessManager::AccessMode_CreateWithoutLODGeneration);
    FillPages(pageAccessor, LODTestVoxelValue);
    accessManager.DestroyVolumeDataPageAccessor(pageAccessor);
  }

  return handle;
}

TEST(OpenVDS_integration, TestVolumeDataReadAccessorLOD)
{
  for (int fullResDim = -1; fullResDim < 2; ++fullResDim)
  {
    OpenVDS::ScopedVDSHandle handle(CreateVolumeDataAccessorLODTestVDS(2, fullResDim));
    auto accessManager = OpenVDS::GetAccessManager(handle);
    auto layout = accessManager.GetVolumeDataLayout();

    const int
      STEPS = 16;

    for (int LOD = 1; LOD <= 4; ++LOD)
    {
      auto readAccessor = accessManager.CreateVolumeData2DReadAccessorR32(OpenVDS::DimensionsND::Dimensions_01, LOD, 0);
      auto interpolatingAccessor = accessManager.CreateVolumeData2DInterpolatingAccessorR32(OpenVDS::DimensionsND::Dimensions_01, LOD, 0, OpenVDS::InterpolationMethod::Cubic);

      int
        sizeK = layout->GetDimensionNumSamples(0),
        sizeJ = layout->GetDimensionNumSamples(1),
        stepK = fullResDim == 0 ? 1 : (1 << LOD),
        stepJ = fullResDim == 1 ? 1 : (1 << LOD);

      for (int j = 0; j < STEPS; ++j)
      for (int k = 0; k < STEPS; ++k)
      {
        OpenVDS::IntVector2
          index(j * (sizeJ - 1) / (STEPS - 1),
                k * (sizeK - 1) / (STEPS - 1));

        float
          expected = LODTestVoxelValue(OpenVDS::IntVector4(0, 0, index.X & -stepJ, index.Y & -stepK)),
          actual = readAccessor.GetValue(index);

        if(actual != expected)
        {
          EXPECT_EQ(actual , expected);
          break;
        }

        OpenVDS::FloatVector2
          floatIndex((index.X & -stepJ) + stepJ * 0.5f,
                     (index.Y & -stepK) + stepK * 0.5f);

        if (floatIndex.X < sizeJ && floatIndex.Y < sizeK)
        {
          float actualInterpolated = interpolatingAccessor.GetValue(floatIndex);
          if(actualInterpolated != expected)
          {
            EXPECT_EQ(actualInterpolated, expected);
            break;
          }
        }
      }
    }
  }

  for (int fullResDim = -1; fullResDim < 3; ++fullResDim)
  {
    OpenVDS::ScopedVDSHandle handle(CreateVolumeDataAccessorLODTestVDS(3, fullResDim));
    auto accessManager = OpenVDS::GetAccessManager(handle);
    auto layout = accessManager.GetVolumeDataLayout();

    const int
      STEPS = 6;

    for (int LOD = 1; LOD <= 4; ++LOD)
    {
      auto readAccessor = accessManager.CreateVolumeData3DReadAccessorR32(OpenVDS::DimensionsND::Dimensions_012, LOD, 0);
      auto interpolatingAccessor = accessManager.CreateVolumeData3DInterpolatingAccessorR32(OpenVDS::DimensionsND::Dimensions_012, LOD, 0, OpenVDS::InterpolationMethod::Cubic);

      int
        sizeK = layout->GetDimensionNumSamples(0),
        sizeJ = layout->GetDimensionNumSamples(1),
        sizeI = layout->GetDimensionNumSamples(2),
        stepK = fullResDim == 0 ? 1 : (1 << LOD),
        stepJ = fullResDim == 1 ? 1 : (1 << LOD),
        stepI = fullResDim == 2 ? 1 : (1 << LOD);

      for (int i = 0; i < STEPS; ++i)
      for (int j = 0; j < STEPS; ++j)
      for (int k = 0; k < STEPS; ++k)
      {
        OpenVDS::IntVector3
          index(i * (sizeI - 1) / (STEPS - 1),
                j * (sizeJ - 1) / (STEPS - 1),
                k * (sizeK - 1) / (STEPS - 1));

        float
          expected = LODTestVoxelValue(OpenVDS::IntVector4(0, index.X & -stepI, index.Y & -stepJ, index.Z & -stepK)),
          actual = readAccessor.GetValue(index);

        if(actual != expected)
        {
          EXPECT_EQ(actual , expected);
          break;
        }

        OpenVDS::FloatVector3
          floatIndex((index.X & -stepI) + stepI * 0.5f,
                     (index.Y & -stepJ) + stepJ * 0.5f,
                     (index.Z & -stepK) + stepK * 0.5f);

        if (floatIndex.X < sizeI && floatIndex.Y < sizeJ && floatIndex.Z < sizeK)
        {
          float actualInterpolated = interpolatingAccessor.GetValue(floatIndex);
          if(actualInterpolated != expected)
          {
            EXPECT_EQ(actualInterpolated, expected);
            break;
          }
        }
      }
    }
  }

  for (int fullResDim = -1; fullResDim < 4; ++fullResDim)
  {
    OpenVDS::ScopedVDSHandle handle(CreateVolumeDataAccessorLODTestVDS(4, fullResDim));
    auto accessManager = OpenVDS::GetAccessManager(handle);
    auto layout = accessManager.GetVolumeDataLayout();

    const int
      STEPS = 4;

    for (int LOD = 1; LOD <= 4; ++LOD)
    {
      auto readAccessor = accessManager.CreateVolumeData4DReadAccessorR32(OpenVDS::DimensionsND::Dimensions_012, LOD, 0);
      auto interpolatingAccessor = accessManager.CreateVolumeData4DInterpolatingAccessorR32(OpenVDS::DimensionsND::Dimensions_012, LOD, 0, OpenVDS::InterpolationMethod::Cubic);

      int
        sizeK = layout->GetDimensionNumSamples(0),
        sizeJ = layout->GetDimensionNumSamples(1),
        sizeI = layout->GetDimensionNumSamples(2),
        sizeT = layout->GetDimensionNumSamples(3),
        stepK = fullResDim == 0 ? 1 : (1 << LOD),
        stepJ = fullResDim == 1 ? 1 : (1 << LOD),
        stepI = fullResDim == 2 ? 1 : (1 << LOD),
        stepT = 1; // 1, since dimension 3 is not in dimension group 012.

      for (int t = 0; t < STEPS; ++t)
      for (int i = 0; i < STEPS; ++i)
      for (int j = 0; j < STEPS; ++j)
      for (int k = 0; k < STEPS; ++k)
      {
        OpenVDS::IntVector4
          index(t * (sizeT - 1) / (STEPS - 1),
                i * (sizeI - 1) / (STEPS - 1),
                j * (sizeJ - 1) / (STEPS - 1),
                k * (sizeK - 1) / (STEPS - 1));

        float
          expected = LODTestVoxelValue(OpenVDS::IntVector4(index.X & -stepT, index.Y & -stepI, index.Z & -stepJ, index.T & -stepK)),
          actual = readAccessor.GetValue(index);

        if(actual != expected)
        {
          EXPECT_EQ(actual , expected);
          break;
        }

        OpenVDS::FloatVector4
          floatIndex((index.X & -stepT) + stepT * 0.5f,
                     (index.Y & -stepI) + stepI * 0.5f,
                     (index.Z & -stepJ) + stepJ * 0.5f,
                     (index.T & -stepK) + stepK * 0.5f);

        if (floatIndex.X < sizeT && floatIndex.Y < sizeI && floatIndex.Z < sizeJ && floatIndex.T < sizeK)
        {
          float actualInterpolated = interpolatingAccessor.GetValue(floatIndex);
          if(actualInterpolated != expected)
          {
            EXPECT_EQ(actualInterpolated, expected);
            break;
          }
        }
      }
    }
  }
}
