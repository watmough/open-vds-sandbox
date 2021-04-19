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

#include <org_opengroup_openvds_VolumeIndexerBase.h>
#include <CommonJni.h>
#include <OpenVDS/VolumeIndexer.h>
#include <OpenVDS/VolumeDataLayout.h>
#include <OpenVDS/VolumeDataAccess.h>
#include <iostream>

using namespace OpenVDS;

#ifdef __cplusplus
extern "C" {
#endif

    inline VolumeIndexer2D * GetVolumeIndexer2D( jlong handle ) {
        return (VolumeIndexer2D*)CheckHandle( handle );
    }

    inline VolumeIndexer3D * GetVolumeIndexer3D( jlong handle ) {
        return (VolumeIndexer3D*)CheckHandle( handle );
    }

    inline VolumeIndexer4D * GetVolumeIndexer4D( jlong handle ) {
        return (VolumeIndexer4D*)CheckHandle( handle );
    }

    inline VolumeIndexer5D * GetVolumeIndexer5D( jlong handle ) {
        return (VolumeIndexer5D*)CheckHandle( handle );
    }

    inline VolumeIndexer6D * GetVolumeIndexer6D( jlong handle ) {
        return (VolumeIndexer6D*)CheckHandle( handle );
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexerBase
    * Method:    cpDeleteHandle
    * Signature: (J)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpDeleteHandle
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension)
    {
        try {
            switch (volumeDimension) {
                case 2:
                    delete GetVolumeIndexer2D(handle);
                    break;
                case 3:
                    delete GetVolumeIndexer3D(handle);
                    break;
                case 4:
                    delete GetVolumeIndexer4D(handle);
                    break;
                case 5:
                    delete GetVolumeIndexer5D(handle);
                    break;
                case 6:
                    delete GetVolumeIndexer6D(handle);
                    break;
            }
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexerBase
    * Method:    cpGetValueRangeMin
    * Signature: (JI)F
    */
    JNIEXPORT jfloat JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpGetValueRangeMin
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension)
    {
        try {
            switch (volumeDimension) {
                case 2:
                    return GetVolumeIndexer2D(handle)->valueRangeMin;
                case 3:
                    return GetVolumeIndexer3D(handle)->valueRangeMin;
                case 4:
                    return GetVolumeIndexer4D(handle)->valueRangeMin;
                case 5:
                    return GetVolumeIndexer5D(handle)->valueRangeMin;
                case 6:
                    return GetVolumeIndexer6D(handle)->valueRangeMin;
            }
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return std::numeric_limits<float>::min();
    }

    /*
     * Class:     org_opengroup_openvds_VolumeIndexerBase
     * Method:    cpGetValueRangeMax
     * Signature: (JI)F
     */
    JNIEXPORT jfloat JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpGetValueRangeMax
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension)
    {
        try {
            switch (volumeDimension) {
                case 2:
                    return GetVolumeIndexer2D(handle)->valueRangeMax;
                case 3:
                    return GetVolumeIndexer3D(handle)->valueRangeMax;
                case 4:
                    return GetVolumeIndexer4D(handle)->valueRangeMax;
                case 5:
                    return GetVolumeIndexer5D(handle)->valueRangeMax;
                case 6:
                    return GetVolumeIndexer6D(handle)->valueRangeMax;
            }
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return std::numeric_limits<float>::max();
    }

    /*
     * Class:     org_opengroup_openvds_VolumeIndexerBase
     * Method:    cpGetDataBlockNumSamples
     * Signature: (JII)I
     */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpGetDataBlockNumSamples
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension, jint dim)
    {
        try {
            switch (volumeDimension) {
                case 2:
                    return GetVolumeIndexer2D(handle)->GetDataBlockNumSamples(dim);
                case 3:
                    return GetVolumeIndexer3D(handle)->GetDataBlockNumSamples(dim);
                case 4:
                    return GetVolumeIndexer4D(handle)->GetDataBlockNumSamples(dim);
                case 5:
                    return GetVolumeIndexer5D(handle)->GetDataBlockNumSamples(dim);
                case 6:
                    return GetVolumeIndexer6D(handle)->GetDataBlockNumSamples(dim);
            }
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexerBase
    * Method:    cpGetLocalChunkNumSamples
    * Signature: (JII)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpGetLocalChunkNumSamples
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension, jint dim)
    {
        try {
            switch (volumeDimension) {
                case 2:
                    return GetVolumeIndexer2D(handle)->GetLocalChunkNumSamples(dim);
                case 3:
                    return GetVolumeIndexer3D(handle)->GetLocalChunkNumSamples(dim);
                case 4:
                    return GetVolumeIndexer4D(handle)->GetLocalChunkNumSamples(dim);
                case 5:
                    return GetVolumeIndexer5D(handle)->GetLocalChunkNumSamples(dim);
                case 6:
                    return GetVolumeIndexer6D(handle)->GetLocalChunkNumSamples(dim);
            }
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }


    /*
    * Class:     org_opengroup_openvds_VolumeIndexerBase
    * Method:    cpLocalIndexToVoxelIndex
    * Signature: (JI[I)[I
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpLocalIndexToVoxelIndex
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension, jintArray localIndex, jintArray resOutIndex)
    {
        try {
            jint *input = env->GetIntArrayElements(localIndex, 0);
            switch (volumeDimension) {
                case 2: {
                    IntVector2 vec2(input[0], input[1]);
                    IntVector2 res2 = GetVolumeIndexer2D(handle)->LocalIndexToVoxelIndex(vec2);
                    env->SetIntArrayRegion(resOutIndex, 0, 2, res2.data);
                    break;
                }
                case 3: {
                    IntVector3 vec3(input[0], input[1], input[2]);
                    IntVector3 res3 = GetVolumeIndexer3D(handle)->LocalIndexToVoxelIndex(vec3);
                    env->SetIntArrayRegion(resOutIndex, 0, 3, res3.data);
                    break;
                }
                case 4:{
                    IntVector4 vec4(input[0], input[1], input[2], input[3]);
                    IntVector4 res4 = GetVolumeIndexer4D(handle)->LocalIndexToVoxelIndex(vec4);
                    env->SetIntArrayRegion(resOutIndex, 0, 4, res4.data);
                    break;
                }
                case 5: {
                    IntVector<5> vec5(input[0], input[1], input[2], input[3], input[4]);
                    IntVector<5> res5 = GetVolumeIndexer5D(handle)->LocalIndexToVoxelIndex(vec5);
                    env->SetIntArrayRegion(resOutIndex, 0, 5, res5.data);
                    break;
                }
                case 6: {
                    IntVector<6> vec6(input[0], input[1], input[2], input[3], input[4], input[5]);
                    IntVector<6> res6 = GetVolumeIndexer6D(handle)->LocalIndexToVoxelIndex(vec6);
                    env->SetIntArrayRegion(resOutIndex, 0, 6, res6.data);
                    break;
                }
            }
            env->ReleaseIntArrayElements(localIndex, (jint *) input, 0);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexerBase
    * Method:    cpLocalIndexToLocalChunkIndex
    * Signature: (JI[I)[I
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpLocalIndexToLocalChunkIndex
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension, jintArray localIndex, jintArray resOutIndex)
    {
        try {
            jint *input = env->GetIntArrayElements(localIndex, 0);
            switch (volumeDimension) {
                case 2: {
                    IntVector2 vec2(input[0], input[1]);
                    IntVector2 res2 = GetVolumeIndexer2D(handle)->LocalIndexToLocalChunkIndex(vec2);
                    env->SetIntArrayRegion(resOutIndex, 0, 2, res2.data);
                    break;
                }
                case 3: {
                    IntVector3 vec3(input[0], input[1], input[2]);
                    IntVector3 res3 = GetVolumeIndexer3D(handle)->LocalIndexToLocalChunkIndex(vec3);
                    env->SetIntArrayRegion(resOutIndex, 0, 3, res3.data);
                    break;
                }
                case 4: {
                    IntVector4 vec4(input[0], input[1], input[2], input[3]);
                    IntVector4 res4 = GetVolumeIndexer4D(handle)->LocalIndexToLocalChunkIndex(vec4);
                    env->SetIntArrayRegion(resOutIndex, 0, 4, res4.data);
                    break;
                }
                case 5: {
                    IntVector<5> vec5(input[0], input[1], input[2], input[3], input[4]);
                    IntVector<5> res5 = GetVolumeIndexer5D(handle)->LocalIndexToLocalChunkIndex(vec5);
                    env->SetIntArrayRegion(resOutIndex, 0, 5, res5.data);
                    break;
                }
                case 6: {
                    IntVector<6> vec6(input[0], input[1], input[2], input[3], input[4], input[5]);
                    IntVector<6> res6 = GetVolumeIndexer6D(handle)->LocalIndexToLocalChunkIndex(vec6);
                    env->SetIntArrayRegion(resOutIndex, 0, 6, res6.data);
                    break;
                }
            }
            env->ReleaseIntArrayElements(localIndex, (jint *) input, 0);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexerBase
    * Method:    cpVoxelIndexToLocalIndex
    * Signature: (JI[I)[I
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpVoxelIndexToLocalIndex
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension, jintArray voxelIndex, jintArray resOutIndex)
    {
        try {
            jint *input = env->GetIntArrayElements(voxelIndex, 0);
            switch (volumeDimension) {
                case 2: {
                    IntVector2 vec2(input[0], input[1]);
                    IntVector2 res2 = GetVolumeIndexer2D(handle)->VoxelIndexToLocalIndex(vec2);
                    env->SetIntArrayRegion(resOutIndex, 0, 2, res2.data);
                    break;
                }
                case 3: {
                    IntVector3 vec3(input[0], input[1], input[2]);
                    IntVector3 res3 = GetVolumeIndexer3D(handle)->VoxelIndexToLocalIndex(vec3);
                    env->SetIntArrayRegion(resOutIndex, 0, 3, res3.data);
                    break;
                }
                case 4: {
                    IntVector4 vec4(input[0], input[1], input[2], input[3]);
                    IntVector4 res4 = GetVolumeIndexer4D(handle)->VoxelIndexToLocalIndex(vec4);
                    env->SetIntArrayRegion(resOutIndex, 0, 4, res4.data);
                    break;
                }
                case 5: {
                    IntVector<5> vec5(input[0], input[1], input[2], input[3], input[4]);
                    IntVector<5> res5 = GetVolumeIndexer5D(handle)->VoxelIndexToLocalIndex(vec5);
                    env->SetIntArrayRegion(resOutIndex, 0, 5, res5.data);
                    break;
                }
                case 6: {
                    IntVector<6> vec6(input[0], input[1], input[2], input[3], input[4], input[5]);
                    IntVector<6> res6 = GetVolumeIndexer6D(handle)->VoxelIndexToLocalIndex(vec6);
                    env->SetIntArrayRegion(resOutIndex, 0, 6, res6.data);
                    break;
                }
            }
            env->ReleaseIntArrayElements(voxelIndex, (jint *) input, 0);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexerBase
    * Method:    cpVoxelIndexToLocalChunkIndex
    * Signature: (JI[I)[I
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpVoxelIndexToLocalChunkIndex
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension, jintArray voxelIndex, jintArray resOutIndex)
    {
        try {
            jint *input = env->GetIntArrayElements(voxelIndex, 0);
            switch (volumeDimension) {
                case 2: {
                    IntVector2 vec2(input[0], input[1]);
                    IntVector2 res2 = GetVolumeIndexer2D(handle)->VoxelIndexToLocalChunkIndex(vec2);
                    env->SetIntArrayRegion(resOutIndex, 0, 2, res2.data);
                    break;
                }
                case 3: {
                    IntVector3 vec3(input[0], input[1], input[2]);
                    IntVector3 res3 = GetVolumeIndexer3D(handle)->VoxelIndexToLocalChunkIndex(vec3);
                    env->SetIntArrayRegion(resOutIndex, 0, 3, res3.data);
                    break;
                }
                case 4: {
                    IntVector4 vec4(input[0], input[1], input[2], input[3]);
                    IntVector4 res4 = GetVolumeIndexer4D(handle)->VoxelIndexToLocalChunkIndex(vec4);
                    env->SetIntArrayRegion(resOutIndex, 0, 4, res4.data);
                    break;
                }
                case 5: {
                    IntVector<5> vec5(input[0], input[1], input[2], input[3], input[4]);
                    IntVector<5> res5 = GetVolumeIndexer5D(handle)->VoxelIndexToLocalChunkIndex(vec5);
                    env->SetIntArrayRegion(resOutIndex, 0, 5, res5.data);
                    break;
                }
                case 6: {
                    IntVector<6> vec6(input[0], input[1], input[2], input[3], input[4], input[5]);
                    IntVector<6> res6 = GetVolumeIndexer6D(handle)->VoxelIndexToLocalChunkIndex(vec6);
                    env->SetIntArrayRegion(resOutIndex, 0, 6, res6.data);
                    break;
                }
            }
            env->ReleaseIntArrayElements(voxelIndex, (jint *) input, 0);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexerBase
    * Method:    cpLocalChunkIndexToLocalIndex
    * Signature: (JI[I)[I
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpLocalChunkIndexToLocalIndex
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension, jintArray localChunkIndex, jintArray resOutIndex)
    {
        try {
            jint *input = env->GetIntArrayElements(localChunkIndex, 0);
            switch (volumeDimension) {
                case 2: {
                    IntVector2 vec2(input[0], input[1]);
                    IntVector2 res2 = GetVolumeIndexer2D(handle)->LocalChunkIndexToLocalIndex(vec2);
                    env->SetIntArrayRegion(resOutIndex, 0, 2, res2.data);
                    break;
                }
                case 3: {
                    IntVector3 vec3(input[0], input[1], input[2]);
                    IntVector3 res3 = GetVolumeIndexer3D(handle)->LocalChunkIndexToLocalIndex(vec3);
                    env->SetIntArrayRegion(resOutIndex, 0, 3, res3.data);
                    break;
                }
                case 4: {
                    IntVector4 vec4(input[0], input[1], input[2], input[3]);
                    IntVector4 res4 = GetVolumeIndexer4D(handle)->LocalChunkIndexToLocalIndex(vec4);
                    env->SetIntArrayRegion(resOutIndex, 0, 4, res4.data);
                    break;
                }
                case 5: {
                    IntVector<5> vec5(input[0], input[1], input[2], input[3], input[4]);
                    IntVector<5> res5 = GetVolumeIndexer5D(handle)->LocalChunkIndexToLocalIndex(vec5);
                    env->SetIntArrayRegion(resOutIndex, 0, 5, res5.data);
                    break;
                }
                case 6: {
                    IntVector<6> vec6(input[0], input[1], input[2], input[3], input[4], input[5]);
                    IntVector<6> res6 = GetVolumeIndexer6D(handle)->LocalChunkIndexToLocalIndex(vec6);
                    env->SetIntArrayRegion(resOutIndex, 0, 6, res6.data);
                    break;
                }
            }
            env->ReleaseIntArrayElements(localChunkIndex, (jint *) input, 0);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexerBase
    * Method:    cpLocalChunkIndexToVoxelIndex
    * Signature: (JI[I)[I
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpLocalChunkIndexToVoxelIndex
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension, jintArray localChunkIndex, jintArray resOutIndex)
    {
        try {
            jint *input = env->GetIntArrayElements(localChunkIndex, 0);
            switch (volumeDimension) {
                case 2: {
                    IntVector2 vec2(input[0], input[1]);
                    IntVector2 res2 = GetVolumeIndexer2D(handle)->LocalChunkIndexToVoxelIndex(vec2);
                    env->SetIntArrayRegion(resOutIndex, 0, 2, res2.data);
                    break;
                }
                case 3: {
                    IntVector3 vec3(input[0], input[1], input[2]);
                    IntVector3 res3 = GetVolumeIndexer3D(handle)->LocalChunkIndexToVoxelIndex(vec3);
                    env->SetIntArrayRegion(resOutIndex, 0, 3, res3.data);
                    break;
                }
                case 4: {
                    IntVector4 vec4(input[0], input[1], input[2], input[3]);
                    IntVector4 res4 = GetVolumeIndexer4D(handle)->LocalChunkIndexToVoxelIndex(vec4);
                    env->SetIntArrayRegion(resOutIndex, 0, 4, res4.data);
                    break;
                }
                case 5: {
                    IntVector<5> vec5(input[0], input[1], input[2], input[3], input[4]);
                    IntVector<5> res5 = GetVolumeIndexer5D(handle)->LocalChunkIndexToVoxelIndex(vec5);
                    env->SetIntArrayRegion(resOutIndex, 0, 5, res5.data);
                    break;
                }
                case 6: {
                    IntVector<6> vec6(input[0], input[1], input[2], input[3], input[4], input[5]);
                    IntVector<6> res6 = GetVolumeIndexer6D(handle)->LocalChunkIndexToVoxelIndex(vec6);
                    env->SetIntArrayRegion(resOutIndex, 0, 6, res6.data);
                    break;
                }
            }
            env->ReleaseIntArrayElements(localChunkIndex, (jint *) input, 0);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
     * Class:     org_opengroup_openvds_VolumeIndexerBase
     * Method:    cpLocalIndexToDataIndex
     * Signature: (JI[I)I
     */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpLocalIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension, jintArray localIndex)
    {
        try {
            jint *input = env->GetIntArrayElements(localIndex, 0);
            switch (volumeDimension) {
                case 2: {
                    IntVector2 vec2(input[0], input[1]);
                    env->ReleaseIntArrayElements(localIndex, (jint *) input, 0);
                    return GetVolumeIndexer2D(handle)->LocalIndexToDataIndex(vec2);
                }
                case 3: {
                    IntVector3 vec3(input[0], input[1], input[2]);
                    env->ReleaseIntArrayElements(localIndex, (jint *) input, 0);
                    return GetVolumeIndexer3D(handle)->LocalIndexToDataIndex(vec3);
                }
                case 4: {
                    IntVector4 vec4(input[0], input[1], input[2], input[3]);
                    env->ReleaseIntArrayElements(localIndex, (jint *) input, 0);
                    return GetVolumeIndexer4D(handle)->LocalIndexToDataIndex(vec4);
                }
                case 5: {
                    IntVector<5> vec5(input[0], input[1], input[2], input[3], input[4]);
                    env->ReleaseIntArrayElements(localIndex, (jint *) input, 0);
                    return GetVolumeIndexer5D(handle)->LocalIndexToDataIndex(vec5);
                }
                case 6: {
                    IntVector<6> vec6(input[0], input[1], input[2], input[3], input[4], input[5]);
                    env->ReleaseIntArrayElements(localIndex, (jint *) input, 0);
                    return GetVolumeIndexer6D(handle)->LocalIndexToDataIndex(vec6);
                }
            }
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexerBase
    * Method:    cpVoxelIndexToDataIndex
    * Signature: (JI[I)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpVoxelIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension, jintArray voxelIndex)
    {
        try {
            jint *input = env->GetIntArrayElements(voxelIndex, 0);
            switch (volumeDimension) {
                case 2: {
                    IntVector2 vec2(input[0], input[1]);
                    env->ReleaseIntArrayElements(voxelIndex, (jint *) input, 0);
                    return GetVolumeIndexer2D(handle)->VoxelIndexToDataIndex(vec2);
                }
                case 3: {
                    IntVector3 vec3(input[0], input[1], input[2]);
                    env->ReleaseIntArrayElements(voxelIndex, (jint *) input, 0);
                    return GetVolumeIndexer3D(handle)->VoxelIndexToDataIndex(vec3);
                }
                case 4: {
                    IntVector4 vec4(input[0], input[1], input[2], input[3]);
                    env->ReleaseIntArrayElements(voxelIndex, (jint *) input, 0);
                    return GetVolumeIndexer4D(handle)->VoxelIndexToDataIndex(vec4);
                }
                case 5: {
                    IntVector<5> vec5(input[0], input[1], input[2], input[3], input[4]);
                    env->ReleaseIntArrayElements(voxelIndex, (jint *) input, 0);
                    return GetVolumeIndexer5D(handle)->VoxelIndexToDataIndex(vec5);
                }
                case 6: {
                    IntVector<6> vec6(input[0], input[1], input[2], input[3], input[4], input[5]);
                    env->ReleaseIntArrayElements(voxelIndex, (jint *) input, 0);
                    return GetVolumeIndexer6D(handle)->VoxelIndexToDataIndex(vec6);
                }
            }
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexerBase
    * Method:    cpLocalChunkIndexToDataIndex
    * Signature: (JI[I)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpLocalChunkIndexToDataIndex
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension, jintArray localChunkIndex)
    {
        try {
            jint *input = env->GetIntArrayElements(localChunkIndex, 0);
            switch (volumeDimension) {
                case 2: {
                    IntVector2 vec2(input[0], input[1]);
                    env->ReleaseIntArrayElements(localChunkIndex, (jint *) input, 0);
                    return GetVolumeIndexer2D(handle)->LocalChunkIndexToDataIndex(vec2);
                }
                case 3: {
                    IntVector3 vec3(input[0], input[1], input[2]);
                    env->ReleaseIntArrayElements(localChunkIndex, (jint *) input, 0);
                    return GetVolumeIndexer3D(handle)->LocalChunkIndexToDataIndex(vec3);
                }
                case 4: {
                    IntVector4 vec4(input[0], input[1], input[2], input[3]);
                    env->ReleaseIntArrayElements(localChunkIndex, (jint *) input, 0);
                    return GetVolumeIndexer4D(handle)->LocalChunkIndexToDataIndex(vec4);
                }
                case 5: {
                    IntVector<5> vec5(input[0], input[1], input[2], input[3], input[4]);
                    env->ReleaseIntArrayElements(localChunkIndex, (jint *) input, 0);
                    return GetVolumeIndexer5D(handle)->LocalChunkIndexToDataIndex(vec5);
                }
                case 6: {
                    IntVector<6> vec6(input[0], input[1], input[2], input[3], input[4], input[5]);
                    env->ReleaseIntArrayElements(localChunkIndex, (jint *) input, 0);
                    return GetVolumeIndexer6D(handle)->LocalChunkIndexToDataIndex(vec6);
                }
            }
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeIndexerBase
    * Method:    cpVoxelIndexInProcessArea
    * Signature: (JI[I)Z
    */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpVoxelIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension, jintArray voxelIndex)
    {
        try {
            jint *input = env->GetIntArrayElements(voxelIndex, 0);
            switch (volumeDimension) {
                case 2: {
                    IntVector2 vec2(input[0], input[1]);
                    env->ReleaseIntArrayElements(voxelIndex, (jint *) input, 0);
                    return GetVolumeIndexer2D(handle)->VoxelIndexInProcessArea(vec2);
                }
                case 3: {
                    IntVector3 vec3(input[0], input[1], input[2]);
                    env->ReleaseIntArrayElements(voxelIndex, (jint *) input, 0);
                    return GetVolumeIndexer3D(handle)->VoxelIndexInProcessArea(vec3);
                }
                case 4: {
                    IntVector4 vec4(input[0], input[1], input[2], input[3]);
                    env->ReleaseIntArrayElements(voxelIndex, (jint *) input, 0);
                    return GetVolumeIndexer4D(handle)->VoxelIndexInProcessArea(vec4);
                }
                case 5: {
                    IntVector<5> vec5(input[0], input[1], input[2], input[3], input[4]);
                    env->ReleaseIntArrayElements(voxelIndex, (jint *) input, 0);
                    return GetVolumeIndexer5D(handle)->VoxelIndexInProcessArea(vec5);
                }
                case 6: {
                    IntVector<6> vec6(input[0], input[1], input[2], input[3], input[4], input[5]);
                    env->ReleaseIntArrayElements(voxelIndex, (jint *) input, 0);
                    return GetVolumeIndexer6D(handle)->VoxelIndexInProcessArea(vec6);
                }
            }
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
     * Class:     org_opengroup_openvds_VolumeIndexerBase
     * Method:    cpLocalIndexInProcessArea
     * Signature: (JI[I)Z
     */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpLocalIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension, jintArray localIndex)
    {
        try {
            jint *input = env->GetIntArrayElements(localIndex, 0);
            switch (volumeDimension) {
                case 2: {
                    IntVector2 vec2(input[0], input[1]);
                    env->ReleaseIntArrayElements(localIndex, (jint *) input, 0);
                    return GetVolumeIndexer2D(handle)->LocalIndexInProcessArea(vec2);
                }
                case 3: {
                    IntVector3 vec3(input[0], input[1], input[2]);
                    env->ReleaseIntArrayElements(localIndex, (jint *) input, 0);
                    return GetVolumeIndexer3D(handle)->LocalIndexInProcessArea(vec3);
                }
                case 4: {
                    IntVector4 vec4(input[0], input[1], input[2], input[3]);
                    env->ReleaseIntArrayElements(localIndex, (jint *) input, 0);
                    return GetVolumeIndexer4D(handle)->LocalIndexInProcessArea(vec4);
                }
                case 5: {
                    IntVector<5> vec5(input[0], input[1], input[2], input[3], input[4]);
                    env->ReleaseIntArrayElements(localIndex, (jint *) input, 0);
                    return GetVolumeIndexer5D(handle)->LocalIndexInProcessArea(vec5);
                }
                case 6: {
                    IntVector<6> vec6(input[0], input[1], input[2], input[3], input[4], input[5]);
                    env->ReleaseIntArrayElements(localIndex, (jint *) input, 0);
                    return GetVolumeIndexer6D(handle)->LocalIndexInProcessArea(vec6);
                }
            }
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
     * Class:     org_opengroup_openvds_VolumeIndexerBase
     * Method:    cpLocalChunkIndexInProcessArea
     * Signature: (JI[I)Z
     */
    JNIEXPORT jboolean JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpLocalChunkIndexInProcessArea
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension, jintArray localChunkIndex)
    {
        try {
            jint *input = env->GetIntArrayElements(localChunkIndex, 0);
            switch (volumeDimension) {
                case 2: {
                    IntVector2 vec2(input[0], input[1]);
                    env->ReleaseIntArrayElements(localChunkIndex, (jint *) input, 0);
                    return GetVolumeIndexer2D(handle)->LocalChunkIndexInProcessArea(vec2);
                }
                case 3: {
                    IntVector3 vec3(input[0], input[1], input[2]);
                    env->ReleaseIntArrayElements(localChunkIndex, (jint *) input, 0);
                    return GetVolumeIndexer3D(handle)->LocalChunkIndexInProcessArea(vec3);
                }
                case 4: {
                    IntVector4 vec4(input[0], input[1], input[2], input[3]);
                    env->ReleaseIntArrayElements(localChunkIndex, (jint *) input, 0);
                    return GetVolumeIndexer4D(handle)->LocalChunkIndexInProcessArea(vec4);
                }
                case 5: {
                    IntVector<5> vec5(input[0], input[1], input[2], input[3], input[4]);
                    env->ReleaseIntArrayElements(localChunkIndex, (jint *) input, 0);
                    return GetVolumeIndexer5D(handle)->LocalChunkIndexInProcessArea(vec5);
                }
                case 6: {
                    IntVector<6> vec6(input[0], input[1], input[2], input[3], input[4], input[5]);
                    env->ReleaseIntArrayElements(localChunkIndex, (jint *) input, 0);
                    return GetVolumeIndexer6D(handle)->LocalChunkIndexInProcessArea(vec6);
                }
            }
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;

    }


#ifdef __cplusplus
}
#endif



