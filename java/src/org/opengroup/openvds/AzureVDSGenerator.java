/*
 * Copyright 2019 The Open Group
 * Copyright 2019 INT, Inc.
 * Copyright 2020 Bluware, Inc.
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


public class AzureVDSGenerator implements AutoCloseable {
    static {
        ManagedBuffer.staticInit();
    }

    private static native long CreateVDSImpl(long o, int nXSamples, int nYSamples, int nZSamples, long format, String[] channels, String[] units);

    protected VDS vds;

	public static VDS createVDS(AzureOpenOptions o, int nXSamples, int nYSamples, int nZSamples, VolumeDataFormat format, String[] channels, String[] units) {
		return VDS.fromNativeObject(CreateVDSImpl(o.getNativeObject(), nXSamples, nYSamples, nZSamples, format.value(), channels, units));
	}
	
    public AzureVDSGenerator(AzureOpenOptions o, int nXSamples, int nYSamples, int nZSamples, VolumeDataFormat format, String[] channels) {
		this.vds = createVDS(o, nXSamples, nYSamples, nZSamples, format, channels, null);
    }

    public AzureVDSGenerator(AzureOpenOptions o, int nXSamples, int nYSamples, int nZSamples, VolumeDataFormat format, String[] channels, String[] units) {
		this.vds = createVDS(o, nXSamples, nYSamples, nZSamples, format, channels, units);
    }

    public AzureVDSGenerator(AzureOpenOptions o, int nXSamples, int nYSamples, VolumeDataFormat format, String[] channels, String[] units) {
		this.vds = createVDS(o, nXSamples, nYSamples, 0, format, channels, units);
    }
    public AzureVDSGenerator(AzureOpenOptions o, int nXSamples, int nYSamples, VolumeDataFormat format, String[] channels) {
        this.vds = createVDS(o, nXSamples, nYSamples, 0, format, channels, null);
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

    public boolean isNull() {
        return this.vds.isNull();
    }
}
