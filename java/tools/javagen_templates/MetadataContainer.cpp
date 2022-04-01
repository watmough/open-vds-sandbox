
///AUTOGEN-IGNORE: CXX_METHOD GetMetadataBLOB void (const char *, const char *, const void **, uint64_t *) const FUNCTIONPROTO
///AUTOGEN-IGNORE: CXX_METHOD SetMetadataBLOB void (const char *, const char *, const void *, uint64_t) FUNCTIONPROTO
JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_SetMetadataBLOBImpl
  (JNIEnv * env, jobject object, jlong native_handle, jstring category, jstring name, jbyteArray data)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    auto tmpdata = CPPJNIArrayAdapter<int8_t, 1>(env, data);
    auto pInstance = CPPJNI_cast<OpenVDS::MetadataContainer>(native_handle);
    pInstance->SetMetadataBLOB(
                               CPPJNIStringWrapper(env, native_handle, category), 
                               CPPJNIStringWrapper(env, native_handle, name), 
                               tmpdata.m_Data);
  }
  CPPJNI_CATCH
}


