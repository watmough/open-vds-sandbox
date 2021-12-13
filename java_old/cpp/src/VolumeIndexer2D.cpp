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

#include <org_opengroup_openvds_VolumeIndexer2D.h>
#include <CommonJni.h>
#include <OpenVDS/VolumeIndexer.h>
#include <OpenVDS/VolumeDataLayout.h>
#include <OpenVDS/VolumeDataAccess.h>

using namespace OpenVDS;

#ifdef __cplusplus
extern "C" {
#endif

    inline VolumeIndexer2D * GetVolumeIndexer2D( jlong handle ) {
        return (VolumeIndexer2D*)CheckHandle( handle );
    }

    inline OpenVDS::VolumeDataLayout *GetLayout(jlong handle) {
        return (OpenVDS::VolumeDataLayout *) CheckHandle(handle);
    }

    inline OpenVDS::VolumeDataPage *GetVolumePage(jlong handle) {
        return (OpenVDS::VolumeDataPage *) CheckHandle(handle);
    }

    /*
     * Class:     org_opengroup_openvds_VolumeIndexer2D
     * Method:    cpCreateVolumeIndexer2D
     * Signature: (JIIIJ)J
     */
    JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeIndexer2D_cpCreateVolumeIndexer2D
            (JNIEnv * env, jclass, jlong vlPageHandle, jint channelIndex, jint lod, jint dimND, jlong layoutHandle)
    {
        try {
            VolumeDataPage* volumeDataPage = GetVolumePage(vlPageHandle);
            VolumeDataLayout* layout = GetLayout(layoutHandle);
            VolumeIndexer2D* indexer2D = new VolumeIndexer2D(volumeDataPage, channelIndex, lod, (DimensionsND)dimND, layout);
            return (jlong)indexer2D;
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return 0;
    }


    /*
    * Class:     org_opengroup_openvds_VolumeIndexer2D
    * Method:    cpLocalIndexToVoxelIndex
    * Signature: (JI[III)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer2D_cpLocalIndexToVoxelIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j)
    {
        try {
            IntVector2 res2 = GetVolumeIndexer2D(handle)->LocalIndexToVoxelIndex(IntVector2(i, j));
            env->SetIntArrayRegion(resOutIndex, 0, 2, (jint*)res2.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer2D
    * Method:    cpLocalIndexToLocalChunkIndex
    * Signature: (J[III)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer2D_cpLocalIndexToLocalChunkIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j)
    {
        try {
            IntVector2 res2 = GetVolumeIndexer2D(handle)->LocalIndexToLocalChunkIndex(IntVector2(i, j));
            env->SetIntArrayRegion(resOutIndex, 0, 2, (jint*)res2.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }


    /*
     * Class:     org_opengroup_openvds_VolumeIndexer2D
     * Method:    cpVoxelIndexToLocalIndex
     * Signature: (J[III)V
     */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer2D_cpVoxelIndexToLocalIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j)
    {
        try {
            IntVector2 res2 = GetVolumeIndexer2D(handle)->VoxelIndexToLocalIndex(IntVector2(i, j));
            env->SetIntArrayRegion(resOutIndex, 0, 2, (jint*)res2.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer2D
    * Method:    cpVoxelIndexToLocalChunkIndex
    * Signature: (J[III)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer2D_cpVoxelIndexToLocalChunkIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j)
    {
        try {
            IntVector2 res2 = GetVolumeIndexer2D(handle)->VoxelIndexToLocalChunkIndex(IntVector2(i, j));
            env->SetIntArrayRegion(resOutIndex, 0, 2, (jint*)res2.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }


    /*
     * Class:     org_opengroup_openvds_VolumeIndexer2D
     * Method:    cpLocalChunkIndexToLocalIndex
     * Signature: (J[III)V
     */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer2D_cpLocalChunkIndexToLocalIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j)
    {
        try {
            IntVector2 res2 = GetVolumeIndexer2D(handle)->LocalChunkIndexToLocalIndex(IntVector2(i, j));
            env->SetIntArrayRegion(resOutIndex, 0, 2, (jint*)res2.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer2D
    * Method:    cpLocalChunkIndexToVoxelIndex
    * Signature: (J[III)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexer2D_cpLocalChunkIndexToVoxelIndex
            (JNIEnv * env, jclass, jlong handle, jintArray resOutIndex, jint i, jint j)
    {
        try {
            IntVector2 res2 = GetVolumeIndexer2D(handle)->LocalChunkIndexToVoxelIndex(IntVector2(i, j));
            env->SetIntArrayRegion(resOutIndex, 0, 2, (jint*)res2.data);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }


    /*
     * Class:     org_opengroup_openvds_VolumeIndexer2D
     * Method:    cpLocalIndexToDataIndex
     * Signature: (JII)I
     */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexer2D_cpLocalIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint i, jint j)
    {
        try {
            return GetVolumeIndexer2D(handle)->LocalIndexToDataIndex(IntVector2(i, j));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer2D
    * Method:    cpVoxelIndexToDataIndex
    * Signature: (JII)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexer2D_cpVoxelIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint i, jint j)
    {
        try {
            return GetVolumeIndexer2D(handle)->VoxelIndexToDataIndex(IntVector2(i, j));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer2D
    * Method:    cpLocalChunkIndexToDataIndex
    * Signature: (JII)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexer2D_cpLocalChunkIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint i, jint j)
    {
        try {
            return GetVolumeIndexer2D(handle)->LocalChunkIndexToDataIndex(IntVector2(i, j));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer2D
    * Method:    cpVoxelIndexInProcessArea
    * Signature: (JII)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexer2D_cpVoxelIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint i, jint j)
    {
        try {
            return GetVolumeIndexer2D(handle)->VoxelIndexInProcessArea(IntVector2(i, j));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer2D
    * Method:    cpLocalIndexInProcessArea
    * Signature: (JII)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexer2D_cpLocalIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint i, jint j)
    {
        try {
            return GetVolumeIndexer2D(handle)->LocalIndexInProcessArea(IntVector2(i, j));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexer2D
    * Method:    cpLocalChunkIndexInProcessArea
    * Signature: (JII)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexer2D_cpLocalChunkIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint i, jint j)
    {
        try {
            return GetVolumeIndexer2D(handle)->LocalChunkIndexInProcessArea(IntVector2(i, j));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

#ifdef __cplusplus
}
#endif



