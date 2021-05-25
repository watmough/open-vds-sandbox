import org.opengroup.openvds.*;

import java.io.IOException;

import static org.testng.Assert.assertEquals;
import static org.testng.Assert.fail;

public class WriteDataDemo {
    public static void main(String[] args) throws IOException {
        roundtrip_2d();
        roundtrip_3d();
    }

    private static void roundtrip_2d() throws IOException {
        final int nz = 113;
        final int nx = 128;

        AzureOpenOptions options = new AzureOpenOptions();
        options.connectionString = System.getenv("CONNECTION_STRING");

        if (options.connectionString == null) {
            return;
        }

        options.container = "test";
        options.blob = "rtrip_2d";

        String[] names = new String[]{"chan1", "chan2"};
        VolumeDataChannelDescriptor.Format f = VolumeDataChannelDescriptor.Format.FORMAT_R32;
        VdsHandle vds = new org.opengroup.openvds.AzureVdsGenerator(options, nz, nx, f, names);

        java.util.Random r = new java.util.Random();

        double[] src1 = r.doubles(nx * nz).toArray();
        double[] src2 = r.doubles(nx * nz).toArray();

        OpenVDS.writeArray(vds, src1, "chan1");
        OpenVDS.writeArray(vds, src2, "chan2");

        vds.getAccessManager().flushUploadQueue();
        VdsHandle vds2;

        try {
            vds2 = OpenVDS.open(options);
        } catch (java.io.IOException e) {
            fail();
            return;
        }

        VolumeDataAccessManager access = vds2.getAccessManager();
        VolumeDataLayout layout = vds2.getLayout();
        NDBox box = new NDBox(0, 0, 0, 0, 0, 0, nz, nx, 0, 0, 0, 0);

        int channel = layout.getChannelIndex("chan1");
        long size = access.getVolumeSubsetBufferSize(box, f, 0, channel);

        java.nio.FloatBuffer outbuf = BufferUtils.createFloatBuffer((int) size / 4);
        DimensionsND dims = DimensionsND.DIMENSIONS_01;

        long t = access.requestVolumeSubset(outbuf, layout, dims, 0, channel, box);
        access.waitForCompletion(t);
        float[] arr = new float[outbuf.remaining()];
        outbuf.get(arr);
        assertEquals(arr[0], src1[0], 1e-6);
        assertEquals(arr[1], src1[1], 1e-6);
        assertEquals(arr[nx], src1[nx], 1e-6);
    }


    private static void roundtrip_3d() throws IOException {
        final int nz = 113;
        final int ny = 107;
        final int nx = 108;

//        AzureOpenOptions options = new AzureOpenOptions();
//        options.connectionString = System.getenv("CONNECTION_STRING");
//
//        if (options.connectionString == null) {
//            return;
//        }
//
//        options.container = "test";
//        options.blob = "rtrip_3d";
//
//        String[] names = new String[]{"chan1", "chan2"};
//        VolumeDataChannelDescriptor.Format f = VolumeDataChannelDescriptor.Format.FORMAT_R32;
//        VdsHandle vds = new org.opengroup.openvds.AzureVdsGenerator(options, nz, ny, nx, f, names);

        VDSFileOpenOptions options = new VDSFileOpenOptions();
        options.filePath = "/mnt/dataDD/tmp/test.vds";

        if (options.filePath == null) {
            return;
        }

        String[] names = new String[]{"chan1", "chan2"};
        VolumeDataChannelDescriptor.Format f = VolumeDataChannelDescriptor.Format.FORMAT_R32;

        VolumeDataLayoutDescriptor ld = new VolumeDataLayoutDescriptor(
                VolumeDataLayoutDescriptor.BrickSize.BRICK_SIZE_64,
                0,
                0,
                4,
                VolumeDataLayoutDescriptor.LODLevels.LOD_LEVELS_NONE,
                false,
                false,
                -1);

        VolumeDataAxisDescriptor[] vda = new VolumeDataAxisDescriptor[]{
                new VolumeDataAxisDescriptor(nz, "Sample", "ms", 0, nz * 4),
                new VolumeDataAxisDescriptor(ny, "Xline", null, 0, ny),
                new VolumeDataAxisDescriptor(nx, "Inline", null, 0, nx)
        };

        /** True data range */
        float valueRangeMax = Float.MAX_VALUE;
        float valueRangeMin = -Float.MAX_VALUE;

        VolumeDataChannelDescriptor[] vdc = new VolumeDataChannelDescriptor[]{
                new VolumeDataChannelDescriptor(
                        VolumeDataChannelDescriptor.Format.FORMAT_R32,
                        VolumeDataChannelDescriptor.Components.COMPONENTS_1,
                        "Amplitude", "",
                        valueRangeMin, valueRangeMax, VolumeDataMapping.DIRECT,
                        1, false, true, true, false, false, 0f, 1f, 0f),
                new VolumeDataChannelDescriptor(
                        VolumeDataChannelDescriptor.Format.FORMAT_U8,
                        VolumeDataChannelDescriptor.Components.COMPONENTS_1,
                        "Trace", "",
                        0, 1, VolumeDataMapping.PER_TRACE,
                        1, true, true, false, false, false, 0, 1f, 0f),
                new VolumeDataChannelDescriptor(
                        VolumeDataChannelDescriptor.Format.FORMAT_U8,
                        VolumeDataChannelDescriptor.Components.COMPONENTS_1,
                        "SEGYTraceHeader", "",
                        0, 255, VolumeDataMapping.PER_TRACE,
                        240, true, true, false, false, false, 0, 1f, 0f)
        };

        MetadataReadAccess md = new MetadataReadAccess(0);
        VdsHandle vds = OpenVDS.create(options, ld, vda, vdc, md);

        java.util.Random r = new java.util.Random();

        double[] src1 = r.doubles(nx * ny * nz).toArray();

        float[] src2 = new float[src1.length];
        for (int i = 0; i < src2.length; i++)
            src2[i] = i;

        OpenVDS.writeArray(vds, src1, "chan1");
        OpenVDS.writeArray(vds, src2, "chan2");

        vds.getAccessManager().flushUploadQueue();
        VdsHandle vds2;

        vds2 = OpenVDS.open(options);

        VolumeDataAccessManager access = vds2.getAccessManager();
        VolumeDataLayout layout = vds2.getLayout();

        NDBox box = new NDBox(0, 0, 0, 0, 0, 0, nz, ny, nx, 0, 0, 0);

        int channel = layout.getChannelIndex("chan2");
        long size = access.getVolumeSubsetBufferSize(box, f, 0, channel);
        java.nio.FloatBuffer outbuf = BufferUtils.createFloatBuffer((int) size / 4);
        DimensionsND dims = DimensionsND.DIMENSIONS_012;

        long t = access.requestVolumeSubset(outbuf, layout, dims, 0, channel, box);
        access.waitForCompletion(t);
        float[] arr = new float[outbuf.remaining()];
        outbuf.get(arr);
//        assertEquals(arr[0], src2[0], 1e-6);
//        assertEquals(arr[nx], src2[nx], 1e-6);
//        assertEquals(arr[nx * ny], src2[nx * ny], 1e-6);
//        assertEquals(arr[nx * ny * nz - 42], src2[nx * ny * nz - 42], 1e-6);
    }
}
