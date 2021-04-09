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
    * Method:    cpLocalIndexToVoxelIndex
    * Signature: (JI[I)[I
    */
    JNIEXPORT jintArray JNICALL Java_org_opengroup_openvds_VolumeIndexerBase_cpLocalIndexToVoxelIndex
            (JNIEnv * env, jclass, jlong handle, jint volumeDimension, jintArray localIndex)
    {
        try {
            std::vector<int> vecLI = JArrayToVector(env, localIndex);
            std::vector<int> vecRes(volumeDimension, 0);
            int arrayRes[volumeDimension];
            switch (volumeDimension) {
                case 2: {
                    IntVector2 vec2;
                    for (int i = 0; i < volumeDimension; ++i) {
                        vec2[i] = vecLI[i];
                    }
                    IntVector2 res2 = GetVolumeIndexer2D(handle)->LocalIndexToVoxelIndex(vec2);
                    for (int i = 0; i < volumeDimension; ++i) {
                        vecRes[i] = res2[i];
                    }
                    break;
                }
                case 3: {
                    IntVector3 vec3;
                    for (int i = 0; i < volumeDimension; ++i) {
                        vec3[i] = vecLI[i];
                    }
                    IntVector3 res3 = GetVolumeIndexer3D(handle)->LocalIndexToVoxelIndex(vec3);
                    for (int i = 0; i < volumeDimension; ++i) {
                        vecRes[i] = res3[i];
                    }
                    break;
                }
                case 4:{
                    IntVector4 vec4;
                    for (int i = 0; i < volumeDimension; ++i) {
                        vec4[i] = vecLI[i];
                    }
                    IntVector4 res4 = GetVolumeIndexer4D(handle)->LocalIndexToVoxelIndex(vec4);
                    for (int i = 0; i < volumeDimension; ++i) {
                        vecRes[i] = res4[i];
                    }
                    break;
                }
                case 5: {
                    IntVector<5> vec5;
                    for (int i = 0; i < volumeDimension; ++i) {
                        vec5[i] = vecLI[i];
                    }
                    IntVector<5> res5 = GetVolumeIndexer5D(handle)->LocalIndexToVoxelIndex(vec5);
                    for (int i = 0; i < volumeDimension; ++i) {
                        vecRes[i] = res5[i];
                    }
                    break;
                }
                case 6: {
                    IntVector<6> vec6;
                    for (int i = 0; i < volumeDimension; ++i) {
                        vec6[i] = vecLI[i];
                    }
                    IntVector<6> res6 = GetVolumeIndexer6D(handle)->LocalIndexToVoxelIndex(vec6);
                    for (int i = 0; i < volumeDimension; ++i) {
                        vecRes[i] = res6[i];
                    }
                    break;
                }
            }
            for (int i = 0; i < volumeDimension; ++i) {
                arrayRes[i] = vecRes[i];
            }
            return NewJIntArray(env, arrayRes, volumeDimension);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return NULL;
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
            switch (volumeDimension) {
                case 2: {
                    std::vector<int> vecLI = JArrayToVector(env, localIndex);
                    IntVector2 vec2;
                    for (int i = 0; i < volumeDimension; ++i) {
                        vec2[i] = vecLI[i];
                    }
                    return GetVolumeIndexer2D(handle)->LocalIndexToDataIndex(vec2);
                }
                case 3: {
                    std::vector<int> vecLI = JArrayToVector(env, localIndex);
                    IntVector3 vec3;
                    for (int i = 0; i < volumeDimension; ++i) {
                        vec3[i] = vecLI[i];
                    }
                    return GetVolumeIndexer3D(handle)->LocalIndexToDataIndex(vec3);
                }
                case 4: {
                    std::vector<int> vecLI = JArrayToVector(env, localIndex);
                    IntVector4 vec4;
                    for (int i = 0; i < volumeDimension; ++i) {
                        vec4[i] = vecLI[i];
                    }
                    return GetVolumeIndexer4D(handle)->LocalIndexToDataIndex(vec4);
                }
                case 5: {
                    std::vector<int> vecLI = JArrayToVector(env, localIndex);
                    IntVector<5> vec5;
                    for (int i = 0; i < volumeDimension; ++i) {
                        vec5[i] = vecLI[i];
                    }
                    return GetVolumeIndexer5D(handle)->LocalIndexToDataIndex(vec5);
                }
                case 6: {
                    std::vector<int> vecLI = JArrayToVector(env, localIndex);
                    IntVector<6> vec6;
                    for (int i = 0; i < volumeDimension; ++i) {
                        vec6[i] = vecLI[i];
                    }
                    return GetVolumeIndexer6D(handle)->LocalIndexToDataIndex(vec6);
                }
            }
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

#ifdef __cplusplus
}
#endif



