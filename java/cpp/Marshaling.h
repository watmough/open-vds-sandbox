//=============================================================================
// <copyright>
// Copyright (c) 2020 Bluware Inc. All rights reserved.
//
// All rights are reserved. Reproduction or transmission in whole or in part,
// in any form or by any means, electronic, mechanical or otherwise,
// is prohibited without the prior written permission of the copyright owner.
// </copyright>
//=============================================================================

#ifndef OPENVDS_JAVA_MARSHALING_H_INCLUDED
#define OPENVDS_JAVA_MARSHALING_H_INCLUDED

#pragma warning(disable : 4996) // Hide exception warning from VolumeDataAccess.h

#include <jni.h>
#include <stack>
#include <memory>
#include <mutex>
#include <assert.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <cstdint>

#define JAVA_WRAPPER_GENERATOR // Needed for some otherwise invisible accessors

#include "OpenVDS/Exceptions.h"
#include "OpenVDS/VolumeDataLayout.h"
#include "OpenVDS/VolumeDataAccess.h"
#include "OpenVDS/GlobalState.h"

#ifdef _MSC_VER
#define strdup _strdup
#endif

namespace OpenVDS {
class Exception;
class VolumeDataChannelDescriptor;
class VolumeDataLayoutDescriptor;
class VolumeDataAxisDescriptor;
struct IJKGridDefinition;
}

// Thrown by CPPJNI_cast if a managed object belongs to an obsolete library generation.
// This should only happen when non-deterministic finalization/garbage collection is attempted.
// Handled by CPPJNI_CATCH and treated as a no-op.
struct ObsoleteObjectException
{
};

template<typename T>
struct CPPJNIArrayAccessor;

template<>
struct CPPJNIArrayAccessor<int8_t>
{
  using element_type = jbyte;
  using array_type = jbyteArray;

  static element_type*  GetArrayElements(JNIEnv* env, jarray arr) { return (element_type*)env->GetByteArrayElements((array_type)arr, nullptr); }
  static void           ReleaseArrayElements(JNIEnv* env, jarray arr, element_type* data) { env->ReleaseByteArrayElements((array_type)arr, (element_type*)data, 0); }
  static array_type     CreateArray(JNIEnv* env, size_t len) { return env->NewByteArray((jsize)len); }
  static void           SetArrayElements(JNIEnv* env, array_type arr, element_type const* data, size_t len) { env->SetByteArrayRegion(arr, 0, (jsize)len, (element_type const*)data); }
};

template<>
struct CPPJNIArrayAccessor<int16_t>
{
  using element_type = jshort;
  using array_type = jshortArray;

  static element_type*  GetArrayElements(JNIEnv* env, jarray arr) { return (element_type*)env->GetShortArrayElements((array_type)arr, nullptr); }
  static void           ReleaseArrayElements(JNIEnv* env, jarray arr, element_type* data) { env->ReleaseShortArrayElements((array_type)arr, (element_type*)data, 0); }
  static array_type     CreateArray(JNIEnv* env, size_t len) { return env->NewShortArray((jsize)len); }
  static void           SetArrayElements(JNIEnv* env, array_type arr, element_type const* data, size_t len) { env->SetShortArrayRegion(arr, 0, (jsize)len, (element_type const*)data); }
};

template<>
struct CPPJNIArrayAccessor<int32_t>
{
  using element_type = jint;
  using array_type = jintArray;

  static element_type*  GetArrayElements(JNIEnv* env, jarray arr) { return (element_type*)env->GetIntArrayElements((array_type)arr, nullptr); }
  static void           ReleaseArrayElements(JNIEnv* env, jarray arr, element_type* data) { env->ReleaseIntArrayElements((array_type)arr, (element_type*)data, 0); }
  static array_type     CreateArray(JNIEnv* env, size_t len) { return env->NewIntArray((jsize)len); }
  static void           SetArrayElements(JNIEnv* env, array_type arr, element_type const* data, size_t len) { env->SetIntArrayRegion(arr, 0, (jsize)len, (element_type const*)data); }
};

template<>
struct CPPJNIArrayAccessor<int64_t>
{
  using element_type = jlong;
  using array_type = jlongArray;

