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

#include <org_opengroup_openvds_VolumeIndexer5D.h>
#include <CommonJni.h>
#include <OpenVDS/VolumeIndexer.h>
#include <OpenVDS/VolumeDataLayout.h>
#include <OpenVDS/VolumeDataAccess.h>

using namespace OpenVDS;

#ifdef __cplusplus
extern "C" {
#endif

    inline VolumeIndexer5D * GetVolumeIndexer5D( jlong handle ) {
        return (VolumeIndexer5D*)CheckHandle( handle );
    }

    inline OpenVDS::VolumeDataLayout *GetLayout(jlong handle) {
        return (OpenVDS::VolumeDataLayout *) CheckHandle(handle);
    }

    inline OpenVDS::VolumeDataPage *GetVolumePage(jlong handle) {
        return (OpenVDS::VolumeDataPage *) CheckHandle(handle);
    }

    /*
     * Class:     org_opengroup_openvds_VolumeIndexer5D
     * Method:    cpCreateVolumeIndexer5D
     * Signature: (JIIIJ)J
     */
    JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeIndexer5D_cpCreateVolumeIndexer5D
            (JNIEnv * env, jclass, jlong vlPageHandle, jint channelIndex, jint lod, jint dimND, jlong layoutHandle)
    {
        try {
            VolumeDataPage* volumeDataPage = GetVolumePage(vlPageHandle);
            VolumeDataLayout* layout = GetLayout(layoutHandle);
            VolumeIndexer5D* indexer5D = new VolumeIndexer5D(volumeDataPage, channelIndex, lod, (DimensionsND)dimND, layout);
            return (jlong)indexer5D;
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return 0;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer5D
    * Method:    cpLocalIndexToVoxelIndex
    * Signature: (J[IIIIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer5D_cpLocalIndexToVoxelIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l, jint m)
    {
        try {
            IntVector<5> res5 = GetVolumeIndexer5D(handle)->LocalIndexToVoxelIndex(IntVector<5>(i, j, k, l, m));
            env->SetIntArrayRegion(resOutIndex, 0, 5, (jint *)res5.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer5D
    * Method:    cpLocalIndexToLocalChunkIndex
    * Signature: (J[IIIIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer5D_cpLocalIndexToLocalChunkIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l, jint m)
    {
        try {
            IntVector<5> res5 = GetVolumeIndexer5D(handle)->LocalIndexToLocalChunkIndex(IntVector<5>(i, j, k, l, m));
            env->SetIntArrayRegion(resOutIndex, 0, 5, (jint *)res5.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer5D
    * Method:    cpVoxelIndexToLocalIndex
    * Signature: (J[IIIIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer5D_cpVoxelIndexToLocalIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l, jint m)
    {
        try {
            IntVector<5> res5 = GetVolumeIndexer5D(handle)->LocalIndexToLocalChunkIndex(IntVector<5>(i, j, k, l, m));
            env->SetIntArrayRegion(resOutIndex, 0, 5, (jint*)res5.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer5D
    * Method:    cpVoxelIndexToLocalChunkIndex
    * Signature: (J[IIIIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer5D_cpVoxelIndexToLocalChunkIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l, jint m)
    {
        try {
            IntVector<5> res5 = GetVolumeIndexer5D(handle)->VoxelIndexToLocalChunkIndex(IntVector<5>(i, j, k, l, m));
            env->SetIntArrayRegion(resOutIndex, 0, 5, (jint *)res5.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer5D
    * Method:    cpLocalChunkIndexToLocalIndex
    * Signature: (J[IIIIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer5D_cpLocalChunkIndexToLocalIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l, jint m)
    {
        try {
            IntVector<5> res5 = GetVolumeIndexer5D(handle)->LocalChunkIndexToLocalIndex(IntVector<5>(i, j, k, l, m));
            env->SetIntArrayRegion(resOutIndex, 0, 5, (jint*)res5.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
     * Class:     org_opengroup_openvds_VolumeIndexer5D
     * Method:    cpLocalChunkIndexToVoxelIndex
     * Signature: (J[IIIIII)V
     */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer5D_cpLocalChunkIndexToVoxelIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k, jint l, jint m)
    {
        try {
            IntVector<5> res5 = GetVolumeIndexer5D(handle)->LocalChunkIndexToVoxelIndex(IntVector<5>(i, j, k, l, m));
            env->SetIntArrayRegion(resOutIndex, 0, 5, (jint *) res5.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer5D
    * Method:    cpLocalIndexToDataIndex
    * Signature: (JIIIII)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexer5D_cpLocalIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l, jint m)
    {
        try {
            return GetVolumeIndexer5D(handle)->LocalIndexToDataIndex(IntVector<5>(i, j, k, l, m));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer5D
    * Method:    cpVoxelIndexToDataIndex
    * Signature: (JIIIII)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexer5D_cpVoxelIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l, jint m)
    {
        try {
            return GetVolumeIndexer5D(handle)->VoxelIndexToDataIndex(IntVector<5>(i, j, k, l, m));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer5D
    * Method:    cpLocalChunkIndexToDataIndex
    * Signature: (JIIIII)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexer5D_cpLocalChunkIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l, jint m)
    {
        try {
            return GetVolumeIndexer5D(handle)->LocalChunkIndexToDataIndex(IntVector<5>(i, j, k, l, m));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer5D
    * Method:    cpVoxelIndexInProcessArea
    * Signature: (JIIIII)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexer5D_cpVoxelIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l, jint m)
    {
        try {
            return GetVolumeIndexer5D(handle)->VoxelIndexInProcessArea(IntVector<5>(i, j, k, l, m));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer5D
    * Method:    cpLocalIndexInProcessArea
    * Signature: (JIIIII)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexer5D_cpLocalIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l, jint m)
    {
        try {
            return GetVolumeIndexer5D(handle)->LocalIndexInProcessArea(IntVector<5>(i, j, k, l, m));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer5D
    * Method:    cpLocalChunkIndexInProcessArea
    * Signature: (JIIIII)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexer5D_cpLocalChunkIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k, jint l, jint m)
    {
        try {
            return GetVolumeIndexer5D(handle)->LocalChunkIndexInProcessArea(IntVector<5>(i, j, k, l, m));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

#ifdef __cplusplus
}
#endif



