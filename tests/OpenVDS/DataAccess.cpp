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
#include <OpenVDS/VolumeDataAccess.h>
#include <OpenVDS/VolumeDataLayout.h>
#include <OpenVDS/VolumeData.h>

#include <VDS/Hash.h>

#include <IO/IOManagerInMemory.h>

#include <cstdlib>

#include <array>

#include <gtest/gtest.h>
#include "../utils/GenerateVDS.h"

GTEST_TEST(OpenVDS_integration, SimpleVolumeDataPageRead)
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

  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);
  OpenVDS::VolumeDataLayout *layout = OpenVDS::GetLayout(handle);

  std::shared_ptr<OpenVDS::VolumeDataPageAccessor> pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 10, OpenVDS::VolumeDataAccessManager::AccessMode_ReadOnly);
  ASSERT_TRUE(pageAccessor);

  int pos[OpenVDS::Dimensionality_Max] = {layout->GetDimensionNumSamples(0) / 2, layout->GetDimensionNumSamples(1) /2, layout->GetDimensionNumSamples(2) / 2};
  OpenVDS::VolumeDataPage *page = pageAccessor->ReadPageAtPosition(pos);
  ASSERT_TRUE(page);

  int pitch[OpenVDS::Dimensionality_Max] = {}; 
  const void *buffer = page->GetBuffer(pitch);
  ASSERT_TRUE(buffer);
  for (int i = 0; i < layout->GetDimensionality(); i++)
    ASSERT_NE(pitch[i], 0);

  int min[6];
  int max[6];
  page->GetMinMax(min, max);

  ASSERT_TRUE(min[0] <=  pos[0]);
  ASSERT_TRUE(min[1] <=  pos[1]);
  ASSERT_TRUE(min[2] <=  pos[2]);
  ASSERT_TRUE(pos[0] <  max[0]);
  ASSERT_TRUE(pos[1] <  max[1]);
  ASSERT_TRUE(pos[2] <  max[2]);
}

GTEST_TEST(OpenVDS_integration, SimpleVolumeDataPageReadVDSFile)
{
  OpenVDS::Error error;
  std::string fileName = TEST_DATA_PATH "/subset.vds";

  OpenVDS::ScopedVDSHandle handle(OpenVDS::Open(OpenVDS::VDSFileOpenOptions(fileName), error));
  ASSERT_TRUE(handle);

  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);
  OpenVDS::VolumeDataLayout *layout = OpenVDS::GetLayout(handle);

  std::shared_ptr<OpenVDS::VolumeDataPageAccessor> pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 10, OpenVDS::VolumeDataAccessManager::AccessMode_ReadOnly);
  ASSERT_TRUE(pageAccessor);

  int pos[OpenVDS::Dimensionality_Max] = {layout->GetDimensionNumSamples(0) / 2, layout->GetDimensionNumSamples(1) /2, layout->GetDimensionNumSamples(2) / 2};
  OpenVDS::VolumeDataPage *page = pageAccessor->ReadPageAtPosition(pos);
  ASSERT_TRUE(page);

  int pitch[OpenVDS::Dimensionality_Max] = {}; 
  const void *buffer = page->GetBuffer(pitch);
  ASSERT_TRUE(buffer);
  for (int i = 0; i < layout->GetDimensionality(); i++)
    ASSERT_NE(pitch[i], 0);

  int min[6];
  int max[6];
  page->GetMinMax(min, max);

  ASSERT_TRUE(min[0] <=  pos[0]);
  ASSERT_TRUE(min[1] <=  pos[1]);
  ASSERT_TRUE(min[2] <=  pos[2]);
  ASSERT_TRUE(pos[0] <  max[0]);
  ASSERT_TRUE(pos[1] <  max[1]);
  ASSERT_TRUE(pos[2] <  max[2]);
}

