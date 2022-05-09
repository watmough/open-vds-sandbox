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

// Thrown by CPPJNIObjectContext if the object parameter is null.
// This is to prevent an invalid object context to be created.
// Handled by CPPJNI_CATCH and treated as a no-op.
struct ObjectNullException
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
    m_Buffer = (uint8_t*)env->GetDirectBufferAddress(bytebuffer);
    if (m_Buffer == nullptr) 
    {
      throw std::runtime_error("No ByteBuffer direct access");
    }
    if (byteoffset < 0)
    {
      throw std::runtime_error("Negative ByteBuffer offset.");
    }
    m_BufferSize = env->GetDirectBufferCapacity(bytebuffer);
    if (m_Offset > m_BufferSize)
    {
      throw std::runtime_error("ByteBuffer offset greater than capacity.");
    }
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

// Polymorphic weak_ptr wrapper
struct CPPJNIWeakPtrWrapper
{
protected:
  virtual void _lock(void* ptrToSharedPtr) const = 0;
public:
  virtual ~CPPJNIWeakPtrWrapper() {}

  template<typename T>
  std::shared_ptr<T>
  lock() const
  {
    std::shared_ptr<T> tmp;
    _lock(&tmp);
    return tmp;
  }
};

template<typename T>
struct CPPJNIWeakPtrWrapper_t : public CPPJNIWeakPtrWrapper
{
  std::weak_ptr<T> m_WeakPtr;

  CPPJNIWeakPtrWrapper_t(std::shared_ptr<T> sharedPtr) 
  {
    m_WeakPtr = sharedPtr;
  }

  void _lock(void* ptrToSharedPtr) const override
  {
    auto shared = reinterpret_cast<std::shared_ptr<T>*>(ptrToSharedPtr);
    *shared = m_WeakPtr.lock();
  }
};

struct CPPJNIObjectContext
{
  constexpr static const uint64_t MAGIC_NUMBER = 0x1234567876543210;

  uint64_t  m_MagicNumber;
  void*     m_OpaqueObject;

  std::vector<char*> m_AllocatedStrings;
  std::vector<jobject> m_GlobalRefs;
  std::unique_ptr<CPPJNIWeakPtrWrapper> m_Manager;

  int m_SharedLibraryGeneration;

  CPPJNIObjectContext() : m_MagicNumber(MAGIC_NUMBER), m_OpaqueObject(nullptr), m_SharedLibraryGeneration(getSharedLibraryGeneration())
  {
  }

  CPPJNIObjectContext(void* object) : m_MagicNumber(MAGIC_NUMBER), m_OpaqueObject(object), m_SharedLibraryGeneration(getSharedLibraryGeneration())
  {
    if (object == nullptr) 
    {
      throw ObjectNullException();
    }
  }
  
  virtual ~CPPJNIObjectContext();

  static int  getSharedLibraryGeneration();
  static CPPJNIObjectContext* ensureValid(CPPJNIObjectContext* context);
  static CPPJNIObjectContext* ensureValid(jlong native_handle);

  // Create a polymorphic weak pointer
  template<typename T>
  void        
  setManager(std::shared_ptr<T> manager)
  {
    if (manager.get() == nullptr)
    {
      throw std::runtime_error("Cannot set null manager.");
    }
    m_Manager = std::make_unique<CPPJNIWeakPtrWrapper_t<T>>(manager);
  }

  template<typename T>
  void        
  setCreator(std::shared_ptr<T> creator)
  {
    if (creator.get() == nullptr)
    {
      throw std::runtime_error("Cannot set null creator.");
    }
    m_Manager = std::make_unique<CPPJNIWeakPtrWrapper_t<T>>(creator);
  }


  template<typename T>
  std::shared_ptr<T>
  getManager() const
  {
    CPPJNIWeakPtrWrapper* wrapper = m_Manager.get();
    if (wrapper != nullptr)
    {
      auto result = wrapper->lock<T>();
      if (result.get() != nullptr)
      {
        return result;
      }
    }
    throw std::runtime_error("Object has no manager");
  }

