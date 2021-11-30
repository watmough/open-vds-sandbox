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

TEST(OpenVDS_integration, WriteReadPage)
{
  OpenVDS::InMemoryOpenOptions options;
  OpenVDS::Error error;
  std::unique_ptr<OpenVDS::IOManager> inMemory(OpenVDS::IOManagerInMemory::CreateIOManager(options, OpenVDS::IOManager::AccessPattern::ReadWrite, error));
  auto ioCreate = new IOManagerFacadeLight(inMemory.get());
  {
    std::unique_ptr<OpenVDS::VDS, decltype(&OpenVDS::Close)> handle(generateSimpleInMemory3DVDS(60, 60, 60, OpenVDS::VolumeDataChannelDescriptor::Format_R32, OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, ioCreate), OpenVDS::Close);
    fill3DVDSWithBitNoise(handle.get());
  }
  SlowIOManager* slowIOManager = new SlowIOManager(50, inMemory.get());
  std::unique_ptr<OpenVDS::VDS, decltype(&OpenVDS::Close)> handle(OpenVDS::Open(slowIOManager, error), OpenVDS::Close);
  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle.get());
  
  auto pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 1024, OpenVDS::VolumeDataAccessManager::AccessMode_ReadWrite);
  {
    auto page = pageAccessor->CreatePage(2);
    int pitch[6];
    auto buffer = page->GetWritableBuffer(pitch);
    auto buffer_size = pitch[0] * pitch[1] * pitch[2];
    memset(buffer, 0xAA, buffer_size);
    page->Release();
    pageAccessor->Commit();
  }

  {
    auto page = pageAccessor->ReadPage(2);
    int pitch[6];
    auto buffer = page->GetBuffer(pitch);
    auto buffer_size = pitch[0] * pitch[1] * pitch[2];
    std::unique_ptr<uint8_t[]> compare_buffer(new uint8_t[buffer_size]);
    memset(compare_buffer.get(), 0xAA, buffer_size);
    ASSERT_TRUE(memcmp(buffer, compare_buffer.get(), buffer_size) == 0);
  }
}

TEST(OpenVDS_integration, ReadWritePage)
{
  OpenVDS::InMemoryOpenOptions options;
  OpenVDS::Error error;
  std::unique_ptr<OpenVDS::IOManager> inMemory(OpenVDS::IOManagerInMemory::CreateIOManager(options, OpenVDS::IOManager::AccessPattern::ReadWrite, error));
  auto ioCreate = new IOManagerFacadeLight(inMemory.get());
  {
    std::unique_ptr<OpenVDS::VDS, decltype(&OpenVDS::Close)> handle(generateSimpleInMemory3DVDS(60, 60, 60, OpenVDS::VolumeDataChannelDescriptor::Format_R32, OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, ioCreate), OpenVDS::Close);
    fill3DVDSWithBitNoise(handle.get());
  }
  SlowIOManager* slowIOManager = new SlowIOManager(50, inMemory.get());
  std::unique_ptr<OpenVDS::VDS, decltype(&OpenVDS::Close)> handle(OpenVDS::Open(slowIOManager, error), OpenVDS::Close);
  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle.get());
  
  auto pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 1024, OpenVDS::VolumeDataAccessManager::AccessMode_ReadWrite);

  {
    auto page = pageAccessor->ReadPage(2);
    int pitch[6];
    auto buffer = page->GetBuffer(pitch);
    auto buffer_size = pitch[0] * pitch[1] * pitch[2];
    std::unique_ptr<uint8_t[]> compare_buffer(new uint8_t[buffer_size]);
    memset(compare_buffer.get(), 0xAA, buffer_size);
    ASSERT_TRUE(memcmp(buffer, compare_buffer.get(), buffer_size) != 0);
    page->Release();
    pageAccessor->SetMaxPages(0);
  }
  
  {
    auto page = pageAccessor->CreatePage(2);
    int pitch[6];
    auto buffer = page->GetWritableBuffer(pitch);
    auto buffer_size = pitch[0] * pitch[1] * pitch[2];
    memset(buffer, 0xAA, buffer_size);
    page->Release();
    pageAccessor->Commit();
  }
  
  {
    auto page = pageAccessor->ReadPage(2);
    int pitch[6];
    auto buffer = page->GetBuffer(pitch);
    auto buffer_size = pitch[0] * pitch[1] * pitch[2];
    std::unique_ptr<uint8_t[]> compare_buffer(new uint8_t[buffer_size]);
    memset(compare_buffer.get(), 0xAA, buffer_size);
    ASSERT_TRUE(memcmp(buffer, compare_buffer.get(), buffer_size) == 0);
  }
}

TEST(OpenVDS_integration, CopyPage)
{
  OpenVDS::InMemoryOpenOptions options;
  OpenVDS::Error error;
  std::unique_ptr<OpenVDS::IOManager> inMemory(OpenVDS::IOManagerInMemory::CreateIOManager(options, OpenVDS::IOManager::AccessPattern::ReadWrite, error));
  auto ioCreate = new IOManagerFacadeLight(inMemory.get());
  {
    std::unique_ptr<OpenVDS::VDS, decltype(&OpenVDS::Close)> handle(generateSimpleInMemory3DVDS(60, 60, 60, OpenVDS::VolumeDataChannelDescriptor::Format_R32, OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, ioCreate), OpenVDS::Close);
    fill3DVDSWithBitNoise(handle.get());
  }
  SlowIOManager* slowIOManager = new SlowIOManager(50, inMemory.get());
  std::unique_ptr<OpenVDS::VDS, decltype(&OpenVDS::Close)> handle(OpenVDS::Open(slowIOManager, error), OpenVDS::Close);
  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle.get());
  
  auto pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 1024, OpenVDS::VolumeDataAccessManager::AccessMode_ReadWrite);

  std::unique_ptr<OpenVDS::VDS, decltype(&OpenVDS::Close)> copyTo(nullptr, OpenVDS::Close);

  {
    auto layout = OpenVDS::GetLayout(handle.get());
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
    auto compressionMethod = OpenVDS::GetCompressionMethod(handle.get());
    auto compressionTolerance = OpenVDS::GetCompressionTolerance(handle.get());
    copyTo.reset(OpenVDS::Create("inmemory://copy_to", "", layoutDescriptor, axisDescriptors, channelDescriptors, *layout, compressionMethod, compressionTolerance, error));
  }

  auto copyToAccessManager = OpenVDS::GetAccessManager(copyTo.get());
  auto copyToPageAccessor = copyToAccessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 1024, OpenVDS::VolumeDataAccessManager::AccessMode_Create);

  {
    copyToPageAccessor->CopyPage(2, *pageAccessor);
    copyToPageAccessor->Commit();
  }
  
  {
    int pitch[6];

    auto sourcePage = pageAccessor->ReadPage(2);
    auto sourceBuffer = sourcePage->GetBuffer(pitch);
    auto sourceBufferSize = pitch[0] * pitch[1] * pitch[2];

    auto targetPage = copyToPageAccessor->ReadPage(2);
    auto targetBuffer = targetPage->GetBuffer(pitch);
    auto targetBufferSize = pitch[0] * pitch[1] * pitch[2];
    ASSERT_EQ(sourceBufferSize, targetBufferSize);
    ASSERT_TRUE(memcmp(sourceBuffer, targetBuffer, sourceBufferSize) == 0);
  }
}