GTEST_TEST(OpenVDS_integration, SimpleVolumeDataPageReadVDSFileWithAdaptiveCompression)
{
  OpenVDS::Error error;
  std::string fileName = TEST_DATA_PATH "/subset.vds";
  auto options = OpenVDS::VDSFileOpenOptions(fileName);
  options.waveletAdaptiveMode = OpenVDS::WaveletAdaptiveMode::Ratio;
  options.waveletAdaptiveRatio = 10.0f;
  OpenVDS::ScopedVDSHandle handle(OpenVDS::Open(options, error));
  ASSERT_TRUE(handle);

  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);
  OpenVDS::VolumeDataLayout *layout = OpenVDS::GetLayout(handle);

  std::shared_ptr<OpenVDS::VolumeDataPageAccessor> pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 10, OpenVDS::VolumeDataAccessManager::AccessMode_ReadOnly);
  ASSERT_TRUE(pageAccessor);

  int pos[OpenVDS::Dimensionality_Max] = {layout->GetDimensionNumSamples(0) / 2, layout->GetDimensionNumSamples(1) /2, layout->GetDimensionNumSamples(2) / 2};
  OpenVDS::VolumeDataPage *page = pageAccessor->ReadPageAtPosition(pos);
  ASSERT_TRUE(page);

  int pitch[OpenVDS::Dimensionality_Max] = {}; 
  const void *buffer = page->GetBuffer(pitch);
  ASSERT_TRUE(buffer);
  for (int i = 0; i < layout->GetDimensionality(); i++)
    ASSERT_NE(pitch[i], 0);

  int min[6];
  int max[6];
  page->GetMinMax(min, max);

  ASSERT_TRUE(min[0] <=  pos[0]);
  ASSERT_TRUE(min[1] <=  pos[1]);
  ASSERT_TRUE(min[2] <=  pos[2]);
  ASSERT_TRUE(pos[0] <  max[0]);
  ASSERT_TRUE(pos[1] <  max[1]);
  ASSERT_TRUE(pos[2] <  max[2]);
}


template<typename T, size_t N> inline T (&PODArrayReference(std::array<T,N> &a))[N] { return *reinterpret_cast<T (*)[N]>(a.data()); }

GTEST_TEST(OpenVDS_integration, SimpleRequestVolumeSubset)
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

  OpenVDS::VolumeDataLayout *layout = OpenVDS::GetLayout(handle);
  ASSERT_TRUE(layout);

  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);

  int loopDimension = 4;
  int groupSize = 100;

  int traceDimension = (loopDimension == 0) ? 1 : 0;
  int traceDimensionSize = layout->GetDimensionNumSamples(traceDimension);

  int groupDimension = (loopDimension == traceDimension + 1) ? traceDimension + 2 : traceDimension + 1;
  int groupDimensionSize = layout->GetDimensionNumSamples(groupDimension);

  if(groupSize == 0)
  {
    groupSize = groupDimensionSize;
  }

  std::array<int, OpenVDS::Dimensionality_Max> voxelMin = { 0, 0, 0, 0, 0, 0};
  std::array<int, OpenVDS::Dimensionality_Max> voxelMax = { 1, 1, 1, 1, 1, 1};

  voxelMin[traceDimension] = 0;
  voxelMax[traceDimension] = traceDimensionSize;
  voxelMin[loopDimension] = 2;
  voxelMax[loopDimension] = 2 + 1;

  auto request = accessManager.RequestVolumeSubset<float>(OpenVDS::Dimensions_012, 0, 0, PODArrayReference(voxelMin), PODArrayReference(voxelMax));
  bool returned = request->WaitForCompletion();
  ASSERT_TRUE(returned);

}

GTEST_TEST(OpenVDS_integration, SimpleRequestVolumeSubsetWithAdaptiveCompression)
{
  OpenVDS::Error error;

  std::string url = TEST_URL;
  std::string connectionString = TEST_CONNECTION;
  if(url.empty())
  {
    GTEST_SKIP() << "Test Environment for connecting to VDS is not set";
  }

  OpenVDS::ScopedVDSHandle handle(OpenVDS::OpenWithAdaptiveCompressionRatio(url, connectionString, 10.0f, error));
  ASSERT_TRUE(handle);

  OpenVDS::VolumeDataLayout *layout = OpenVDS::GetLayout(handle);
  ASSERT_TRUE(layout);

  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);

  int loopDimension = 4;
  int groupSize = 100;

  int traceDimension = (loopDimension == 0) ? 1 : 0;
  int traceDimensionSize = layout->GetDimensionNumSamples(traceDimension);

  int groupDimension = (loopDimension == traceDimension + 1) ? traceDimension + 2 : traceDimension + 1;
  int groupDimensionSize = layout->GetDimensionNumSamples(groupDimension);

  if(groupSize == 0)
  {
    groupSize = groupDimensionSize;
  }

  std::array<int, OpenVDS::Dimensionality_Max> voxelMin = { 0, 0, 0, 0, 0, 0};
  std::array<int, OpenVDS::Dimensionality_Max> voxelMax = { 1, 1, 1, 1, 1, 1};

  voxelMin[traceDimension] = 0;
  voxelMax[traceDimension] = traceDimensionSize;
  voxelMin[loopDimension] = 2;
  voxelMax[loopDimension] = 2 + 1;

  auto request = accessManager.RequestVolumeSubset<float>(OpenVDS::Dimensions_012, 0, 0, PODArrayReference(voxelMin), PODArrayReference(voxelMax));
  bool returned = request->WaitForCompletion();
  ASSERT_TRUE(returned);

}

