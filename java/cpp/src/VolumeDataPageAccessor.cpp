/*
 * Copyright 2019 The Open Group
 * Copyright 2019 INT, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <org_opengroup_openvds_VolumeDataPageAccessor.h>
#include <CommonJni.h>
#include <OpenVDS/VolumeDataAccess.h>

#ifdef __cplusplus
extern "C" {
#endif

    inline OpenVDS::VolumeDataPageAccessor* GetPageAccessor( jlong handle ) {
        return (OpenVDS::VolumeDataPageAccessor*) CheckHandle( handle );
    }


    /*
     * Class:     org_opengroup_openvds_VolumeDataPageAccessor
     * Method:    cpGetLayout
     * Signature: (J)J
     */
    JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataPageAccessor_cpGetLayout
      (JNIEnv * env, jclass, jlong handle)
    {
        try {
            const OpenVDS::VolumeDataLayout* layout = GetPageAccessor( handle )->GetLayout();
            return (jlong) layout;
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return 0;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeDataPageAccessor
    * Method:    cpGetLOD
    * Signature: (J)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeDataPageAccessor_cpGetLOD
            (JNIEnv * env, jclass, jlong handle)
    {
        try {
            return GetPageAccessor( handle )->GetLOD();
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
     * Class:     org_opengroup_openvds_VolumeDataPageAccessor
     * Method:    cpGetChannelIndex
     * Signature: (J)I
     */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeDataPageAccessor_cpGetChannelIndex
            (JNIEnv * env, jclass, jlong handle)
    {
        try {
            return GetPageAccessor( handle )->GetChannelIndex();
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
     * Class:     org_opengroup_openvds_VolumeDataPageAccessor
     * Method:    cpGetNumSamples
     * Signature: (J)[I
     */
    JNIEXPORT jintArray JNICALL Java_org_opengroup_openvds_VolumeDataPageAccessor_cpGetNumSamples
            (JNIEnv * env, jclass, jlong handle)
    {
        try {
            int dims[OpenVDS::Dimensionality_Max];
            GetPageAccessor( handle )->GetNumSamples(dims);
            return NewJIntArray(env, dims, OpenVDS::Dimensionality_Max);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return NULL;
    }


    /*
     * Class:     org_opengroup_openvds_VolumeDataPageAccessor
     * Method:    cpGetChunkCount
     * Signature: (J)J
     */
    JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataPageAccessor_cpGetChunkCount
            (JNIEnv * env, jclass, jlong handle)
    {
        try {
            return GetPageAccessor( handle )->GetChunkCount();
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeDataPageAccessor
    * Method:    cpCreatePage
    * Signature: (JJ)J
    */
    JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataPageAccessor_cpCreatePage
            (JNIEnv * env, jclass, jlong handle, jlong pageIndex)
    {
        try {
            OpenVDS::VolumeDataPage* volumePage = GetPageAccessor( handle )->CreatePage(pageIndex);
            return (jlong) volumePage;
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeDataPageAccessor
    * Method:    cpReadPage
    * Signature: (JJ)J
    */
    JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeDataPageAccessor_cpReadPage
            (JNIEnv * env, jclass, jlong handle, jlong pageIndex)
    {
        try {
            OpenVDS::VolumeDataPage* volumePage = GetPageAccessor( handle )->ReadPage(pageIndex);
            return (jlong) volumePage;
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeDataPageAccessor
    * Method:    cpGetChunkMinMax
    * Signature: (JI[I[I)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeDataPageAccessor_cpGetChunkMinMax
            (JNIEnv * env, jclass, jlong handle, jint chunk, jintArray chunkMin, jintArray chunkMax)
    {
        try {
            int cMin[OpenVDS::Dimensionality_Max];
            int cMax[OpenVDS::Dimensionality_Max];
            GetPageAccessor( handle )->GetChunkMinMax(chunk, cMin, cMax);
            env->SetIntArrayRegion(chunkMin, 0, OpenVDS::Dimensionality_Max, cMin);
            env->SetIntArrayRegion(chunkMax, 0, OpenVDS::Dimensionality_Max, cMax);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeDataPageAccessor
    * Method:    cpGetChunkMinMaxExcludingMargin
    * Signature: (JI[I[I)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeDataPageAccessor_cpGetChunkMinMaxExcludingMargin
            (JNIEnv * env, jclass, jlong handle, jint chunk, jintArray chunkMin, jintArray chunkMax)
    {
        try {
            int cMin[OpenVDS::Dimensionality_Max];
            int cMax[OpenVDS::Dimensionality_Max];
            GetPageAccessor( handle )->GetChunkMinMaxExcludingMargin(chunk, cMin, cMax);
            env->SetIntArrayRegion(chunkMin, 0, OpenVDS::Dimensionality_Max, cMin);
            env->SetIntArrayRegion(chunkMax, 0, OpenVDS::Dimensionality_Max, cMax);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }


    /*
    * Class:     org_opengroup_openvds_VolumeDataPageAccessor
    * Method:    cpCommit
    * Signature: (J)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeDataPageAccessor_cpCommit
            (JNIEnv * env, jclass, jlong handle)
    {
        try {
            GetPageAccessor( handle )->Commit();
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
     * Class:     org_opengroup_openvds_VolumeDataPageAccessor
     * Method:    cpSetMaxPage
     * Signature: (JI)V
     */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeDataPageAccessor_cpSetMaxPage
            (JNIEnv * env, jclass, jlong handle, jint maxPages)
    {
        try {
            GetPageAccessor( handle )->SetMaxPages(maxPages);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

#ifdef __cplusplus
}
#endif
