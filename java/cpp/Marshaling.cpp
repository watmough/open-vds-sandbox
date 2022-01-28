//=============================================================================
// <copyright>
// Copyright (c) 2020 Bluware Inc. All rights reserved.
//
// All rights are reserved. Reproduction or transmission in whole or in part,
// in any form or by any means, electronic, mechanical or otherwise,
// is prohibited without the prior written permission of the copyright owner.
// </copyright>
//=============================================================================

#include "Marshaling.h"
#include "OpenVDS/OpenVDS.h"
#include "OpenVDS/VolumeData.h"
//#include "OpenVDS/VolumeDataChannelDescriptorAccessor.h"
//#include "OpenVDS/VolumeDataLayoutDescriptorAccessor.h"
//#include "OpenVDS/VolumeDataAxisDescriptorAccessor.h"
//#include "IJKGridDefinitionAccessor.h"
#include <assert.h>

static std::mutex
  s_FinalizerMutex;

thread_local std::stack<JNIEnv*>
JEnvPushPop::s_JNIEnvStack;

thread_local std::stack<jobject>
JEnvPushPop::s_JProxyObjectStack;

jobject Marshaling::CreateJavaObject(const char* type) {
  jclass clazz = GetJNIEnv()->FindClass(type);
  if (clazz) {
    return CreateJavaObject(clazz);
  }
  return NULL;
}

jobject Marshaling::CreateJavaObject(jclass clazz) {
  jmethodID methodID = GetJNIEnv()->GetMethodID(clazz, "<init>", "()V");
  if (methodID) {
    jobject obj = GetJNIEnv()->NewObject(clazz, methodID);
    return obj;
  }
  return NULL;
}

int
HueJNIObjectContext::getHueSpaceGeneration()
{
  return 1;
}

// Ensure that the context wraps a resource allocated in the current HueSpace library instance.
// If this is not the case, throw ObsoleteObjectException which in turn is caught by HUE_JNI_CATCH
// thus making the outer function call a no-op.
void
HueJNIObjectContext::ensureValid() const
{
  assert(this);
  if (m_MagicNumber != MAGIC_NUMBER)
  {
    throw std::runtime_error("Invalid HueJNIObjectContext");
  }
  if (m_HueSpaceGeneration != getHueSpaceGeneration())
  {
    throw ObsoleteObjectException();
  }
}

HueJNIFinalizerMutexGuard::HueJNIFinalizerMutexGuard() : std::lock_guard<std::mutex>(s_FinalizerMutex)
{
}

HueJNIFinalizerMutexGuard::~HueJNIFinalizerMutexGuard()
{
}