GTEST_TEST(VolumeSubset, BadLOD)
{
  OpenVDS::Error error;
  OpenVDS::ScopedVDSHandle handle(generateSimpleInMemory3DVDS(60, 60, 60));

  fill3DVDSWithNoise(handle);

  ASSERT_TRUE(handle);

  OpenVDS::VolumeDataLayout *layout = OpenVDS::GetLayout(handle);
  ASSERT_TRUE(layout);

  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);

  int loopDimension = 4;
  int groupSize = 100;

  int traceDimension = (loopDimension == 0) ? 1 : 0;
  int traceDimensionSize = layout->GetDimensionNumSamples(traceDimension);

  int groupDimension = (loopDimension == traceDimension + 1) ? traceDimension + 2 : traceDimension + 1;
  int groupDimensionSize = layout->GetDimensionNumSamples(groupDimension);

  if(groupSize == 0)
  {
    groupSize = groupDimensionSize;
  }

  std::array<int, OpenVDS::Dimensionality_Max> voxelMin = { 0, 0, 0, 0, 0, 0};
  std::array<int, OpenVDS::Dimensionality_Max> voxelMax = { 1, 1, 1, 1, 1, 1};

  voxelMin[traceDimension] = 0;
  voxelMax[traceDimension] = traceDimensionSize;
  voxelMin[loopDimension] = 2;
  voxelMax[loopDimension] = 2 + 1;

  try
  {
    auto request = accessManager.RequestVolumeSubset<float>(OpenVDS::Dimensions_012, 1, 0, PODArrayReference(voxelMin), PODArrayReference(voxelMax));
    bool returned = request->WaitForCompletion();
    ASSERT_TRUE(returned);
  }
  catch (OpenVDS::Exception &)
  {
    return;
  }
  ASSERT_TRUE(false);
}

template<typename T>
uint64_t
CalculateHash(const int (&index)[OpenVDS::VolumeDataLayout::Dimensionality_Max], int dimensionality, T value)
{
  OpenVDS::HashCombiner
    hash;

  for(int i = 0; i < dimensionality; i++)
  {
    hash.Add(index[i]);
  }

  hash.Add(value);

  return hash.GetCombinedHash();
}

template <typename T>
uint64_t
ProcessBuffer(T *buffer, int dimensionality, const int (&voxelMin)[OpenVDS::VolumeDataLayout::Dimensionality_Max], const int (&voxelMax)[OpenVDS::VolumeDataLayout::Dimensionality_Max], int LOD)
{
  uint64_t
    hash = 0;

  int
    voxelPos[OpenVDS::VolumeDataLayout::Dimensionality_Max] = { 0, 0, 0, 0, 0, 0};

  int
    dataIndex = 0;

  for(voxelPos[5] = voxelMin[5]; voxelPos[5] < voxelMax[5]; voxelPos[5] += (1 << LOD))
  for(voxelPos[4] = voxelMin[4]; voxelPos[4] < voxelMax[4]; voxelPos[4] += (1 << LOD))
  for(voxelPos[3] = voxelMin[3]; voxelPos[3] < voxelMax[3]; voxelPos[3] += (1 << LOD))
  for(voxelPos[2] = voxelMin[2]; voxelPos[2] < voxelMax[2]; voxelPos[2] += (1 << LOD))
  for(voxelPos[1] = voxelMin[1]; voxelPos[1] < voxelMax[1]; voxelPos[1] += (1 << LOD))
  for(voxelPos[0] = voxelMin[0]; voxelPos[0] < voxelMax[0]; voxelPos[0] += (1 << LOD))
  {
    T value = OpenVDS::ReadElement(buffer, dataIndex++);
    hash ^= CalculateHash(voxelPos, dimensionality, value);
  }

  return hash;
}

