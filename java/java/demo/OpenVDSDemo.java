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

import org.opengroup.openvds.*;

import java.io.FileOutputStream;
import java.nio.FloatBuffer;
import java.util.Arrays;
import static org.opengroup.openvds.VolumeDataFormat.*;

public class OpenVDSDemo {

    public static void main(String[] args) {
        try {
            process(args);
        } catch (Throwable t) {
            System.out.println();
            t.printStackTrace();
        }
    }

    static void process(String[] args) throws Exception {
        String url = null;
        String connection = null;
        for (int i = 1; i < args.length; i+=2)
        {
            if (args[i].equals("--url"))
                url = args[i+1];
            if (args[i].equals("--connection"))
                connection = args[i+1];
        }
        int output_width = 1000;
        int output_height = 1000;
        int axis_position = Integer.MIN_VALUE;

        System.out.println("Library: " + OpenVDS.getOpenVDSName() + " v" + OpenVDS.getOpenVDSVersion() + " rev. " + OpenVDS.getOpenVDSRevision());
        int nXSamples = 64, nYSamples = 64, nZSamples = 64;
        VolumeDataFormat format = Format_U8;

        VDS vds;
        if (url != null && !url.isEmpty())
        {
            System.out.println("Open existing VDS with: " + url);
            vds = OpenVDS.open(url, connection);
        }
        else
        {
            System.out.println("Create MemoryVdsGenerator...");
            vds = new InMemoryVDSGenerator(nXSamples, nYSamples, nZSamples, format).getVDS();
        }

        VolumeDataLayout layout = vds.getLayout();
        printLayout(layout);

        VolumeDataAccessManager accessManager = vds.getAccessManager();

        int[] axis_mapper = {0, 1, 2};
        int[] sampleCount = new int[3];
        sampleCount[0] = layout.getDimensionNumSamples(axis_mapper[0]);
        sampleCount[1] = layout.getDimensionNumSamples(axis_mapper[1]);
        sampleCount[2] = layout.getDimensionNumSamples(axis_mapper[2]);

        System.out.println("\nFound data set with sample count "
                + sampleCount[0] + "x" + sampleCount[1] + "x" + sampleCount[2]);

        System.out.println("\nCreate request for samples...");

        axis_position = sampleCount[0] / 2;
        axis_position = Math.max(0, axis_position);
        axis_position = Math.min(sampleCount[0], axis_position);

        float x_sample_shift = (float) sampleCount[1] / output_width;
        float y_sample_shift = (float) sampleCount[2] / output_height;
        final int elemSize = VolumeDataLayout.Dimensionality_Max;
        final int elemCount = output_width * output_height;
        float[] samples = new float[elemCount * elemSize];

        for (int y = 0; y < output_height; y++) {
            float y_pos = y * y_sample_shift + 0.5f;
            for (int x = 0; x < output_width; x++) {
                float x_pos = x * x_sample_shift + 0.5f;
                int offset = (y * output_width + x) * elemSize;
                samples[offset + axis_mapper[0]] = axis_position + 0.5f;
                samples[offset + axis_mapper[1]] = x_pos;
                samples[offset + axis_mapper[2]] = y_pos;
            }
        }

        System.out.println("Request samples from VolumeDataAccessManager...");
        try (VolumeDataRequest request = accessManager.requestVolumeSamples(DimensionsND.Dimensions_012, 0, 0, samples, InterpolationMethod.Linear)) {
            System.out.println("Wait for request completion...");
            while (!request.waitForCompletion(100)) {
                if (request.isCanceled()) throw new RuntimeException("Cancelled job");

                // Timeout, so let display progress
                final float completionFactor = request.getCompletionFactor();
                System.out.println("Completion : " + completionFactor * 100. + " %");
            }

            System.out.println("Create bitmap " + output_width + "x" + output_height + " from samples...");
            String outFileName = "OpenVdsDemo_Output.bmp";
            FloatBuffer data = request.getBuffer().asFloatBuffer();
            float[] floats = new float[data.capacity()];
            data.get(floats);
            writeBitmap(outFileName, layout, floats, output_width, output_height);
            System.out.println("Picture is written to file: " + outFileName);
        }

        vds.close();
        System.out.println("Finished");
    }

