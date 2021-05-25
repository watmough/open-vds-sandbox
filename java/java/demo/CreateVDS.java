import org.opengroup.openvds.*;

import java.io.File;
import java.io.IOException;
import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.List;

public class CreateVDS {

    public static void main(String[] args) {
        try {
            if (!checkParams(args)) {
                System.out.println("Bad params, usage  : <CreateVDS> /path/to/file");
                System.out.println("path must be a valid non existing file path");
            }
            else {
                Instant start = Instant.now();
                process(args[0]);
                Instant end = Instant.now();

                Duration elapsed = Duration.between(start, end);
                long hrs = elapsed.toHours();
                long min = elapsed.toMinutes() - (hrs * 60);
                long s = (elapsed.toMillis() - (elapsed.toMinutes() * 60 * 1000)) / 1000;
                System.out.println("Write VDS TIME : " + hrs + " hrs " + min + " min " + s + "s (" + elapsed.toMillis() + " ms)");
            }
        }
        catch (Throwable t) {
            System.out.println();
            t.printStackTrace();
        }
    }

    private static boolean checkParams(String[] args) {
        if (args == null || args.length != 1) {
            return false;
        }
        String path = args[0];
        File file = new File(path);
        try {
            file.getCanonicalPath();
        }
        catch (IOException e) {
            System.err.println("VDS File path is invalid !");
            return false;
        }
        if (!path.endsWith(".vds")) {
            System.err.println("VDS File path must have vds extension !");
            return false;
        }
        if (file.exists()) {
            System.err.println("VDS File already exists !");
            return false;
        }
        return true;
    }

