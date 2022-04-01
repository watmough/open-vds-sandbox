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
  return nullptr;
}

jobject Marshaling::CreateJavaObject(jclass clazz) {
  jmethodID methodID = GetJNIEnv()->GetMethodID(clazz, "<init>", "()V");
  if (methodID) {
    jobject obj = GetJNIEnv()->NewObject(clazz, methodID);
    return obj;
  }
  return nullptr;
}

jobjectArray Marshaling::CreateJavaArray(int elementCount, const char* elementType, jobject initialElement) 
{
  jclass clazz = GetJNIEnv()->FindClass(elementType ? elementType : "java/lang/Object");
  if (clazz) {
    auto arr = GetJNIEnv()->NewObjectArray(elementCount, clazz, initialElement);
    return arr;
  }
  return nullptr;
}

template<>
jobject Marshaling::CreatePODJavaObject<int>(int const& value) 
{
  jclass clazz = GetJNIEnv()->FindClass("java/lang/Integer");
  jmethodID methodID = GetJNIEnv()->GetMethodID(clazz, "<init>", "(I)V");
  if (methodID)
  {
    jobject obj = GetJNIEnv()->NewObject(clazz, methodID, value);
    return obj;
  }
  return nullptr;
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
  if (env->ExceptionCheck())
  { // We don't want to override any pending exceptions
    return;
  }

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

std::string 
JStringToString(JNIEnv* env, jstring str)
{
  const char* utfChars = env->GetStringUTFChars(str, nullptr);
  auto result = std::string(utfChars);
  env->ReleaseStringUTFChars(str, utfChars);
  return result;
}

JNIDirectBuffer::JNIDirectBuffer(jlong capacity) : m_Buffer(0), m_Memory(nullptr)
{
  m_Memory = malloc(capacity);
  if (m_Memory == nullptr)
  {
    throw std::bad_alloc();
  }
  jobject tmp = JNIDirectBuffer::CreateDirectBuffer(m_Memory, capacity);
  if (tmp)
  {
    m_Buffer = Marshaling::GetJNIEnv()->NewGlobalRef(tmp);
  }
}

JNIDirectBuffer::~JNIDirectBuffer()
{
  DeleteBufferGlobalRef();
  free(m_Memory);
}

// Create a new DirectByteBuffer with native byte order.
jobject
JNIDirectBuffer::CreateDirectBuffer(void* mem, jlong capacity) 
{
  auto env = Marshaling::GetJNIEnv();
  jobject tmp = env->NewDirectByteBuffer(mem, capacity);
  CPPJNI_ensureNotNull(tmp, "Failed to create DirectByteBuffer");

  // Call "ByteOrder.nativeOrder()"
  jclass byteOrderClass = env->FindClass("java/nio/ByteOrder");
  CPPJNI_ensureNotNull(byteOrderClass);
  jmethodID nativeOrdermethodID = env->GetStaticMethodID(byteOrderClass, "nativeOrder", "()Ljava/nio/ByteOrder;");
  CPPJNI_ensureNotNull(nativeOrdermethodID);
  jobject nativeOrder = env->CallStaticObjectMethod(byteOrderClass, nativeOrdermethodID);
  CPPJNI_ensureNotNull(nativeOrder);

  // Call order() with the ByteOrder obtained in the previous step to ensure native byte order.
  jclass directByteBufferClass = env->GetObjectClass(tmp);
  CPPJNI_ensureNotNull(directByteBufferClass);
  jmethodID orderMethodID = env->GetMethodID(directByteBufferClass, "order", "(Ljava/nio/ByteOrder;)Ljava/nio/ByteBuffer;");
  CPPJNI_ensureNotNull(orderMethodID);
  jobject byteBuffer = env->CallObjectMethod(tmp, orderMethodID, nativeOrder);
  CPPJNI_ensureNotNull(byteBuffer);
  return byteBuffer;
}

jobject
JNIDirectBuffer::GetBufferGlobalRef()
{
  return m_Buffer;
}

  void
JNIDirectBuffer::DeleteBufferGlobalRef()
{
  if (m_Buffer)
  {
    jobject buffer = m_Buffer;
    m_Buffer = 0;
    Marshaling::GetJNIEnv()->DeleteGlobalRef(buffer);
  }
}

extern "C" {


/**
* The Java ManagedBuffer class inherits from ManagedBase and its constructor
* calls the base class constructor with the native handle. To keep the
* DirectByteBuffer alive, we create a GlobalRef to it, before it is
* read out from the java side and finally the GlobalRef is destroyed so
* that it won't be kept alive artificially. Seems cumbersome, but is there
* a better way?
* 
*/

JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_ManagedBuffer_ctorImpl
  (JNIEnv * env, jclass clazz, jlong capacity)
{
  JEnvPushPop
    stackitem(env);

  CPPJNI_TRY
  {
    auto directBuffer = new JNIDirectBuffer(capacity);
    auto context = new CPPJNIOwningObjectContext_t<JNIDirectBuffer>(directBuffer);
    auto native_handle = context->handle();
    return native_handle;
  }
  CPPJNI_CATCH
  return 0;
}

JNIEXPORT jobject JNICALL Java_org_opengroup_openvds_ManagedBuffer_getBufferRefImpl
  (JNIEnv * env, jobject object, jlong native_handle, jboolean is_disposing)
{
  JEnvPushPop
    stackitem(env);

  CPPJNI_TRY
  {
    auto buffer = CPPJNI_cast<JNIDirectBuffer>(native_handle);
    return buffer->GetBufferGlobalRef();
  }
  CPPJNI_CATCH
  return 0;
}

JNIEXPORT void JNICALL Java_org_opengroup_openvds_ManagedBuffer_deleteBufferRefImpl
  (JNIEnv * env, jobject object, jlong native_handle, jboolean is_disposing)
{
  JEnvPushPop
    stackitem(env);

  CPPJNI_TRY
  {
    auto buffer = CPPJNI_cast<JNIDirectBuffer>(native_handle);
    buffer->DeleteBufferGlobalRef();
  }
  CPPJNI_CATCH
}

JNIEXPORT void JNICALL Java_org_opengroup_openvds_ManagedBuffer_dtorImpl
  (JNIEnv * env, jobject object, jlong native_handle, jboolean is_disposing)
{
  JEnvPushPop
    stackitem(env);

  CPPJNI_TRY
  {
    CPPJNI_destroyHandle<JNIDirectBuffer>(env, native_handle);
  }
  CPPJNI_CATCH
}

}