    static void printLayout(VolumeDataLayout layout) {

        System.out.println("\nVolumeDataLayout");
        System.out.println("GetContentsHash = " + layout.getContentsHash());
        System.out.println("getDimensionality = " + layout.getDimensionality());
        System.out.println("getChannelCount = " + layout.getChannelCount());

        printLayoutDescriptor(layout.getLayoutDescriptor());

        int channelIndex = 0;
        String channelName = layout.getChannelName(channelIndex);
        System.out.println("\ngetChannelName(" + channelIndex + ") = " + channelName);
        System.out.println("getChannelUnit(" + channelIndex + ") = " + layout.getChannelUnit(channelIndex));
        System.out.println("isChannelAvailable(" + channelName + ") = " + layout.isChannelAvailable(channelName));
        System.out.println("getChannelIndex(" + channelName + ") = " + layout.getChannelIndex(channelName));
        System.out.println("getChannelFormat(" + channelIndex + ") = " + layout.getChannelFormat(channelIndex));
        System.out.println("getChannelComponents(" + channelIndex + ") = " + layout.getChannelComponents(channelIndex));
        System.out.println("getChannelValueRangeMin(" + channelIndex + ") = " + layout.getChannelValueRangeMin(channelIndex));
        System.out.println("getChannelValueRangeMax(" + channelIndex + ") = " + layout.getChannelValueRangeMax(channelIndex));
        System.out.println("isChannelDiscrete(" + channelIndex + ") = " + layout.isChannelDiscrete(channelIndex));
        System.out.println("isChannelRenderable(" + channelIndex + ") = " + layout.isChannelRenderable(channelIndex));
        System.out.println("isChannelAllowingLossyCompression(" + channelIndex + ") = " + layout.isChannelAllowingLossyCompression(channelIndex));
        System.out.println("isChannelUseZipForLosslessCompression(" + channelIndex + ") = " + layout.isChannelUseZipForLosslessCompression(channelIndex));
        System.out.println("getChannelMapping(" + channelIndex + ") = " + layout.getChannelMapping(channelIndex));
        System.out.println("isChannelUseNoValue(" + channelIndex + ") = " + layout.isChannelUseNoValue(channelIndex));
        System.out.println("getChannelNoValue(" + channelIndex + ") = " + layout.getChannelNoValue(channelIndex));
        System.out.println("getChannelIntegerScale(" + channelIndex + ") = " + layout.getChannelIntegerScale(channelIndex));
        System.out.println("getChannelIntegerOffset(" + channelIndex + ") = " + layout.getChannelIntegerOffset(channelIndex));

        printChannelDescriptor("Channel " + channelIndex, layout.getChannelDescriptor(0));

        int dimensionIndex = 0;
        printAxisDescriptor("Axis " + dimensionIndex, layout.getAxisDescriptor(dimensionIndex));

        System.out.println("\ngetDimensionNumSamples(" + dimensionIndex + ") = " + layout.getDimensionNumSamples(dimensionIndex));
        System.out.println("getDimensionName(" + dimensionIndex + ") = " + layout.getDimensionName(dimensionIndex));
        System.out.println("getDimensionUnit(" + dimensionIndex + ") = " + layout.getDimensionUnit(dimensionIndex));
        System.out.println("getDimensionMin(" + dimensionIndex + ") = " + layout.getDimensionMin(dimensionIndex));
        System.out.println("getDimensionMax(" + dimensionIndex + ") = " + layout.getDimensionMax(dimensionIndex));

        printMetaData(layout);
    }

