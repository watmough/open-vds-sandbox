JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeDataPageAccessor_dtorImpl
  (JNIEnv * env, jobject object, jlong native_handle, jboolean is_disposing)
{
  JNIEnvGuard
    envGuard(env);

  CPPJNI_TRY
  {
    if (is_disposing) 
    { // Clean up 
      auto pInstance = CPPJNI_cast<OpenVDS::VolumeDataPageAccessor>(native_handle);
      pInstance->Commit();
	  
	  // The access manager sets itself as manager from 
	  // Java_org_opengroup_openvds_VolumeDataAccessManager_CreateVolumeDataPageAccessorImpl
      auto accessManager = CPPJNIObjectContext::ensureValid(native_handle)->getManager<OpenVDS::VolumeDataAccessManager>(); // May throw
      accessManager->DestroyVolumeDataPageAccessor(pInstance);
    }
    CPPJNI_destroyHandle<OpenVDS::VolumeDataPageAccessor>(native_handle, is_disposing);
  }
  CPPJNI_CATCH
}
