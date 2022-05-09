
///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestVolumeSubsetTyped.cpp, <DataType> -> Byte  , <data_type> -> uint8_t
///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestVolumeSubsetTyped.cpp, <DataType> -> UShort, <data_type> -> uint16_t
///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestVolumeSubsetTyped.cpp, <DataType> -> UInt  , <data_type> -> uint32_t
///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestVolumeSubsetTyped.cpp, <DataType> -> ULong , <data_type> -> uint64_t
///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestVolumeSubsetTyped.cpp, <DataType> -> Float , <data_type> -> float 
///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestVolumeSubsetTyped.cpp, <DataType> -> Double, <data_type> -> double

///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestProjectedVolumeSubsetTyped.cpp, <DataType> -> Byte  , <data_type> -> uint8_t
///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestProjectedVolumeSubsetTyped.cpp, <DataType> -> UShort, <data_type> -> uint16_t
///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestProjectedVolumeSubsetTyped.cpp, <DataType> -> UInt  , <data_type> -> uint32_t
///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestProjectedVolumeSubsetTyped.cpp, <DataType> -> ULong , <data_type> -> uint64_t
///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestProjectedVolumeSubsetTyped.cpp, <DataType> -> Float , <data_type> -> float 
///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestProjectedVolumeSubsetTyped.cpp, <DataType> -> Double, <data_type> -> double

///AUTOGEN-IGNORE: CXX_METHOD RequestVolumeSamples std::shared_ptr<OpenVDS::VolumeDataRequest_t<float>> (float *, int64_t, OpenVDS::DimensionsND, int, int, float const (*)[6], int, OpenVDS::InterpolationMethod, OpenVDS::optional<float>) FUNCTIONPROTO
JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataAccessManager_RequestVolumeSamplesImpl
  (JNIEnv * env, jobject object, jlong native_handle, jobject buffer, jlong dimensionsND, jint LOD, jint channel, jobject samplePositionsbytebuffer, jlong interpolationMethod, jfloat replacementNoValue, jboolean use_replacementNoValue)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto tmpbuffer = CPPJNIAsyncBuffer<float>(env, buffer);
    auto pInstance = CPPJNI_cast<OpenVDS::VolumeDataAccessManager>(native_handle);
    auto result = pInstance->RequestVolumeSamples(
                               tmpbuffer.buffer(), tmpbuffer.byteSize(), 
                               (OpenVDS::DimensionsND)dimensionsND, 
                               LOD, 
                               channel, 
                               CPPJNIByteBufferAdapter<float[6]>(env, samplePositionsbytebuffer, 0).m_Data, 
                               (int)env->GetDirectBufferCapacity(samplePositionsbytebuffer) / sizeof(float[6]), 
                               (OpenVDS::InterpolationMethod)interpolationMethod, 
                               use_replacementNoValue ? OpenVDS::optional<float>(replacementNoValue) : OpenVDS::optional<float>());
    auto context = CPPJNI_createObjectContext(result);
    context->registerGlobalRef(env, buffer);
    return context->handle();
  }
  CPPJNI_CATCH
  return 0;
}

///AUTOGEN-IGNORE: CXX_METHOD RequestVolumeSamples std::shared_ptr<OpenVDS::VolumeDataRequest_t<float>> (OpenVDS::DimensionsND, int, int, float const (*)[6], int, OpenVDS::InterpolationMethod, OpenVDS::optional<float>) FUNCTIONPROTO
JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataAccessManager_RequestVolumeSamples2Impl
  (JNIEnv * env, jobject object, jlong native_handle, jlong dimensionsND, jint LOD, jint channel, jobject samplePositionsbytebuffer, jlong interpolationMethod, jfloat replacementNoValue, jboolean use_replacementNoValue)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto pInstance = CPPJNI_cast<OpenVDS::VolumeDataAccessManager>(native_handle);
    auto result = pInstance->RequestVolumeSamples(
                               (OpenVDS::DimensionsND)dimensionsND, 
                               LOD, 
                               channel, 
                               CPPJNIByteBufferAdapter<float[6]>(env, samplePositionsbytebuffer, 0).m_Data, 
                               (int)env->GetDirectBufferCapacity(samplePositionsbytebuffer) / sizeof(float[6]), 
                               (OpenVDS::InterpolationMethod)interpolationMethod, 
                               use_replacementNoValue ? OpenVDS::optional<float>(replacementNoValue) : OpenVDS::optional<float>());
    auto context = CPPJNI_createObjectContext(result);
    return context->handle();
  }
  CPPJNI_CATCH
  return 0;
}

