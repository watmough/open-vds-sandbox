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

#include <org_opengroup_openvds_MetadataContainer.h>
#include <CommonJni.h>
#include <OpenVDS/MetadataContainer.h>

using namespace OpenVDS;

#ifdef __cplusplus
extern "C" {
#endif
    inline MetadataContainer* GetAccess( jlong handle ) {
        return (MetadataContainer*)CheckHandle( handle );
    }

    /*
    * Class:     org_opengroup_openvds_MetadataContainer
    * Method:    cpCreateMetadataContainerHandle
    * Signature: ()J
    */
    JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_MetadataContainer_cpCreateMetadataContainerHandle
            (JNIEnv * env, jclass)
    {
        try {
            MetadataContainer* container = new MetadataContainer();
            return reinterpret_cast<jlong>(container);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;

        return -1L;
    }

    /*
    * Class:     org_opengroup_openvds_MetadataContainer
    * Method:    cpDeleteHandle
    * Signature: (J)J
    */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpDeleteHandle
            (JNIEnv * env, jclass, jlong handle) {
        try {
            delete GetAccess( handle );
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

    /*
     * Class:     org_opengroup_openvds_MetadataContainer
     * Method:    cpSetMetadataInt
     * Signature: (JLjava/lang/String;Ljava/lang/String;I)V
     */
    JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataInt
            (JNIEnv * env, jclass, jlong handle, jstring category, jstring name, jint value)
    {
        try {
            GetAccess( handle )->SetMetadataInt( JStringToString( env, category ).c_str(), JStringToString( env, name ).c_str(), value);
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
    }

/*
 * Class:     org_opengroup_openvds_MetadataContainer
 * Method:    cpSetMetadataIntVector2
 * Signature: (JLjava/lang/String;Ljava/lang/String;[I)V
 */
JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataIntVector2
        (JNIEnv * env, jclass, jlong handle, jstring category, jstring name, jintArray vec2i)
{
    try {
        std::vector<int> vec = JArrayToVector(env, vec2i);
        IntVector2 vdsVec;
        vdsVec[0] = vec[0];
        vdsVec[1] = vec[1];
        GetAccess( handle )->SetMetadataIntVector2( JStringToString( env, category ).c_str(), JStringToString( env, name ).c_str(), vdsVec);
    }
    CATCH_EXCEPTIONS_FOR_JAVA;
}

/*
 * Class:     org_opengroup_openvds_MetadataContainer
 * Method:    cpSetMetadataIntVector3
 * Signature: (JLjava/lang/String;Ljava/lang/String;[I)V
 */
JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataIntVector3
        (JNIEnv * env, jclass, jlong handle, jstring category, jstring name, jintArray vec3i)
{
    try {
        std::vector<int> vec = JArrayToVector(env, vec3i);
        IntVector3 vdsVec;
        vdsVec[0] = vec[0];
        vdsVec[1] = vec[1];
        vdsVec[2] = vec[2];
        GetAccess( handle )->SetMetadataIntVector3( JStringToString( env, category ).c_str(), JStringToString( env, name ).c_str(), vdsVec);
    }
    CATCH_EXCEPTIONS_FOR_JAVA;
}

/*
 * Class:     org_opengroup_openvds_MetadataContainer
 * Method:    cpSetMetadataIntVector4
 * Signature: (JLjava/lang/String;Ljava/lang/String;[I)V
 */
JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataIntVector4
        (JNIEnv * env, jclass, jlong handle, jstring category, jstring name, jintArray vec4i)
{
    try {
        std::vector<int> vec = JArrayToVector(env, vec4i);
        IntVector4 vdsVec;
        vdsVec[0] = vec[0];
        vdsVec[1] = vec[1];
        vdsVec[2] = vec[2];
        vdsVec[3] = vec[3];
        GetAccess( handle )->SetMetadataIntVector4( JStringToString( env, category ).c_str(), JStringToString( env, name ).c_str(), vdsVec);
    }
    CATCH_EXCEPTIONS_FOR_JAVA;
}

/*
 * Class:     org_opengroup_openvds_MetadataContainer
 * Method:    cpSetMetadataFloat
 * Signature: (JLjava/lang/String;Ljava/lang/String;F)V
 */
JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataFloat
        (JNIEnv * env, jclass, jlong handle, jstring category, jstring name, jfloat value)
{
    try {
        GetAccess( handle )->SetMetadataFloat( JStringToString( env, category ).c_str(), JStringToString( env, name ).c_str(), value);
    }
    CATCH_EXCEPTIONS_FOR_JAVA;
}

/*
 * Class:     org_opengroup_openvds_MetadataContainer
 * Method:    cpSetMetadataFloatVector2
 * Signature: (JLjava/lang/String;Ljava/lang/String;[F)V
 */
JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataFloatVector2
        (JNIEnv * env, jclass, jlong handle, jstring category, jstring name, jfloatArray vec2f)
{
    try {
        std::vector<float> vec = JArrayToVector(env, vec2f);
        FloatVector2 vdsVec;
        vdsVec[0] = vec[0];
        vdsVec[1] = vec[1];
        GetAccess( handle )->SetMetadataFloatVector2( JStringToString( env, category ).c_str(), JStringToString( env, name ).c_str(), vdsVec);
    }
    CATCH_EXCEPTIONS_FOR_JAVA;
}

/*
 * Class:     org_opengroup_openvds_MetadataContainer
 * Method:    cpSetMetadataFloatVector3
 * Signature: (JLjava/lang/String;Ljava/lang/String;[F)V
 */
JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataFloatVector3
        (JNIEnv * env, jclass, jlong handle, jstring category, jstring name, jfloatArray vec3f)
{
    try {
        std::vector<float> vec = JArrayToVector(env, vec3f);
        FloatVector3 vdsVec;
        vdsVec[0] = vec[0];
        vdsVec[1] = vec[1];
        vdsVec[2] = vec[2];
        GetAccess( handle )->SetMetadataFloatVector3( JStringToString( env, category ).c_str(), JStringToString( env, name ).c_str(), vdsVec);
    }
    CATCH_EXCEPTIONS_FOR_JAVA;
}

/*
 * Class:     org_opengroup_openvds_MetadataContainer
 * Method:    cpSetMetadataFloatVector4
 * Signature: (JLjava/lang/String;Ljava/lang/String;[F)V
 */
JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataFloatVector4
        (JNIEnv * env, jclass, jlong handle, jstring category, jstring name, jfloatArray vec4f)
{
    try {
        std::vector<float> vec = JArrayToVector(env, vec4f);
        FloatVector4 vdsVec;
        vdsVec[0] = vec[0];
        vdsVec[1] = vec[1];
        vdsVec[2] = vec[2];
        vdsVec[3] = vec[3];
        GetAccess( handle )->SetMetadataFloatVector4( JStringToString( env, category ).c_str(), JStringToString( env, name ).c_str(), vdsVec);
    }
    CATCH_EXCEPTIONS_FOR_JAVA;
}

/*
 * Class:     org_opengroup_openvds_MetadataContainer
 * Method:    cpSetMetadataDouble
 * Signature: (JLjava/lang/String;Ljava/lang/String;D)V
 */
JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataDouble
        (JNIEnv * env, jclass, jlong handle, jstring category, jstring name, jdouble value)
{
    try {
        GetAccess( handle )->SetMetadataDouble( JStringToString( env, category ).c_str(), JStringToString( env, name ).c_str(), value);
    }
    CATCH_EXCEPTIONS_FOR_JAVA;
}

/*
 * Class:     org_opengroup_openvds_MetadataContainer
 * Method:    cpSetMetadataDoubleVector2
 * Signature: (JLjava/lang/String;Ljava/lang/String;[D)V
 */
JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataDoubleVector2
        (JNIEnv * env, jclass, jlong handle, jstring category, jstring name, jdoubleArray vec2d)
{
    try {
        std::vector<double> vec = JArrayToVector(env, vec2d);
        DoubleVector2 vdsVec;
        vdsVec[0] = vec[0];
        vdsVec[1] = vec[1];
        GetAccess( handle )->SetMetadataDoubleVector2( JStringToString( env, category ).c_str(), JStringToString( env, name ).c_str(), vdsVec);
    }
    CATCH_EXCEPTIONS_FOR_JAVA;
}

/*
 * Class:     org_opengroup_openvds_MetadataContainer
 * Method:    cpSetMetadataDoubleVector3
 * Signature: (JLjava/lang/String;Ljava/lang/String;[D)V
 */
JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataDoubleVector3
        (JNIEnv * env, jclass, jlong handle, jstring category, jstring name, jdoubleArray vec3d)
{
    try {
        std::vector<double> vec = JArrayToVector(env, vec3d);
        DoubleVector3 vdsVec;
        vdsVec[0] = vec[0];
        vdsVec[1] = vec[1];
        vdsVec[2] = vec[2];
        GetAccess( handle )->SetMetadataDoubleVector3( JStringToString( env, category ).c_str(), JStringToString( env, name ).c_str(), vdsVec);
    }
    CATCH_EXCEPTIONS_FOR_JAVA;
}

/*
 * Class:     org_opengroup_openvds_MetadataContainer
 * Method:    cpSetMetadataDoubleVector4
 * Signature: (JLjava/lang/String;Ljava/lang/String;[D)V
 */
JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataDoubleVector4
        (JNIEnv * env, jclass, jlong handle, jstring category, jstring name, jdoubleArray vec4d)
{
    try {
        std::vector<double> vec = JArrayToVector(env, vec4d);
        DoubleVector4 vdsVec;
        vdsVec[0] = vec[0];
        vdsVec[1] = vec[1];
        vdsVec[2] = vec[2];
        vdsVec[3] = vec[3];
        GetAccess( handle )->SetMetadataDoubleVector4( JStringToString( env, category ).c_str(), JStringToString( env, name ).c_str(), vdsVec);
    }
    CATCH_EXCEPTIONS_FOR_JAVA;
}

/*
 * Class:     org_opengroup_openvds_MetadataContainer
 * Method:    cpSetMetadataString
 * Signature: (JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataString
        (JNIEnv * env, jclass, jlong handle, jstring category, jstring name, jstring value)
{
    try {
        GetAccess( handle )->SetMetadataString(JStringToString( env, category ).c_str(), JStringToString( env, name ).c_str(), JStringToString(env, value).c_str());
    }
    CATCH_EXCEPTIONS_FOR_JAVA;
}

#ifdef __cplusplus
}
#endif
