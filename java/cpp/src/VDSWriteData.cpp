/*
 * Copyright 2020 The Open Group
 * Copyright 2020 INT, Inc.
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

#include <OpenVDS/OpenVDS.h>
#include <OpenVDS/VolumeDataLayout.h>
#include <OpenVDS/VolumeDataAccess.h>
#include <OpenVDS/VolumeData.h>
#include <OpenVDS/VolumeDataChannelDescriptor.h>
#include <OpenVDS/ValueConversion.h>

#include "CPPJNI.h"
#include "NDVector.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <type_traits>

using OpenVDS::VDSHandle;

// Work-around for MSVC
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type check_isfinite(T arg) {
    return std::isfinite(arg);
}

template <typename T>
typename std::enable_if<std::is_integral<T>::value, bool>::type check_isfinite(T arg) {
    return true;
}

template <int DIMENSIONS, typename T_src, typename T_dst>
static void copy_data_to_chunk_nd(OpenVDS::VolumeDataPageAccessor *p_access, const T_src *src, size_t src_length,
                               const OpenVDS::VolumeDataLayout *layout, int chunk)
{
  using Vec_t = ndvec::NDVec<DIMENSIONS, int>;
  using Iterator_t = ndvec::NDIterator<DIMENSIONS, int>;

  const T_dst no_value = OpenVDS::ConvertNoValue<T_dst>(p_access->GetChannelDescriptor().GetNoValue());

  int min[OpenVDS::Dimensionality_Max];
  int max[OpenVDS::Dimensionality_Max];
  int chunk_pitch[OpenVDS::Dimensionality_Max];
  OpenVDS::VolumeDataPage *page(p_access->CreatePage(chunk));

  T_dst *dest = reinterpret_cast<T_dst *>(page->GetWritableBuffer(chunk_pitch));
  Vec_t chunkPitch = Vec_t::create(chunk_pitch, OpenVDS::Dimensionality_Max);

  p_access->GetChunkMinMax(chunk, min, max);
  Vec_t pos = Vec_t::create(min, OpenVDS::Dimensionality_Max);
  Vec_t shape = Vec_t::create(max, OpenVDS::Dimensionality_Max) - pos;
  Vec_t srcShape;
  for (int i = 0; i < DIMENSIONS; ++i) {
    srcShape.Val[i] = layout->GetAxisDescriptor(i).GetNumSamples();
  }
  if (src_length < srcShape.size()) {
    throw std::invalid_argument("Source array too small.");
  }

  auto destIterator = Iterator_t(shape, chunkPitch);
  auto srcIterator = Iterator_t(shape, srcShape.to_pitch(), pos);

  assert(destIterator.size() == srcIterator.size());

  while(destIterator.valid()) {
    auto i_src = srcIterator.offset();
    auto i_dest = destIterator.offset();
    dest[i_dest] = check_isfinite((double) src[i_src]) ? static_cast<T_dst>(src[i_src]) : no_value;
    ++srcIterator;
    ++destIterator;
  }

  page->Release();
}

template <typename T>
using copy_fcn_t = std::function<void(OpenVDS::VolumeDataPageAccessor *, const T *, size_t, const OpenVDS::VolumeDataLayout *, int)>;

template <int DIMENSIONS, typename T>
static copy_fcn_t<T> getCopyFunction_nd(OpenVDS::VolumeDataChannelDescriptor::Format format)
{
    using OpenVDS::VolumeDataChannelDescriptor;

    switch(format) {
        case VolumeDataChannelDescriptor::Format::Format_1Bit:
            // [[fallthrough]]
        case VolumeDataChannelDescriptor::Format::Format_U8:
            return &copy_data_to_chunk_nd<DIMENSIONS, T, std::uint8_t>;
        case VolumeDataChannelDescriptor::Format::Format_U16:
            return &copy_data_to_chunk_nd<DIMENSIONS, T, std::uint16_t>;
        case VolumeDataChannelDescriptor::Format::Format_R32:
            return &copy_data_to_chunk_nd<DIMENSIONS, T, float>;
        case VolumeDataChannelDescriptor::Format::Format_U32:
            return &copy_data_to_chunk_nd<DIMENSIONS, T, std::uint32_t>;
        case VolumeDataChannelDescriptor::Format::Format_R64:
            return &copy_data_to_chunk_nd<DIMENSIONS, T, double>;
        case VolumeDataChannelDescriptor::Format::Format_U64:
            return &copy_data_to_chunk_nd<DIMENSIONS, T, std::uint64_t>;
        case VolumeDataChannelDescriptor::Format::Format_Any:
            // [[fallthrough]]
        default:
            throw std::runtime_error("Cannot process format 'any'");
    }
}

template <class T>
void copy_data(const VDSHandle handle, const T *src, size_t src_length, const std::string& channelName)
{
    auto accessManager = OpenVDS::GetAccessManager(handle);
    const auto *layout = accessManager.GetVolumeDataLayout();

    const int lod_level = 0;
    const int max_pages = 8;
    const int channel = layout->GetChannelIndex(channelName.c_str());

    OpenVDS::DimensionsND dim;
    copy_fcn_t<T> copy_fcn;

    switch (layout->GetDimensionality()) {
    case 2:
        copy_fcn = getCopyFunction_nd<2, T>(layout->GetChannelFormat(channel));
        dim = OpenVDS::DimensionsND::Dimensions_01;
        break;
    case 3:
        copy_fcn = getCopyFunction_nd<3, T>(layout->GetChannelFormat(channel));
        dim = OpenVDS::DimensionsND::Dimensions_012;
        break;
    default:
        throw std::domain_error("Only 2D or 3D data can be written");
    }

    auto pageAccessor = accessManager.CreateVolumeDataPageAccessor(dim, lod_level, channel, max_pages,
                                                                   OpenVDS::VolumeDataAccessManager::AccessMode_Create);

    for (int chunk = 0; chunk < pageAccessor->GetChunkCount(); chunk++) {
        copy_fcn(pageAccessor, src, src_length, layout, chunk);
    }

    pageAccessor->Commit();
}

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_org_opengroup_openvds_OpenVDS_writeArrayR32Impl(JNIEnv *env, jclass, jlong native_object, jfloatArray jSrc, jstring jChannel)
{
  CPPJNI_TRY 
  {
    auto vds = CPPJNI_cast<OpenVDS::VDS>(native_object);
    float* src = reinterpret_cast<float*>(env->GetFloatArrayElements(jSrc, 0));
    size_t src_length = env->GetArrayLength(jSrc);
    std::string channelName = CPPJNI_getString(env, jChannel);
    copy_data(vds, src, src_length, channelName);
    env->ReleaseFloatArrayElements(jSrc, src, 0);
  }
  CPPJNI_CATCH;
}

JNIEXPORT void JNICALL Java_org_opengroup_openvds_OpenVDS_writeArrayR64Impl(JNIEnv *env, jclass, jlong native_object, jdoubleArray jSrc, jstring jChannel)
{
  CPPJNI_TRY 
  {
    auto vds = CPPJNI_cast<OpenVDS::VDS>(native_object);
    double* src = reinterpret_cast<double*>(env->GetDoubleArrayElements(jSrc, 0));
    size_t src_length = env->GetArrayLength(jSrc);
    std::string channelName = CPPJNI_getString(env, jChannel);
    copy_data(vds, src, src_length, channelName);
    env->ReleaseDoubleArrayElements(jSrc, src, 0);
  }
  CPPJNI_CATCH;
}

JNIEXPORT void JNICALL Java_org_opengroup_openvds_OpenVDS_writeArrayBoolImpl(JNIEnv *env, jclass, jlong native_object, jbooleanArray jSrc, jstring jChannel)
{
  CPPJNI_TRY 
  {
    auto vds = CPPJNI_cast<OpenVDS::VDS>(native_object);
    std::uint8_t* src = reinterpret_cast<std::uint8_t *>(env->GetBooleanArrayElements(jSrc, 0));
    size_t src_length = env->GetArrayLength(jSrc);
    std::string channelName = CPPJNI_getString(env, jChannel);
    copy_data(vds, src, src_length, channelName);
    env->ReleaseBooleanArrayElements(jSrc, src, 0);
  }
  CPPJNI_CATCH;
}

JNIEXPORT void JNICALL Java_org_opengroup_openvds_OpenVDS_writeArrayU8Impl(JNIEnv *env, jclass, jlong native_object, jbyteArray jSrc, jstring jChannel)
{
  CPPJNI_TRY 
  {
    auto vds = CPPJNI_cast<OpenVDS::VDS>(native_object);
    std::int8_t* src = reinterpret_cast<std::int8_t *>(env->GetByteArrayElements(jSrc, 0));
    size_t src_length = env->GetArrayLength(jSrc);
    std::string channelName = CPPJNI_getString(env, jChannel);
    copy_data(vds, src, src_length, channelName);
    env->ReleaseByteArrayElements(jSrc, src, 0);
  }
  CPPJNI_CATCH;
}

JNIEXPORT void JNICALL Java_org_opengroup_openvds_OpenVDS_writeArrayU16Impl(JNIEnv *env, jclass, jlong native_object, jshortArray jSrc, jstring jChannel)
{
  CPPJNI_TRY 
  {
    auto vds = CPPJNI_cast<OpenVDS::VDS>(native_object);
    std::int16_t* src = reinterpret_cast<std::int16_t *>(env->GetShortArrayElements(jSrc, 0));
    size_t src_length = env->GetArrayLength(jSrc);
    std::string channelName = CPPJNI_getString(env, jChannel);
    copy_data(vds, src, src_length, channelName);
    env->ReleaseShortArrayElements(jSrc, src, 0);
  }
  CPPJNI_CATCH;
}

JNIEXPORT void JNICALL Java_org_opengroup_openvds_OpenVDS_writeArrayU32Impl(JNIEnv *env, jclass, jlong native_object, jintArray jSrc, jstring jChannel)
{
  CPPJNI_TRY 
  {
    auto vds = CPPJNI_cast<OpenVDS::VDS>(native_object);
    std::int32_t* src = reinterpret_cast<std::int32_t *>(env->GetIntArrayElements(jSrc, 0));
    size_t src_length = env->GetArrayLength(jSrc);
    std::string channelName = CPPJNI_getString(env, jChannel);
    copy_data(vds, src, src_length, channelName);
    env->ReleaseIntArrayElements(jSrc, (jint *) src, 0);
  }
  CPPJNI_CATCH;
}

JNIEXPORT void JNICALL Java_org_opengroup_openvds_OpenVDS_writeArrayU64Impl(JNIEnv *env, jclass, jlong native_object, jlongArray jSrc, jstring jChannel)
{
  CPPJNI_TRY 
  {
    auto vds = CPPJNI_cast<OpenVDS::VDS>(native_object);
    std::int64_t* src = reinterpret_cast<std::int64_t *>(env->GetLongArrayElements(jSrc, 0));
    size_t src_length = env->GetArrayLength(jSrc);
    std::string channelName = CPPJNI_getString(env, jChannel);
    copy_data(vds, src, src_length, channelName);
    env->ReleaseLongArrayElements(jSrc, src, 0);
  }
  CPPJNI_CATCH;
}

#ifdef __cplusplus
}
#endif
