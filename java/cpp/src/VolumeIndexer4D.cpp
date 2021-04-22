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

#include <org_opengroup_openvds_VolumeIndexer4D.h>
#include <CommonJni.h>
#include <OpenVDS/VolumeIndexer.h>
#include <OpenVDS/VolumeDataLayout.h>
#include <OpenVDS/VolumeDataAccess.h>

using namespace OpenVDS;

#ifdef __cplusplus
extern "C" {
#endif

    inline VolumeIndexer4D * GetVolumeIndexer4D( jlong handle ) {
        return (VolumeIndexer4D*)CheckHandle( handle );
    }

    inline OpenVDS::VolumeDataLayout *GetLayout(jlong handle) {
        return (OpenVDS::VolumeDataLayout *) CheckHandle(handle);
    }

    inline OpenVDS::VolumeDataPage *GetVolumePage(jlong handle) {
        return (OpenVDS::VolumeDataPage *) CheckHandle(handle);
    }

    /*
     * Class:     org_opengroup_openvds_VolumeIndexer4D
     * Method:    cpCreateVolumeIndexer4D
     * Signature: (JIIIJ)J
     */
    JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeIndexer4D_cpCreateVolumeIndexer4D
            (JNIEnv * env, jclass, jlong vlPageHandle, jint channelIndex, jint lod, jint dimND, jlong layoutHandle)
    {
        try {
            VolumeDataPage* volumeDataPage = GetVolumePage(vlPageHandle);
            VolumeDataLayout* layout = GetLayout(layoutHandle);
            VolumeIndexer4D* indexer4D = new VolumeIndexer4D(volumeDataPage, channelIndex, lod, (DimensionsND)dimND, layout);
            return (jlong)indexer4D;
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return 0;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer4D
    * Method:    cpLocalIndexToVoxelIndex
    * Signature: (J[IIIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer4D_cpLocalIndexToVoxelIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l)
    {
        try {
            IntVector4 res4 = GetVolumeIndexer4D(handle)->LocalIndexToVoxelIndex(IntVector4(i, j, k, l));
            env->SetIntArrayRegion(resOutIndex, 0, 4, res4.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer4D
    * Method:    cpLocalIndexToLocalChunkIndex
    * Signature: (J[IIIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer4D_cpLocalIndexToLocalChunkIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l)
    {
        try {
            IntVector4 res4 = GetVolumeIndexer4D(handle)->LocalIndexToLocalChunkIndex(IntVector4(i, j, k, l));
            env->SetIntArrayRegion(resOutIndex, 0, 4, res4.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer4D
    * Method:    cpVoxelIndexToLocalIndex
    * Signature: (J[IIIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer4D_cpVoxelIndexToLocalIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l)
    {
        try {
            IntVector4 res4 = GetVolumeIndexer4D(handle)->VoxelIndexToLocalIndex(IntVector4(i, j, k, l));
            env->SetIntArrayRegion(resOutIndex, 0, 4, res4.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer4D
    * Method:    cpVoxelIndexToLocalChunkIndex
    * Signature: (J[IIIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer4D_cpVoxelIndexToLocalChunkIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l)
    {
        try {
            IntVector4 res4 = GetVolumeIndexer4D(handle)->VoxelIndexToLocalChunkIndex(IntVector4(i, j, k, l));
            env->SetIntArrayRegion(resOutIndex, 0, 4, res4.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer4D
    * Method:    cpLocalChunkIndexToLocalIndex
    * Signature: (J[IIIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer4D_cpLocalChunkIndexToLocalIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l)
    {
        try {
            IntVector4 res4 = GetVolumeIndexer4D(handle)->LocalChunkIndexToLocalIndex(IntVector4(i, j, k, l));
            env->SetIntArrayRegion(resOutIndex, 0, 4, res4.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer4D
    * Method:    cpLocalChunkIndexToVoxelIndex
    * Signature: (J[IIIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer4D_cpLocalChunkIndexToVoxelIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l)
    {
        try {
            IntVector4 res4 = GetVolumeIndexer4D(handle)->LocalChunkIndexToVoxelIndex(IntVector4(i, j, k, l));
            env->SetIntArrayRegion(resOutIndex, 0, 4, res4.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer4D
    * Method:    cpLocalIndexToDataIndex
    * Signature: (JIIII)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexer4D_cpLocalIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l)
    {
        try {
            return GetVolumeIndexer4D(handle)->LocalIndexToDataIndex(IntVector4(i, j, k, l));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer4D
    * Method:    cpVoxelIndexToDataIndex
    * Signature: (JIIII)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexer4D_cpVoxelIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l)
    {
        try {
            return GetVolumeIndexer4D(handle)->VoxelIndexToDataIndex(IntVector4(i, j, k, l));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer4D
    * Method:    cpLocalChunkIndexToDataIndex
    * Signature: (JIIII)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexer4D_cpLocalChunkIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l)
    {
        try {
            return GetVolumeIndexer4D(handle)->LocalChunkIndexToDataIndex(IntVector4(i, j, k, l));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer4D
    * Method:    cpVoxelIndexInProcessArea
    * Signature: (JIIII)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexer4D_cpVoxelIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l)
    {
        try {
            return GetVolumeIndexer4D(handle)->VoxelIndexInProcessArea(IntVector4(i, j, k, l));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer4D
    * Method:    cpLocalIndexInProcessArea
    * Signature: (JIIII)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexer4D_cpLocalIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l)
    {
        try {
            return GetVolumeIndexer4D(handle)->LocalIndexInProcessArea(IntVector4(i, j, k, l));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer4D
    * Method:    cpLocalChunkIndexInProcessArea
    * Signature: (JIIII)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexer4D_cpLocalChunkIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l)
    {
        try {
            return GetVolumeIndexer4D(handle)->LocalChunkIndexInProcessArea(IntVector4(i, j, k, l));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

#ifdef __cplusplus
}
#endif



