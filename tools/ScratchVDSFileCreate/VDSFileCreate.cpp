/****************************************************************************
** Copyright 2019 The Open Group
** Copyright 2019 Bluware, Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**   http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
****************************************************************************/

#define _CRT_SECURE_NO_WARNINGS 1

#include <SEGYUtils/SEGYFileInfo.h>
#include "IO/File.h"
#include "VDS/Hash.h"
#include <SEGYUtils/DataProvider.h>
#include <SEGYUtils/TraceDataManager.h>

#include <OpenVDS/OpenVDS.h>
#include <OpenVDS/MetadataContainer.h>
#include <OpenVDS/VolumeDataLayoutDescriptor.h>
#include <OpenVDS/VolumeDataAxisDescriptor.h>
#include <OpenVDS/VolumeDataChannelDescriptor.h>
#include <OpenVDS/VolumeDataAccess.h>
#include <OpenVDS/VolumeIndexer.h>
#include <OpenVDS/Range.h>
#include <OpenVDS/VolumeDataLayout.h>
#include <OpenVDS/KnownMetadata.h>
#include <OpenVDS/GlobalMetadataCommon.h>

#include <mutex>
#include <cstdlib>
#include <climits>
#include <cassert>
#include <algorithm>

#include "cxxopts.hpp"
#include <json/json.h>
#include <fmt/format.h>

#include <chrono>

#if defined(WIN32)
#undef WIN32_LEAN_AND_MEAN // avoid warnings if defined on command line
#define WIN32_LEAN_AND_MEAN 1
#define NOMINMAX 1
#include <io.h>
#include <windows.h>

int64_t GetTotalSystemMemory()
{
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return int64_t(status.ullTotalPhys);
}

#else

#include <unistd.h>