    static void process(String vdsPath) throws Exception {

        int samplesX = 500; // time
        int samplesY = 800; // XL
        int samplesZ = 800; // IL
        VolumeDataChannelDescriptor.Format format = VolumeDataChannelDescriptor.Format.FORMAT_R32;

        double sizeX = samplesX;
        double sizeY = samplesX;
        double sizeZ = samplesX;

        double distMax = distance3D(0, 0, 0, sizeX, sizeY, sizeZ);
        double cycles = Math.PI * 2 * 6;
        double midX = samplesX / 2f;
        double midY = samplesY / 2f;
        double midZ = samplesY / 2f;

        VolumeDataLayoutDescriptor.BrickSize brickSize = VolumeDataLayoutDescriptor.BrickSize.BRICK_SIZE_64;
        int negativeMargin = 4;
        int positiveMargin = 4;
        int brickSize2DMultiplier = 4;
        VolumeDataLayoutDescriptor.LODLevels lodLevels = VolumeDataLayoutDescriptor.LODLevels.LOD_LEVELS_NONE;
        //VolumeDataLayoutDescriptor.Options layoutOptions = VolumeDataLayoutDescriptor.Options.NONE;
        VolumeDataLayoutDescriptor layoutDescriptor = new VolumeDataLayoutDescriptor(brickSize, negativeMargin, positiveMargin,
                brickSize2DMultiplier, lodLevels, false,false,0);

        List<VolumeDataAxisDescriptor> axisDescriptors = new ArrayList<>();
        axisDescriptors.add(new VolumeDataAxisDescriptor(samplesX, "Sample", "ms", 0.0f,4.f));
        axisDescriptors.add(new VolumeDataAxisDescriptor(samplesY, "Crossline", "",1000f, 1000f + samplesY - 1f));
        axisDescriptors.add(new VolumeDataAxisDescriptor(samplesZ, "Inline", "", 1500f, 1500f + samplesZ - 1f));

        List<VolumeDataChannelDescriptor> channelDescriptors = new ArrayList<>();

        float rangeMin = -1f;
        float rangeMax = 1f;
        float[] scaleOffset = getScaleOffsetForFormat(rangeMin, rangeMax, true, format);

        VolumeDataChannelDescriptor channelDescriptor = new VolumeDataChannelDescriptor(format, VolumeDataChannelDescriptor.Components.COMPONENTS_1,
                "Amplitude",
                "",
                rangeMin, // range min
                rangeMax, // range max
                VolumeDataMapping.DIRECT, // mapping
                1, // mapped value count
                false, // is discrete
                true, // is renderable
                false, // allow lossy compression
                false, // use zip for lossless compresion
                true,  // use no value
                -999.25f, // no value
                scaleOffset[0], // integer scale
                scaleOffset[1]); // integer offset
        channelDescriptors.add(channelDescriptor);


        //OpenVDS::InMemoryOpenOptions options;
        VDSFileOpenOptions options = new VDSFileOpenOptions(vdsPath);
        VdsError error;

        MetadataContainer metadataContainer = new MetadataContainer();
        metadataContainer.setMetadataInt("categoryInt", "Int", 123);
        metadataContainer.setMetadataIntVector2("categoryInt", "IntVector2", new int[] {45, 78});
        metadataContainer.setMetadataIntVector3("categoryInt", "IntVector3", new int[] {45, 78, 72});
        metadataContainer.setMetadataIntVector4("categoryInt", "IntVector4", new int[] {45, 78, 72, 84});
        metadataContainer.setMetadataFloat("categoryFloat", "Float", 123.f);
        metadataContainer.setMetadataFloatVector2("categoryFloat", "FloatVector2", new float[] {45.5f, 78.75f});
        metadataContainer.setMetadataFloatVector3("categoryFloat", "FloatVector3", new float[] {45.5f, 78.75f, 72.75f});
        metadataContainer.setMetadataFloatVector4("categoryFloat", "FloatVector4", new float[] {45.5f, 78.75f, 72.75f, 84.1f});
        metadataContainer.setMetadataDouble("categoryDouble", "Double", 123.);
        metadataContainer.setMetadataDoubleVector2("categoryDouble", "DoubleVector2", new double[] {45.5, 78.75});
        metadataContainer.setMetadataDoubleVector3("categoryDouble", "DoubleVector3", new double[] {45.5, 78.75, 72.75});
        metadataContainer.setMetadataDoubleVector4("categoryDouble", "DoubleVector4", new double[] {45.5, 78.75, 72.75, 84.1});
        metadataContainer.setMetadataString("categoryString", "String", "Test string");
        //metadataContainer.SetMetadataBLOB("categoryBLOB", "BLOB", data, 4 );

        VdsHandle vds = OpenVDS.create(options, layoutDescriptor,
                axisDescriptors.toArray(new VolumeDataAxisDescriptor[0]),
                channelDescriptors.toArray(new VolumeDataChannelDescriptor[0]), metadataContainer);


        VolumeDataLayout layout = vds.getLayout();
        //ASSERT_TRUE(layout);
        VolumeDataAccessManager accessManager = vds.getAccessManager();
        //ASSERT_TRUE(accessManager);

        int channel = 0;

        VolumeDataPageAccessor pageAccessor = accessManager.createVolumeDataPageAccessor(
                layout, // layout
                DimensionsND.DIMENSIONS_012.ordinal(), // dimension ND
                0, // lod
                channel, // channel
                1000, // max pages
                VolumeDataPageAccessor.AccessMode.Create.getCode()); // access mode

        //ASSERT_TRUE(pageAccessor);

        int chunkCount = (int) pageAccessor.getChunkCount();
        System.out.println("Chunk count :  " + chunkCount);

        long timeRead = 0L;
        long timeWrite = 0L;
        long timeVolIndex = 0L;

        int[] pitch = new int[VolumeDataLayout.Dimensionality_Max];
        int[] numSamplesChunk = new int[3];
        int[] localChunkIndex = new int[3];
        int[] voxelPos = new int[3];

        for (int i = 0; i < chunkCount; i++) {

            long time_1 = System.currentTimeMillis();

            VolumeDataPage page = pageAccessor.createPage(i);
            System.out.println("Page " + (i+1) + " / " + chunkCount);

            VolumeIndexer3D outputIndexer = new VolumeIndexer3D(page, 0, 0, DimensionsND.DIMENSIONS_012.ordinal(), layout);

            for (int j = 0; j < 3; j++) {
                numSamplesChunk[j] = outputIndexer.getLocalChunkNumSamples(j);
            }

            page.getPitch(pitch);
            float[] output = new float[page.getAllocatedBufferSize()];

            long time_2 = System.currentTimeMillis();

            int[] numSamples = numSamplesChunk;
            for (int iDim2 = 0; iDim2 < numSamples[2]; iDim2++)
                for (int iDim1 = 0; iDim1 < numSamples[1]; iDim1++)
                    for (int iDim0 = 0; iDim0 < numSamples[0]; iDim0++) {
                        localChunkIndex[0] = iDim0;
                        localChunkIndex[1] = iDim1;
                        localChunkIndex[2] = iDim2;
                        int[] voxelIndex = outputIndexer.localIndexToVoxelIndex(localChunkIndex);

                        for (int vp = 0 ; vp < 3 ; ++vp) {
                            voxelPos[vp] = voxelIndex[vp];
                        }

                        float value = 0f;
                        double dist = 0;
                        if (voxelPos[0] >= midX) {
                            dist = distance2D(midY, midZ, voxelPos[1], voxelPos[2]);
                        }
                        else {
                            dist = distance3D(midX, midY, midZ, voxelPos[0], voxelPos[1], voxelPos[2]);
                        }
                        value = (float)Math.sin((dist * cycles) / distMax);

                        int iPos = outputIndexer.localIndexToDataIndex(localChunkIndex);
                        output[iPos] = value;
                    }

            // write buffer then release page
            long time_3 = System.currentTimeMillis();
            page.writeFloatBuffer(output, pitch);
            page.pageRelease();
            long time_4 = System.currentTimeMillis();

            timeRead += (time_2 - time_1);
            timeVolIndex += (time_3 - time_2);
            timeWrite += (time_4 - time_3);
        }

        displayStatistics(timeRead, timeVolIndex, timeWrite);

        pageAccessor.commit();
        pageAccessor.setMaxPages(0);
        accessManager.flushUploadQueue();
        accessManager.destroyVolumeDataPageAccessor(pageAccessor);

        vds.close(); // equivalent to OpenVDS.close(vds); ?
        System.out.println("VDS closed");
    }