GTEST_TEST(OpenVDS_integration, RequestVolumeSubsetWithDifferentFormatsAndDimensionGroups)
{
  int dim[3] = { 100, 100, 200 };

  OpenVDS::Error error;
  OpenVDS::IOManager *inMemory = OpenVDS::IOManagerInMemory::CreateIOManagerInMemory("", error);
  EXPECT_EQ(error.code, 0);
  EXPECT_EQ(error.string, "");

  OpenVDS::ScopedVDSHandle handle(generateSimpleInMemory3DVDS(dim[0], dim[1], dim[2], OpenVDS::VolumeDataChannelDescriptor::Format_R32, OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, 0.0f, inMemory));
  ASSERT_TRUE(handle);
  fill3DVDSWithNoise(handle);

  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);

  int voxelMin[] = { 13, 13, 119,  0, 0, 0};
  int voxelMax[] = { 23, 23, 129,  1, 1, 1};

  const int LOD = 0;
  const int channel = 0;

  static const OpenVDS::DimensionsND
    dimensionGroups[] = { OpenVDS::DimensionsND::Dimensions_01, OpenVDS::DimensionsND::Dimensions_02, OpenVDS::DimensionsND::Dimensions_12 };

  static const OpenVDS::VolumeDataFormat
    formats[] = { OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataFormat::Format_U8, OpenVDS::VolumeDataFormat::Format_U16, OpenVDS::VolumeDataFormat::Format_R64, OpenVDS::VolumeDataFormat::Format_1Bit };

  for(auto format : formats)
  {
    auto request012 = accessManager.RequestVolumeSubset(OpenVDS::DimensionsND::Dimensions_012, LOD, channel, voxelMin, voxelMax, format);
    request012->WaitForCompletion();
    uint64_t hash012 = 0;
    switch(format)
    {
    case OpenVDS::VolumeDataFormat::Format_R32:  hash012 = ProcessBuffer(reinterpret_cast<float    *>(request012->Buffer()), 3, voxelMin, voxelMax, LOD); break;
    case OpenVDS::VolumeDataFormat::Format_U8:   hash012 = ProcessBuffer(reinterpret_cast<uint8_t  *>(request012->Buffer()), 3, voxelMin, voxelMax, LOD); break;
    case OpenVDS::VolumeDataFormat::Format_U16:  hash012 = ProcessBuffer(reinterpret_cast<uint16_t *>(request012->Buffer()), 3, voxelMin, voxelMax, LOD); break;
    case OpenVDS::VolumeDataFormat::Format_R64:  hash012 = ProcessBuffer(reinterpret_cast<double   *>(request012->Buffer()), 3, voxelMin, voxelMax, LOD); break;
    case OpenVDS::VolumeDataFormat::Format_1Bit: hash012 = ProcessBuffer(reinterpret_cast<bool     *>(request012->Buffer()), 3, voxelMin, voxelMax, LOD); break;
    default: throw std::runtime_error("Unexpected format");
    }

    for(auto dimensiongroup : dimensionGroups)
    {
      auto request = accessManager.RequestVolumeSubset(dimensiongroup, LOD, channel, voxelMin, voxelMax, format);
      request->WaitForCompletion();
      uint64_t hash = 0;
      switch(format)
      {
      case OpenVDS::VolumeDataFormat::Format_R32:  hash = ProcessBuffer(reinterpret_cast<float    *>(request->Buffer()), 3, voxelMin, voxelMax, LOD); break;
      case OpenVDS::VolumeDataFormat::Format_U8:   hash = ProcessBuffer(reinterpret_cast<uint8_t  *>(request->Buffer()), 3, voxelMin, voxelMax, LOD); break;
      case OpenVDS::VolumeDataFormat::Format_U16:  hash = ProcessBuffer(reinterpret_cast<uint16_t *>(request->Buffer()), 3, voxelMin, voxelMax, LOD); break;
      case OpenVDS::VolumeDataFormat::Format_R64:  hash = ProcessBuffer(reinterpret_cast<double   *>(request->Buffer()), 3, voxelMin, voxelMax, LOD); break;
      case OpenVDS::VolumeDataFormat::Format_1Bit: hash = ProcessBuffer(reinterpret_cast<bool     *>(request->Buffer()), 3, voxelMin, voxelMax, LOD); break;
      default: throw std::runtime_error("Unexpected format");
      }
      EXPECT_EQ(hash012, hash);
    }
  }
}

