///AUTOGEN-IGNORE: CXX_METHOD Buffer void *() const FUNCTIONPROTO

JNIEXPORT jobject JNICALL Java_org_opengroup_openvds_VolumeDataRequest_GetBufferImpl
  (JNIEnv * env, jobject object, jlong native_handle)
{
  using namespace OpenVDS;

  JEnvPushPop
    stackitem(env);

  HUE_JNI_TRY
  {
    auto pInstance = HueJNI_cast<OpenVDS::VolumeDataRequest>(native_handle);

    void *buffer = pInstance->Buffer();
    jlong nBufferSize = pInstance->BufferByteSize();

    return env->NewDirectByteBuffer(buffer, nBufferSize);
  }
  HUE_JNI_CATCH
  return 0;
}