    static void printMetaData(VolumeDataLayout layout) {
        final MetadataKey[] metadataKeys = layout.getMetadataKeys();
        Arrays.stream(metadataKeys)
                .forEach(k -> {
                    System.out.print("Category : " + k.getCategory() + " Name : " + k.getName());
                    switch (k.getType()) {
                        case Int:
                            if (layout.isMetadataIntAvailable(k.getCategory(), k.getName()))
                                System.out.print(" Value : " + layout.getMetadataInt(k.getCategory(), k.getName()));
                            else
                                System.out.print(" no values");
                            break;
                        case IntVector2:
                            if (layout.isMetadataIntVector2Available(k.getCategory(), k.getName()))
                                System.out.print(" value : " + layout.getMetadataIntVector2(k.getCategory(), k.getName()));
                            else
                                System.out.print(" no values");
                            break;
                        case IntVector3:
                            if (layout.isMetadataIntVector3Available(k.getCategory(), k.getName()))
                                System.out.print(" value : " + layout.getMetadataIntVector3(k.getCategory(), k.getName()));
                            else
                                System.out.print(" no values");
                            break;
                        case IntVector4:
                            if (layout.isMetadataIntVector4Available(k.getCategory(), k.getName()))
                                System.out.print(" value : " + layout.getMetadataIntVector4(k.getCategory(), k.getName()));
                            else
                                System.out.print(" no values");
                            break;
                        case Float:
                            if (layout.isMetadataFloatAvailable(k.getCategory(), k.getName()))
                                System.out.print(" Value : " + layout.getMetadataFloat(k.getCategory(), k.getName()));
                            else
                                System.out.print(" no values");
                            break;
                        case FloatVector2:
                            if (layout.isMetadataFloatVector2Available(k.getCategory(), k.getName()))
                                System.out.print(" value : " + layout.getMetadataFloatVector2(k.getCategory(), k.getName()));
                            else
                                System.out.print(" no values");
                            break;
                        case FloatVector3:
                            if (layout.isMetadataFloatVector3Available(k.getCategory(), k.getName()))
                                System.out.print(" value : " + layout.getMetadataFloatVector3(k.getCategory(), k.getName()));
                            else
                                System.out.print(" no values");
                            break;
                        case FloatVector4:
                            if (layout.isMetadataFloatVector4Available(k.getCategory(), k.getName()))
                                System.out.print(" value : " + layout.getMetadataFloatVector4(k.getCategory(), k.getName()));
                            else
                                System.out.print(" no values");
                            break;
                        case Double:
                            if (layout.isMetadataDoubleAvailable(k.getCategory(), k.getName()))
                                System.out.print(" Value : " + layout.getMetadataDouble(k.getCategory(), k.getName()));
                            else
                                System.out.print(" no values");
                            break;
                        case DoubleVector2:
                            if (layout.isMetadataDoubleVector2Available(k.getCategory(), k.getName()))
                                System.out.print(" value : " + layout.getMetadataDoubleVector2(k.getCategory(), k.getName()));
                            else
                                System.out.print(" no values");
                            break;
                        case DoubleVector3:
                            if (layout.isMetadataDoubleVector3Available(k.getCategory(), k.getName()))
                                System.out.print(" value : " + layout.getMetadataDoubleVector3(k.getCategory(), k.getName()));
                            else
                                System.out.print(" no values");
                            break;
                        case DoubleVector4:
                            if (layout.isMetadataDoubleVector4Available(k.getCategory(), k.getName()))
                                System.out.print(" value : " + layout.getMetadataDoubleVector4(k.getCategory(), k.getName()));
                            else
                                System.out.print(" no values");
                            break;
                        case String:
                            if (layout.isMetadataStringAvailable(k.getCategory(), k.getName()))
                                System.out.print(" value : " + layout.getMetadataString(k.getCategory(), k.getName()));
                            else
                                System.out.print(" no values");
                        default:
                            System.out.println("Unmanaged key type");
                    }
                    System.out.println();
                });
    }

    static void printLayoutDescriptor(VolumeDataLayoutDescriptor descr) {
        System.out.println("\nVolumeDataLayoutDescriptor");
        System.out.println("IsValid = " + descr.isValid());
        System.out.println("GetBrickSize = " + descr.getBrickSize());
        System.out.println("GetNegativeMargin = " + descr.getNegativeMargin());
        System.out.println("GetPositiveMargin = " + descr.getPositiveMargin());
        System.out.println("GetBrickSizeMultiplier2D = " + descr.getBrickSizeMultiplier2D());
        System.out.println("GetLODLevels = " + descr.getLODLevels());
        System.out.println("IsCreate2DLODs = " + descr.isCreate2DLODs());
        System.out.println("IsForceFullResolutionDimension = " + descr.isForceFullResolutionDimension());
        System.out.println("GetFullResolutionDimension = " + descr.getFullResolutionDimension());
    }

    static void printChannelDescriptor(String title, VolumeDataChannelDescriptor descr) {
        System.out.println("\nVolumeDataChannelDescriptor " + title);
        System.out.println("m_format = " + descr.getFormat());
        System.out.println("m_components = " + descr.getComponents());
        System.out.println("m_name = " + descr.getName());
        System.out.println("m_unit = " + descr.getUnit());
        System.out.println("m_valueRangeMin = " + descr.getValueRangeMin());
        System.out.println("m_valueRangeMax = " + descr.getValueRangeMax());
        System.out.println("m_mapping = " + descr.getMapping());
        System.out.println("m_mappedValueCount = " + descr.getMappedValueCount());
        System.out.println("m_isDiscrete = " + descr.isDiscrete());
        System.out.println("m_isRenderable = " + descr.isRenderable());
        System.out.println("m_isAllowLossyCompression = " + descr.isAllowLossyCompression());
        System.out.println("m_isUseZipForLosslessCompression = " + descr.isUseZipForLosslessCompression());
        System.out.println("m_useNoValue = " + descr.isUseNoValue());
        System.out.println("m_noValue = " + descr.getNoValue());
        System.out.println("m_integerScale = " + descr.getIntegerScale());
        System.out.println("m_integerOffset = " + descr.getIntegerOffset());
    }

