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
#include <vector>

#include <gtest/gtest.h>
#include <fmt/format.h>

#include "../utils/GenerateVDS.h"

GTEST_TEST(OpenVDS_integration, SimpleRequestVolumeTraces)
{
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

    const int LOD = 0;
    int sampleCount0 = OpenVDS::GetLODSize(0, layout->GetDimensionNumSamples(0), LOD);
    std::vector<float> buffer(10 * sampleCount0);

    int sampleCount1 = layout->GetDimensionNumSamples(1);
    int sampleCount2 = layout->GetDimensionNumSamples(2);

    float tracePos[10][6];

    for (int trace = 0; trace < 10; trace++)
    {
      tracePos[trace][0] = 0;
      tracePos[trace][1] = (float)(trace * (sampleCount1 / 10));
      tracePos[trace][2] = (float)(trace * (sampleCount2 / 10));
      tracePos[trace][3] = 0;
      tracePos[trace][4] = 0;
      tracePos[trace][5] = 0;
    }

    auto request = accessManager.RequestVolumeTraces(buffer.data(), buffer.size() * sizeof(float), OpenVDS::Dimensions_012, LOD, 0, tracePos, 10, OpenVDS::InterpolationMethod::Nearest,0);

    float previousProgress = -1;
    while (!request->WaitForCompletion(1000)) {
      ASSERT_FALSE(request->IsCanceled());

      float progress = request->GetCompletionFactor();
      if (progress != previousProgress) {
        previousProgress = progress;
        GTEST_LOG_(INFO) << "Request progress : " << progress * 100. << " %";
      }
    }

    auto valueReader = accessManager.CreateVolumeData3DInterpolatingAccessorR32(OpenVDS::Dimensions_012, LOD, 0, OpenVDS::InterpolationMethod::Nearest, 1000);

    std::vector<float> verify(10 * sampleCount0);
    for (int trace = 0; trace < 10; trace++)
    {
      for (int i = 0; i < sampleCount0; i++)
      {
        verify[trace * sampleCount0 + i] = valueReader.GetValue(OpenVDS::FloatVector3(tracePos[trace][2], tracePos[trace][1], float(i << LOD)));
      }
    }
    for (int i = 0; i < int(verify.size()); i++)
    {
      if (buffer[i] != verify[i])
      {
        ASSERT_FLOAT_EQ(buffer[i], verify[i]);
      }
    }
  }
}

const float HALF_CELL_WIDTH = 0.5f;
typedef std::array<float, 6> NDPos;
typedef std::vector<NDPos> NDPosArray;

// compute coords of traces, draw a zig zag in the cube
NDPosArray computeTraceCoords(OpenVDS::VolumeDataAxisDescriptor iAxis, OpenVDS::VolumeDataAxisDescriptor jAxis, int nbTraces) {
  float iStep = (float)iAxis.GetNumSamples() / (float)nbTraces;
  float jStep = (float)jAxis.GetNumSamples() / (float)(nbTraces/2);
  NDPosArray posArray(nbTraces);
  int indexT = 0;
    
  for (indexT = 0 ; indexT < nbTraces / 2 ; ++indexT) {
    float coordI = (indexT * iStep) + HALF_CELL_WIDTH;
    float coordJ = (indexT * jStep) + HALF_CELL_WIDTH;
    NDPos coords = {{HALF_CELL_WIDTH, coordJ, coordI, HALF_CELL_WIDTH, HALF_CELL_WIDTH, HALF_CELL_WIDTH}};
    posArray[indexT] = coords;
  }
    
  float maxJ = (float)jAxis.GetNumSamples();
  for (int ind = indexT  ; ind < nbTraces ; ++ind) {
    float coordI = (ind * iStep) + HALF_CELL_WIDTH;
    float coordJ = (maxJ - (ind * jStep)) + HALF_CELL_WIDTH;
    NDPos coords = {{HALF_CELL_WIDTH, coordJ, coordI, HALF_CELL_WIDTH, HALF_CELL_WIDTH, HALF_CELL_WIDTH}};
    posArray[ind] = coords;
  }
   
  return posArray;
}
  