GTEST_TEST(OpenVDS_integration, RequestVolumeSubsetWithDifferentFormatsAndDimensionGroups1BitSource)
{
  int dim[3] = { 100, 100, 200 };

  OpenVDS::Error error;
  OpenVDS::IOManager *inMemory = OpenVDS::IOManagerInMemory::CreateIOManagerInMemory("", error);
  EXPECT_EQ(error.code, 0);
  EXPECT_EQ(error.string, "");

  OpenVDS::ScopedVDSHandle handle(generateSimpleInMemory3DVDS(dim[0], dim[1], dim[2], OpenVDS::VolumeDataChannelDescriptor::Format_1Bit, OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, 0.0f, inMemory));
  ASSERT_TRUE(handle);
  fill3DVDSWithBitNoise(handle);

  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);

  int voxelMin[] = { 13, 13, 119,  0, 0, 0};
  int voxelMax[] = { 23, 23, 129,  1, 1, 1};

  const int LOD = 0;
  const int channel = 0;

  static const OpenVDS::DimensionsND
    dimensionGroups[] = { OpenVDS::DimensionsND::Dimensions_01, OpenVDS::DimensionsND::Dimensions_02, OpenVDS::DimensionsND::Dimensions_12 };

  static const OpenVDS::VolumeDataFormat
    formats[] = { OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataFormat::Format_U8, OpenVDS::VolumeDataFormat::Format_U16, OpenVDS::VolumeDataFormat::Format_R64, OpenVDS::VolumeDataFormat::Format_1Bit };

  for(auto format : formats)
  {
    auto request012 = accessManager.RequestVolumeSubset(OpenVDS::DimensionsND::Dimensions_012, LOD, channel, voxelMin, voxelMax, format);
    request012->WaitForCompletion();
    uint64_t hash012 = 0;
    switch(format)
    {
    case OpenVDS::VolumeDataFormat::Format_R32:  hash012 = ProcessBuffer(reinterpret_cast<float    *>(request012->Buffer()), 3, voxelMin, voxelMax, LOD); break;
    case OpenVDS::VolumeDataFormat::Format_U8:   hash012 = ProcessBuffer(reinterpret_cast<uint8_t  *>(request012->Buffer()), 3, voxelMin, voxelMax, LOD); break;
    case OpenVDS::VolumeDataFormat::Format_U16:  hash012 = ProcessBuffer(reinterpret_cast<uint16_t *>(request012->Buffer()), 3, voxelMin, voxelMax, LOD); break;
    case OpenVDS::VolumeDataFormat::Format_R64:  hash012 = ProcessBuffer(reinterpret_cast<double   *>(request012->Buffer()), 3, voxelMin, voxelMax, LOD); break;
    case OpenVDS::VolumeDataFormat::Format_1Bit: hash012 = ProcessBuffer(reinterpret_cast<bool     *>(request012->Buffer()), 3, voxelMin, voxelMax, LOD); break;
    default: throw std::runtime_error("Unexpected format");
    }

    for(auto dimensiongroup : dimensionGroups)
    {
      auto request = accessManager.RequestVolumeSubset(dimensiongroup, LOD, channel, voxelMin, voxelMax, format);
      request->WaitForCompletion();
      uint64_t hash = 0;
      switch(format)
      {
      case OpenVDS::VolumeDataFormat::Format_R32:  hash = ProcessBuffer(reinterpret_cast<float    *>(request->Buffer()), 3, voxelMin, voxelMax, LOD); break;
      case OpenVDS::VolumeDataFormat::Format_U8:   hash = ProcessBuffer(reinterpret_cast<uint8_t  *>(request->Buffer()), 3, voxelMin, voxelMax, LOD); break;
      case OpenVDS::VolumeDataFormat::Format_U16:  hash = ProcessBuffer(reinterpret_cast<uint16_t *>(request->Buffer()), 3, voxelMin, voxelMax, LOD); break;
      case OpenVDS::VolumeDataFormat::Format_R64:  hash = ProcessBuffer(reinterpret_cast<double   *>(request->Buffer()), 3, voxelMin, voxelMax, LOD); break;
      case OpenVDS::VolumeDataFormat::Format_1Bit: hash = ProcessBuffer(reinterpret_cast<bool     *>(request->Buffer()), 3, voxelMin, voxelMax, LOD); break;
      default: throw std::runtime_error("Unexpected format");
      }
      EXPECT_EQ(hash012, hash);
    }
  }
}