    static void printAxisDescriptor(String title, VolumeDataAxisDescriptor descr) {
        System.out.println("\nVolumeDataAxisDescriptor " + title);
        System.out.println("m_numSamples = " + descr.getNumSamples());
        System.out.println("m_name = " + descr.getName());
        System.out.println("m_unit = " + descr.getUnit());
        System.out.println("m_coordinateMin = " + descr.getCoordinateMin());
        System.out.println("m_coordinateMax = " + descr.getCoordinateMax());
    }

    static class FloatToByteConverter {
        float m_ReciprocalScale;
        float m_ValueRangeMin;

        static int QuantizeValueWithReciprocalScale(float value, float offset, float reciprocalScale, int buckets)
        {
            float  bucket = (value - offset) * reciprocalScale;
            return (bucket <= 0)           ? 0 :
                    (bucket >= buckets - 1) ? (buckets - 1) :
                            (int)(bucket + 0.5f);
        }

        public FloatToByteConverter(float minValue, float maxValue, float intScale, float intOffset) {
            m_ValueRangeMin = minValue;
            m_ReciprocalScale = 255.f / (maxValue - minValue);
        }

        byte convertValue(float value ) {
            return (byte)QuantizeValueWithReciprocalScale(value, m_ValueRangeMin, m_ReciprocalScale, 256);
        }
    }

    static void writeBitmap(String fileName, VolumeDataLayout layout, float[] data, int output_width, int output_height) throws Exception {

        float minValue = layout.getChannelValueRangeMin(0);
        float maxValue = layout.getChannelValueRangeMax(0);
        float intScale = layout.getChannelIntegerScale(0);
        float intOffset = layout.getChannelIntegerOffset(0);

        FloatToByteConverter converter = new FloatToByteConverter(minValue, maxValue, intScale, intOffset);
        final int colorComponentsCount = 3;
        int dataSize = output_width * output_height * colorComponentsCount;
        byte[] fileData = new byte[dataSize];

        for (int y = 0; y < output_height; y++) {
            for (int x = 0; x < output_width; x++) {
                int inOffset = y * output_width + x;
                byte value = converter.convertValue(data[inOffset]);
                int outOffset = inOffset * colorComponentsCount;
                fileData[outOffset] = value;
                fileData[outOffset + 1] = value;
                fileData[outOffset + 2] = value;
            }
        }

        long filesize = 54 + dataSize;
        byte[] bmpinfoheader = new byte[40];
        bmpinfoheader[0] = 40;
        bmpinfoheader[12] = 1;
        bmpinfoheader[14] = 24;
        byte[] bmppad = {0, 0, 0};

        byte[] bmpfileheader = {'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0};
        bmpfileheader[2] = (byte) (filesize);
        bmpfileheader[3] = (byte) (filesize >> 8);
        bmpfileheader[4] = (byte) (filesize >> 16);
        bmpfileheader[5] = (byte) (filesize >> 24);

        bmpinfoheader[4] = (byte) (output_width);
        bmpinfoheader[5] = (byte) (output_width >> 8);
        bmpinfoheader[6] = (byte) (output_width >> 16);
        bmpinfoheader[7] = (byte) (output_width >> 24);
        bmpinfoheader[8] = (byte) (output_height);
        bmpinfoheader[9] = (byte) (output_height >> 8);
        bmpinfoheader[10] = (byte) (output_height >> 16);
        bmpinfoheader[11] = (byte) (output_height >> 24);

        FileOutputStream file = new FileOutputStream(fileName);

        file.write(bmpfileheader);
        file.write(bmpinfoheader);

        int scanLineSize = output_width * colorComponentsCount;

        for (int i = 0; i < output_height; i++) {
            // In bmp file, lines go from bottom to top
            int offset = (output_height - i - 1) * scanLineSize;
            file.write(fileData, offset, scanLineSize);
            file.write(bmppad, 0, (-scanLineSize) & 3);
        }
        file.close();
    }
}
