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

#include <org_opengroup_openvds_VolumeDataPage.h>
#include <CommonJni.h>
#include <OpenVDS/VolumeDataAccess.h>

#ifdef __cplusplus
extern "C" {
#endif

    inline OpenVDS::VolumeDataPage* GetVolumePage( jlong handle ) {
        return (OpenVDS::VolumeDataPage*) CheckHandle( handle );
    }

    /*
     * Class:     org_opengroup_openvds_VolumeDataPage
     * Method:    cpRelease
     * Signature: (J)V
     */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeDataPage_cpRelease
      (JNIEnv * env, jclass, jlong handle)
    {
        try {
            GetVolumePage( handle )->Release();
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeDataPage
    * Method:    cpGetWritableFloatBuffer
    * Signature: (J[I)[F
    */
    JNIEXPORT jfloatArray JNICALL Java_org_opengroup_openvds_VolumeDataPage_cpGetFloatBuffer
            (JNIEnv * env, jclass, jlong handle, jintArray pitchParam)
    {
        try {
            std::vector<int> vecPitch = JArrayToVector(env, pitchParam);
            int pitch[OpenVDS::Dimensionality_Max];
            int cMin[OpenVDS::Dimensionality_Max];
            int cMax[OpenVDS::Dimensionality_Max];
            GetVolumePage(handle)->GetMinMax(cMin, cMax);

            int chunk_size_i = cMax[2] - cMin[2];
            int chunk_size_x = cMax[1] - cMin[1];
            int chunk_size_z = cMax[0] - cMin[0];

            float* readData = reinterpret_cast<float*>(GetVolumePage(handle)->GetWritableBuffer(pitch));
            env->SetIntArrayRegion(pitchParam, 0, OpenVDS::Dimensionality_Max, pitch);

            int nbElem = chunk_size_i * chunk_size_x * chunk_size_z;
            return NewJFloatArray(env, readData, nbElem);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return NULL;
    }

#ifdef __cplusplus
}
#endif
