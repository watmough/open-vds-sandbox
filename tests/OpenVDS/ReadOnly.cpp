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

#include <OpenVDS/OpenVDS.h>
#include <OpenVDS/VolumeDataAccess.h>
#include <OpenVDS/VolumeDataLayout.h>
#include <OpenVDS/VolumeData.h>

#include <VDS/Hash.h>

#include <IO/IOManagerInMemory.h>

#include <cstdlib>

#include <array>

#include <gtest/gtest.h>
#include "../utils/GenerateVDS.h"
#include "../utils/FacadeIOManager.h"

struct Counter
{
  int ReadInfoCount = 0;
  int ReadCount = 0;
  int WriteCount = 0;
};

class IOManagerCounting : public OpenVDS::IOManager
{
public:
  IOManagerCounting(OpenVDS::IOManager *backend, Counter &counter)
    : IOManager(backend->connectionType())
    , backend(backend)
    , counter(counter)
  {}

  std::shared_ptr<OpenVDS::Request> ReadObjectInfo(const std::string &objectName, std::shared_ptr<OpenVDS::TransferDownloadHandler> handler) override
  {
    counter.ReadInfoCount++;
    return backend->ReadObjectInfo(objectName, handler);
  }

  std::shared_ptr<OpenVDS::Request> ReadObject(const std::string &objectName, std::shared_ptr<OpenVDS::TransferDownloadHandler> handler, const OpenVDS::IORange& range = OpenVDS::IORange()) override
  {
    counter.ReadCount++;
    return backend->ReadObject(objectName, handler, range);
  }

  std::shared_ptr<OpenVDS::Request> WriteObject(const std::string &objectName, const std::string& contentDispostionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::function<void(const OpenVDS::Request & request, const OpenVDS::Error & error)> completedCallback = nullptr) override
  {
    counter.WriteCount++;
    return backend->WriteObject(objectName, contentDispostionFilename, contentType, metadataHeader, data, completedCallback);
  }

  bool Close(OpenVDS::Error &error) override
  {
    return backend->Close(error);
  }

  IOManager *backend;
  Counter& counter;
};

GTEST_TEST(OpenVDS_integration, VerifyReadOnly)
{
  OpenVDS::Error error;
  OpenVDS::IOManager *inMemory = OpenVDS::IOManagerInMemory::CreateIOManagerInMemory("", error);
  EXPECT_EQ(error.code, 0);
  EXPECT_EQ(error.string, "");

  {
    IOManagerFacadeLight* createIoManager = new IOManagerFacadeLight(inMemory);
    OpenVDS::ScopedVDSHandle handle(generateSimpleInMemory3DVDS(100, 100, 100, OpenVDS::VolumeDataChannelDescriptor::Format_R32, OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, createIoManager));
    ASSERT_TRUE(handle);
    fill3DVDSWithNoise(handle);
  }

  Counter counter;
  int64_t chunkCount = 0;
  {
    IOManagerCounting* countingIoManager = new IOManagerCounting(inMemory, counter);
    OpenVDS::ScopedVDSHandle handle = OpenVDS::Open(countingIoManager, error);
    EXPECT_EQ(error.code, 0);
    auto accessManager = OpenVDS::GetAccessManager(handle);
    auto pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 1000, OpenVDS::VolumeDataPageAccessor::AccessMode_ReadWrite);
    chunkCount = pageAccessor->GetChunkCount();
    for (int64_t i = 0; i < chunkCount; i++)
    {
      auto page = pageAccessor->ReadPage(i);
      int pitch[6];
      auto buffer = page->GetBuffer(pitch);
      EXPECT_TRUE(buffer != nullptr);
      page->Release();
    }

    auto metadataWriteAccess = OpenVDS::GetMetadataWriteAccessInterface(handle);
    metadataWriteAccess->SetMetadataInt("hello", "world", 3);
    accessManager.Flush(error);
    EXPECT_EQ(error.code, 0);
  }
  EXPECT_TRUE(chunkCount != 0);
  EXPECT_EQ(counter.WriteCount, 0);
  EXPECT_TRUE(counter.ReadCount > chunkCount);
}
