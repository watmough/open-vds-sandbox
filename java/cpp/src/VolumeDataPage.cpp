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
#include <VDS/VolumeDataPageImpl.h>

#ifdef __cplusplus
extern "C" {
#endif

    inline OpenVDS::VolumeDataPage* GetVolumePage( jlong handle ) {
        return (OpenVDS::VolumeDataPage*) CheckHandle( handle );
    }

    inline int GetBufferAllocatedSize(OpenVDS::VolumeDataPage* page) {
        OpenVDS::VolumeDataPageImpl* pageImpl = (OpenVDS::VolumeDataPageImpl*)page;
        const OpenVDS::DataBlock block = pageImpl->GetDataBlock();
        int nbElem = 1;
        for (int i = 0 ; i < OpenVDS::DataBlock::Dimensionality::Dimensionality_Max ; i++) {
            nbElem *= block.AllocatedSize[i];
        }
        return nbElem;
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
            env->SetIntArrayRegion(minArr, 0, OpenVDS::Dimensionality_Max, (jint*) cMin);
            env->SetIntArrayRegion(maxArr, 0, OpenVDS::Dimensionality_Max, (jint*) cMax);
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
            env->SetIntArrayRegion(minArr, 0, OpenVDS::Dimensionality_Max, (jint*) cMin);
            env->SetIntArrayRegion(maxArr, 0, OpenVDS::Dimensionality_Max, (jint*) cMax);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeDataPage
    * Method:    cpGetAllocatedSize
    * Signature: (J)I
    */
    JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VolumeDataPage_cpGetAllocatedSize
            (JNIEnv * env, jclass, jlong handle)
    {
        try {
            return GetBufferAllocatedSize(GetVolumePage(handle));
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return -1;
    }

    /*
     * Class:     org_opengroup_openvds_VolumeDataPage
     * Method:    cpGetPitch
     * Signature: (J[I)[I
     */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_VolumeDataPage_cpGetPitch
            (JNIEnv * env, jclass, jlong handle, jintArray pitchArray)
    {
        try {
            int pitch[OpenVDS::Dimensionality_Max];
            OpenVDS::VolumeDataPage* page = GetVolumePage(handle);
            page->GetBuffer(pitch);
            env->SetIntArrayRegion(pitchArray, 0, OpenVDS::Dimensionality_Max, (jint *)pitch);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
    * Class:     org_opengroup_openvds_VolumeDataPage
    * Method:    cpGetByteBuffer
    * Signature: (J[I)[B
    */
    JNIEXPORT jbyteArray JNICALL Java_org_opengroup_openvds_VolumeDataPage_cpGetByteBuffer
            (JNIEnv * env, jclass, jlong handle, jintArray pitchParam, jint lod)
    {
        try {
            int pitch[OpenVDS::Dimensionality_Max];
            OpenVDS::VolumeDataPage* page = GetVolumePage(handle);

            const char* readData = static_cast<const char*>(page->GetBuffer(pitch));
            env->SetIntArrayRegion(pitchParam, 0, OpenVDS::Dimensionality_Max, (jint *)pitch);
            int nbElem = GetBufferAllocatedSize(page);
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
            (JNIEnv * env, jclass, jlong handle, jintArray pitchParam, jint lod)
    {
        try {
            int pitch[OpenVDS::Dimensionality_Max];
            OpenVDS::VolumeDataPage* page = GetVolumePage(handle);

            const float* readData = static_cast<const float*>(page->GetBuffer(pitch));
            env->SetIntArrayRegion(pitchParam, 0, OpenVDS::Dimensionality_Max, (jint *)pitch);
            int nbElem = GetBufferAllocatedSize(page);
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
            (JNIEnv * env, jclass, jlong handle, jintArray pitchParam, jint lod)
    {
        try {
            int pitch[OpenVDS::Dimensionality_Max];
            OpenVDS::VolumeDataPage* page = GetVolumePage(handle);

            const double* readData = static_cast<const double*>(page->GetBuffer(pitch));
            env->SetIntArrayRegion(pitchParam, 0, OpenVDS::Dimensionality_Max, (jint *)pitch);
            int nbElem = GetBufferAllocatedSize(page);
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
            env->ReleaseDoubleArrayElements(values, src, 0);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

#ifdef __cplusplus
}
#endif