  template<typename T>
  std::shared_ptr<T>
  getCreator() const
  {
    CPPJNIWeakPtrWrapper* wrapper = m_Manager.get();
    if (wrapper != nullptr)
    {
      auto result = wrapper->lock<T>();
      if (result.get() != nullptr)
      {
        return result;
      }
    }
    throw std::runtime_error("Object has no creator");
  }

  void
  registerGlobalRef(JNIEnv* env, jobject obj)
  {
    m_GlobalRefs.push_back(env->NewGlobalRef(obj));
  }

  void
  cleanupGlobalRefs(JNIEnv* env)
  {
    for (auto gref : m_GlobalRefs)
    {
      env->DeleteGlobalRef(gref);
    }
    m_GlobalRefs.clear();
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

void CPPJNI_onVDSError(OpenVDS::VDSError const& error);

template<typename T>
struct Cleaner
{
  static void cleanup(struct CPPJNIObjectContext& context, std::shared_ptr<T> instancePtr, bool is_disposing) { }
};

template<typename T>
std::shared_ptr<T>
CPPJNI_createSharedPtr(T* instance)
{
  return std::shared_ptr<T>(instance);
}

template<typename T>
std::shared_ptr<T>
CPPJNI_createSharedPtrNoDelete(T* instance)
{
  return std::shared_ptr<T>(instance, [](T*){});
}

template<class T, class... Args>
std::shared_ptr<T>
CPPJNI_makeShared(Args&&... args)
{
  T* object = new T(args...);
  return CPPJNI_createSharedPtr<T>(object);
}

#define CPPJNI_SPECIALIZE_SHAREDPTR_NODELETE(TYPENAME)          \
template<>                                                      \
inline std::shared_ptr<TYPENAME>                                       \
CPPJNI_createSharedPtr<TYPENAME>(TYPENAME* instance)            \
{                                                               \
  return CPPJNI_createSharedPtrNoDelete<TYPENAME>(instance);    \
}

CPPJNI_SPECIALIZE_SHAREDPTR_NODELETE(OpenVDS::VDS)
CPPJNI_SPECIALIZE_SHAREDPTR_NODELETE(OpenVDS::VolumeDataPageAccessor)

template<>
struct Cleaner<OpenVDS::VDS>
{
  static void 
  cleanup(struct CPPJNIObjectContext& context, std::shared_ptr<OpenVDS::VDS> instancePtr, bool is_disposing) 
  { 
    OpenVDS::VDSError error;
    OpenVDS::Close(instancePtr.get(), error); 
    if (error.code) 
    {
      CPPJNI_onVDSError(error);
    }
  }
};

template<>
struct Cleaner<OpenVDS::VolumeDataPage>
{
  static void 
  cleanup(struct CPPJNIObjectContext& context, std::shared_ptr<OpenVDS::VolumeDataPage> instancePtr, bool is_disposing) 
  { 
    if (is_disposing)
    {
      instancePtr.get()->Release();
    }
    //else
    //{
    //  int debug = 0;
    //}
  }
};

template<>
struct Cleaner<OpenVDS::VolumeDataPageAccessor>
{
  static void 
  cleanup(struct CPPJNIObjectContext& context, std::shared_ptr<OpenVDS::VolumeDataPageAccessor> instancePtr, bool is_disposing) 
  { 
    if (is_disposing)
    {
      instancePtr.get()->Commit();
      auto creator = context.getCreator<OpenVDS::VolumeDataAccessManager>(); // May throw
      creator->DestroyVolumeDataPageAccessor(instancePtr.get());
    }
    //else
    //{
    //  int debug = 0;
    //}
  }
};

template<typename T>
struct CPPJNIObjectContext_t : public CPPJNIObjectContext
{
  bool m_IsOwner;
  std::shared_ptr<T> m_SharedPtr;

  CPPJNIObjectContext_t(T* object, bool isOwner, std::shared_ptr<T> sharedPtr = std::shared_ptr<T>()) : CPPJNIObjectContext(object), m_IsOwner(isOwner), m_SharedPtr(sharedPtr)
  {
  }

  CPPJNIObjectContext_t() : m_IsOwner(false)
  {
  }

  CPPJNIObjectContext_t(T* object) : CPPJNIObjectContext_t(object, false)
  {
  }

  CPPJNIObjectContext_t(std::shared_ptr<T> sharedPtr) : CPPJNIObjectContext_t(sharedPtr.get(), true, sharedPtr)
  {
    if (sharedPtr.get() == nullptr) 
    {
      throw std::runtime_error("Cannot create object context from empty shared_ptr");
    }
  }

  ~CPPJNIObjectContext_t() override
  {
    if (m_SharedPtr.get()) 
    {
      Cleaner<T> cleaner;
      cleaner.cleanup(*this, m_SharedPtr, false);
    }
    m_OpaqueObject = nullptr;
  }


  static CPPJNIObjectContext_t<T>*
  ensureValid(jlong native_handle)
  {
    auto context = CPPJNIObjectContext::ensureValid(native_handle);
    auto real_context = reinterpret_cast<CPPJNIObjectContext_t<T>*>(context);
    return real_context;
  }

  void
  setObject(std::shared_ptr<T> object)
  {
    if (object.get() == nullptr)
    {
      throw std::runtime_error("cannot set null opaque object");
    }
    m_SharedPtr = object;
    m_OpaqueObject = object.get();
    m_IsOwner = true;
  }

  std::weak_ptr<T>
  getWeakPtr()
  {
    if (m_SharedPtr.get() == nullptr) 
    {
      throw std::runtime_error("cannot create weak_ptr to null object");
    }
    return std::weak_ptr<T>(m_SharedPtr);
  }

  T* 
  getObject() 
  {
    if (m_OpaqueObject == nullptr) 
    {
      throw std::runtime_error("opaque object is null");
    }
    return (T*)m_OpaqueObject;
  }

  jlong 
  handle()
  {
    return (jlong)this;
  }

};

template<typename T>
CPPJNIObjectContext_t<T>*
CPPJNI_createObjectContext()
{
  return new CPPJNIObjectContext_t<T>();
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
  return new CPPJNIObjectContext_t<T>(CPPJNI_createSharedPtrNoDelete<T>((T*)pNativeObject));
}

template<typename T, typename CREATOR_TYPE>
CPPJNIObjectContext_t<T>*
CPPJNI_createNonOwningObjectContext(T const* pNativeObject, jlong creator_native_handle, CREATOR_TYPE* creator)
{
  auto context = new CPPJNIObjectContext_t<T>(CPPJNI_createSharedPtrNoDelete<T>((T*)pNativeObject));
  auto creatorContext = CPPJNIObjectContext_t<CREATOR_TYPE>::ensureValid(creator_native_handle);
  context->setManager(creatorContext->m_SharedPtr);
  return context;
}

template<typename T>
T*
CPPJNI_cast(jlong handle)
{
  auto pContext = CPPJNIObjectContext_t<T>::ensureValid(handle); // May throw
  return pContext->getObject();
}

struct CPPJNIFinalizerMutexGuard : std::lock_guard<std::mutex>
{
  CPPJNIFinalizerMutexGuard();
  ~CPPJNIFinalizerMutexGuard();
};

template<typename T>
void
CPPJNI_destroyHandle(jlong handle, bool is_disposing)
{
  auto pContext = CPPJNIObjectContext_t<T>::ensureValid(handle); // May throw
  if (pContext->m_SharedPtr.get())
  {
    Cleaner<T> cleaner;
    cleaner.cleanup(*pContext, pContext->m_SharedPtr, is_disposing);
    pContext->m_SharedPtr.reset();
  }
  delete pContext;
}

jstring     CPPJNI_newString(JNIEnv * env, const char* str);
jstring     CPPJNI_newString(JNIEnv * env, std::string const& str);
std::string CPPJNI_getString(JNIEnv* env, jstring str);
const char* CPPJNI_internString(JNIEnv* env, jstring str);

class Marshaling;

void CPPJNI_HandleSharedLibraryException(JNIEnv* env, OpenVDS::Exception& e);
void CPPJNI_HandleStdRuntimeError(JNIEnv *env, std::runtime_error& e);
void CPPJNI_HandleStdException(JNIEnv *env, std::exception& e);

template<typename T>
void CPPJNI_ensureNotNull(T value, const char* failMessage=nullptr) 
{
  if (!value)
  {
    throw std::runtime_error(failMessage ? failMessage : "Unexpected null values");
  }
}


#define CPPJNI_TRY try
#define CPPJNI_CATCH \
 catch(ObsoleteObjectException&) { /* No-op. See comment for ObsoleteObjectException */ } \
 catch(ObjectNullException&) { /* No-op. See comment for ObjectNullException */ } \
 catch(OpenVDS::Exception& e) { CPPJNI_HandleSharedLibraryException(env, e); } \
 catch(std::runtime_error& e) { CPPJNI_HandleStdRuntimeError(env, e); } \
 catch(std::exception& e) { CPPJNI_HandleStdException(env, e); }

class JNIEnvGuard
{
  friend class Marshaling;

  struct StringRecord {
    StringRecord(jstring str, const char* utf8) : m_String(str), m_Utf8(utf8) {
      assert(str);
      assert(utf8);
    }
    jstring     m_String;
    const char* m_Utf8;
  };

  static thread_local std::stack<JNIEnv*>
    ts_JNIEnvStack;

  static thread_local std::vector<struct StringRecord>
    ts_TempStringRecords;

  static JavaVM* 
    s_JavaVM;

  bool 
    m_isThreadAttach;

  static void         pop();
  static void         push(JNIEnv* env);
  static JNIEnv*      top();
  static void         checkInit(JNIEnv* env);
public:

                      JNIEnvGuard();
                      JNIEnvGuard(JNIEnv* env);
                      ~JNIEnvGuard();
  static bool         isJNIEnvValid();
  static JNIEnv*      getJNIEnv();
  static const char*  getStringUTFChars(jstring value);
  static void         flushStrings();
};

struct CPPJNIStringWrapper
{
  JNIEnv *              m_Env;
  jlong                 m_NativeHandle;
  jstring               m_JString;
  mutable const char *  m_PersistentUTF8;

  CPPJNIStringWrapper(JNIEnv * env, jlong native_handle, jstring str) : m_Env(env), m_NativeHandle(native_handle), m_JString(str), m_PersistentUTF8()
  {
  }

  CPPJNIStringWrapper(JNIEnv * env, jstring str) : CPPJNIStringWrapper(env, 0, str)
  {
  }

  const char*
  utf8() const
  {
    if (m_PersistentUTF8 == nullptr)
    {
      if (m_NativeHandle)
      { // Lifetime of C string data is managed by the native handle
        auto pObjectContext = (CPPJNIObjectContext*)m_NativeHandle;
        auto tmp = m_Env->GetStringUTFChars(m_JString, 0);
        m_PersistentUTF8 = pObjectContext->addString(tmp);
        m_Env->ReleaseStringUTFChars(m_JString, tmp);
      }
      else
      { // The string is interned and will live on until the library is unloaded.
        m_PersistentUTF8 = CPPJNI_internString(m_Env, m_JString);
      }
    }
    return m_PersistentUTF8;
  }

  operator const char* () const
  {
    return utf8();
  }
};

struct JNIDirectBuffer 
{
  jobject m_Buffer; // Not a global ref!!!
  void*   m_Memory;

                  JNIDirectBuffer(jlong capacity);
                  ~JNIDirectBuffer();
                  // Create a new DirectByteBuffer with native endianness.
  static jobject  CreateDirectBuffer(void* mem, jlong capacity);
};

class Marshaling {
public:
  static jobject CreateJavaObject(const char* type);
  static jobject CreateJavaObject(jclass clazz);
  static jobjectArray CreateJavaArray(int elements, const char* elementType = nullptr, jobject initialElement = 0);

  template<typename T>
  static jobject CreatePODJavaObject(T const& value);

  static JNIEnv* GetJNIEnv() {
    assert(JNIEnvGuard::isJNIEnvValid());
    return JNIEnvGuard::getJNIEnv();
  }

  static bool IsJavaContextActive() {
    return JNIEnvGuard::isJNIEnvValid();
  }

};

template<> jobject Marshaling::CreatePODJavaObject<int>(int const& value);
template<> jobject Marshaling::CreatePODJavaObject<int64_t>(int64_t const& value);

#endif
