
JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataAccessManager_RequestProjectedVolumeSubset<DataType>Impl
  (JNIEnv * env, jobject object, jlong native_handle, jobject buffer, jlong dimensionsND, jint LOD, jint channel, jintArray minVoxelCoordinates, jintArray maxVoxelCoordinates, jobject voxelPlanebytebuffer, jlong voxelPlanebyteoffset, jlong projectedDimensions, jlong interpolationMethod, jfloat replacementNoValue, jboolean use_replacementNoValue)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto tmpminVoxelCoordinates = CPPJNIArrayAdapter<int,6,false>(env, minVoxelCoordinates);
    auto tmpmaxVoxelCoordinates = CPPJNIArrayAdapter<int,6,false>(env, maxVoxelCoordinates);
    auto pInstance = CPPJNI_cast<OpenVDS::VolumeDataAccessManager>(native_handle);
    auto tmpbuffer = CPPJNIAsyncBuffer<void>(env, buffer);
    auto result = pInstance->RequestProjectedVolumeSubset<<data_type>>(
                               (<data_type>*)tmpbuffer.buffer(), tmpbuffer.byteSize(),
                               (OpenVDS::DimensionsND)dimensionsND, 
                               LOD, 
                               channel, 
                               tmpminVoxelCoordinates.getArray(), 
                               tmpmaxVoxelCoordinates.getArray(), 
                               CPPJNIByteBufferAdapter<OpenVDS::FloatVector4>(env, voxelPlanebytebuffer, voxelPlanebyteoffset), 
                               (OpenVDS::DimensionsND)projectedDimensions, 
                               (OpenVDS::InterpolationMethod)interpolationMethod, 
                               use_replacementNoValue ? OpenVDS::optional<float>(replacementNoValue) : OpenVDS::optional<float>());
    // Create a context with a reference to the buffer. A GlobalRef is created to ensure the buffer is not garbage collected 
    // before the request object is destroyed.
    auto context = CPPJNI_createObjectContext(result);
    context->registerBuffer(buffer);
    return context->handle();
  }
  CPPJNI_CATCH
  return 0;
}

JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataAccessManager_RequestProjectedVolumeSubset<DataType>2Impl
  (JNIEnv * env, jobject object, jlong native_handle, jlong dimensionsND, jint LOD, jint channel, jintArray minVoxelCoordinates, jintArray maxVoxelCoordinates, jobject voxelPlanebytebuffer, jlong voxelPlanebyteoffset, jlong projectedDimensions, jlong interpolationMethod, jfloat replacementNoValue, jboolean use_replacementNoValue)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto tmpminVoxelCoordinates = CPPJNIArrayAdapter<int,6,false>(env, minVoxelCoordinates);
    auto tmpmaxVoxelCoordinates = CPPJNIArrayAdapter<int,6,false>(env, maxVoxelCoordinates);
    auto pInstance = CPPJNI_cast<OpenVDS::VolumeDataAccessManager>(native_handle);
    auto bufferByteSize = pInstance->GetProjectedVolumeSubsetBufferSize<<data_type>>(tmpminVoxelCoordinates.getArray(), tmpmaxVoxelCoordinates.getArray(), (OpenVDS::DimensionsND)projectedDimensions, LOD, channel);
    auto buffer = JNIDirectBuffer::CreateDirectBuffer(bufferByteSize);
    auto tmpbuffer = CPPJNIAsyncBuffer<void>(env, buffer);
    auto result = pInstance->RequestProjectedVolumeSubset<<data_type>>(
                               (<data_type>*)tmpbuffer.buffer(), tmpbuffer.byteSize(), 
                               (OpenVDS::DimensionsND)dimensionsND, 
                               LOD, 
                               channel, 
                               tmpminVoxelCoordinates.getArray(), 
                               tmpmaxVoxelCoordinates.getArray(), 
                               CPPJNIByteBufferAdapter<OpenVDS::FloatVector4>(env, voxelPlanebytebuffer, voxelPlanebyteoffset), 
                               (OpenVDS::DimensionsND)projectedDimensions, 
                               (OpenVDS::InterpolationMethod)interpolationMethod, 
                               use_replacementNoValue ? OpenVDS::optional<float>(replacementNoValue) : OpenVDS::optional<float>());
    // Create a context with a reference to the buffer. A GlobalRef is created to ensure the buffer is not garbage collected 
    // before the request object is destroyed.
    auto context = CPPJNI_createObjectContext(result);
    context->registerBuffer(buffer);
    return context->handle();
  }
  CPPJNI_CATCH
  return 0;
}