///AUTOGEN-IGNORE: CXX_METHOD RequestVolumeTraces std::shared_ptr<OpenVDS::VolumeDataRequest_t<float>> (OpenVDS::DimensionsND, int, int, float const (*)[6], int, OpenVDS::InterpolationMethod, int, OpenVDS::optional<float>) FUNCTIONPROTO
JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataAccessManager_RequestVolumeTracesImpl
  (JNIEnv * env, jobject object, jlong native_handle, jlong dimensionsND, jint LOD, jint channel, jobject tracePositionsbytebuffer, jlong interpolationMethod, jint traceDimension, jfloat replacementNoValue, jboolean use_replacementNoValue)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto pInstance = CPPJNI_cast<OpenVDS::VolumeDataAccessManager>(native_handle);
    auto result = pInstance->RequestVolumeTraces(
                               (OpenVDS::DimensionsND)dimensionsND, 
                               LOD, 
                               channel, 
                               CPPJNIByteBufferAdapter<float[6]>(env, tracePositionsbytebuffer, 0).m_Data, 
                               (int)env->GetDirectBufferCapacity(tracePositionsbytebuffer) / sizeof(float[6]), 
                               (OpenVDS::InterpolationMethod)interpolationMethod, 
                               traceDimension, 
                               use_replacementNoValue ? OpenVDS::optional<float>(replacementNoValue) : OpenVDS::optional<float>());
    auto context = CPPJNI_createObjectContext(result);
    return context->handle();
  }
  CPPJNI_CATCH
  return 0;
}

///AUTOGEN-IGNORE: CXX_METHOD RequestVolumeTraces std::shared_ptr<OpenVDS::VolumeDataRequest_t<float>> (float *, int64_t, OpenVDS::DimensionsND, int, int, float const (*)[6], int, OpenVDS::InterpolationMethod, int, OpenVDS::optional<float>) FUNCTIONPROTO
JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataAccessManager_RequestVolumeTraces2Impl
  (JNIEnv * env, jobject object, jlong native_handle, jobject buffer, jlong dimensionsND, jint LOD, jint channel, jobject tracePositionsbytebuffer, jlong interpolationMethod, jint traceDimension, jfloat replacementNoValue, jboolean use_replacementNoValue)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto tmpbuffer = CPPJNIAsyncBuffer<float>(env, buffer);
    auto pInstance = CPPJNI_cast<OpenVDS::VolumeDataAccessManager>(native_handle);
    auto result = pInstance->RequestVolumeTraces(
                               tmpbuffer.buffer(), tmpbuffer.byteSize(), 
                               (OpenVDS::DimensionsND)dimensionsND, 
                               LOD, 
                               channel, 
                               CPPJNIByteBufferAdapter<float[6]>(env, tracePositionsbytebuffer, 0).m_Data, 
                               (int)env->GetDirectBufferCapacity(tracePositionsbytebuffer) / sizeof(float[6]), 
                               (OpenVDS::InterpolationMethod)interpolationMethod, 
                               traceDimension, 
                               use_replacementNoValue ? OpenVDS::optional<float>(replacementNoValue) : OpenVDS::optional<float>());
    auto context = CPPJNI_createObjectContext(result);
    context->registerGlobalRef(env, buffer);
    return context->handle();
  }
  CPPJNI_CATCH
  return 0;

}

///AUTOGEN-IGNORE: CXX_METHOD GetCurrentUploadError void (const char **, int *, const char **) FUNCTIONPROTO
///AUTOGEN-IGNORE: CXX_METHOD GetCurrentDownloadError void (int *, const char **) FUNCTIONPROTO	

JNIEXPORT jobjectArray JNICALL Java_org_opengroup_openvds_VolumeDataAccessManager_GetCurrentUploadErrorImpl
  (JNIEnv * env, jobject object, jlong native_handle)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto pInstance = CPPJNI_cast<OpenVDS::VolumeDataAccessManager>(native_handle);
    const char* objectID = nullptr;
    int32_t code = 0;
    const char* errorString = nullptr;
    pInstance->GetCurrentUploadError(&objectID, &code, &errorString);
    auto arr = Marshaling::CreateJavaArray(3);
    if (arr)
    {
      env->SetObjectArrayElement(arr, 0, env->NewStringUTF(objectID));
      env->SetObjectArrayElement(arr, 1, Marshaling::CreatePODJavaObject<int>(code));
      env->SetObjectArrayElement(arr, 2, env->NewStringUTF(errorString));

    }
    return arr;
  }
  CPPJNI_CATCH
  return 0;
}

