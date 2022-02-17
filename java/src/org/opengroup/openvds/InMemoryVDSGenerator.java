/*
 * Copyright 2019 The Open Group
 * Copyright 2019 INT, Inc.
 * Copyright 2022 Bluware, Inc.
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

package org.opengroup.openvds;

public class InMemoryVDSGenerator implements AutoCloseable {
    static private native long CreateVDSImpl(int nXSamples, int nYSamples, int nZSamples, long format);

    protected VDS vds;

    public static VDS createVDS(int nXSamples, int nYSamples, int nZSamples, VolumeDataChannelDescriptor.Format format) {
        return VDS.fromNativeObject(CreateVDSImpl(nXSamples, nYSamples, nZSamples, format.value()));
    }

    public InMemoryVDSGenerator(int nXSamples, int nYSamples, int nZSamples, VolumeDataChannelDescriptor.Format format) {
        this.vds = createVDS(nXSamples, nYSamples, nZSamples, format);
    }

    public VDS getVDS()
    {
        return this.vds;
    }

    public VolumeDataLayout getLayout() {
        return getVDS().getLayout();
    }

    public VolumeDataAccessManager getAccessManager() {
        return getVDS().getAccessManager();
    }

    public void close() {
        this.vds.close();
    }
}