  static element_type*  GetArrayElements(JNIEnv* env, jarray arr) { return (element_type*)env->GetLongArrayElements((array_type)arr, nullptr); }
  static void           ReleaseArrayElements(JNIEnv* env, jarray arr, element_type* data) { env->ReleaseLongArrayElements((array_type)arr, (element_type*)data, 0); }
  static array_type     CreateArray(JNIEnv* env, size_t len) { return env->NewLongArray((jsize)len); }
  static void           SetArrayElements(JNIEnv* env, array_type arr, element_type const* data, size_t len) { env->SetLongArrayRegion(arr, 0, (jsize)len, (element_type const*)data); }
};

template<>
struct CPPJNIArrayAccessor<float>
{
  using element_type = jfloat;
  using array_type = jfloatArray;

  static element_type*  GetArrayElements(JNIEnv* env, jarray arr) { return (element_type*)env->GetFloatArrayElements((array_type)arr, nullptr); }
  static void           ReleaseArrayElements(JNIEnv* env, jarray arr, element_type* data) { env->ReleaseFloatArrayElements((array_type)arr, (element_type*)data, 0); }
  static array_type     CreateArray(JNIEnv* env, size_t len) { return env->NewFloatArray((jsize)len); }
  static void           SetArrayElements(JNIEnv* env, array_type arr, element_type const* data, size_t len) { env->SetFloatArrayRegion(arr, 0, (jsize)len, (element_type const*)data); }
};

template<>
struct CPPJNIArrayAccessor<double>
{
  using element_type = jdouble;
  using array_type = jdoubleArray;

  static element_type*  GetArrayElements(JNIEnv* env, jarray arr) { return (element_type*)env->GetDoubleArrayElements((array_type)arr, nullptr); }
  static void           ReleaseArrayElements(JNIEnv* env, jarray arr, element_type* data) { env->ReleaseDoubleArrayElements((array_type)arr, (element_type*)data, 0); }
  static array_type     CreateArray(JNIEnv* env, size_t len) { return env->NewDoubleArray((jsize)len); }
  static void           SetArrayElements(JNIEnv* env, array_type arr, element_type const* data, size_t len) { env->SetDoubleArrayRegion(arr, 0, (jsize)len, (element_type const*)data); }
};

template<typename T>
struct CPPtoJNI_t
{
  using type = T;
};

template<> struct CPPtoJNI_t<uint8_t>            { using type = int8_t; };
template<> struct CPPtoJNI_t<char>               { using type = int8_t; };
//template<> struct CPPtoJNI_t<unsigned char>      { using type = int8_t; };

template<> struct CPPtoJNI_t<uint16_t>           { using type = int16_t; };
template<> struct CPPtoJNI_t<short>              { using type = int16_t; };
//template<> struct CPPtoJNI_t<unsigned short>     { using type = int16_t; };


template<> struct CPPtoJNI_t<uint32_t>           { using type = int32_t; };
template<> struct CPPtoJNI_t<int>                { using type = int32_t; };
//template<> struct CPPtoJNI_t<unsigned int>       { using type = int32_t; };
template<> struct CPPtoJNI_t<long>               { using type = int32_t; };
//template<> struct CPPtoJNI_t<unsigned long>      { using type = int32_t; };

template<> struct CPPtoJNI_t<uint64_t>           { using type = int64_t; };
template<> struct CPPtoJNI_t<long long>          { using type = int64_t; };
//template<> struct CPPtoJNI_t<unsigned long long> { using type = int64_t; };

// Adapter class for converting std::vector -> jArray
template<typename T>
struct CPPJNIVectorAdapter
{
  using JNITYPE = typename CPPtoJNI_t<T>::type;
  using ELEMENT_TYPE = typename CPPJNIArrayAccessor<JNITYPE>::element_type;
  using ARRAYTYPE = typename CPPJNIArrayAccessor<JNITYPE>::array_type;

  JNIEnv*               m_Env;
  std::vector<T> const& m_Vector;

  CPPJNIVectorAdapter(JNIEnv* env, std::vector<T> const& vec) : m_Env(env), m_Vector(vec)
  {
  }

  ARRAYTYPE
  toArray() const
  {
    auto arr = CPPJNIArrayAccessor<JNITYPE>::CreateArray(m_Env, m_Vector.size());
    CPPJNIArrayAccessor<JNITYPE>::SetArrayElements(m_Env, arr, (ELEMENT_TYPE const*)m_Vector.data(), m_Vector.size());
    return arr;
  }
};

template<typename T> T* CPPJNI_cast(jlong handle);

template<typename T>
struct CPPJNIVectorWrapperAdapter
{
  JNIEnv*                 m_Env;
  jlongArray              m_Array;
  mutable std::vector<T>  m_Vector;

