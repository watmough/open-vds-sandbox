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

CPPJNIObjectContext::~CPPJNIObjectContext()
{
  cleanupGlobalRefs(JNIEnvGuard::JNIEnvGuard::getJNIEnv());
  assert(m_GlobalRefs.empty());
  assert(m_OpaqueObject == nullptr);
  for (char* str : m_AllocatedStrings)
  {
    free(str);
  }
}

std::stack<JNIEnv*>&
JNIEnvGuard::getJNIEnvStack()
{
  static thread_local std::stack<JNIEnv*> ts_JNIEnvStack;
  return ts_JNIEnvStack;
}

std::vector<struct JNIEnvGuard::StringRecord>&
JNIEnvGuard::getTempStringRecords()
{
  static thread_local std::vector<struct StringRecord> ts_TempStringRecords;
  return ts_TempStringRecords;
}

void
JNIEnvGuard::push(JNIEnv* env)
{
  assert(env);
  getJNIEnvStack().push(env);
}

JNIEnv*
JNIEnvGuard::top()
{
  std::stack<JNIEnv*>& jniEnvStack = getJNIEnvStack();
  assert(!jniEnvStack.empty());
  return jniEnvStack.top();
}

void
JNIEnvGuard::pop()
{
  std::stack<JNIEnv*>& jniEnvStack = getJNIEnvStack();
  assert(!jniEnvStack.empty());
  if (jniEnvStack.size() == 1)
  {
    flushStrings();
  }
  jniEnvStack.pop();
}

const char* 
JNIEnvGuard::getStringUTFChars(jstring value) 
{
  assert(isJNIEnvValid());
  JNIEnv* env = JNIEnvGuard::getJNIEnv();
  const char* utf8 = env->GetStringUTFChars(value, nullptr);
  getTempStringRecords().emplace_back(StringRecord(value, utf8));
  return utf8;
}

void 
JNIEnvGuard::flushStrings() 
{
  assert(isJNIEnvValid());
  JNIEnv* env = JNIEnvGuard::getJNIEnv();
  std::vector<JNIEnvGuard::StringRecord>& tempStringRecords = getTempStringRecords();
  for (auto r : tempStringRecords)
  {
    env->ReleaseStringUTFChars(r.m_String, r.m_Utf8);
  }
  tempStringRecords.clear();
}

