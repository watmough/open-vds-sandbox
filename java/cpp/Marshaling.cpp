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

#include "Marshaling.h"
#include "OpenVDS/OpenVDS.h"
#include "OpenVDS/VolumeData.h"
#include "OpenVDS/Exceptions.h"
#include <assert.h>

#ifdef _WIN32
#define ENABLE_DEBUGGER_ATTACH 1
#include <Windows.h>
#endif

static std::mutex
  s_FinalizerMutex;

thread_local std::stack<JNIEnv*>
JEnvPushPop::s_JNIEnvStack;

void 
JEnvPushPop::checkInit()
{
#ifdef ENABLE_DEBUGGER_ATTACH
  {
    static bool s_IsCancel;
    char buffer[256];
    *buffer = '\0';
    GetEnvironmentVariableA("OPENVDS_ENABLE_JAVA_ATTACH", buffer, sizeof(buffer));
    if (!s_IsCancel && *buffer && isdigit(*buffer))
    {
      int enable_attach = atoi(buffer);
      if (enable_attach)
      {
        s_IsCancel = IDCANCEL == MessageBoxA(0, "Attach now!", "OpenVDS Java API", MB_OKCANCEL);
      }
    }
  }
#endif

}
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
CPPJNIObjectContext::getSharedLibraryGeneration()
{ // The HueSpace shared library supports unloading and reloading within the same process.
  // Here, we just return 1.
  return 1;
}

// Ensure that the context wraps a resource allocated in the current shared library instance.
// If this is not the case, throw ObsoleteObjectException which in turn is caught by CPPJNI_CATCH
// thus making the outer function call a no-op.
void
CPPJNIObjectContext::ensureValid() const
{
  assert(this);
  if (m_MagicNumber != MAGIC_NUMBER)
  {
    throw std::runtime_error("Invalid CPPJNIObjectContext");
  }
  if (m_SharedLibraryGeneration != getSharedLibraryGeneration())
  {
    throw ObsoleteObjectException();
  }
}

CPPJNIFinalizerMutexGuard::CPPJNIFinalizerMutexGuard() : std::lock_guard<std::mutex>(s_FinalizerMutex)
{
}

CPPJNIFinalizerMutexGuard::~CPPJNIFinalizerMutexGuard()
{
}

enum class JavaExceptionType
{
  Exception,
  RuntimeException,
  IOException,
};

void
CPPJNI_Throw(struct JNIEnv_ *env, const char* message, JavaExceptionType exceptionType)
{
  const char* 
    ex = "java/lang/Exception";

  switch (exceptionType)
  {
  case JavaExceptionType::RuntimeException:
    ex = "java/lang/RuntimeException";
    break;
  case JavaExceptionType::IOException:
    ex = "java/io/IOException";
    break;
  default:
    break;
  }
  env->ThrowNew(env->FindClass(ex), message);
}

void 
CPPJNI_HandleStdException(struct JNIEnv_ *env, class std::exception &e)
{
  CPPJNI_Throw(env, e.what(), JavaExceptionType::Exception);
}

void 
CPPJNI_HandleStdRuntimeError(struct JNIEnv_ *env, class std::runtime_error &e)
{
  CPPJNI_Throw(env, e.what(), JavaExceptionType::RuntimeException);
}

void 
CPPJNI_HandleSharedLibraryException(struct JNIEnv_ *env, class OpenVDS::Exception &e)
{
  OpenVDS::Exception
    *pException = &e;

  if (dynamic_cast<OpenVDS::ReadErrorException*>(pException) != nullptr)
  {
    CPPJNI_Throw(env, e.what(), JavaExceptionType::IOException);
  }
  else
  {
    CPPJNI_Throw(env, e.what(), JavaExceptionType::Exception);
  }
}