JNIEXPORT jobjectArray JNICALL Java_org_opengroup_openvds_VolumeDataAccessManager_GetCurrentDownloadErrorImpl
  (JNIEnv * env, jobject object, jlong native_handle)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto pInstance = CPPJNI_cast<OpenVDS::VolumeDataAccessManager>(native_handle);
    int32_t code = 0;
    const char* errorString = "";
    pInstance->GetCurrentDownloadError(&code, &errorString);
    auto arr = Marshaling::CreateJavaArray(2);
    if (arr)
    {
      env->SetObjectArrayElement(arr, 0, Marshaling::CreatePODJavaObject<int>(code));
      env->SetObjectArrayElement(arr, 1, env->NewStringUTF(errorString));

    }
    return arr;
  }
  CPPJNI_CATCH
  return 0;
}

/* FIXME? DOES NOT EXIST IN OPENVDS 
///AUTOGEN-IGNORE: CXX_METHOD RequestVolumeTraceRanges std::shared_ptr<OpenVDS::VolumeDataRequest_t<float>> (OpenVDS::DimensionsND, int, int, float const (*)[6], int, OpenVDS::InterpolationMethod, int, int, int, OpenVDS::optional<float>) FUNCTIONPROTO
JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataAccessManager_RequestVolumeTraceRangesImpl
  (JNIEnv * env, jobject object, jlong native_handle, jlong dimensionsND, jint LOD, jint channel, jobject tracePositionsbytebuffer, jlong interpolationMethod, jint traceDimension, jint traceMin, jint traceMax, jfloat replacementNoValue, jboolean use_replacementNoValue)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto pInstance = CPPJNI_cast<OpenVDS::VolumeDataAccessManager>(native_handle);
    auto result = pInstance->RequestVolumeTraceRanges(
                               (OpenVDS::DimensionsND)dimensionsND, 
                               LOD, 
                               channel, 
                               CPPJNIByteBufferAdapter<float[6]>(env, tracePositionsbytebuffer, 0).m_Data, 
                               (int)env->GetDirectBufferCapacity(tracePositionsbytebuffer) / sizeof(float[6]), 
                               (OpenVDS::InterpolationMethod)interpolationMethod, 
                               traceDimension, 
                               traceMin,
                               traceMax,
                               use_replacementNoValue ? OpenVDS::optional<float>(replacementNoValue) : OpenVDS::optional<float>());
    auto context = CPPJNI_createObjectContext(result);
    return context->handle();
  }
  CPPJNI_CATCH
  return 0;
}

///AUTOGEN-IGNORE: CXX_METHOD RequestVolumeTraceRanges std::shared_ptr<OpenVDS::VolumeDataRequest_t<float>> (float *, int64_t, OpenVDS::DimensionsND, int, int, float const (*)[6], int, OpenVDS::InterpolationMethod, int, int, int, OpenVDS::optional<float>) FUNCTIONPROTO
JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataAccessManager_RequestVolumeTraceRanges2Impl
  (JNIEnv * env, jobject object, jlong native_handle, jobject buffer, jlong dimensionsND, jint LOD, jint channel, jobject tracePositionsbytebuffer, jlong interpolationMethod, jint traceDimension, jint traceMin, jint traceMax, jfloat replacementNoValue, jboolean use_replacementNoValue)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto tmpbuffer = CPPJNIAsyncBuffer<float>(env, buffer);
    auto pInstance = CPPJNI_cast<OpenVDS::VolumeDataAccessManager>(native_handle);
    auto result = pInstance->RequestVolumeTraceRanges(
                               tmpbuffer.buffer(), tmpbuffer.byteSize(), 
                               (OpenVDS::DimensionsND)dimensionsND, 
                               LOD, 
                               channel, 
                               CPPJNIByteBufferAdapter<float[6]>(env, tracePositionsbytebuffer, 0).m_Data, 
                               (int)env->GetDirectBufferCapacity(tracePositionsbytebuffer) / sizeof(float[6]), 
                               (OpenVDS::InterpolationMethod)interpolationMethod, 
                               traceDimension, 
                               traceMin,
                               traceMax,
                               use_replacementNoValue ? OpenVDS::optional<float>(replacementNoValue) : OpenVDS::optional<float>());
    auto context = CPPJNI_createObjectContext(result);
    context->registerGlobalRef(env, buffer);
    return context->handle();
  }
  CPPJNI_CATCH
  return 0;
}

*/

