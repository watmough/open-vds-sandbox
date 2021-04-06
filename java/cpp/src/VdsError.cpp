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

#include <org_opengroup_openvds_VdsError.h>
#include <CommonJni.h>
#include <OpenVDS/OpenVDS.h>

using namespace OpenVDS;

#ifdef __cplusplus
extern "C" {
#endif

inline Error* GetError( jlong handle ) {
    return (Error*)CheckHandle( handle );
}

/*
 * Class:     org_opengroup_openvds_VdsError
 * Method:    cpCreateErrorHandle
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VdsError_cpCreateErrorHandle
        (JNIEnv * env, jclass)
{
    try {
        Error *error = new Error();
        return reinterpret_cast<jlong>(error);
    }
    CATCH_EXCEPTIONS_FOR_JAVA;

    return -1L;
}

/*
 * Class:     org_opengroup_openvds_VdsError
 * Method:    cpDeleteHandle
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_org_opengroup_openvds_VdsError_cpDeleteHandle
        (JNIEnv * env, jclass, jlong handle)
{
    try {
        delete &GetError( handle )->string;
        delete GetError( handle );
    }
    CATCH_EXCEPTIONS_FOR_JAVA;
}

/*
 * Class:     org_opengroup_openvds_VdsError
 * Method:    cpSetErrorCode
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_org_opengroup_openvds_VdsError_cpSetErrorCode
        (JNIEnv * env, jclass, jlong handle, jint errorCode)
{
    try {
        GetError( handle )->code = errorCode;
    }
    CATCH_EXCEPTIONS_FOR_JAVA;
}

/*
 * Class:     org_opengroup_openvds_VdsError
 * Method:    cpSetErrorMessage
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_org_opengroup_openvds_VdsError_cpSetErrorMessage
        (JNIEnv * env, jclass, jlong handle, jstring errorMsg)
{
    try {
        GetError( handle )->string = JStringToString( env, errorMsg ).c_str();
    }
    CATCH_EXCEPTIONS_FOR_JAVA;
}

/*
 * Class:     org_opengroup_openvds_VdsError
 * Method:    cpGetErrorCode
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_org_opengroup_openvds_VdsError_cpGetErrorCode
        (JNIEnv * env, jclass, jlong handle)
{
    try {
        return GetError(handle)->code;
    }
    CATCH_EXCEPTIONS_FOR_JAVA;
    return 0;
}

/*
 * Class:     org_opengroup_openvds_VdsError
 * Method:    cpGetErrorMessage
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_opengroup_openvds_VdsError_cpGetErrorMessage
        (JNIEnv * env, jclass, jlong handle)
{
    try {
        return NewJString(env, GetError(handle)->string);
    }
    CATCH_EXCEPTIONS_FOR_JAVA;
    return 0;
}


#ifdef __cplusplus
}
#endif
