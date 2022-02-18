
#include <algorithm>

JNIEXPORT jintArray JNICALL Java_org_opengroup_openvds_MetadataReadAccess_GetMetadataKeyTypesImpl
(JNIEnv* env, jobject object, jlong native_handle)
{
  using namespace OpenVDS;

  JEnvPushPop
    stackitem(env);

  CPPJNI_TRY
  {
    auto pInstance = CPPJNI_cast<OpenVDS::MetadataReadAccess>(native_handle);

    auto lsKey = pInstance->GetMetadataKeys();

    jsize nKey = (jsize)std::distance(lsKey.begin(), lsKey.end());

    jintArray metadata_key_types = env->NewIntArray(nKey);

    jsize iKey = 0;
    for (const auto& key : lsKey)
    {
      jint keyType = (int)key.GetType();
      env->SetIntArrayRegion(metadata_key_types, iKey, 1, &keyType);
      iKey++;
    }

    return metadata_key_types;
  }
  CPPJNI_CATCH
  return 0;
}

JNIEXPORT jobjectArray JNICALL Java_org_opengroup_openvds_MetadataReadAccess_GetMetadataKeyCategoriesImpl
(JNIEnv* env, jobject object, jlong native_handle)
{
  using namespace OpenVDS;

  JEnvPushPop
    stackitem(env);

  CPPJNI_TRY
  {
    auto pInstance = CPPJNI_cast<OpenVDS::MetadataReadAccess>(native_handle);

    auto lsKey = pInstance->GetMetadataKeys();

    jsize nKey = (jsize)std::distance(lsKey.begin(), lsKey.end());

    jobjectArray metadata_key_categories = (jobjectArray)env->NewObjectArray(nKey, env->FindClass("java/lang/String"), env->NewStringUTF(""));

    jsize iKey = 0;
    for (const auto& key : lsKey)
    {
      env->SetObjectArrayElement(metadata_key_categories, iKey, env->NewStringUTF(key.GetCategory()));
      iKey++;
    }

    return metadata_key_categories;
  }
  CPPJNI_CATCH
  return 0;
}

JNIEXPORT jobjectArray JNICALL Java_org_opengroup_openvds_MetadataReadAccess_GetMetadataKeyNamesImpl
(JNIEnv* env, jobject object, jlong native_handle)
{
  using namespace OpenVDS;

  JEnvPushPop
    stackitem(env);

  CPPJNI_TRY
  {
    auto pInstance = CPPJNI_cast<OpenVDS::MetadataReadAccess>(native_handle);

    auto lsKey = pInstance->GetMetadataKeys();

    jsize nKey = (jsize)std::distance(lsKey.begin(), lsKey.end());

    jobjectArray metadata_key_names = (jobjectArray)env->NewObjectArray(nKey, env->FindClass("java/lang/String"), env->NewStringUTF(""));

    jsize iKey = 0;
    for (const auto& key : lsKey)
    {
      env->SetObjectArrayElement(metadata_key_names, iKey, env->NewStringUTF(key.GetName()));
      iKey++;
    }

    return metadata_key_names;
  }
  CPPJNI_CATCH
  return 0;
}

JNIEXPORT jbyteArray JNICALL Java_org_opengroup_openvds_MetadataReadAccess_GetMetadataBLOBImpl
  (JNIEnv * env, jobject object, jlong native_handle, jstring category, jstring name)
{
  JEnvPushPop
    stackitem(env);

  CPPJNI_TRY
  {
    auto pInstance = CPPJNI_cast<OpenVDS::MetadataReadAccess>(native_handle);
    std::vector<uint8_t> data;
    pInstance->GetMetadataBLOB(CPPJNIStringWrapper(env, category), CPPJNIStringWrapper(env, name), data);
    if (data.size() > UINT_MAX) 
    {
      throw std::runtime_error("BLOB is too big for Java bytearray.");
    }
    jbyteArray arr = env->NewByteArray((jsize)data.size());
    env->SetByteArrayRegion(arr, 0, (jsize)data.size(), (jbyte const*)data.data());
    return arr;
  }
  CPPJNI_CATCH
  return 0;
}