JNIEnvGuard::JNIEnvGuard() : m_isThreadAttach(false)
{
  assert(s_JavaVM != nullptr);
  if (isJNIEnvValid())
  {
    push(JNIEnvGuard::getJNIEnv());
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
  return !getJNIEnvStack().empty();
}

JNIEnv* 
JNIEnvGuard::JNIEnvGuard::getJNIEnv() {
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
        s_IsCancel = IDCANCEL == MessageBoxA(0, "Attach now!", "CPPJNI Java CPP API Wrapper", MB_OKCANCEL);
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
CPPJNI_createJavaObject(const char* type) {
  jclass clazz = JNIEnvGuard::getJNIEnv()->FindClass(type);
  if (clazz) {
    return CPPJNI_createJavaObject(clazz);
  }
  return nullptr;
}

jobject 
CPPJNI_createJavaObject(jclass clazz) {
  jmethodID methodID = JNIEnvGuard::getJNIEnv()->GetMethodID(clazz, "<init>", "()V");
  if (methodID) {
    jobject obj = JNIEnvGuard::getJNIEnv()->NewObject(clazz, methodID);
    return obj;
  }
  return nullptr;
}

jobjectArray 
CPPJNI_createJavaArray(int elementCount, const char* elementType, jobject initialElement) 
{
  jclass clazz = JNIEnvGuard::getJNIEnv()->FindClass(elementType ? elementType : "java/lang/Object");
  if (clazz) {
    auto arr = JNIEnvGuard::getJNIEnv()->NewObjectArray(elementCount, clazz, initialElement);
    return arr;
  }
  return nullptr;
}

template<>
jobject 
CPPJNI_createPODJavaObject<int>(int const& value) 
{
  jclass clazz = JNIEnvGuard::getJNIEnv()->FindClass("java/lang/Integer");
  jmethodID methodID = JNIEnvGuard::getJNIEnv()->GetMethodID(clazz, "<init>", "(I)V");
  if (methodID)
  {
    jobject obj = JNIEnvGuard::getJNIEnv()->NewObject(clazz, methodID, value);
    return obj;
  }
  return nullptr;
}

template<>
jobject 
CPPJNI_createPODJavaObject<int64_t>(int64_t const& value) 
{
  jclass clazz = JNIEnvGuard::getJNIEnv()->FindClass("java/lang/Long");
  jmethodID methodID = JNIEnvGuard::getJNIEnv()->GetMethodID(clazz, "<init>", "(J)V");
  if (methodID)
  {
    jobject obj = JNIEnvGuard::getJNIEnv()->NewObject(clazz, methodID, value);
    return obj;
  }
  return nullptr;
}

// Ensure that the context wraps a resource allocated in the current shared library instance.
// If this is not the case, throw ObsoleteObjectException which in turn is caught by CPPJNI_CATCH
// thus making the outer function call a no-op.
CPPJNIObjectContext*
CPPJNIObjectContext::ensureValid(CPPJNIObjectContext* context)
{
  if (context == nullptr)
  {
    throw std::runtime_error("Null CPPJNIObjectContext");
  }
  else if (context->m_MagicNumber != MAGIC_NUMBER)
  {
    throw std::runtime_error("Invalid CPPJNIObjectContext");
  }
  else if (context->m_SharedLibraryGeneration != getSharedLibraryGeneration())
  {
    throw ObsoleteObjectException();
  }
  return context;
}

CPPJNIObjectContext*
CPPJNIObjectContext::ensureValid(jlong native_handle)
{
  return ensureValid((CPPJNIObjectContext*)native_handle);
}

CPPJNIFinalizerMutexGuard::CPPJNIFinalizerMutexGuard() : std::lock_guard<std::mutex>(s_FinalizerMutex)
{
}

CPPJNIFinalizerMutexGuard::~CPPJNIFinalizerMutexGuard()
{
}

void
CPPJNI_Throw(struct JNIEnv_ *env, const char* message, CPPJNIExceptionType exceptionType)
{
  if (env->ExceptionCheck())
  { // We don't want to override any pending exceptions
    return;
  }

  const char* 
    ex = "java/lang/Exception";

  switch (exceptionType)
  {
  case CPPJNIExceptionType::RuntimeException:
    ex = "java/lang/RuntimeException";
    break;
  case CPPJNIExceptionType::IOException:
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
  CPPJNI_Throw(env, e.what(), CPPJNIExceptionType::Exception);
}

void 
CPPJNI_HandleStdRuntimeError(struct JNIEnv_ *env, class std::runtime_error &e)
{
  CPPJNI_Throw(env, e.what(), CPPJNIExceptionType::RuntimeException);
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
  auto env = JNIEnvGuard::getJNIEnv();
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

/**
* 
* Returns an array of 2 objects: The native handle and the directbuffer object
* 
*/

jobjectArray 
CPPJNI_createManagedBuffer(JNIEnv * env, jclass clazz, jlong capacity)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto directBuffer = std::make_shared<JNIDirectBuffer>(capacity);
    auto context = CPPJNI_createObjectContext<JNIDirectBuffer>(directBuffer);
    auto native_handle = context->handle();
    jobjectArray arr = CPPJNI_createJavaArray(2);
    env->SetObjectArrayElement(arr, 0, CPPJNI_createPODJavaObject<jlong>(native_handle));
    env->SetObjectArrayElement(arr, 1, directBuffer->m_Buffer);
    directBuffer->m_Buffer = 0;
    return arr;
  }
  CPPJNI_CATCH
  return 0;
}

void 
CPPJNI_destroyManagedBuffer(JNIEnv * env, jobject object, jlong native_handle, jboolean is_disposing)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    CPPJNI_destroyHandle<JNIDirectBuffer>(native_handle, is_disposing);
  }
  CPPJNI_CATCH
}
