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

    inline OpenVDS::VolumeDataLayout *GetLayout(jlong handle) {
        return (OpenVDS::VolumeDataLayout *) CheckHandle(handle);
    }

    inline OpenVDS::VolumeDataPage *GetVolumePage(jlong handle) {
        return (OpenVDS::VolumeDataPage *) CheckHandle(handle);
    }

    /*
     * Class:     org_opengroup_openvds_VolumeIndexer5D
     * Method:    cpCreateVolumeIndexer5D
     * Signature: (JIIIJ)J
     */
    JNIEXPORT jlong JNICALL Java_org_opengroup_openvds_VolumeIndexer5D_cpCreateVolumeIndexer5D
            (JNIEnv * env, jclass, jlong vlPageHandle, jint channelIndex, jint lod, jint dimND, jlong layoutHandle)
    {
        try {
            VolumeDataPage* volumeDataPage = GetVolumePage(vlPageHandle);
            VolumeDataLayout* layout = GetLayout(layoutHandle);
            VolumeIndexer5D* indexer5D = new VolumeIndexer5D(volumeDataPage, channelIndex, lod, (DimensionsND)dimND, layout);
            return (jlong)indexer5D;
        }
        CATCH_EXCEPTIONS_FOR_JAVA;
        return 0;
    }

#ifdef __cplusplus
}
#endif



