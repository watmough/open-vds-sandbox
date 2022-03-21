/*
 * Copyright 2020 The Open Group
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

package test.org.opengroup.openvds;

import org.opengroup.openvds.*;
import org.junit.Test;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import static org.junit.Assert.*;
import static org.junit.Assert.assertThrows;

import static org.opengroup.openvds.VolumeDataChannelDescriptor.Format;
import static org.opengroup.openvds.VolumeDataLayoutDescriptor.BrickSize;
import static org.opengroup.openvds.VolumeDataLayoutDescriptor.LODLevels;
import static org.opengroup.openvds.VolumeDataChannelDescriptor.Components;

public class CreateVDSTest {
    @Before
    public void init() {
        vds = new InMemoryVDSGenerator(16, 16, 16, Format.U8);
        url = "inmemory://create_test";
        o = new AzureOpenOptions();
        error = new VDSError();
        VolumeDataLayout volumeDataLayout = vds.getLayout();

        int nbChannel = volumeDataLayout.getChannelCount();
        VolumeDataAccessManager accessManager = vds.getAccessManager();

        for (VolumeDataLayoutDescriptor.LODLevels l : VolumeDataLayoutDescriptor.LODLevels.values()) {
            for (int channel = 0; channel < nbChannel; channel++) {
                for (DimensionsND dimGroup : DimensionsND.values()) {
                    VDSProduceStatus vdsProduceStatus = accessManager.getVDSProduceStatus(dimGroup, l.ordinal(), channel);
                }
            }
        }

        vda = new VolumeDataAxisDescriptor[] {volumeDataLayout.getAxisDescriptor(0),
                                                volumeDataLayout.getAxisDescriptor(1),
                                                volumeDataLayout.getAxisDescriptor(2)};
        vdc = new VolumeDataChannelDescriptor[] {volumeDataLayout.getChannelDescriptor(0)};

        md = volumeDataLayout;
        ld = volumeDataLayout.getLayoutDescriptor();
    }


    @Test
    public void testCreateVDS() {
        VDS openvds1 = OpenVDS.create(url, "", ld, vda, vdc, md, error);
        openvds1.getAccessManager().flushUploadQueue();

        VDS openvds2 = OpenVDS.open(url, "", error);

        VolumeDataLayout layout = openvds2.getLayout();

        assertEquals(layout.getDimensionality(), 3);
        assertEquals(layout.getChannelCount(), 1);
        assertEquals(layout.getChannelFormat(0), Format.U8);
        assertEquals(layout.getDimensionName(1), openvds1.getLayout().getDimensionName(1));
    }

    @Test
    public void testException1() {
        assertThrows(IllegalArgumentException.class, () -> { OpenVDS.create((OpenOptions)null, ld, vda, vdc, md, error); });
    }

    @Test
    public void testException2() {
        assertThrows(IllegalArgumentException.class, () -> { OpenVDS.create(o, null, vda, vdc, md, error); });
    }

    @Test
    public void testException3() {
        assertThrows(IllegalArgumentException.class, () -> { OpenVDS.create(o, ld, null, vdc, md, error); });
    }

    @Test
    public void testException4() {
        assertThrows(IllegalArgumentException.class, () -> { OpenVDS.create(o, ld, vda, null, md, error); });
    }

    @Test
    public void testException5() {
        assertThrows(IllegalArgumentException.class, () -> { OpenVDS.create(o, ld, vda, vdc, null, error); });
    }

    @Test
    public void testException6() {
        assertThrows(IllegalArgumentException.class, () -> { OpenVDS.create(o, ld, vda, vdc, md, null); });
    }

    public AzureOpenOptions o;
    public String url;
    public VolumeDataLayoutDescriptor ld;
    public VolumeDataAxisDescriptor[] vda;
    public VolumeDataChannelDescriptor[] vdc;
    public MetadataReadAccess md;
    public InMemoryVDSGenerator vds;
    public VDSError error;
}