  CPPJNIVectorWrapperAdapter(JNIEnv* env, jlongArray arr) : m_Env(env), m_Array(arr)
  {
  }

  OpenVDS::VectorWrapper<T>
  toVector() const
  {
    if (m_Vector.empty())
    {
      jlong* elements = m_Env->GetLongArrayElements(m_Array, nullptr);
      for (int i = 0; i < m_Env->GetArrayLength(m_Array); ++i)
      {
        auto item = CPPJNI_cast<T>(elements[i]);
        m_Vector.push_back(*item);
      }
      m_Env->ReleaseLongArrayElements(m_Array, elements, 0);
    }
    return OpenVDS::VectorWrapper<T>(m_Vector);
  }
};

// Adapter class to check N-component java arrays
template<typename T, int N, bool MUTABLE = false>
struct CPPJNIArrayAdapter;

template<typename T, int N>
struct CPPJNIArrayAdapter<T, N, false>
{
  using JNITYPE = typename CPPtoJNI_t<T>::type;
  using ELEMENT_TYPE = typename CPPJNIArrayAccessor<JNITYPE>::element_type;
  using ARRAYTYPE = typename CPPJNIArrayAccessor<JNITYPE>::array_type;

  std::vector<T> m_Data;

  CPPJNIArrayAdapter(JNIEnv *env, jarray arr)
  {
    if (arr)
    { // We copy out the values so we don't need to do any pinning.
      jsize len = env->GetArrayLength(arr);
      auto data = (T*)CPPJNIArrayAccessor<T>::GetArrayElements(env, arr);
      for (jsize i = 0; i < len; ++i)
      {
        m_Data.push_back(data[i]);
      }
      CPPJNIArrayAccessor<T>::ReleaseArrayElements(env, arr, (ELEMENT_TYPE*)data);
    }
    else
    {
      throw std::runtime_error("Null array reference.");
    }
  }

  size_t
  getArrayLength()
  {
    return m_Data.size() / N; // Since this is a flattened N-component array, we divide by N
  }

  T const*
  getArrayBuffer()
  {
    return m_Data.data();
  }

  T const (&getArray())[N]
  {
    auto tmp = getArrayBuffer();
    return *reinterpret_cast<T const (*)[N]>(tmp);
  }
};

template<typename T, int N>
struct CPPJNIArrayAdapter<T, N, true>
{
  using JNITYPE = typename CPPtoJNI_t<T>::type;
  using ELEMENT_TYPE = typename CPPJNIArrayAccessor<JNITYPE>::element_type;
  using ARRAYTYPE = typename CPPJNIArrayAccessor<JNITYPE>::array_type;

  JNIEnv *m_env;
  jarray m_Arr;
  std::vector<T> m_Data;

  CPPJNIArrayAdapter(JNIEnv *env, jarray arr)
  {
    m_env = env;
    m_Arr = arr;

    if (arr)
    { // We copy out the values so we don't need to do any pinning.
      jsize len = env->GetArrayLength(arr);
      if (len != N)
      {
        throw std::runtime_error("Array has incorrect length.");
      }
      auto data = (T*)CPPJNIArrayAccessor<T>::GetArrayElements(env, arr);
      for (jsize i = 0; i < len; ++i)
      {
        m_Data.push_back(data[i]);
      }
      CPPJNIArrayAccessor<T>::ReleaseArrayElements(env, arr, (ELEMENT_TYPE*)data);
    }
    else
    {
      throw std::runtime_error("Null array reference.");
    }
  }

  ~CPPJNIArrayAdapter()
  {
    CPPJNIArrayAccessor<T>::SetArrayElements(m_env, (ARRAYTYPE)m_Arr, (ELEMENT_TYPE*)m_Data.data(), m_Data.size());
  }

  size_t
  getArrayLength()
  {
    return m_Data.size() / N; // Since this is a flattened N-component array, we divide by N
  }

  T *
  getArrayBuffer()
  {
    return m_Data.data();
  }

  T (&getArray())[N]
  {
    auto tmp = getArrayBuffer();
    return *reinterpret_cast<T (*)[N]>(tmp);
  }
};

// This class is used for asynchronous buffer-filling operations.
// Users must keep a global handle to ensure the target buffer is not
// garbage collected before it is written to.
template<typename T>
struct CPPJNIAsyncBuffer
{
  size_t    m_BufferSize;
  size_t    m_Offset;
  uint8_t*  m_Buffer;

