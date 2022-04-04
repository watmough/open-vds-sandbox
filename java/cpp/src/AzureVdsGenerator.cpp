/*
 * Copyright 2020 The Open Group
 * Copyright 2020 Bluware, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Marshaling.h"

#include <OpenVDS/OpenVDS.h>
#include <OpenVDS/VolumeDataLayoutDescriptor.h>
#include <OpenVDS/VolumeDataAxisDescriptor.h>
#include <OpenVDS/VolumeDataChannelDescriptor.h>
#include <OpenVDS/KnownMetadata.h>
#include <OpenVDS/MetadataContainer.h>

#include <limits>
#include <stdexcept>
#include <string>
#include <vector>


static OpenVDS::VDS *generate(const OpenVDS::AzureOpenOptions& opts, int32_t samplesX, int32_t samplesY, int32_t samplesZ,
                              OpenVDS::VolumeDataChannelDescriptor::Format format, const std::vector<std::string>& channels,
                              const std::vector<std::string>& units)
{
    if (channels.size() != units.size()) {
        throw std::runtime_error("Channel and unit array size differ in length");
    }
 
    OpenVDS::VolumeDataLayoutDescriptor::BrickSize brickSize;
    if (samplesZ == 0) {
        brickSize = OpenVDS::VolumeDataLayoutDescriptor::BrickSize_1024;
    } else {
        brickSize = OpenVDS::VolumeDataLayoutDescriptor::BrickSize_128;
    }

    int negativeMargin = 4;
    int positiveMargin = 4;
    int brickSize2DMultiplier = 4;
    auto lodLevels = OpenVDS::VolumeDataLayoutDescriptor::LODLevels_None;
    auto layoutOptions = OpenVDS::VolumeDataLayoutDescriptor::Options_None;
    OpenVDS::VolumeDataLayoutDescriptor layoutDescriptor(brickSize, negativeMargin, positiveMargin,
                                                         brickSize2DMultiplier, lodLevels, layoutOptions);

    std::vector<OpenVDS::VolumeDataAxisDescriptor> axisDescriptors;
    axisDescriptors.emplace_back(samplesX, KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_SAMPLE, "ms", 0.0f, 4.f);
    axisDescriptors.emplace_back(samplesY, KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_CROSSLINE, "", 1932.f, 2536.f);

    if (samplesZ != 0) {
        axisDescriptors.emplace_back(samplesZ, KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_INLINE,    "", 9985.f, 10369.f);
    }

    std::vector<OpenVDS::VolumeDataChannelDescriptor> channelDescriptors;

    float rangeMin = -0.1234f;
    float rangeMax = 0.1234f;
    float intScale = 1.f;
    float intOffset = 0.f;

    for (int i = 0; i < channels.size(); i++) {
        channelDescriptors.emplace_back(format, OpenVDS::VolumeDataChannelDescriptor::Components_1,
            channels[i].c_str(), units[i].c_str(), rangeMin, rangeMax, OpenVDS::VolumeDataMapping::Direct, 1,
            OpenVDS::VolumeDataChannelDescriptor::Default, std::numeric_limits<float>::lowest(), intScale, intOffset);
    }

    OpenVDS::MetadataContainer metadataContainer;
    OpenVDS::Error error;

    return OpenVDS::Create(opts, layoutDescriptor, axisDescriptors, channelDescriptors, metadataContainer, error);
}

static std::vector<std::string> convertStringArray(JNIEnv *env, jobjectArray obj)
{
    std::vector<std::string> res;

    for (int i = 0; i < env->GetArrayLength(obj); i++) {
        jstring jstr = (jstring) env->GetObjectArrayElement(obj, i);
        res.push_back(CPPJNI_getString(env, jstr));
    }
    return res;
}

#ifdef __cplusplus
extern "C" {
#endif

jlong JNICALL Java_org_opengroup_openvds_AzureVDSGenerator_CreateVDSImpl(JNIEnv *env, jclass,
        jlong azureOptions, jint nXSamples, jint nYSamples, jint nZSamples, jlong format,
        jobjectArray jChannelNames, jobjectArray jUnitNames)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto openOptions = CPPJNI_cast<OpenVDS::AzureOpenOptions>(azureOptions);
    auto channel_names = convertStringArray(env, jChannelNames);
    std::vector<std::string> unit_names;

    if (jUnitNames != nullptr) {
      unit_names = convertStringArray(env, jUnitNames);
      if (unit_names.size() != channel_names.size()) {
        throw std::runtime_error("OpenVDS::Channels and units must have the same size");
      }
    }
    else 
    {
      for (auto& name : channel_names) {
          unit_names.push_back("");
      }
    }

    OpenVDS::VDSHandle handle = generate(*openOptions, nXSamples, nYSamples, nZSamples, 
                                          (OpenVDS::VolumeDataChannelDescriptor::Format)format, channel_names, unit_names);

    if (!handle) 
    {
      throw std::runtime_error("OpenVDS::Create returned NULL");
    }
    return CPPJNI_createObjectContext<OpenVDS::VDS>(handle)->handle();
  }
  CPPJNI_CATCH;
  return 0;
}

#ifdef __cplusplus
}
#endif
