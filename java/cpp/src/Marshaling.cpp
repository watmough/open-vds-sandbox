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
#include <cassert>
#include <set>

#ifdef _WIN32
#define ENABLE_DEBUGGER_ATTACH 1
#include <Windows.h>
#endif

static std::mutex
  s_FinalizerMutex;

JavaVM*
  JNIEnvGuard::s_JavaVM;

thread_local std::stack<JNIEnv*>
  JNIEnvGuard::ts_JNIEnvStack;

thread_local std::vector<struct JNIEnvGuard::StringRecord>
  JNIEnvGuard::ts_TempStringRecords;

void
JNIEnvGuard::push(JNIEnv* env)
{
  assert(env);
  ts_JNIEnvStack.push(env);
}

JNIEnv*
JNIEnvGuard::top()
{
  assert(!ts_JNIEnvStack.empty());
  return ts_JNIEnvStack.top();
}

void
JNIEnvGuard::pop()
{
  assert(!ts_JNIEnvStack.empty());
  if (ts_JNIEnvStack.size() == 1) 
  {
    flushStrings();
  }
  ts_JNIEnvStack.pop();
}

const char* 
JNIEnvGuard::getStringUTFChars(jstring value) 
{
  assert(isJNIEnvValid());
  JNIEnv* env = getJNIEnv();
  const char* utf8 = env->GetStringUTFChars(value, nullptr);
  ts_TempStringRecords.emplace_back(StringRecord(value, utf8));
  return utf8;
}

void 
JNIEnvGuard::flushStrings() 
{
  assert(isJNIEnvValid());
  JNIEnv* env = getJNIEnv();
  for (auto r : ts_TempStringRecords)
  {
    env->ReleaseStringUTFChars(r.m_String, r.m_Utf8);
  }
  ts_TempStringRecords.clear();
}

JNIEnvGuard::JNIEnvGuard() : m_isThreadAttach(false)
{
  assert(s_JavaVM != nullptr);
  if (isJNIEnvValid())
  {
    push(getJNIEnv());
  }
  else
  {
    JNIEnv* env = nullptr;
    s_JavaVM->AttachCurrentThread((void**)&env, nullptr);
    if (env == nullptr) 
    {
      throw std::runtime_error("Unable to attach to JavaVM");
    }
    m_isThreadAttach = true;
    push(env);
  } 
}

JNIEnvGuard::JNIEnvGuard(JNIEnv* env) : m_isThreadAttach(false)
{
  checkInit(env);
  assert(env != NULL);
  push(env);
  assert(isJNIEnvValid());
}

JNIEnvGuard::~JNIEnvGuard()
{
  assert(isJNIEnvValid());
  pop();
  if (m_isThreadAttach)
  {
    assert(!isJNIEnvValid());
    s_JavaVM->DetachCurrentThread();
  }
}

bool 
JNIEnvGuard::isJNIEnvValid() 
{
  return !ts_JNIEnvStack.empty();
}

JNIEnv* 
JNIEnvGuard::getJNIEnv() {
  return top();
}

void 
JNIEnvGuard::checkInit(JNIEnv* env)
{
  if (s_JavaVM == nullptr)
  {
    env->GetJavaVM(&s_JavaVM);
  }
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

class GlobalStringContainer
{
public:
  static const char *
  intern(std::string const& descriptorString)
  {
    static std::set<std::string> s_descriptorStrings;
    static std::mutex s_mutex;
    std::unique_lock<std::mutex> lock(s_mutex);
    return s_descriptorStrings.insert(descriptorString).first->c_str();
  }
};

jstring 
CPPJNI_newString(JNIEnv * env, const char* str) 
{ 
  return env->NewStringUTF(str ? str : ""); 
}

jstring 
CPPJNI_newString(JNIEnv * env, std::string const& str) 
{ 
  return env->NewStringUTF(str.c_str()); 
}

std::string 
CPPJNI_getString(JNIEnv* env, jstring str)
{
  const char* utfChars = env->GetStringUTFChars(str, nullptr);
  auto result = std::string(utfChars);
  env->ReleaseStringUTFChars(str, utfChars);
  return result;
}

const char*
CPPJNI_internString(JNIEnv* env, jstring str)
{
  return GlobalStringContainer::intern(CPPJNI_getString(env, str));
}

jobject 
Marshaling::CreateJavaObject(const char* type) {
  jclass clazz = GetJNIEnv()->FindClass(type);
  if (clazz) {
    return CreateJavaObject(clazz);
  }
  return nullptr;
}

jobject 
Marshaling::CreateJavaObject(jclass clazz) {
  jmethodID methodID = GetJNIEnv()->GetMethodID(clazz, "<init>", "()V");
  if (methodID) {
    jobject obj = GetJNIEnv()->NewObject(clazz, methodID);
    return obj;
  }
  return nullptr;
}

jobjectArray 
Marshaling::CreateJavaArray(int elementCount, const char* elementType, jobject initialElement) 
{
  jclass clazz = GetJNIEnv()->FindClass(elementType ? elementType : "java/lang/Object");
  if (clazz) {
    auto arr = GetJNIEnv()->NewObjectArray(elementCount, clazz, initialElement);
    return arr;
  }
  return nullptr;
}

template<>
jobject 
Marshaling::CreatePODJavaObject<int>(int const& value) 
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

template<>
jobject 
Marshaling::CreatePODJavaObject<int64_t>(int64_t const& value) 
{
  jclass clazz = GetJNIEnv()->FindClass("java/lang/Long");
  jmethodID methodID = GetJNIEnv()->GetMethodID(clazz, "<init>", "(J)V");
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

JNIDirectBuffer::JNIDirectBuffer(jlong capacity) : m_Buffer(0), m_Memory(nullptr)
{
  m_Memory = malloc(capacity);
  if (m_Memory == nullptr)
  {
    throw std::bad_alloc();
  }
  m_Buffer = JNIDirectBuffer::CreateDirectBuffer(m_Memory, capacity);
}

JNIDirectBuffer::~JNIDirectBuffer()
{
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
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto directBuffer = new JNIDirectBuffer(capacity);
    auto context = new CPPJNIOwningObjectContext_t<JNIDirectBuffer>(directBuffer);
    auto native_handle = context->handle();
    jobjectArray arr = Marshaling::CreateJavaArray(2);
    env->SetObjectArrayElement(arr, 0, Marshaling::CreatePODJavaObject<jlong>(native_handle));
    env->SetObjectArrayElement(arr, 1, directBuffer->m_Buffer);
    directBuffer->m_Buffer = 0;
    return arr;
  }
  CPPJNI_CATCH
  return 0;
}

JNIEXPORT void JNICALL Java_org_opengroup_openvds_ManagedBuffer_dtorImpl
  (JNIEnv * env, jobject object, jlong native_handle, jboolean is_disposing)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    CPPJNI_destroyHandle<JNIDirectBuffer>(env, native_handle);
  }
  CPPJNI_CATCH
}

} // extern "C"
