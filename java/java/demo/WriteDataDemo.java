import org.opengroup.openvds.*;
import org.testng.annotations.Test;

import java.io.IOException;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.EnumSet;

import static org.opengroup.openvds.DimensionsND.Dimensions_01;
import static org.opengroup.openvds.DimensionsND.Dimensions_012;
import static org.opengroup.openvds.VolumeDataComponents.Components_1;
import static org.opengroup.openvds.VolumeDataLayoutDescriptor.BrickSize.BrickSize_1024;
import static org.opengroup.openvds.VolumeDataLayoutDescriptor.BrickSize.BrickSize_128;
import static org.opengroup.openvds.VolumeDataLayoutDescriptor.LODLevels.LODLevels_None;
import static org.testng.Assert.assertEquals;
import static org.testng.Assert.fail;
import static org.opengroup.openvds.VolumeDataFormat.*;

public class WriteDataDemo {
    public static void main(String[] args) throws IOException {
        roundtrip_2d();
        roundtrip_3d();
    }

    public static class FloatScaleAndOffset {
        public float scale = 1.0f;
        public float offset = 0.0f;

        public FloatScaleAndOffset() {
        }

        public FloatScaleAndOffset(float scale, float offset) {
            this.scale = scale;
            this.offset = offset;
        }
    }

    public static FloatScaleAndOffset getScaleOffsetForFormat(float min, float max, boolean usingNovalue, VolumeDataFormat format)
    {
        float novalue = usingNovalue ? 1.0f : 0.0f;
        float scale = 1.0f;
        float offset = 0.0f;
        switch (format) {
            case Format_U8:
                scale = 1.f / (255.f - novalue) * (max - min);
                offset = min;
                break;
            case Format_U16:
                scale = 1.f/(65535.f - novalue) * (max - min);
                offset = min;
                break;
            case Format_R32:
            case Format_U32:
            case Format_R64:
            case Format_U64:
            case Format_1Bit:
            case Format_Any:
                scale = 1.0f;
                offset = 0.0f;
        }
        return new FloatScaleAndOffset(scale, offset);
    }

    public static VolumeDataChannelDescriptor[] createDefaultChannelDescriptors(String[] channelNames, VolumeDataFormat format) {
        ArrayList<VolumeDataChannelDescriptor> channelDescriptors = new ArrayList<>();
        for (String channel: channelNames) {
            float rangeMin = -0.1234f;
            float rangeMax = 0.1234f;
            EnumSet<VolumeDataChannelDescriptor.Flags> channelFlags = EnumSet.noneOf(VolumeDataChannelDescriptor.Flags.class);
            FloatScaleAndOffset scaleAndOffset = getScaleOffsetForFormat(rangeMin, rangeMax, true, format);
            channelDescriptors.add(new VolumeDataChannelDescriptor(format, Components_1,  channel, "", rangeMin, rangeMax, VolumeDataMapping.Direct, 1, channelFlags, 0.f, scaleAndOffset.scale, scaleAndOffset.offset));
        }
        return channelDescriptors.toArray(new VolumeDataChannelDescriptor[channelDescriptors.size()]);
    }

    public static VolumeDataChannelDescriptor[] createDefaultChannelDescriptors(String channelName, VolumeDataFormat format) {
        return createDefaultChannelDescriptors(new String[] { channelName }, format);
    }

