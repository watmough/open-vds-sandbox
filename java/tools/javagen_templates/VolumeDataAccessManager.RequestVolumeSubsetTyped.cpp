
JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataAccessManager_RequestVolumeSubset<DataType>Impl
  (JNIEnv * env, jobject object, jlong native_handle, jobject buffer, jlong dimensionsND, jint LOD, jint channel, jintArray minVoxelCoordinates, jintArray maxVoxelCoordinates, jfloat replacementNoValue, jboolean use_replacementNoValue)
{
  JEnvPushPop
    stackitem(env);

  HUE_JNI_TRY
  {
    auto tmpbuffer = HueJNIAsyncBuffer<void>(env, buffer);
    auto tmpminVoxelCoordinates = HueJNIArrayAdapter<int,6,false>(env, minVoxelCoordinates);
    auto tmpmaxVoxelCoordinates = HueJNIArrayAdapter<int,6,false>(env, maxVoxelCoordinates);
    auto pInstance = HueJNI_cast<OpenVDS::VolumeDataAccessManager>(native_handle);
    auto result = pInstance->RequestVolumeSubset<<data_type>>(
                               (<data_type>*)tmpbuffer.buffer(), tmpbuffer.byteSize(), 
                               (OpenVDS::DimensionsND)dimensionsND, 
                               LOD, 
                               channel, 
                               tmpminVoxelCoordinates.getArray(), 
                               tmpmaxVoxelCoordinates.getArray(), 
                               use_replacementNoValue ? OpenVDS::optional<float>(replacementNoValue) : OpenVDS::optional<float>());
    auto context = HueJNI_createObjectContext(result);
    context->registerGlobalRef(env, buffer);
    return context->handle();
  }
  HUE_JNI_CATCH
  return 0;
}

JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataAccessManager_RequestVolumeSubset<DataType>2Impl
  (JNIEnv * env, jobject object, jlong native_handle, jlong dimensionsND, jint LOD, jint channel, jintArray minVoxelCoordinates, jintArray maxVoxelCoordinates, jfloat replacementNoValue, jboolean use_replacementNoValue)
{
  JEnvPushPop
    stackitem(env);

  HUE_JNI_TRY
  {
    auto tmpminVoxelCoordinates = HueJNIArrayAdapter<int,6,false>(env, minVoxelCoordinates);
    auto tmpmaxVoxelCoordinates = HueJNIArrayAdapter<int,6,false>(env, maxVoxelCoordinates);
    auto pInstance = HueJNI_cast<OpenVDS::VolumeDataAccessManager>(native_handle);
    auto result = pInstance->RequestVolumeSubset<<data_type>>(
                               (OpenVDS::DimensionsND)dimensionsND, 
                               LOD, 
                               channel, 
                               tmpminVoxelCoordinates.getArray(), 
                               tmpmaxVoxelCoordinates.getArray(), 
                               use_replacementNoValue ? OpenVDS::optional<float>(replacementNoValue) : OpenVDS::optional<float>());
    auto context = HueJNI_createObjectContext(result);
    return context->handle();
  }
  HUE_JNI_CATCH
  return 0;
}
