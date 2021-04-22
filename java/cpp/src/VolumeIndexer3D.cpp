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

#include <org_opengroup_openvds_VolumeIndexer3D.h>
#include <CommonJni.h>
#include <OpenVDS/VolumeIndexer.h>
#include <OpenVDS/VolumeDataLayout.h>
#include <OpenVDS/VolumeDataAccess.h>

using namespace OpenVDS;

#ifdef __cplusplus
extern "C" {
#endif

    inline VolumeIndexer3D * GetVolumeIndexer3D( jlong handle ) {
        return (VolumeIndexer3D*)CheckHandle( handle );
    }

    inline OpenVDS::VolumeDataLayout *GetLayout(jlong handle) {
        return (OpenVDS::VolumeDataLayout *) CheckHandle(handle);
    }

    inline OpenVDS::VolumeDataPage *GetVolumePage(jlong handle) {
        return (OpenVDS::VolumeDataPage *) CheckHandle(handle);
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer3D
    * Method:    cpCreateVolumeIndexer3D
    * Signature: (Lorg/opengroup/openvds/VolumeDataPage;IILorg/opengroup/openvds/DimensionsND;Lorg/opengroup/openvds/VolumeDataLayout;)J
    */
    JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeIndexer3D_cpCreateVolumeIndexer3D
            (JNIEnv * env, jclass, jlong vlPageHandle, jint channelIndex, jint lod, jint dimND, jlong layoutHandle)
    {
        try {
            VolumeDataPage* volumeDataPage = GetVolumePage(vlPageHandle);
            VolumeDataLayout* layout = GetLayout(layoutHandle);
            VolumeIndexer3D* indexer3D = new VolumeIndexer3D(volumeDataPage, channelIndex, lod, (DimensionsND)dimND, layout);
            return (jlong)indexer3D;
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return 0;
    }

    /*
     * Class:     org_opengroup_openvds_VolumeIndexer3D
     * Method:    cpLocalIndexToVoxelIndex
     * Signature: (JI[IIII)V
     */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer3D_cpLocalIndexToVoxelIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j , jint k)
    {
        try {
            IntVector3 res3 = GetVolumeIndexer3D(handle)->LocalIndexToVoxelIndex(IntVector3(i, j, k));
            env->SetIntArrayRegion(resOutIndex, 0, 3, res3.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer3D
    * Method:    cpLocalIndexToLocalChunkIndex
    * Signature: (J[IIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer3D_cpLocalIndexToLocalChunkIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k)
    {
        try {
            IntVector3 res3 = GetVolumeIndexer3D(handle)->LocalIndexToLocalChunkIndex(IntVector3(i, j, k));
            env->SetIntArrayRegion(resOutIndex, 0, 3, res3.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer3D
    * Method:    cpVoxelIndexToLocalIndex
    * Signature: (J[IIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer3D_cpVoxelIndexToLocalIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k)
    {
        try {
            IntVector3 res3 = GetVolumeIndexer3D(handle)->VoxelIndexToLocalIndex(IntVector3(i, j, k));
            env->SetIntArrayRegion(resOutIndex, 0, 3, res3.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer3D
    * Method:    cpVoxelIndexToLocalChunkIndex
    * Signature: (J[IIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer3D_cpVoxelIndexToLocalChunkIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k)
    {
        try {
            IntVector3 res3 = GetVolumeIndexer3D(handle)->VoxelIndexToLocalChunkIndex(IntVector3(i, j, k));
            env->SetIntArrayRegion(resOutIndex, 0, 3, res3.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer3D
    * Method:    cpLocalChunkIndexToLocalIndex
    * Signature: (J[IIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer3D_cpLocalChunkIndexToLocalIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k)
    {
        try {
            IntVector3 res3 = GetVolumeIndexer3D(handle)->LocalChunkIndexToLocalIndex(IntVector3(i, j, k));
            env->SetIntArrayRegion(resOutIndex, 0, 3, res3.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer3D
    * Method:    cpLocalChunkIndexToVoxelIndex
    * Signature: (J[IIII)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer3D_cpLocalChunkIndexToVoxelIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j, jint k)
    {
        try {
            IntVector3 res3 = GetVolumeIndexer3D(handle)->LocalChunkIndexToVoxelIndex(IntVector3(i, j, k));
            env->SetIntArrayRegion(resOutIndex, 0, 3, res3.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }


    /*
    * Class:     org_opengroup_openvds_VolumeIndexer3D
    * Method:    cpLocalIndexToDataIndex
    * Signature: (JIIII)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexer3D_cpLocalIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k)
    {
        try {
            return GetVolumeIndexer3D(handle)->LocalIndexToDataIndex(IntVector3(i, j, k));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }


    /*
     * Class:     org_opengroup_openvds_VolumeIndexer3D
     * Method:    cpVoxelIndexToDataIndex
     * Signature: (JIII)I
     */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexer3D_cpVoxelIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k)
    {
        try {
            return GetVolumeIndexer3D(handle)->VoxelIndexToDataIndex(IntVector3(i, j, k));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer3D
    * Method:    cpLocalChunkIndexToDataIndex
    * Signature: (JIII)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexer3D_cpLocalChunkIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k)
    {
        try {
            return GetVolumeIndexer3D(handle)->LocalChunkIndexToDataIndex(IntVector3(i, j, k));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer3D
    * Method:    cpVoxelIndexInProcessArea
    * Signature: (JIII)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexer3D_cpVoxelIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k)
    {
        try {
            return GetVolumeIndexer3D(handle)->VoxelIndexInProcessArea(IntVector3(i, j, k));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer3D
    * Method:    cpLocalIndexInProcessArea
    * Signature: (JIII)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexer3D_cpLocalIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k)
    {
        try {
            return GetVolumeIndexer3D(handle)->LocalIndexInProcessArea(IntVector3(i, j, k));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer3D
    * Method:    cpLocalChunkIndexInProcessArea
    * Signature: (JIII)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexer3D_cpLocalChunkIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint i, jint j, jint k)
    {
        try {
            return GetVolumeIndexer3D(handle)->LocalChunkIndexInProcessArea(IntVector3(i, j, k));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

#ifdef __cplusplus
}
#endif