  CPPJNIAsyncBuffer(JNIEnv* env, jobject bytebuffer, jlong byteoffset = 0) : m_BufferSize(), m_Offset(byteoffset), m_Buffer()
  {
    if (byteoffset < 0)
    {
      throw std::runtime_error("Negative ByteBuffer offset.");
    }
    m_BufferSize = env->GetDirectBufferCapacity(bytebuffer);
    if (m_Offset > m_BufferSize)
    {
      throw std::runtime_error("ByteBuffer offset greater than capacity.");
    }
    m_Buffer = (uint8_t*)env->GetDirectBufferAddress(bytebuffer);
  }

  T*      
  buffer()    
  { 
    return (T*)(m_Buffer + m_Offset); 
  }

  size_t  
  byteSize()  
  { 
    return m_BufferSize - m_Offset; 
  }
};

// This class is used for POD value types backed by a java.nio.ByteBuffer
// It implements implicit conversion to the target type.
template<typename T>
struct CPPJNIByteBufferAdapter
{
  size_t  m_DataSize;
  T*      m_Data;

  CPPJNIByteBufferAdapter(JNIEnv* env, jobject bytebuffer, jlong byteoffset) : m_DataSize(), m_Data()
  {
    m_DataSize = env->GetDirectBufferCapacity(bytebuffer);
    if (m_DataSize < sizeof(T))
    {
      throw std::runtime_error("ByteBuffer too small to hold element T");
    }
    if (byteoffset < 0)
    {
      throw std::runtime_error("Negative ByteBuffer offset.");
    }
    if ((size_t)byteoffset + sizeof(T) > m_DataSize)
    {
      throw std::runtime_error("ByteBuffer offset greater than capacity.");
    }
    m_Data = (T*)((char*)env->GetDirectBufferAddress(bytebuffer) + byteoffset);
  }

  operator T const& () const { return *m_Data; }
};


struct CPPJNIObjectContext
{
  constexpr static const uint64_t MAGIC_NUMBER = 0x1234567876543210;

  uint64_t  m_MagicNumber;
  void*     m_OpaqueObject;

  std::vector<char*> m_AllocatedStrings;
  std::vector<jobject> m_GlobalRefs;
  int m_SharedLibraryGeneration;

  CPPJNIObjectContext(void* object) : m_MagicNumber(MAGIC_NUMBER), m_OpaqueObject(object), m_SharedLibraryGeneration(getSharedLibraryGeneration())
  {
  }
  
  static int  getSharedLibraryGeneration();
  void        ensureValid() const;

  // Add a global reference to an external java object to ensure it stays alive until
  // the ObjectContex is deleted.
  // NOTE: cleanupGlobalRefs() is NOT called from the dtor, since it needs
  // the JNIEnv to perform its duty.
  void
  registerGlobalRef(JNIEnv* env, jobject obj)
  {
    m_GlobalRefs.push_back(env->NewGlobalRef(obj));
  }

  // Delete all global references to external objects.
  // NOTE: cleanupGlobalRefs() is NOT called from the dtor, since it needs
  // the JNIEnv to perform its duty.
  void
  cleanupGlobalRefs(JNIEnv* env)
  {
    for (auto gref : m_GlobalRefs)
    {
      env->DeleteGlobalRef(gref);
    }
    m_GlobalRefs.clear();
  }

  virtual ~CPPJNIObjectContext()
  {
    assert(m_GlobalRefs.empty());
    assert(m_OpaqueObject == nullptr);
    for (char* str : m_AllocatedStrings)
    {
      free(str);
    }
  }

  const char*
  addString(const char* contents)
  {
    static const char
      *empty = "";

    if (contents && *contents)
    {
      auto copy = strdup(contents);
      m_AllocatedStrings.push_back(copy);
      return copy;
    }
    else
    {
      return empty;
    }
  }
};

template<typename T, bool IS_DESTRUCTIBLE = true>
struct Destroyer
{
  static void destroy(T* instance) { delete instance; }
};

template<typename T>
struct Destroyer<T, false>
{
  static void destroy(T* instance) { }
};

template<>
struct Destroyer<OpenVDS::VDS, true>
{
  static void 
  destroy(OpenVDS::VDS* instance) 
  { 
    OpenVDS::Close(instance); 
  }
};

template<typename T>
struct CPPJNIObjectContext_t : public CPPJNIObjectContext
{
  bool m_IsOwner;
  std::shared_ptr<T> m_SharedPtr;