    private static void displayStatistics(long timeRead, long timeVolIndex, long timeWrite) {
        long totalTime = timeRead + timeVolIndex + timeWrite;
        float pctRead = (timeRead * 100f) /  (float)totalTime;
        float pctVolIndex = (timeVolIndex * 100f) /  (float)totalTime;
        float pctWrite = (timeWrite * 100f) /  (float)totalTime;

        System.out.println("Read took " + timeRead + "ms : " + pctRead + "%");
        System.out.println("Write took " + timeWrite + "ms : " + pctWrite + "%");
        System.out.println("Index compute took " + timeVolIndex + "ms : " + pctVolIndex + "%");
    }

    private static double distance2D(double x1, double y1, double x2, double y2) {
        double diffX = x2 - x1;
        double diffY = y2 - y1;
        return Math.sqrt((diffX * diffX) + (diffY * diffY));
    }

    private static double distance3D(double x1, double y1, double z1, double x2, double y2, double z2) {
        double diffX = x2 - x1;
        double diffY = y2 - y1;
        double diffZ = z2 - z1;
        return Math.sqrt((diffX * diffX) + (diffY * diffY) + (diffZ * diffZ));
    }

    static float[] getScaleOffsetForFormat(float min, float max, boolean useNoValue, VolumeDataChannelDescriptor.Format format) {
        float res[] = new float[] {1f, 0f};
        float noValueCmp = useNoValue ? 1f : 0f;
        switch (format) {
            case FORMAT_U8:
                res[0] = 1.f / (255.f - noValueCmp) * (max - min);
                res[1] = min;
                break;
            case FORMAT_U16:
                res[0] = 1.f / (65535.f - noValueCmp) * (max - min);
                res[1] = min;
                break;
            case FORMAT_R32:
            case FORMAT_U32:
            case FORMAT_R64:
            case FORMAT_U64:
            case FORMAT_1BIT:
            case FORMAT_ANY:
                res[0] = 1.0f;
                res[1] = 0.0f;
        }
        return res;
    }

}