    public static VDS createVDS(int samplesX, int samplesY, int samplesZ, VolumeDataFormat format, OpenOptions options, VolumeDataChannelDescriptor[] channelDescriptors,  VDSError error) {
        if (options == null) {
            options = new InMemoryOpenOptions();
        }
        if (error == null) {
            error = new VDSError();
        }
        if (channelDescriptors == null) {
            channelDescriptors = createDefaultChannelDescriptors("Amplitude", Format_R32);
        }
        VolumeDataLayoutDescriptor.LODLevels lodLevels = LODLevels_None;
        VolumeDataLayoutDescriptor.BrickSize brickSize = (samplesZ == 0) ? BrickSize_1024 : BrickSize_128;
        EnumSet<VolumeDataLayoutDescriptor.Options> layoutOptions = EnumSet.noneOf(VolumeDataLayoutDescriptor.Options.class);
        int negativeMargin = 4;
        int positiveMargin = 4;
        int brickSize2DMultiplier = 4;

        VolumeDataLayoutDescriptor layoutDescriptor = new VolumeDataLayoutDescriptor(brickSize, negativeMargin, positiveMargin, brickSize2DMultiplier, lodLevels, layoutOptions, 0);

        ArrayList<VolumeDataAxisDescriptor> axisDescriptors = new ArrayList<>();
        axisDescriptors.add(new VolumeDataAxisDescriptor(samplesX, KnownAxisNames.sample(), "ms", 0.0f, 4.f));
        axisDescriptors.add(new VolumeDataAxisDescriptor(samplesY, KnownAxisNames.crossline(), "", 1932.f, 2536.f));
        if (samplesZ != 0) {
            axisDescriptors.add(new VolumeDataAxisDescriptor(samplesZ, KnownAxisNames.inline(), "", 9985.f, 10369.f));
        }

        MetadataContainer metadataContainer = new MetadataContainer();
        metadataContainer.setMetadataInt( "categoryInt", "Int", 123 );
        metadataContainer.setMetadataIntVector2( "categoryInt", "IntVector2", new IntVector2( 45, 78 ) );
        metadataContainer.setMetadataIntVector3( "categoryInt", "IntVector3", new IntVector3( 45, 78 , 72) );
        metadataContainer.setMetadataIntVector4( "categoryInt", "IntVector4", new IntVector4( 45, 78 , 72,84 ));
        metadataContainer.setMetadataFloat( "categoryFloat", "Float", 123.f );
        metadataContainer.setMetadataFloatVector2( "categoryFloat", "FloatVector2", new FloatVector2( 45.5f, 78.75f ) );
        metadataContainer.setMetadataFloatVector3( "categoryFloat", "FloatVector3", new FloatVector3( 45.5f, 78.75f , 72.75f) );
        metadataContainer.setMetadataFloatVector4( "categoryFloat", "FloatVector4", new FloatVector4( 45.5f, 78.75f , 72.75f,84.1f) );
        metadataContainer.setMetadataDouble( "categoryDouble", "Double", 123.);
        metadataContainer.setMetadataDoubleVector2( "categoryDouble", "DoubleVector2", new DoubleVector2( 45.5, 78.75 ) );
        metadataContainer.setMetadataDoubleVector3( "categoryDouble", "DoubleVector3", new DoubleVector3( 45.5, 78.75 , 72.75) );
        metadataContainer.setMetadataDoubleVector4( "categoryDouble", "DoubleVector4", new DoubleVector4( 45.5, 78.75 , 72.75,84.1) );
        metadataContainer.setMetadataString( "categoryString", "String", "Test string" );
        byte[] data = new byte[] { 1,2,3,4 };
        metadataContainer.setMetadataBLOB("categoryBLOB", "BLOB", data);

        return OpenVDS.create(options, layoutDescriptor, axisDescriptors.toArray(new VolumeDataAxisDescriptor[axisDescriptors.size()]), channelDescriptors, metadataContainer, error);
    }

    public static VDS createVDS(int samplesX, int samplesY, VolumeDataFormat format, OpenOptions options, VolumeDataChannelDescriptor[] channelDescriptors, VDSError error) {
        return createVDS(samplesX, samplesY, 0, format, options, channelDescriptors, error);
    }

    public static VDS createVDS(int samplesX, int samplesY, VolumeDataFormat format, OpenOptions options, VDSError error) {
        return createVDS(samplesX, samplesY, 0, format, options, createDefaultChannelDescriptors("Amplitude", format), error);
    }

    public static void roundtrip_2d() {
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
            vds = createVDS(nz, nx, f, options, createDefaultChannelDescriptors(names, f), vdsError);
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
            DimensionsND dims = Dimensions_01;
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

    public static void roundtrip_3d() {
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
            vds = createVDS(nz, ny, nx, f, options, createDefaultChannelDescriptors(names, f), vdsError);
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
            DimensionsND dims = Dimensions_012;
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
