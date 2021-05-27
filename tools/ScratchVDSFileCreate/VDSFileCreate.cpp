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
#include <cmath>
#include <chrono>


void createNoLODVDS(const std::string &vdsFileName, int32_t samplesX, int32_t samplesY, int32_t samplesZ);
void createLODVDS(const std::string &vdsFileName, int32_t samplesX, int32_t samplesY, int32_t samplesZ, OpenVDS::VolumeDataLayoutDescriptor::LODLevels);
double distance2D(double x1, double y1, double x2, double y2);
double distance3D(double x1, double y1, double x2, double y2, double x3, double y3);

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
    cxxopts::Options options("VDSFileCreate", "Create synthetic vds file");
    std::string vdsFileName;
    options.add_option("", "", "vdsfile", "Output VDS file name.", cxxopts::value<std::string>(vdsFileName), "<string>");

    if (argc == 1) {
        std::cout << options.help();
        return EXIT_SUCCESS;
    }

    try {
        options.parse(argc, argv);
    }
    catch(cxxopts::OptionParseException &e) {
        fmt::print(stderr, "{}", e.what());
        return EXIT_FAILURE;
    }

    int32_t samplesX = 500;
    int32_t samplesY = 800;
    int32_t samplesZ = 800;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    createNoLODVDS(vdsFileName, samplesX, samplesY, samplesZ);
    //createLODVDS(vdsFileName, samplesX, samplesY, samplesZ, OpenVDS::VolumeDataLayoutDescriptor::LODLevels_3);

    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    long elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
    long hrs = elapsed / (60 * 60 * 1000);
    long min = (elapsed - (hrs * 60 * 60 * 1000)) / (60 * 1000);
    long s = (elapsed - (hrs * 60 * 1000) - (min * 60 * 1000)) / 1000;
    std::cout << "Write VDS TIME [CPP native] : " <<  hrs << " hrs " << min << " min " << s << "s (" << elapsed << " ms)" << std::endl;
    return EXIT_SUCCESS;
}

void createNoLODVDS(const std::string &vdsFileName, int32_t samplesX, int32_t samplesY, int32_t samplesZ) {
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
                                    OpenVDS::VolumeDataChannelDescriptor::Default, -999.25f, intScale, intOffset);

    //OpenVDS::InMemoryOpenOptions options;
    OpenVDS::VDSFileOpenOptions options(vdsFileName);
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
    OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(vds);
    //ASSERT_TRUE(accessManager);

    int32_t channel = 0;
    OpenVDS::VolumeDataPageAccessor *pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, channel, 100, OpenVDS::VolumeDataAccessManager::AccessMode_Create);
    //ASSERT_TRUE(pageAccessor);

    double distMax = distance3D(0, 0, 0, samplesX, samplesY, samplesZ);
    double cycles = M_PI * 2 * 6;
    double midX = samplesX / 2.f;
    double midY = samplesY / 2.f;
    double midZ = samplesZ / 2.f;

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
                    OpenVDS::IntVector<3> vox = outputIndexer.LocalIndexToVoxelIndex(localOutIndex);
                    float value = 0.f;
                    double dist = 0.;
                    if (vox[0] >= midX) {
                        dist = distance2D(midY, midZ, vox[1], vox[2]);
                    } else {
                        dist = distance3D(midX, midY, midZ, vox[0], vox[1], vox[2]);
                    }
                    value = (float) sin((dist * cycles) / distMax);
                    int dataPos = outputIndexer.LocalIndexToDataIndex(localOutIndex);
                    output[dataPos] = value;
                }

        page->Release();
    }

    pageAccessor->Commit();
    pageAccessor->SetMaxPages(0);
    accessManager.FlushUploadQueue();
    accessManager.DestroyVolumeDataPageAccessor(pageAccessor);

    OpenVDS::Close(vds);
}

