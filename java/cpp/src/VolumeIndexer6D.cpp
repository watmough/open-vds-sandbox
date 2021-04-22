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

#include <org_opengroup_openvds_VolumeIndexer6D.h>
#include <CommonJni.h>
#include <OpenVDS/VolumeIndexer.h>
#include <OpenVDS/VolumeDataLayout.h>
#include <OpenVDS/VolumeDataAccess.h>

using namespace OpenVDS;

#ifdef __cplusplus
extern "C" {
#endif

    inline VolumeIndexer6D * GetVolumeIndexer6D( jlong handle ) {
        return (VolumeIndexer6D*)CheckHandle( handle );
    }

    inline OpenVDS::VolumeDataLayout *GetLayout(jlong handle) {
        return (OpenVDS::VolumeDataLayout *) CheckHandle(handle);
    }

    inline OpenVDS::VolumeDataPage *GetVolumePage(jlong handle) {
        return (OpenVDS::VolumeDataPage *) CheckHandle(handle);
    }

    /*
     * Class:     org_opengroup_openvds_VolumeIndexer6D
     * Method:    cpCreateVolumeIndexer6D
     * Signature: (JIIIJ)J
     */
    JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeIndexer6D_cpCreateVolumeIndexer6D
            (JNIEnv * env, jclass, jlong vlPageHandle, jint channelIndex, jint lod, jint dimND, jlong layoutHandle)
    {
        try {
            VolumeDataPage* volumeDataPage = GetVolumePage(vlPageHandle);
            VolumeDataLayout* layout = GetLayout(layoutHandle);
            VolumeIndexer6D* indexer6D = new VolumeIndexer6D(volumeDataPage, channelIndex, lod, (DimensionsND)dimND, layout);
            return (jlong)indexer6D;
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return 0;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer6D
    * Method:    cpLocalIndexToVoxelIndex
    * Signature: (J[IIIIIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer6D_cpLocalIndexToVoxelIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l, jint m, jint n)
    {
        try {
            IntVector<6> res6 = GetVolumeIndexer6D(handle)->LocalIndexToVoxelIndex(IntVector<6>(i, j, k, l, m, n));
            env->SetIntArrayRegion(resOutIndex, 0, 6, res6.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer6D
    * Method:    cpLocalIndexToLocalChunkIndex
    * Signature: (J[IIIIIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer6D_cpLocalIndexToLocalChunkIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l, jint m, jint n)
    {
        try {
            IntVector<6> res6 = GetVolumeIndexer6D(handle)->LocalIndexToLocalChunkIndex(IntVector<6>(i, j, k, l, m, n));
            env->SetIntArrayRegion(resOutIndex, 0, 6, res6.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer6D
    * Method:    cpVoxelIndexToLocalIndex
    * Signature: (J[IIIIIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer6D_cpVoxelIndexToLocalIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l, jint m, jint n)
    {
        try {
            IntVector<6> res6 = GetVolumeIndexer6D(handle)->VoxelIndexToLocalIndex(IntVector<6>(i, j, k, l, m, n));
            env->SetIntArrayRegion(resOutIndex, 0, 6, res6.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer6D
    * Method:    cpVoxelIndexToLocalChunkIndex
    * Signature: (J[IIIIIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer6D_cpVoxelIndexToLocalChunkIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l, jint m, jint n)
    {
        try {
            IntVector<6> res6 = GetVolumeIndexer6D(handle)->VoxelIndexToLocalChunkIndex(IntVector<6>(i, j, k, l, m, n));
            env->SetIntArrayRegion(resOutIndex, 0, 6, res6.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer6D
    * Method:    cpLocalChunkIndexToLocalIndex
    * Signature: (J[IIIIIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer6D_cpLocalChunkIndexToLocalIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l, jint m, jint n)
    {
        try {
            IntVector<6> res6 = GetVolumeIndexer6D(handle)->LocalChunkIndexToLocalIndex(IntVector<6>(i, j, k, l, m, n));
            env->SetIntArrayRegion(resOutIndex, 0, 6, res6.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
     * Class:     org_opengroup_openvds_VolumeIndexer6D
     * Method:    cpLocalChunkIndexToVoxelIndex
     * Signature: (J[IIIIIII)V
     */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer6D_cpLocalChunkIndexToVoxelIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l, jint m, jint n)
    {
        try {
            IntVector<6> res6 = GetVolumeIndexer6D(handle)->LocalChunkIndexToVoxelIndex(IntVector<6>(i, j, k, l, m, n));
            env->SetIntArrayRegion(resOutIndex, 0, 6, res6.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer6D
    * Method:    cpLocalIndexToDataIndex
    * Signature: (JIIIIII)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexer6D_cpLocalIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l, jint m, jint n)
    {
        try {
            return GetVolumeIndexer6D(handle)->LocalIndexToDataIndex(IntVector<6>(i, j, k, l, m, n));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer6D
    * Method:    cpVoxelIndexToDataIndex
    * Signature: (JIIIIII)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexer6D_cpVoxelIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l, jint m, jint n)
    {
        try {
            return GetVolumeIndexer6D(handle)->VoxelIndexToDataIndex(IntVector<6>(i, j, k, l, m, n));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer6D
    * Method:    cpLocalChunkIndexToDataIndex
    * Signature: (JIIIIII)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexer6D_cpLocalChunkIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l, jint m, jint n)
    {
        try {
            return GetVolumeIndexer6D(handle)->LocalChunkIndexToDataIndex(IntVector<6>(i, j, k, l, m, n));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer6D
    * Method:    cpVoxelIndexInProcessArea
    * Signature: (JIIIIII)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexer6D_cpVoxelIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l, jint m, jint n)
    {
        try {
            return GetVolumeIndexer6D(handle)->VoxelIndexInProcessArea(IntVector<6>(i, j, k, l, m, n));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer6D
    * Method:    cpLocalIndexInProcessArea
    * Signature: (JIIIIII)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexer6D_cpLocalIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l, jint m, jint n)
    {
        try {
            return GetVolumeIndexer6D(handle)->LocalIndexInProcessArea(IntVector<6>(i, j, k, l, m, n));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer6D
    * Method:    cpLocalChunkIndexInProcessArea
    * Signature: (JIIIIII)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexer6D_cpLocalChunkIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l, jint m, jint n)
    {
        try {
            return GetVolumeIndexer6D(handle)->LocalChunkIndexInProcessArea(IntVector<6>(i, j, k, l, m, n));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

#ifdef __cplusplus
}
#endif