  CPPJNIObjectContext_t() : CPPJNIObjectContext_t(nullptr, false)
  {
  }

  CPPJNIObjectContext_t(T* object) : CPPJNIObjectContext_t(object, true)
  {
  }

  CPPJNIObjectContext_t(std::shared_ptr<T> sharedPtr) : CPPJNIObjectContext_t(sharedPtr.get(), false, sharedPtr)
  {
  }

  CPPJNIObjectContext_t(T* object, bool isOwner, std::shared_ptr<T> sharedPtr = std::shared_ptr<T>()) : CPPJNIObjectContext(object), m_IsOwner(isOwner), m_SharedPtr(sharedPtr)
  {
  }

  ~CPPJNIObjectContext_t() override
  {
    auto object = (T*)m_OpaqueObject;
    m_OpaqueObject = nullptr;
  }

  void
  setObject(T* object, bool owned = true)
  {
    m_OpaqueObject = object;
    m_IsOwner = owned;
  }

  T* 
  getObject() 
  {
    return (T*)m_OpaqueObject;
  }

  jlong 
  handle()
  {
    return (jlong)this;
  }

};

template<typename T>
struct CPPJNIOwningObjectContext_t : public CPPJNIObjectContext_t<T> 
{
  CPPJNIOwningObjectContext_t(T* object) : CPPJNIObjectContext_t<T>(object, true)
  {
  }

  ~CPPJNIOwningObjectContext_t() override
  {
    auto object = (T*)this->getObject();
    Destroyer<T, std::is_destructible<T>::value>::destroy(object);
  }
};

template<typename T>
CPPJNIObjectContext_t<T>*
CPPJNI_createObjectContext(T* pNativeObject)
{
  return new CPPJNIOwningObjectContext_t<T>(pNativeObject);
}

template<typename T>
CPPJNIObjectContext_t<T>*
CPPJNI_createObjectContext(std::shared_ptr<T> pNativeObject)
{
  return new CPPJNIObjectContext_t<T>(pNativeObject);
}

template<typename T>
CPPJNIObjectContext_t<T>*
CPPJNI_createNonOwningObjectContext(T const* pNativeObject)
{
  return new CPPJNIObjectContext_t<T>((T*)pNativeObject, false);
}

template<typename T>
T*
CPPJNI_cast(jlong handle)
{
  if (handle == 0)
  {
    throw std::runtime_error("null handle");
  }
  auto pContext = ((CPPJNIObjectContext_t<T>*)(handle));
  pContext->ensureValid(); // May throw 
  return pContext->getObject();
}

struct CPPJNIFinalizerMutexGuard : std::lock_guard<std::mutex>
{
  CPPJNIFinalizerMutexGuard();
  ~CPPJNIFinalizerMutexGuard();
};

template<typename T>
void
CPPJNI_destroyHandle(JNIEnv* env, jlong handle)
{
  auto pContext = ((CPPJNIObjectContext_t<T>*)(handle));
  pContext->ensureValid(); // May throw 
  pContext->cleanupGlobalRefs(env);
  delete pContext;
}

inline jstring CPPJNI_newString(JNIEnv * env, const char* str) { return env->NewStringUTF(str ? str : ""); }
inline jstring CPPJNI_newString(JNIEnv * env, std::string const& str) { return env->NewStringUTF(str.c_str()); }

class Marshaling;

void CPPJNI_HandleSharedLibraryException(JNIEnv* env, OpenVDS::Exception& e);
void CPPJNI_HandleStdRuntimeError(JNIEnv *env, std::runtime_error& e);
void CPPJNI_HandleStdException(JNIEnv *env, std::exception& e);

#define CPPJNI_TRY try
#define CPPJNI_CATCH \
 catch(ObsoleteObjectException&) { /* No-op. See comment for ObsoleteObjectException */ } \
 catch(OpenVDS::Exception& e) { CPPJNI_HandleSharedLibraryException(env, e); } \
 catch(std::runtime_error& e) { CPPJNI_HandleStdRuntimeError(env, e); } \
 catch(std::exception& e) { CPPJNI_HandleStdException(env, e); }

class JEnvPushPop
{
  friend class Marshaling;

  static thread_local std::stack<JNIEnv*>
    s_JNIEnvStack;

  static void checkInit();

public:
  JEnvPushPop(JNIEnv* env)
  {
    checkInit();
    assert(env != NULL);
    s_JNIEnvStack.push(env);
  }

