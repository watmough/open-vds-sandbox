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

#include <cstdlib>

#include <random>
#include <array>

#include <gtest/gtest.h>

#include "../utils/GenerateVDS.h"

static inline void GenerateRandomCoordinate(std::mt19937 &generator, std::vector<std::uniform_real_distribution<float>> &dimensionDistribution, int dimensionality, float (&coordinate)[OpenVDS::Dimensionality_Max])
{
  for (int dimension = 0; dimension < OpenVDS::Dimensionality_Max; dimension++)
  {
    coordinate[dimension] = (dimension < dimensionality) ? dimensionDistribution[dimension](generator) : 0.0f;
  }
}

static inline void CoordinateToSamplePosition(OpenVDS::VolumeDataLayout *layout, int dimensionality, float (&coordinate)[OpenVDS::Dimensionality_Max])
{
  for (int dimension = 0; dimension < dimensionality; dimension++)
  {
    coordinate[dimension] = layout->GetAxisDescriptor(dimension).CoordinateToSamplePosition(coordinate[dimension]);
  }
}
OpenVDS::FloatVector3 getNoValueVoxel(OpenVDS::VolumeDataLayout* layout, OpenVDS::VolumeDataAccessManager &accessManager)
{
  auto readAccessor = accessManager.CreateVolumeData3DInterpolatingAccessorR32(OpenVDS::Dimensions_012, 0, 0, OpenVDS::InterpolationMethod::Linear, 100);
  OpenVDS::FloatVector3 voxel;
  for (int i = 0; i < layout->GetDimensionNumSamples(0); i++)
  {
    voxel[0] = float(i);
    for (int j = 0; j < layout->GetDimensionNumSamples(1); j++)
    {
      voxel[1] = float(j);
      for (int k = 0; k < layout->GetDimensionNumSamples(2); k++)
      {
        voxel[2] = float(k);
        if (readAccessor.GetValue(voxel) == layout->GetChannelNoValue(0))
        {
          return voxel;
        }
      }
    }
  }
  return voxel;
}

GTEST_TEST(OpenVDS_integration, SimpleRequestVolumeSamples)
{
  OpenVDS::Error error;
  
  OpenVDS::VolumeDataFormat formats[] = {
  OpenVDS::VolumeDataFormat::Format_U8,
  OpenVDS::VolumeDataFormat::Format_U16,
  OpenVDS::VolumeDataFormat::Format_R32,
  OpenVDS::VolumeDataFormat::Format_U32,
  OpenVDS::VolumeDataFormat::Format_R64,
  OpenVDS::VolumeDataFormat::Format_U64,
  };
  for (auto format : formats)
  {
    OpenVDS::Error error;
    auto handle = generateSimpleInMemory3DVDS(60, 60, 60, format, OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, 33.2f);
    ASSERT_TRUE(handle);
    fill3DVDSWithNoise(handle);

    OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);
    OpenVDS::VolumeDataLayout* layout = OpenVDS::GetLayout(handle);

    int32_t dimensionality = layout->GetDimensionality();
    OpenVDS::FloatVector3 novalueVoxel = getNoValueVoxel(layout, accessManager);


    std::mt19937 gen(5746);
    std::vector<std::uniform_real_distribution<float>> dimensionDistribution;
    dimensionDistribution.reserve(OpenVDS::Dimensionality_Max);
    for (int i = 0; i < dimensionality; i++)
    {
      float min = layout->GetDimensionMin(i) + 1;
      float max = layout->GetDimensionMax(i);
      dimensionDistribution.emplace_back(min, max);
    }

    float positions[100][OpenVDS::Dimensionality_Max];
  
    memset(positions[0], 0, sizeof(positions[0]));
    positions[0][0] = novalueVoxel[0];
    positions[0][1] = novalueVoxel[1];
    positions[0][2] = novalueVoxel[2];

    for (int i = 1; i < 100; i++)
    {
      GenerateRandomCoordinate(gen, dimensionDistribution, dimensionality, positions[i]);
      CoordinateToSamplePosition(layout, dimensionality, positions[i]);
    }

    float buffer[100];
    auto request = accessManager.RequestVolumeSamples(buffer, sizeof(buffer) * sizeof(float), OpenVDS::Dimensions_012, 0, 0, positions, 100, OpenVDS::InterpolationMethod::Linear);
    bool success = request->WaitForCompletion();
    if (!success)
    {
      if (request->IsCanceled())
      {
        request->IsCanceled(); //get the error
        int errorCode = request->GetErrorCode();
        std::string errorMessage = request->GetErrorMessage();
        ASSERT_NE(errorCode, 0);
        ASSERT_NE(errorMessage.size(), 0);
        FAIL() << std::string(errorMessage);
      }
      else
      {
        // We need to cancel and then wait until we can safely free the buffer
        request->CancelAndWaitForCompletion();
        FAIL() << std::string("Timeout waiting for RequestVolumeSamples");
      }
    }

    auto readAccessor = accessManager.CreateVolumeData3DInterpolatingAccessorR32(OpenVDS::Dimensions_012, 0, 0, OpenVDS::InterpolationMethod::Linear, 100);

    float verifyBuffer[100];
    for (int i = 0; i < 100; i++)
    {
      auto& p = positions[i];
      OpenVDS::FloatVector3 pos(p[2], p[1], p[0]);
      verifyBuffer[i] = readAccessor.GetValue(pos);
    }

    for (int i = 0; i < 100; i++)
    {
      float diff = std::abs(buffer[i] - verifyBuffer[i]);
      ASSERT_EQ(diff, 0.0f);
    }
  }
}
