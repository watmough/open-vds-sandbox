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
//
///*
// * Class:     org_opengroup_openvds_MetadataContainer
// * Method:    cpSetMetadataIntVector2
// * Signature: (JLjava/lang/String;Ljava/lang/String;[I)V
// */
//JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataIntVector2
//        (JNIEnv *, jclass, jlong, jstring, jstring, jintArray);
//
///*
// * Class:     org_opengroup_openvds_MetadataContainer
// * Method:    cpSetMetadataIntVector3
// * Signature: (JLjava/lang/String;Ljava/lang/String;[I)V
// */
//JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataIntVector3
//        (JNIEnv *, jclass, jlong, jstring, jstring, jintArray);
//
///*
// * Class:     org_opengroup_openvds_MetadataContainer
// * Method:    cpSetMetadataIntVector4
// * Signature: (JLjava/lang/String;Ljava/lang/String;[I)V
// */
//JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataIntVector4
//        (JNIEnv *, jclass, jlong, jstring, jstring, jintArray);
//
///*
// * Class:     org_opengroup_openvds_MetadataContainer
// * Method:    cpSetMetadataFloat
// * Signature: (JLjava/lang/String;Ljava/lang/String;F)V
// */
//JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataFloat
//        (JNIEnv *, jclass, jlong, jstring, jstring, jfloat);
//
///*
// * Class:     org_opengroup_openvds_MetadataContainer
// * Method:    cpSetMetadataFloatVector2
// * Signature: (JLjava/lang/String;Ljava/lang/String;[F)V
// */
//JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataFloatVector2
//        (JNIEnv *, jclass, jlong, jstring, jstring, jfloatArray);
//
///*
// * Class:     org_opengroup_openvds_MetadataContainer
// * Method:    cpSetMetadataFloatVector3
// * Signature: (JLjava/lang/String;Ljava/lang/String;[F)V
// */
//JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataFloatVector3
//        (JNIEnv *, jclass, jlong, jstring, jstring, jfloatArray);
//
///*
// * Class:     org_opengroup_openvds_MetadataContainer
// * Method:    cpSetMetadataFloatVector4
// * Signature: (JLjava/lang/String;Ljava/lang/String;[F)V
// */
//JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataFloatVector4
//        (JNIEnv *, jclass, jlong, jstring, jstring, jfloatArray);
//
///*
// * Class:     org_opengroup_openvds_MetadataContainer
// * Method:    cpSetMetadataDouble
// * Signature: (JLjava/lang/String;Ljava/lang/String;D)V
// */
//JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataDouble
//        (JNIEnv *, jclass, jlong, jstring, jstring, jdouble);
//
///*
// * Class:     org_opengroup_openvds_MetadataContainer
// * Method:    cpSetMetadataDoubleVector2
// * Signature: (JLjava/lang/String;Ljava/lang/String;[D)V
// */
//JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataDoubleVector2
//        (JNIEnv *, jclass, jlong, jstring, jstring, jdoubleArray);
//
///*
// * Class:     org_opengroup_openvds_MetadataContainer
// * Method:    cpSetMetadataDoubleVector3
// * Signature: (JLjava/lang/String;Ljava/lang/String;[D)V
// */
//JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataDoubleVector3
//        (JNIEnv *, jclass, jlong, jstring, jstring, jdoubleArray);
//
///*
// * Class:     org_opengroup_openvds_MetadataContainer
// * Method:    cpSetMetadataDoubleVector4
// * Signature: (JLjava/lang/String;Ljava/lang/String;[D)V
// */
//JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataDoubleVector4
//        (JNIEnv *, jclass, jlong, jstring, jstring, jdoubleArray);
//
///*
// * Class:     org_opengroup_openvds_MetadataContainer
// * Method:    cpSetMetadataString
// * Signature: (JLjava/lang/String;Ljava/lang/String;Ljava/lang/String;)V
// */
//JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataString
//        (JNIEnv *, jclass, jlong, jstring, jstring, jstring);
//
///*
// * Class:     org_opengroup_openvds_MetadataContainer
// * Method:    cpSetMetadataKeys
// * Signature: (J[Lorg/opengroup/openvds/MetadataKey;)V
// */
//JNIEXPORT void JNICALL Java_org_opengroup_openvds_MetadataContainer_cpSetMetadataKeys
//        (JNIEnv *, jclass, jlong, jobjectArray);

#ifdef __cplusplus
}
#endif