void createLODVDS(const std::string &vdsFileName, int32_t samplesX, int32_t samplesY, int32_t samplesZ, OpenVDS::VolumeDataLayoutDescriptor::LODLevels lodLevel) {

    double distMax = distance3D(0, 0, 0, samplesX, samplesY, samplesZ);
    double cycles = M_PI * 2 * 6;
    double midX = samplesX / 2.f;
    double midY = samplesY / 2.f;
    double midZ = samplesZ / 2.f;

    OpenVDS::VolumeDataChannelDescriptor::Format format = OpenVDS::VolumeDataChannelDescriptor::Format_R32;

    auto brickSize = OpenVDS::VolumeDataLayoutDescriptor::BrickSize_256;
    int negativeMargin = 4;
    int positiveMargin = 4;
    int brickSize2DMultiplier = 4;
    auto lodLevels = lodLevel;
    int lodLevelCount = lodLevel;
    auto layoutOptions = OpenVDS::VolumeDataLayoutDescriptor::Options_None;
    OpenVDS::VolumeDataLayoutDescriptor layoutDescriptor(brickSize, negativeMargin, positiveMargin,
                                                         brickSize2DMultiplier, lodLevels, layoutOptions);

    std::vector<OpenVDS::VolumeDataAxisDescriptor> axisDescriptors;
    axisDescriptors.emplace_back(samplesX, "Sample", "s", 0.0f,4.f);
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
                                    OpenVDS::VolumeDataChannelDescriptor::Default, -999.25f, intScale, intOffset);

    //OpenVDS::InMemoryOpenOptions options;
    OpenVDS::VDSFileOpenOptions options(vdsFileName);
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
    OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(vds);
    //ASSERT_TRUE(accessManager);

    for (int lod = 0 ; lod <= lodLevelCount ; lod ++) {
        int32_t channel = 0;
        OpenVDS::VolumeDataPageAccessor *pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, lod, channel, 100,
                                                                                                    OpenVDS::VolumeDataAccessManager::AccessMode_Create);
        //ASSERT_TRUE(pageAccessor);

        int32_t chunkCount = int32_t(pageAccessor->GetChunkCount());
        //OpenVDS::VolumeDataChannelDescriptor::Format format = layout->GetChannelFormat(channel);
        for (int i = 0; i < chunkCount; i++) {
            OpenVDS::VolumeDataPage *page = pageAccessor->CreatePage(i);
            OpenVDS::VolumeIndexer3D outputIndexer(page, 0, lod, OpenVDS::Dimensions_012, layout);
            OpenVDS::IntVector<3> numSamples;

            for (int j = 0; j < 3; j++) {
                numSamples[j] = outputIndexer.GetDataBlockNumSamples(j);
            }

            int pitch[OpenVDS::VolumeDataLayout::Dimensionality_Max];
            int chunkMin[OpenVDS::VolumeDataLayout::Dimensionality_Max];
            int chunkMax[OpenVDS::VolumeDataLayout::Dimensionality_Max];

            page->GetMinMax(chunkMin, chunkMax);

            void *buffer = page->GetWritableBuffer(pitch);
            auto output = static_cast<float *>(buffer);

            for (int iDim2 = 0; iDim2 < numSamples[2]; iDim2++)
                for (int iDim1 = 0; iDim1 < numSamples[1]; iDim1++)
                    for (int iDim0 = 0; iDim0 < numSamples[0]; iDim0++) {
                        OpenVDS::IntVector<3> localIndex(iDim0, iDim1, iDim2);
                        OpenVDS::IntVector<3> voxelPos = outputIndexer.LocalIndexToVoxelIndex(localIndex);

                        float value = 0.f;
                        double dist = 0.;
                        if (voxelPos[0] >= midX) {
                            dist = distance2D(midY, midZ, voxelPos[1], voxelPos[2]);
                        } else {
                            dist = distance3D(midX, midY, midZ, voxelPos[0], voxelPos[1], voxelPos[2]);
                        }
                        value = (float) sin((dist * cycles) / distMax);

                        int dataPos = outputIndexer.LocalIndexToDataIndex(localIndex);
                        output[dataPos] = value;
                    }

            page->Release();
        }

        pageAccessor->Commit();
        pageAccessor->SetMaxPages(0);
        accessManager.FlushUploadQueue();
        accessManager.DestroyVolumeDataPageAccessor(pageAccessor);
    }

    OpenVDS::Close(vds);
}

double distance2D(double x1, double y1, double x2, double y2) {
    double diffX = x2 - x1;
    double diffY = y2 - y1;
    return sqrt((diffX * diffX) + (diffY * diffY));
}

double distance3D(double x1, double y1, double z1, double x2, double y2, double z2) {
    double diffX = x2 - x1;
    double diffY = y2 - y1;
    double diffZ = z2 - z1;
    return sqrt((diffX * diffX) + (diffY * diffY) + (diffZ * diffZ));
}