void checkAndWaitRequest(std::shared_ptr<OpenVDS::VolumeDataRequest> request, int timeout) {
  while (!request->WaitForCompletion(timeout)) {
    if (request->IsCanceled()) {
      throw OpenVDS::ReadErrorException(request->GetErrorMessage().c_str(), request->GetErrorCode());
    }
    // let display progress
    // float completionFactor = request->GetCompletionFactor();
  }
}

GTEST_TEST(OpenVDS_integration, TestLODTracesRequest)
{
  try {
    OpenVDS::Error error;
    auto handle = generateSimpleInMemory3DVDS(60, 60, 60, OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, OpenVDS::VolumeDataLayoutDescriptor::LODLevels_2);
    ASSERT_TRUE(handle);
    fill3DVDSWithNoise(handle, 0, OpenVDS::FloatVector3(0.6f, 2.f, 4.f), 0.0f);
    if(error.code)
    {
      throw OpenVDS::ReadErrorException(error.string.c_str(), error.code);
    }
    OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);
      
    OpenVDS::VolumeDataAxisDescriptor jAxis = OpenVDS::GetLayout(handle)->GetAxisDescriptor(1);
    OpenVDS::VolumeDataAxisDescriptor iAxis = OpenVDS::GetLayout(handle)->GetAxisDescriptor(2);

    NDPosArray tracesCoords = computeTraceCoords(iAxis, jAxis, 1000);
      
    // query with no LOD
    SUCCEED() << "Query with no LOD";
    std::shared_ptr<OpenVDS::VolumeDataRequest> requestNoLOD = accessManager.RequestVolumeTraces(OpenVDS::DimensionsND::Dimensions_012, 0, 0, (float(*)[6])tracesCoords.data(), int(tracesCoords.size()), OpenVDS::InterpolationMethod::Cubic, 0);
    checkAndWaitRequest(requestNoLOD, 1000);
    SUCCEED() << "Done";
      
    // query with LOD
    SUCCEED() << "Query with LOD";
    std::shared_ptr<OpenVDS::VolumeDataRequest> requestLOD = accessManager.RequestVolumeTraces(OpenVDS::DimensionsND::Dimensions_012, 2, 0, (float(*)[6])tracesCoords.data(), int(tracesCoords.size()), OpenVDS::InterpolationMethod::Cubic, 0);
    checkAndWaitRequest(requestLOD, 1000);
    SUCCEED() << "Done";

    // check that LOD samples are reasonably equal to LOD 0
    int sampleCount = OpenVDS::GetLayout(handle)->GetDimensionNumSamples(0);
    const int LOD = 2;
    int sampleCountLOD = OpenVDS::GetLODSize(0, sampleCount, LOD);
    auto dataNoLOD = static_cast<const float *>(requestNoLOD->Buffer());
    auto dataLOD = static_cast<const float *>(requestLOD->Buffer());
    float maxDiff = 0.0f;
    for(int trace = 0; trace < int(tracesCoords.size()); trace++)
    {
      for(int sample = 0; sample < sampleCount; sample += 4)
      {
        const float tolerance = 0.05f;
        float diff = std::abs(dataNoLOD[trace * sampleCount + sample] - dataLOD[trace * sampleCountLOD + (sample >> LOD)]);
        maxDiff = std::max(maxDiff, diff);
        EXPECT_NEAR(dataNoLOD[trace * sampleCount + sample], dataLOD[trace * sampleCountLOD + (sample >> LOD)], tolerance);
      }
    }
    SUCCEED() << "Max difference: " << maxDiff;
  }
  catch (OpenVDS::ReadErrorException &e) {
    FAIL() << e.GetErrorMessage();
  }
}
