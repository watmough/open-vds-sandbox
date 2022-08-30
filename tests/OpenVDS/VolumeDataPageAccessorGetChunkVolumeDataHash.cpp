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

GTEST_TEST(OpenVDS_integration, VolumeDataPageAccessorGetChunkVolumeDataHash)
{
  std::string url = TEST_URL;
  std::string connectionString = TEST_CONNECTION;
  if(url.empty())
  {
    GTEST_SKIP() << "Test Environment for connecting to VDS is not set";
  }

  // Create and configure VDS
  const int sizeX = 200, sizeY = 200, sizeZ = 200;

  OpenVDS::VolumeDataLayoutDescriptor
    layoutDescriptor = OpenVDS::VolumeDataLayoutDescriptor(OpenVDS::VolumeDataLayoutDescriptor::BrickSize_64, 0, 0, 4,
      OpenVDS::VolumeDataLayoutDescriptor::LODLevels_None,
      OpenVDS::VolumeDataLayoutDescriptor::Options_None);

  std::vector<OpenVDS::VolumeDataAxisDescriptor>
    axisDescriptors = { OpenVDS::VolumeDataAxisDescriptor(sizeX, "X", "m", 0.0, float((sizeX - 1) * 10.0)),
                        OpenVDS::VolumeDataAxisDescriptor(sizeY, "Y", "m", 0.0, float((sizeY - 1) * 10.0)),
                        OpenVDS::VolumeDataAxisDescriptor(sizeZ, "Z", "m", 0.0, float((sizeZ - 1) * 10.0)) };

  std::vector<OpenVDS::VolumeDataChannelDescriptor>
    channelDescriptors = { OpenVDS::VolumeDataChannelDescriptor(OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataComponents::Components_1, "Value", "", 0.0, float(sizeX + sizeY + sizeZ)) };

  OpenVDS::MetadataContainer
    metadataContainer;

  OpenVDS::Error error;
  OpenVDS::ScopedVDSHandle handle = OpenVDS::Create(url, connectionString, layoutDescriptor, axisDescriptors, channelDescriptors, metadataContainer, error);
  ASSERT_EQ(error.code, 0);

  // Create volume data page accessor
  auto accessManager = OpenVDS::GetAccessManager(handle);
  const int channel = 0;
  const int lod = 0;
  std::shared_ptr<OpenVDS::VolumeDataPageAccessor> pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::DimensionsND::Dimensions_012, channel, lod, 100, OpenVDS::VolumeDataAccessManager::AccessMode_Create);

  // Verify chunk 0 volume data hash is 0 / unknown
  auto volumeDataHash = pageAccessor->GetChunkVolumeDataHash(0);
  ASSERT_EQ(0, volumeDataHash);

  // Write page for chunk 0
  OpenVDS::VolumeDataPage* page = pageAccessor->CreatePage(0);
  int pitch[OpenVDS::VolumeDataLayout::Dimensionality_Max];
  auto buffer = reinterpret_cast<float*>(page->GetWritableBuffer(pitch));
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
        buffer[offset] = float(i + j + k);
      }
    }
  }
  page->Release();
  pageAccessor->Commit();

  // Verify chunk 0 volume data hash is NOT 0 / unknown
  volumeDataHash = pageAccessor->GetChunkVolumeDataHash(0);
  ASSERT_NE(0, volumeDataHash);

  // Destroy volume data page accessor
  pageAccessor.reset();
  accessManager.Flush(error);
  ASSERT_EQ(error.code, 0);

  // Close VDS
  handle.Close();

  // Reopen VDS
  handle = OpenVDS::Open(url, connectionString, error);
  ASSERT_EQ(error.code, 0);

  // Create volume data page accessor
  accessManager = OpenVDS::GetAccessManager(handle);
  pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, channel, lod, 100, OpenVDS::VolumeDataAccessManager::AccessMode_ReadOnly);

  // Verify chunk 0 volume data hash is equal to expected value
  ASSERT_EQ(volumeDataHash, pageAccessor->GetChunkVolumeDataHash(0));
}