int64_t GetTotalSystemMemory() {
    long pages = sysconf(_SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    return int64_t(pages) * int64_t(page_size);
}

#endif

static void
getScaleOffsetForFormat(float min, float max, bool novalue, OpenVDS::VolumeDataChannelDescriptor::Format format,
                        float &scale, float &offset) {
    switch (format) {
        case OpenVDS::VolumeDataChannelDescriptor::Format_U8:
            scale = 1.f / (255.f - novalue) * (max - min);
            offset = min;
            break;
        case OpenVDS::VolumeDataChannelDescriptor::Format_U16:
            scale = 1.f / (65535.f - novalue) * (max - min);
            offset = min;
            break;
        case OpenVDS::VolumeDataChannelDescriptor::Format_R32:
        case OpenVDS::VolumeDataChannelDescriptor::Format_U32:
        case OpenVDS::VolumeDataChannelDescriptor::Format_R64:
        case OpenVDS::VolumeDataChannelDescriptor::Format_U64:
        case OpenVDS::VolumeDataChannelDescriptor::Format_1Bit:
        case OpenVDS::VolumeDataChannelDescriptor::Format_Any:
            scale = 1.0f;
            offset = 0.0f;
    }
}

int
main(int argc, char *argv[]) {
    int32_t samplesX = 200;
    int32_t samplesY = 300;
    int32_t samplesZ = 300;
    OpenVDS::VolumeDataChannelDescriptor::Format format = OpenVDS::VolumeDataChannelDescriptor::Format_R32;

    auto brickSize = OpenVDS::VolumeDataLayoutDescriptor::BrickSize_64;
    int negativeMargin = 4;
    int positiveMargin = 4;
    int brickSize2DMultiplier = 4;
    auto lodLevels = OpenVDS::VolumeDataLayoutDescriptor::LODLevels_None;
    auto layoutOptions = OpenVDS::VolumeDataLayoutDescriptor::Options_None;
    OpenVDS::VolumeDataLayoutDescriptor layoutDescriptor(brickSize, negativeMargin, positiveMargin,
                                                         brickSize2DMultiplier, lodLevels, layoutOptions);

    std::vector<OpenVDS::VolumeDataAxisDescriptor> axisDescriptors;
    axisDescriptors.emplace_back(samplesX, "Sample", "ms", 0.0f,4.f);
    axisDescriptors.emplace_back(samplesY, "Crossline", "",1932.f, 1932.f + samplesY - 1.f);
    axisDescriptors.emplace_back(samplesZ, "Inline", "", 9985.f, 9985.f + samplesZ - 1.f);

    std::vector<OpenVDS::VolumeDataChannelDescriptor> channelDescriptors;
    float rangeMin = -1.f;
    float rangeMax = 1.f;
    float intScale;
    float intOffset;
    getScaleOffsetForFormat(rangeMin, rangeMax, true, format, intScale, intOffset);
    channelDescriptors.emplace_back(format, OpenVDS::VolumeDataChannelDescriptor::Components_1,
                                    AMPLITUDE_ATTRIBUTE_NAME, "", rangeMin, rangeMax,
                                    OpenVDS::VolumeDataMapping::Direct, 1,
                                    OpenVDS::VolumeDataChannelDescriptor::Default, 0.f, intScale, intOffset);

    //OpenVDS::InMemoryOpenOptions options;
    OpenVDS::VDSFileOpenOptions options("/tmp/createCPP_bis.vds");
    OpenVDS::Error error;

    OpenVDS::MetadataContainer metadataContainer;
    metadataContainer.SetMetadataInt("categoryInt", "Int", 123);
    metadataContainer.SetMetadataIntVector2("categoryInt", "IntVector2", OpenVDS::IntVector2(45, 78));
    metadataContainer.SetMetadataIntVector3("categoryInt", "IntVector3", OpenVDS::IntVector3(45, 78, 72));
    metadataContainer.SetMetadataIntVector4("categoryInt", "IntVector4", OpenVDS::IntVector4(45, 78, 72, 84));
    metadataContainer.SetMetadataFloat("categoryFloat", "Float", 123.f);
    metadataContainer.SetMetadataFloatVector2("categoryFloat", "FloatVector2", OpenVDS::FloatVector2(45.5f, 78.75f));
    metadataContainer.SetMetadataFloatVector3("categoryFloat", "FloatVector3",
                                              OpenVDS::FloatVector3(45.5f, 78.75f, 72.75f));
    metadataContainer.SetMetadataFloatVector4("categoryFloat", "FloatVector4",
                                              OpenVDS::FloatVector4(45.5f, 78.75f, 72.75f, 84.1f));
    metadataContainer.SetMetadataDouble("categoryDouble", "Double", 123.);
    metadataContainer.SetMetadataDoubleVector2("categoryDouble", "DoubleVector2", OpenVDS::DoubleVector2(45.5, 78.75));
    metadataContainer.SetMetadataDoubleVector3("categoryDouble", "DoubleVector3",
                                               OpenVDS::DoubleVector3(45.5, 78.75, 72.75));
    metadataContainer.SetMetadataDoubleVector4("categoryDouble", "DoubleVector4",
                                               OpenVDS::DoubleVector4(45.5, 78.75, 72.75, 84.1));
    metadataContainer.SetMetadataString("categoryString", "String", std::string("Test string"));

    auto vds = OpenVDS::Create(options, layoutDescriptor, axisDescriptors, channelDescriptors, metadataContainer,
                                     error);

    OpenVDS::VolumeDataLayout *layout = OpenVDS::GetLayout(vds);
    //ASSERT_TRUE(layout);
    OpenVDS::VolumeDataAccessManager *accessManager = OpenVDS::GetAccessManager(vds);
    //ASSERT_TRUE(accessManager);

    int32_t channel = 0;
    OpenVDS::VolumeDataPageAccessor *pageAccessor = accessManager->CreateVolumeDataPageAccessor(layout, OpenVDS::Dimensions_012, channel, 0, 1000, OpenVDS::VolumeDataAccessManager::AccessMode_Create);
    //ASSERT_TRUE(pageAccessor);

    int32_t chunkCount = int32_t(pageAccessor->GetChunkCount());
    //OpenVDS::VolumeDataChannelDescriptor::Format format = layout->GetChannelFormat(channel);
    for (int i = 0; i < chunkCount; i++)
    {
        OpenVDS::VolumeDataPage *page =  pageAccessor->CreatePage(i);
        OpenVDS::VolumeIndexer3D outputIndexer(page, 0, 0, OpenVDS::Dimensions_012, layout);
        OpenVDS::IntVector<3> numSamples;

        for (int j=0; j<3; j++)
        {
            numSamples[j] = outputIndexer.GetDataBlockNumSamples(j);
        }

        int pitch[OpenVDS::Dimensionality_Max];
        void *buffer = page->GetWritableBuffer(pitch);
        auto output = static_cast<float *>(buffer);

        for (int iDim2 = 0; iDim2 < numSamples[2]; iDim2++)
                for (int iDim1 = 0; iDim1 < numSamples[1]; iDim1++)
                    for (int iDim0 = 0; iDim0 < numSamples[0]; iDim0++)
                    {
                        OpenVDS::IntVector<3> localOutIndex(iDim0, iDim1, iDim2);

                        float value = (float) ((iDim0 + iDim1 + iDim2)%numSamples[0]) / numSamples[0];
                        value = rangeMin + 2.f * value;
                        int dataPos = outputIndexer.LocalIndexToDataIndex(localOutIndex);
                        output[dataPos] = value;
                    }

        page->Release();
    }

    pageAccessor->Commit();
    pageAccessor->SetMaxPages(0);
    accessManager->FlushUploadQueue();
    accessManager->DestroyVolumeDataPageAccessor(pageAccessor);

    OpenVDS::Close(vds);

    return EXIT_SUCCESS;
}
