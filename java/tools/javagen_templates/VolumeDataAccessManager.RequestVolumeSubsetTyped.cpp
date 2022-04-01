
JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataAccessManager_RequestVolumeSubset<DataType>Impl
  (JNIEnv * env, jobject object, jlong native_handle, jobject buffer, jlong dimensionsND, jint LOD, jint channel, jintArray minVoxelCoordinates, jintArray maxVoxelCoordinates, jfloat replacementNoValue, jboolean use_replacementNoValue)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto tmpbuffer = CPPJNIAsyncBuffer<void>(env, buffer);
    auto tmpminVoxelCoordinates = CPPJNIArrayAdapter<int,6,false>(env, minVoxelCoordinates);
    auto tmpmaxVoxelCoordinates = CPPJNIArrayAdapter<int,6,false>(env, maxVoxelCoordinates);
    auto pInstance = CPPJNI_cast<OpenVDS::VolumeDataAccessManager>(native_handle);
    auto result = pInstance->RequestVolumeSubset<<data_type>>(
                               (<data_type>*)tmpbuffer.buffer(), tmpbuffer.byteSize(), 
                               (OpenVDS::DimensionsND)dimensionsND, 
                               LOD, 
                               channel, 
                               tmpminVoxelCoordinates.getArray(), 
                               tmpmaxVoxelCoordinates.getArray(), 
                               use_replacementNoValue ? OpenVDS::optional<float>(replacementNoValue) : OpenVDS::optional<float>());
    auto context = CPPJNI_createObjectContext(result);
    context->registerGlobalRef(env, buffer);
    return context->handle();
  }
  CPPJNI_CATCH
  return 0;
}

JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataAccessManager_RequestVolumeSubset<DataType>2Impl
  (JNIEnv * env, jobject object, jlong native_handle, jlong dimensionsND, jint LOD, jint channel, jintArray minVoxelCoordinates, jintArray maxVoxelCoordinates, jfloat replacementNoValue, jboolean use_replacementNoValue)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto tmpminVoxelCoordinates = CPPJNIArrayAdapter<int,6,false>(env, minVoxelCoordinates);
    auto tmpmaxVoxelCoordinates = CPPJNIArrayAdapter<int,6,false>(env, maxVoxelCoordinates);
    auto pInstance = CPPJNI_cast<OpenVDS::VolumeDataAccessManager>(native_handle);
    auto result = pInstance->RequestVolumeSubset<<data_type>>(
                               (OpenVDS::DimensionsND)dimensionsND, 
                               LOD, 
                               channel, 
                               tmpminVoxelCoordinates.getArray(), 
                               tmpmaxVoxelCoordinates.getArray(), 
                               use_replacementNoValue ? OpenVDS::optional<float>(replacementNoValue) : OpenVDS::optional<float>());
    auto context = CPPJNI_createObjectContext(result);
    return context->handle();
  }
  CPPJNI_CATCH
  return 0;
}
