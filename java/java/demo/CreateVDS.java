import org.opengroup.openvds.*;

import java.util.ArrayList;
import java.util.List;

public class CreateVDS {

    public static void main(String[] args) {
        try {
            process(args);
        } catch (Throwable t) {
            System.out.println();
            t.printStackTrace();
        }
    }

    static void process(String[] args) throws Exception {

        int samplesX = 1000;
        int samplesY = 1000;
        int samplesZ = 1000;
        VolumeDataChannelDescriptor.Format format = VolumeDataChannelDescriptor.Format.FORMAT_R32;

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
        axisDescriptors.add(new VolumeDataAxisDescriptor(samplesY, "Crossline", "",1932.f, 2536.f));
        axisDescriptors.add(new VolumeDataAxisDescriptor(samplesZ, "Inline", "", 9985.f,10369.f));

        List<VolumeDataChannelDescriptor> channelDescriptors = new ArrayList<>();

        float rangeMin = -0.1234f;
        float rangeMax = 0.1234f;
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
        VDSFileOpenOptions options = new VDSFileOpenOptions("/tmp/create.vds");
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
                100, // max pages
                VolumeDataAccessManager.AccessMode.Create.getCode()); // access mode

        //ASSERT_TRUE(pageAccessor);

        int chunkCount = (int) pageAccessor.getChunkCount();

        //OpenVDS::VolumeDataChannelDescriptor::Format format = layout->GetChannelFormat(channel);

        for (int i = 0; i < chunkCount; i++) {
            VolumeDataPage page = pageAccessor.createPage(i);
            VolumeDataLayoutDescriptor.BrickSize brickSize1 = pageAccessor.getLayout().getLayoutDescriptor().getBrickSize();

            VolumeIndexer3D outputIndexer = new VolumeIndexer3D(page, 0, 0, DimensionsND.DIMENSIONS_012.ordinal(), layout);
            float valueRangeScale = outputIndexer.getValueRangeMax() - outputIndexer.getValueRangeMin();
            //QuantizingValueConverterWithNoValue<T, float, useNoValue> converter(outputIndexer3D.valueRangeMin, outputIndexer3D.valueRangeMax, valueRangeScale, outputIndexer3D.valueRangeMin, noValue, noValue);

            int[] numSamples = new int[3];
            //OpenVDS::IntVector<3> localOutIndex;

            for (int j = 0; j < 3; j++) {
                numSamples[j] = outputIndexer.getDataBlockNumSamples(j);
            }

            int[] pitch = new int[VolumeDataLayout.Dimensionality_Max];
            //float[] buffer = page.readFloatBuffer(pitch);

            int[] chunkMin = new int[VolumeDataLayout.Dimensionality_Max];
            int[] chunkMax = new int[VolumeDataLayout.Dimensionality_Max];

            pageAccessor.getChunkMinMax(i, chunkMin, chunkMax);
            int chunkSizeI = chunkMax[2] - chunkMin[2];
            int chunkSizeX = chunkMax[1] - chunkMin[1];
            int chunkSizeZ = chunkMax[0] - chunkMin[0];
            int nbElem = chunkSizeI * chunkSizeX * chunkSizeZ;
            float[] buffer = new float[nbElem];

            float[] output = page.readFloatBuffer(pitch);

            //void* buffer = page.getWritableBuffer(pitch);
            //auto output = static_cast < float *>(buffer);

            for (int iDim2 = 0; iDim2 < numSamples[2]; iDim2++)
                for (int iDim1 = 0; iDim1 < numSamples[1]; iDim1++)
                    for (int iDim0 = 0; iDim0 < numSamples[0]; iDim0++) {
                        int[] localOutIndex = new int[]{iDim0, iDim1, iDim2};

                        int[] voxelIndex = outputIndexer.localIndexToVoxelIndex(localOutIndex);

                        int pos[] = new int[]{
                                    voxelIndex[0],
                                    voxelIndex[1],
                                    voxelIndex[2]
                        };

                        float value = pos[0];

                        output[outputIndexer.localIndexToDataIndex(localOutIndex)] = value;
                    }


            page.writeFloatBuffer(buffer);

            //OpenVDS::CalculateNoise3D(buffer, format, &outputIndexer, frequency, 0.021f, 0.f, true, 345);
            page.release();
        }


        pageAccessor.commit();
        pageAccessor.setMaxPages(0);
        accessManager.flushUploadQueue();
        accessManager.destroyVolumeDataPageAccessor(pageAccessor);

        vds.close(); // equivalent to OpenVDS.close(vds); ?
    }

//    static float[] getScaleOffsetForFormat(float min, float max, boolean novalue, VolumeDataChannelDescriptor.Format format) {
//        float res[] = new float[] {1f, 0f};
//        switch (format) {
//            case VolumeDataChannelDescriptor.Format.FORMAT_U8:
//                res[0] = 1.f / (255.f - novalue) * (max - min);
//                res[1] = min;
//                break;
//            case VolumeDataChannelDescriptor.Format.FORMAT_U16:
//                res[0] = 1.f / (65535.f - novalue) * (max - min);
//                res[1] = min;
//                break;
//            case VolumeDataChannelDescriptor.Format.FORMAT_R32:
//            case VolumeDataChannelDescriptor.Format.FORMAT_U32:
//            case VolumeDataChannelDescriptor.Format.FORMAT_R64:
//            case VolumeDataChannelDescriptor.Format.FORMAT_U64:
//            case VolumeDataChannelDescriptor.Format.FORMAT_1BIT:
//            case VolumeDataChannelDescriptor.Format.FORMAT_ANY:
//                res[0] = 1.0f;
//                res[1] = 0.0f;
//        }
//        return res;
//    }

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
