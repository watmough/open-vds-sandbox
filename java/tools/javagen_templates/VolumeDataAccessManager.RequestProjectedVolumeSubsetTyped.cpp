
/* FIXME THIS OVERLOAD DOES NOT EXIST IN OPENVDS
JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataAccessManager_RequestProjectedVolumeSubset<DataType>Impl
  (JNIEnv * env, jobject object, jlong native_handle, jobject buffer, jlong dimensionsND, jint LOD, jint channel, jintArray minVoxelCoordinates, jintArray maxVoxelCoordinates, jobject voxelPlanebytebuffer, jlong voxelPlanebyteoffset, jlong projectedDimensions, jlong interpolationMethod, jfloat replacementNoValue, jboolean use_replacementNoValue)
{
  JEnvPushPop
    stackitem(env);

  CPPJNI_TRY
  {
    auto tmpbuffer = CPPJNIAsyncBuffer<void>(env, buffer);
    auto tmpminVoxelCoordinates = CPPJNIArrayAdapter<int,6,false>(env, minVoxelCoordinates);
    auto tmpmaxVoxelCoordinates = CPPJNIArrayAdapter<int,6,false>(env, maxVoxelCoordinates);
    auto pInstance = CPPJNI_cast<OpenVDS::VolumeDataAccessManager>(native_handle);
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
    auto context = CPPJNI_createObjectContext(result);
    context->registerGlobalRef(env, buffer);
    return context->handle();
  }
  CPPJNI_CATCH
  return 0;
}
*/

JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataAccessManager_RequestProjectedVolumeSubset<DataType>2Impl
  (JNIEnv * env, jobject object, jlong native_handle, jlong dimensionsND, jint LOD, jint channel, jintArray minVoxelCoordinates, jintArray maxVoxelCoordinates, jobject voxelPlanebytebuffer, jlong voxelPlanebyteoffset, jlong projectedDimensions, jlong interpolationMethod, jfloat replacementNoValue, jboolean use_replacementNoValue)
{
  JEnvPushPop
    stackitem(env);

  CPPJNI_TRY
  {
    auto tmpminVoxelCoordinates = CPPJNIArrayAdapter<int,6,false>(env, minVoxelCoordinates);
    auto tmpmaxVoxelCoordinates = CPPJNIArrayAdapter<int,6,false>(env, maxVoxelCoordinates);
    auto pInstance = CPPJNI_cast<OpenVDS::VolumeDataAccessManager>(native_handle);
    auto result = pInstance->RequestProjectedVolumeSubset<<data_type>>(
                               (OpenVDS::DimensionsND)dimensionsND, 
                               LOD, 
                               channel, 
                               tmpminVoxelCoordinates.getArray(), 
                               tmpmaxVoxelCoordinates.getArray(), 
                               CPPJNIByteBufferAdapter<OpenVDS::FloatVector4>(env, voxelPlanebytebuffer, voxelPlanebyteoffset), 
                               (OpenVDS::DimensionsND)projectedDimensions, 
                               (OpenVDS::InterpolationMethod)interpolationMethod, 
                               use_replacementNoValue ? OpenVDS::optional<float>(replacementNoValue) : OpenVDS::optional<float>());
    auto context = CPPJNI_createObjectContext(result);
    return context->handle();
  }
  CPPJNI_CATCH
  return 0;
}
