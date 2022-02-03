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

void Hue_Handle_StdException(struct JNIEnv_ *,class std::exception &)
{
}

void Hue_Handle_StdRuntimeError(struct JNIEnv_ *,class std::runtime_error &)
{
}

void Hue_Handle_HueSpaceLibException(struct JNIEnv_ *,class OpenVDS::Exception &)
{
}



