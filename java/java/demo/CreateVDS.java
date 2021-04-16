import org.opengroup.openvds.*;

import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.List;

public class CreateVDS {

    public static void main(String[] args) {
        try {
            Instant start = Instant.now();
            process(args);
            Instant end = Instant.now();

            Duration elapsed = Duration.between(start, end);
            long hrs = elapsed.toHours();
            long min = elapsed.toMinutes() - (hrs * 60);
            long s = (elapsed.toMillis() - (elapsed.toMinutes() * 1000)) / 1000;
            System.out.println("Write VDS TIME : " + hrs + " hrs " + min + " min " + s + "s (" + elapsed.toMillis() + " ms)");
        }
        catch (Throwable t) {
            System.out.println();
            t.printStackTrace();
        }
    }

    static void process(String[] args) throws Exception {

        int samplesX = 500; // time
        int samplesY = 1000; // XL
        int samplesZ = 1000; // IL
        VolumeDataChannelDescriptor.Format format = VolumeDataChannelDescriptor.Format.FORMAT_R32;

        VolumeDataLayoutDescriptor.BrickSize brickSize = VolumeDataLayoutDescriptor.BrickSize.BRICK_SIZE_128;
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
        VDSFileOpenOptions options = new VDSFileOpenOptions("/tmp/createJava_1000_1000_500_B128.vds");
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
                VolumeDataAccessManager.AccessMode.Create.getCode()); // access mode

        //ASSERT_TRUE(pageAccessor);
        int dimensionality = layout.getDimensionality();

        int chunkCount = (int) pageAccessor.getChunkCount();
        System.out.println("Chunk count :  " + chunkCount);

        for (int i = 0; i < chunkCount; i++) {
            VolumeDataPage page = pageAccessor.createPage(i);
            System.out.print("Page " + (i+1) + " / " + chunkCount);

            VolumeIndexer3D outputIndexer = new VolumeIndexer3D(page, 0, 0, DimensionsND.DIMENSIONS_012.ordinal(), layout);
            float valueRangeScale = outputIndexer.getValueRangeMax() - outputIndexer.getValueRangeMin();

            int[] numSamplesChunk = new int[3];
            int[] numSamplesDB = new int[3];

            for (int j = 0; j < 3; j++) {
                numSamplesChunk[j] = outputIndexer.getLocalChunkNumSamples(j);
                numSamplesDB[j] = outputIndexer.getDataBlockNumSamples(j);
            }
            System.out.print(" Dimensions :  [" + numSamplesChunk[0] + ", " + numSamplesChunk[1] + ", " + numSamplesChunk[2] + "]");

            int[] pitch = new int[VolumeDataLayout.Dimensionality_Max];
            int[] chunkMin = new int[VolumeDataLayout.Dimensionality_Max];
            int[] chunkMax = new int[VolumeDataLayout.Dimensionality_Max];

            page.getMinMax(chunkMin, chunkMax);

            System.out.print(" Coords page : " + chunkMin[0] + "/" + chunkMax[0] + " " + chunkMin[1] + "/" + chunkMax[1] + " " + chunkMin[2] + "/" + chunkMax[2]);
            int chunkSizeY = chunkMax[2] - chunkMin[2];
            int chunkSizeX = chunkMax[1] - chunkMin[1];
            int chunkSizeZ = chunkMax[0] - chunkMin[0];

            float[] output = page.readFloatBuffer(pitch);
            // dimension of buffer
            System.out.println(" Pitch : [" + pitch[0] + ", " + pitch[1] + ", " + pitch[2] + "]");

            int dimPitch = pitch[dimensionality - 1] * (chunkMax[dimensionality - 1] - chunkMin[dimensionality - 1]);

            int[] numSamples = numSamplesChunk;
            for (int iDim2 = 0; iDim2 < numSamples[2]; iDim2++)
                for (int iDim1 = 0; iDim1 < numSamples[1]; iDim1++)
                    for (int iDim0 = 0; iDim0 < numSamples[0]; iDim0++) {
                        int[] localChunkIndex = new int[]{iDim0, iDim1, iDim2};
                        int[] localIndex = outputIndexer.localChunkIndexToLocalIndex(localChunkIndex);
                        int[] voxelIndex = outputIndexer.localIndexToVoxelIndex(localIndex);

                        int pos[] = new int[] {
                                    voxelIndex[0],
                                    voxelIndex[1],
                                    voxelIndex[2]
                        };

                        float value = (float)((iDim0 + iDim1 + iDim2)%numSamples[0]) / numSamples[0];
                        value = rangeMin + 2 * value;

                        int iPos = outputIndexer.localIndexToDataIndex(localChunkIndex);
                        output[iPos] = value;
                    }

            // write buffer then release page
            page.writeFloatBuffer(output, pitch, dimensionality);
            page.pageRelease();
        }

        pageAccessor.commit();
        pageAccessor.setMaxPages(0);
        accessManager.flushUploadQueue();
        accessManager.destroyVolumeDataPageAccessor(pageAccessor);

        vds.close(); // equivalent to OpenVDS.close(vds); ?
        System.out.println("VDS closed");
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