//void 
//Marshaling::Convert(jobject& to, OpenVDS::VolumeDataChannelDescriptor const& from)
//{
//  Hue::ProxyLib::VolumeDataChannelDescriptor 
//    from_tmp((Hue::ProxyLib::VCVoxelFormat)(int)from.GetFormat(),
//        (Hue::ProxyLib::VCVoxelComponents)(int)from.GetComponents(),
//        from.GetName(),
//        from.GetUnit(),
//        OpenVDS::FloatRange(from.GetValueRangeMin(), from.GetValueRangeMax()),
//        from.GetIntegerScale(),
//        from.GetIntegerOffset(),
//        from.IsUseNoValue(),
//        from.GetNoValue(),
//        from.IsDiscrete(),
//        from.IsRenderable(),
//        from.IsAllowLossyCompression(),
//        from.IsUseZipForLosslessCompression(),
//        (int64_t)from.GetMapping(),
//        from.GetMappedValueCount());
//  to = VolumeDataChannelDescriptorAccessor::CreateJavaObject(GetJNIEnv(), from_tmp);
//}
//void 
//Marshaling::Convert(OpenVDS::VolumeDataChannelDescriptor& to, jobject from)
//{
//  Hue::ProxyLib::VolumeDataChannelDescriptor 
//    tmp;
//
//  VolumeDataChannelDescriptorAccessor::FromJavaObject(GetJNIEnv(), tmp, from);
//  auto 
//    to_tmp = OpenVDS::VolumeDataChannelDescriptor::CreateFromExplicitParameters(
//                (OpenVDS::VolumeDataFormat)(int)tmp.Format, 
//                (OpenVDS::VolumeDataComponents)(int)tmp.Components,
//                tmp.Name.c_str(),
//                tmp.Unit.c_str(),
//                tmp.ValueRange.Min,
//                tmp.ValueRange.Max,
//                tmp.IntegerScale, 
//                tmp.IntegerOffset, 
//                tmp.UseNoValue, 
//                tmp.NoValue, 
//                tmp.Discrete, 
//                tmp.Renderable, 
//                tmp.AllowLossyCompression, 
//                tmp.UseZipForLosslessCompression, 
//                (OpenVDS::VolumeDataMapping)(int)tmp.ChannelMapping, 
//                tmp.MappedValues);
//  to = to_tmp;
//}
//
//void 
//Marshaling::Convert(jobject& to, OpenVDS::VolumeDataLayoutDescriptor const& from)
//{
//  Hue::ProxyLib::VolumeDataLayoutDescriptor 
//    tmp(
//      (Hue::ProxyLib::VCSize)from.GetBrickSize(), 
//      from.GetNegativeMargin(), 
//      from.GetPositiveMargin(), 
//      (Hue::ProxyLib::LODLevels)from.GetLODLevels(), 
//      from.IsCreate2DLODs(), 
//      from.IsForceFullResolutionDimension(), 
//      from.GetFullResolutionDimension());
//  to = VolumeDataLayoutDescriptorAccessor::CreateJavaObject(GetJNIEnv(), tmp);
//}
//
//void 
//Marshaling::Convert(OpenVDS::VolumeDataLayoutDescriptor& to, jobject from)
//{
//  Hue::ProxyLib::VolumeDataLayoutDescriptor 
//    tmp;
//
//  VolumeDataLayoutDescriptorAccessor::FromJavaObject(GetJNIEnv(), tmp, from);
//  OpenVDS::VolumeDataLayoutDescriptor::Options 
//    options = (OpenVDS::VolumeDataLayoutDescriptor::Options)(
//      (tmp.ForceFullResolutionDimension ? OpenVDS::VolumeDataLayoutDescriptor::Options_ForceFullResolutionDimension : 0) |
//      (tmp.Create2DLODs ? OpenVDS::VolumeDataLayoutDescriptor::Options_Create2DLODs : 0)
//    );
//  OpenVDS::VolumeDataLayoutDescriptor 
//    to_tmp(
//      (enum OpenVDS::VolumeDataLayoutDescriptor::BrickSize)(int)tmp.FullVCSize, 
//      tmp.NegativeMargin, 
//      tmp.PositiveMargin, 
//      (enum OpenVDS::VolumeDataLayoutDescriptor::LODLevels)(int)tmp.LODLevels, 
//      options, 
//      tmp.FullResolutionDimension);
//  to = to_tmp;
//}
//
//void
//Marshaling::Convert(jobject& to, OpenVDS::VolumeDataAxisDescriptor const& from)
//{
//  Hue::ProxyLib::Dimension
//    tmp(
//      from.GetName(),
//      from.GetUnit(),
//      from.GetNumSamples(),
//      OpenVDS::FloatRange(from.GetCoordinateMin(), from.GetCoordinateMax()));
//  to = VolumeDataAxisDescriptorAccessor::CreateJavaObject(GetJNIEnv(), tmp);
//}
//
//void
//Marshaling::Convert(OpenVDS::VolumeDataAxisDescriptor& to, jobject from)
//{
//  Hue::ProxyLib::Dimension
//    tmp;
//
//  VolumeDataAxisDescriptorAccessor::FromJavaObject(GetJNIEnv(), tmp, from);
//  OpenVDS::VolumeDataAxisDescriptor
//    to_tmp(
//      tmp.Size,
//      tmp.Name.c_str(),
//      tmp.Unit.c_str(),
//      tmp.Coordinate.Min,
//      tmp.Coordinate.Max
//      );
//  to = to_tmp;
//}
//
//void 
//Marshaling::Convert(jobject& to, OpenVDS::IJKGridDefinition const& from)
//{
//  to = IJKGridDefinitionAccessor::CreateJavaObject(GetJNIEnv(), from);
//}
//
//void 
//Marshaling::Convert(OpenVDS::IJKGridDefinition& to, jobject from)
//{
//  IJKGridDefinitionAccessor::FromJavaObject(GetJNIEnv(), to, from);
//}
//