GTEST_TEST(OpenVDS_integration, VolumeDataPageAccessorGetChunkVolumeDataHash_VDSFile)
{
  // Make sure file not already on disk
  const OpenVDS::VDSFileOpenOptions openOptions("VolumeDataPageAccessorGetChunkVolumeDataHash_VDSFile.vds");
  remove(openOptions.fileName.c_str());

  // Create and configure VDS
  const int sizeX = 200, sizeY = 200, sizeZ = 200;

  OpenVDS::VolumeDataLayoutDescriptor
    layoutDescriptor = OpenVDS::VolumeDataLayoutDescriptor(OpenVDS::VolumeDataLayoutDescriptor::BrickSize_64, 0, 0, 4,
      OpenVDS::VolumeDataLayoutDescriptor::LODLevels_None,
      OpenVDS::VolumeDataLayoutDescriptor::Options_None);

  std::vector<OpenVDS::VolumeDataAxisDescriptor>
    axisDescriptors = { OpenVDS::VolumeDataAxisDescriptor(sizeX, "X", "m", 0.0, float((sizeX - 1) * 10.0)),
                        OpenVDS::VolumeDataAxisDescriptor(sizeY, "Y", "m", 0.0, float((sizeY - 1) * 10.0)),
                        OpenVDS::VolumeDataAxisDescriptor(sizeZ, "Z", "m", 0.0, float((sizeZ - 1) * 10.0)) };

  std::vector<OpenVDS::VolumeDataChannelDescriptor>
    channelDescriptors = { OpenVDS::VolumeDataChannelDescriptor(OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataComponents::Components_1, "Value", "", 0.0, float(sizeX + sizeY + sizeZ)) };

  OpenVDS::MetadataContainer
    metadataContainer;

  OpenVDS::Error error;
  OpenVDS::ScopedVDSHandle handle = OpenVDS::Create(openOptions, layoutDescriptor, axisDescriptors, channelDescriptors, metadataContainer, error);
  ASSERT_EQ(error.code, 0);

  // Create volume data page accessor
  auto accessManager = OpenVDS::GetAccessManager(handle);
  const int channel = 0;
  const int lod = 0;
  std::shared_ptr<OpenVDS::VolumeDataPageAccessor> pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::DimensionsND::Dimensions_012, channel, lod, 100, OpenVDS::VolumeDataAccessManager::AccessMode_Create);

  // Verify chunk 0 volume data hash is 0 / unknown
  auto volumeDataHash = pageAccessor->GetChunkVolumeDataHash(0);
  ASSERT_EQ(0, volumeDataHash);

  // Write page for chunk 0
  OpenVDS::VolumeDataPage* page = pageAccessor->CreatePage(0);
  int pitch[OpenVDS::VolumeDataLayout::Dimensionality_Max];
  auto buffer = reinterpret_cast<float*>(page->GetWritableBuffer(pitch));
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
        buffer[offset] = float(i + j + k);
      }
    }
  }
  page->Release();
  pageAccessor->Commit();

  // Verify chunk 0 volume data hash is NOT 0 / unknown
  volumeDataHash = pageAccessor->GetChunkVolumeDataHash(0);
  ASSERT_NE(0, volumeDataHash);

  // Destroy volume data page accessor
  pageAccessor.reset();
  accessManager.Flush(error);
  ASSERT_EQ(0, error.code);

  // Close VDS
  handle.Close();

  // Reopen VDS
  handle = OpenVDS::Open(openOptions, error);
  ASSERT_EQ(error.code, 0);

  // Create volume data page accessor
  accessManager = OpenVDS::GetAccessManager(handle);
  pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, channel, lod, 100, OpenVDS::VolumeDataAccessManager::AccessMode_ReadOnly);

  // Verify chunk 0 volume data hash is equal to expected value
  ASSERT_EQ(volumeDataHash, pageAccessor->GetChunkVolumeDataHash(0));

  // Close VDS and delete file
  handle.Close();
  remove(openOptions.fileName.c_str());
}
