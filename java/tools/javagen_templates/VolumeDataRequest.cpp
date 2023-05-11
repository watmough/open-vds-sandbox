///AUTOGEN-IGNORE: CXX_METHOD Buffer void *() const FUNCTIONPROTO

JNIEXPORT jobject JNICALL Java_org_opengroup_openvds_VolumeDataRequest_GetBufferImpl
  (JNIEnv * env, jobject object, jlong native_handle)
{
  using namespace OpenVDS;

  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto buffer = CPPJNIObjectContext::getObjectBuffer(native_handle);
    if (buffer)
    {
      return buffer;
    }
    else
    { // We don't own the buffer, so we have to create it using memory allocated by the request
      auto pInstance = CPPJNI_cast<OpenVDS::VolumeDataRequest>(native_handle);

      void *buffer = pInstance->Buffer();
      jlong nBufferSize = pInstance->BufferByteSize();

      return JNIDirectBuffer::CreateDirectBuffer(buffer, nBufferSize);
    }
  }
  CPPJNI_CATCH
  return 0;
}
