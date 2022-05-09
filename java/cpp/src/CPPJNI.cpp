/*
 * Copyright 2022 The Open Group
 * Copyright 2022 Bluware Inc. 
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

#include "CPPJNI.h"
#include "OpenVDS/OpenVDS.h"
#include "OpenVDS/VolumeData.h"
#include "OpenVDS/Exceptions.h"
#include <cassert>
#include <set>

int
CPPJNIObjectContext::getSharedLibraryGeneration()
{ // The HueSpace shared library supports unloading and reloading within the same process.
  // Here, we just return 1.
  return 1;
}

void 
CPPJNI_HandleSharedLibraryException(struct JNIEnv_ *env, class OpenVDS::Exception &e)
{
  OpenVDS::Exception
    *pException = &e;

  if (dynamic_cast<OpenVDS::ReadErrorException*>(pException) != nullptr)
  {
    CPPJNI_Throw(env, e.what(), CPPJNIExceptionType::IOException);
  }
  else
  {
    CPPJNI_Throw(env, e.what(), CPPJNIExceptionType::Exception);
  }
}

void
CPPJNI_onVDSError(OpenVDS::VDSError const& error)
{
  CPPJNI_Throw(JNIEnvGuard::getJNIEnv(), error.string.c_str(), CPPJNIExceptionType::IOException);
}

extern "C" 
{

/**
* 
* Returns an array of 2 objects: The native handle and the directbuffer object
* 
*/

JNIEXPORT jobjectArray JNICALL Java_org_opengroup_openvds_ManagedBuffer_ctorImpl
  (JNIEnv * env, jclass clazz, jlong capacity)
{
  return CPPJNI_createManagedBuffer(env, clazz, capacity);
}

JNIEXPORT void JNICALL Java_org_opengroup_openvds_ManagedBuffer_dtorImpl
  (JNIEnv * env, jobject object, jlong native_handle, jboolean is_disposing)
{
  CPPJNI_destroyManagedBuffer(env, object, native_handle, is_disposing);
}

} // extern "C"
