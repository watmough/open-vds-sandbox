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
#include <iostream>

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
    * Method:    cpGetMinMax
    * Signature: (J[I[I)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeDataPage_cpGetMinMax
            (JNIEnv * env, jclass, jlong handle, jintArray minArr, jintArray maxArr)
    {
        try {
            int cMin[OpenVDS::Dimensionality_Max];
            int cMax[OpenVDS::Dimensionality_Max];
            GetVolumePage(handle)->GetMinMax(cMin, cMax);
            env->SetIntArrayRegion(minArr, 0, OpenVDS::Dimensionality_Max, cMin);
            env->SetIntArrayRegion(maxArr, 0, OpenVDS::Dimensionality_Max, cMax);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeDataPage
    * Method:    cpGetMinMaxExcludingMargin
    * Signature: (J[I[I)V
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeDataPage_cpGetMinMaxExcludingMargin
            (JNIEnv * env, jclass, jlong handle, jintArray minArr, jintArray maxArr)
    {
        try {
            int cMin[OpenVDS::Dimensionality_Max];
            int cMax[OpenVDS::Dimensionality_Max];
            GetVolumePage(handle)->GetMinMaxExcludingMargin(cMin, cMax);
            env->SetIntArrayRegion(minArr, 0, OpenVDS::Dimensionality_Max, cMin);
            env->SetIntArrayRegion(maxArr, 0, OpenVDS::Dimensionality_Max, cMax);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeDataPage
    * Method:    cpGetByteBuffer
    * Signature: (J[I)[B
    */
    JNIEXPORT jbyteArray JNICALL Java_org_opengroup_openvds_VolumeDataPage_cpGetByteBuffer
            (JNIEnv * env, jclass, jlong handle, jintArray pitchParam)
    {
        try {
            int pitch[OpenVDS::Dimensionality_Max];
            int cMin[OpenVDS::Dimensionality_Max];
            int cMax[OpenVDS::Dimensionality_Max];
            GetVolumePage(handle)->GetMinMax(cMin, cMax);

            int chunk_size_y = cMax[2] - cMin[2];

            const char* readData = reinterpret_cast<const char*>(GetVolumePage(handle)->GetBuffer(pitch));
            env->SetIntArrayRegion(pitchParam, 0, OpenVDS::Dimensionality_Max, pitch);

            int nbElem = chunk_size_y * pitch[2];
            return NewJByteArray(env, readData, nbElem);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return NULL;
    }
    /*
     * Class:     org_opengroup_openvds_VolumeDataPage
     * Method:    cpSetByteBuffer
     * Signature: (J[B)V
     */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeDataPage_cpSetByteBuffer
            (JNIEnv * env, jclass, jlong handle, jbyteArray values)
    {
        try {
            OpenVDS::VolumeDataPage * page = GetVolumePage(handle);
            int pitch[OpenVDS::Dimensionality_Max];
            unsigned char* pageBuffer = reinterpret_cast<unsigned char*>(page->GetWritableBuffer(pitch));
            int valueSize = env->GetArrayLength(values);
            jbyte *src = env->GetByteArrayElements(values, 0);
            std::memcpy(pageBuffer, src, valueSize * sizeof (char));
            // needed or not?
            env->ReleaseByteArrayElements(values, src, 0);
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
            int pitch[OpenVDS::Dimensionality_Max];
            int cMin[OpenVDS::Dimensionality_Max];
            int cMax[OpenVDS::Dimensionality_Max];
            GetVolumePage(handle)->GetMinMax(cMin, cMax);

            int chunk_size_y = cMax[2] - cMin[2];

            const float* readData = static_cast<const float*>(GetVolumePage(handle)->GetBuffer(pitch));
            env->SetIntArrayRegion(pitchParam, 0, OpenVDS::Dimensionality_Max, pitch);

            int nbElem = chunk_size_y * pitch[2];
            return NewJFloatArray(env, readData, nbElem);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return NULL;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeDataPage
    * Method:    cpSetFloatBuffer
    * Signature: (J[F[I)[F
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeDataPage_cpSetFloatBuffer
            (JNIEnv * env, jclass, jlong handle, jfloatArray values)
    {
        try {
            OpenVDS::VolumeDataPage * page = GetVolumePage(handle);
            int pitch[OpenVDS::Dimensionality_Max];
            float* pageBuffer = reinterpret_cast<float*>(page->GetWritableBuffer(pitch));
            int valueSize = env->GetArrayLength(values);
            jfloat *src = env->GetFloatArrayElements(values, 0);
            std::memcpy(pageBuffer, src, valueSize * sizeof (float));
            // needed or not?
            env->ReleaseFloatArrayElements(values, src, 0);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeDataPage
    * Method:    cpGetDoubleBuffer
    * Signature: (J[I)[D
    */
    JNIEXPORT jdoubleArray JNICALL Java_org_opengroup_openvds_VolumeDataPage_cpGetDoubleBuffer
            (JNIEnv * env, jclass, jlong handle, jintArray pitchParam)
    {
        try {
            int pitch[OpenVDS::Dimensionality_Max];
            int cMin[OpenVDS::Dimensionality_Max];
            int cMax[OpenVDS::Dimensionality_Max];
            GetVolumePage(handle)->GetMinMax(cMin, cMax);

            int chunk_size_j = cMax[2] - cMin[2];

            const double* readData = reinterpret_cast<const double*>(GetVolumePage(handle)->GetBuffer(pitch));
            env->SetIntArrayRegion(pitchParam, 0, OpenVDS::Dimensionality_Max, pitch);

            int nbElem = chunk_size_j * pitch[2];
            return NewJDoubleArray(env, readData, nbElem);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return NULL;
    }

    /*
     * Class:     org_opengroup_openvds_VolumeDataPage
     * Method:    cpSetDoubleBuffer
     * Signature: (J[D)V
     */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeDataPage_cpSetDoubleBuffer
            (JNIEnv * env, jclass, jlong handle, jdoubleArray values)
    {
        try {
            OpenVDS::VolumeDataPage * page = GetVolumePage(handle);
            int pitch[OpenVDS::Dimensionality_Max];
            double* pageBuffer = reinterpret_cast<double*>(page->GetWritableBuffer(pitch));
            int valueSize = env->GetArrayLength(values);
            jdouble *src = env->GetDoubleArrayElements(values, 0);
            std::memcpy(pageBuffer, src, valueSize * sizeof (double));
            // needed or not?
            env->ReleaseDoubleArrayElements(values, src, 0);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

#ifdef __cplusplus
}
#endif