  ~JEnvPushPop()
  {
    FlushStrings();
    s_JNIEnvStack.pop();
  }

  const char* GetStringUTFChars(jstring value) {
    jboolean is_copy = false;
    assert(!s_JNIEnvStack.empty());
    JNIEnv* env = s_JNIEnvStack.top();
    const char* utf8 = env->GetStringUTFChars(value, &is_copy);
    m_lUtf8Chars.push_back(StringRecord(env, value, utf8));
    return utf8;
  }

  void FlushStrings() {
    for (size_t i = 0; i < m_lUtf8Chars.size(); ++i) {
      m_lUtf8Chars[i].m_Env->ReleaseStringUTFChars(m_lUtf8Chars[i].m_String, m_lUtf8Chars[i].m_Utf8);
    }
    m_lUtf8Chars.clear();
  }

private:
  struct StringRecord {
    StringRecord(JNIEnv* env, jstring str, const char* utf8) : m_Env(env), m_String(str), m_Utf8(utf8) {
      assert(env);
      assert(str);
      assert(utf8);
    }
    JNIEnv*     m_Env;
    jstring     m_String;
    const char* m_Utf8;
  };

  std::vector<StringRecord> m_lUtf8Chars;
};

struct CPPJNIStringWrapper
{
  JNIEnv *              m_Env;
  jlong                 m_NativeHandle;
  jstring               m_JString;
  mutable const char *  m_TmpUtf8;
  mutable const char *  m_PersistentUTF8;

  CPPJNIStringWrapper(JNIEnv * env, jlong native_handle, jstring str) : m_Env(env), m_NativeHandle(native_handle), m_JString(str), m_TmpUtf8(), m_PersistentUTF8()
  {
  }

  CPPJNIStringWrapper(JNIEnv * env, jstring str) : CPPJNIStringWrapper(env, 0, str)
  {
  }

  ~CPPJNIStringWrapper()
  {
    if (m_TmpUtf8)
    {
      m_Env->ReleaseStringUTFChars(m_JString, m_TmpUtf8);
    }
  }

  const char*
  utf8() const
  {
    if (m_PersistentUTF8 == nullptr)
    {
      if (m_NativeHandle)
      {
        m_TmpUtf8 = m_Env->GetStringUTFChars(m_JString, 0);
        auto pObjectContext = (CPPJNIObjectContext*)m_NativeHandle;
        m_PersistentUTF8 = pObjectContext->addString(m_TmpUtf8);
      }
      else
      { // This will currently leak... Could be solved by an interning mechanism or deferred registration?
        m_PersistentUTF8 = m_Env->GetStringUTFChars(m_JString, 0);
      }
    }
    return m_PersistentUTF8;
  }

  operator const char* () const
  {
    return utf8();
  }
};

class Marshaling {
public:
  static jobject CreateJavaObject(const char* type);
  static jobject CreateJavaObject(jclass clazz);

  static JNIEnv* GetJNIEnv() {
    assert(!JEnvPushPop::s_JNIEnvStack.empty());
    return JEnvPushPop::s_JNIEnvStack.top();
  }

  static bool IsJavaContextActive() {
    return !JEnvPushPop::s_JNIEnvStack.empty();
  }

  static void Convert(jobject& to, OpenVDS::VolumeDataChannelDescriptor const& from);
  static void Convert(OpenVDS::VolumeDataChannelDescriptor& to, jobject from);

  static void Convert(jobject& to, OpenVDS::VolumeDataLayoutDescriptor const& from);
  static void Convert(OpenVDS::VolumeDataLayoutDescriptor& to, jobject from);

  static void Convert(jobject& to, OpenVDS::VolumeDataAxisDescriptor const& from);
  static void Convert(OpenVDS::VolumeDataAxisDescriptor& to, jobject from);

  static void Convert(jobject& to, OpenVDS::IJKGridDefinition const& from);
  static void Convert(OpenVDS::IJKGridDefinition& to, jobject from);
};

template<typename T>
inline void
ReverseEndianness(char* data)
{
  for(int iElement = 0; iElement < T::element_count; ++iElement)
  {
    char* left = data + iElement * sizeof(T::element_type);
    char* right = left + sizeof(T::element_type) - 1;
    size_t middle = sizeof(T::element_type) / 2;
    for (size_t iByte = 0; iByte < middle; ++iByte)
    {
      char tmp = *right;
      *right = *left;
      *left = tmp;
      ++left;
      --right;
    }
  }
}

#endif
