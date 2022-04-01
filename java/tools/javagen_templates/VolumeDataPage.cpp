///AUTOGEN-IGNORE: CXX_METHOD GetBuffer const void *(int (&)[6]) FUNCTIONPROTO
///AUTOGEN-IGNORE: CXX_METHOD GetWritableBuffer void *(int (&)[6]) FUNCTIONPROTO

#ifdef __cplusplus
}
#endif

template<bool WRITEABLE>
jobject
VolumeDataPage_GetWritableBufferImpl(JNIEnv * env, jobject object, jlong native_handle, jintArray outPitch)
{
  using namespace OpenVDS;

  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto pInstance = CPPJNI_cast<OpenVDS::VolumeDataPage>(native_handle);

    int pitch[VolumeDataLayout::Dimensionality_Max];
    void *buffer = WRITEABLE ? pInstance->GetWritableBuffer(pitch) : (void*)pInstance->GetBuffer(pitch);
    env->SetIntArrayRegion(outPitch, 0, VolumeDataLayout::Dimensionality_Max, (jint*)pitch);

    auto channelDescriptor = pInstance->GetVolumeDataPageAccessor().GetChannelDescriptor();

    auto format = channelDescriptor.GetFormat();
    auto components = channelDescriptor.GetComponents();
    int itemSize = 0;
    switch(format)
    {
    default:
      throw std::runtime_error("Illegal format");
    case OpenVDS::VolumeDataChannelDescriptor::Format::Format_1Bit:
      itemSize = 1;
	  break;
    case OpenVDS::VolumeDataChannelDescriptor::Format::Format_U8:
      itemSize = 1 * components;
	  break;
    case OpenVDS::VolumeDataChannelDescriptor::Format::Format_U16:
      itemSize = 2 * components;
	  break;
    case OpenVDS::VolumeDataChannelDescriptor::Format::Format_R32:
    case OpenVDS::VolumeDataChannelDescriptor::Format::Format_U32:
      itemSize = 4 * components;
	  break;
    case OpenVDS::VolumeDataChannelDescriptor::Format::Format_U64:
    case OpenVDS::VolumeDataChannelDescriptor::Format::Format_R64:
      itemSize = 8 * components;
	  break;
    }
    jlong nBufferSize = 1;
    for (int i = 0; i < VolumeDataLayout::Dimensionality_Max; ++i)
    {
      if (pitch[i] > 0)
      {
        nBufferSize *= pitch[i];
      }
    }
    nBufferSize *= itemSize;
    return JNIDirectBuffer::CreateDirectBuffer(buffer, nBufferSize);
  }
  CPPJNI_CATCH
  return 0;
}

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT jobject JNICALL Java_org_opengroup_openvds_VolumeDataPage_GetWritableBufferImpl
  (JNIEnv * env, jobject object, jlong native_handle, jintArray outPitch)
{
  return VolumeDataPage_GetWritableBufferImpl<true>(env, object, native_handle, outPitch);
}

JNIEXPORT jobject JNICALL Java_org_opengroup_openvds_VolumeDataPage_GetBufferImpl
  (JNIEnv * env, jobject object, jlong native_handle, jintArray outPitch)
{
  return VolumeDataPage_GetWritableBufferImpl<false>(env, object, native_handle, outPitch);
}
