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

import org.junit.*;
import org.opengroup.openvds.*;

import static org.junit.Assert.*;
import java.nio.FloatBuffer;

import static org.opengroup.openvds.VolumeDataFormat.*;
import static org.opengroup.openvds.VolumeDataComponents.*;
import static org.opengroup.openvds.VolumeDataLayoutDescriptor.BrickSize;
import static org.opengroup.openvds.VolumeDataLayoutDescriptor.LODLevels;

public class WriteDataTest {
    @Test
    public void roundtrip_2d() {
        final int nz = 113;
        final int nx = 128;

        VDS vds;
        VDSError vdsError = new VDSError();
        String[] names = new String[]{"chan1", "chan2"};
        VolumeDataFormat f = Format_R32;
        OpenOptions options;

        String connectionString = System.getenv("CONNECTION_STRING");
        if (connectionString == null) {
            options = new InMemoryOpenOptions("rtrip_2d");
            vds = CreateVDSTest.createVDS(nz, nx, f, options, CreateVDSTest.createDefaultChannelDescriptors(names, f), vdsError);
        } else {
            options = new AzureOpenOptions(connectionString, "test", "rtrip_2d");
            AzureVDSGenerator generator = new AzureVDSGenerator((AzureOpenOptions)options, nz, nx, f, names);
            vds = generator.getVDS();
        }

        java.util.Random r = new java.util.Random();

        double[] src1 = r.doubles(nx * nz).toArray();
        double[] src2 = r.doubles(nx * nz).toArray();

        double[] data = src1;

        OpenVDS.writeArray(vds, src1, "chan1");
        OpenVDS.writeArray(vds, src2, "chan2");

        vds.getAccessManager().flushUploadQueue();
        vds.close();

        try (VDS vds2 = OpenVDS.open(options, vdsError);
             VolumeDataAccessManager access = vds2.getAccessManager();) {
            VolumeDataLayout layout = vds2.getLayout();
            int[] min = new int[]{0, 0, 0, 0, 0, 0};
            int[] max = new int[]{nz, nx, 0, 0, 0, 0};
            int channel = layout.getChannelIndex("chan1");
            DimensionsND dims = DimensionsND.Dimensions_01;
            try (VolumeDataRequest req = access.requestVolumeSubset(dims, 0, channel, min, max, Format_R32)) {
                req.waitForCompletion();
                FloatBuffer outbuf = req.getBuffer().asFloatBuffer();
                float[] arr = new float[outbuf.remaining()];
                outbuf.get(arr);
                assertEquals(arr[0], src1[0], 1e-6);
                assertEquals(arr[1], src1[1], 1e-6);
                assertEquals(arr[nx], src1[nx], 1e-6);
            } catch (Exception e) {
                fail(e.getMessage());
            }
        }
    }


    @Test
    public void roundtrip_3d() {
        final int nz = 113;
        final int ny = 107;
        final int nx = 108;

        VDS vds;
        VDSError vdsError = new VDSError();
        String[] names = new String[]{"chan1", "chan2"};
        OpenOptions options;

        String connectionString = System.getenv("CONNECTION_STRING");
        VolumeDataFormat f = Format_R32;
        if (connectionString == null) {
            options = new InMemoryOpenOptions("rtrip_3d");
            vds = CreateVDSTest.createVDS(nz, ny, nx, f, options, CreateVDSTest.createDefaultChannelDescriptors(names, f), vdsError);
        } else {
            options = new AzureOpenOptions(connectionString, "test", "rtrip_3d");
            AzureVDSGenerator generator = new AzureVDSGenerator((AzureOpenOptions)options, nz, ny, nx, f, names);
            vds = generator.getVDS();
        }

        java.util.Random r = new java.util.Random();

        double[] src1 = r.doubles(nx * ny * nz).toArray();


        float[] src2 = new float[src1.length];
        for (int i = 0; i < src2.length; i++)
            src2[i] = i;

        OpenVDS.writeArray(vds, src1, "chan1");
        OpenVDS.writeArray(vds, src2, "chan2");

        vds.getAccessManager().flushUploadQueue();
        vds.close();

        try (VDS vds2 = OpenVDS.open(options, vdsError);
             VolumeDataAccessManager access = vds2.getAccessManager();) {
            VolumeDataLayout layout = vds2.getLayout();
            int[] min = new int[]{0, 0, 0, 0, 0, 0};
            int[] max = new int[]{nz, ny, nx, 0, 0, 0};
            int channel = layout.getChannelIndex("chan2");
            DimensionsND dims = DimensionsND.Dimensions_012;
            try (VolumeDataRequest req = access.requestVolumeSubset(dims, 0, channel, min, max, Format_R32)) {
                req.waitForCompletion();
                FloatBuffer outbuf = req.getBuffer().asFloatBuffer();
                float[] arr = new float[outbuf.remaining()];
                outbuf.get(arr);
                assertEquals(arr[0], src2[0], 1e-6);
                assertEquals(arr[nx], src2[nx], 1e-6);
                assertEquals(arr[nx * ny], src2[nx * ny], 1e-6);
                assertEquals(arr[nx * ny * nz - 42], src2[nx * ny * nz - 42], 1e-6);
            } catch (Exception e) {
                fail(e.getMessage());
            }
        }
    }
}
