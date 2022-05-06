import org.opengroup.openvds.*;

import javax.imageio.ImageIO;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.IOException;
import java.nio.FloatBuffer;
import java.time.Duration;
import java.time.Instant;
import java.util.Arrays;
import java.util.DoubleSummaryStatistics;
import java.util.Locale;
import java.util.stream.IntStream;

public class SliceDump {

    public static void main(String[] args) {
        try {
            process(args);
        } catch (Throwable t) {
            System.out.println();
            t.printStackTrace();
        }
    }

    public static SliceDumpParameters parseParameters(String[] args) throws IllegalArgumentException {
        SliceDumpParameters params = new SliceDumpParameters();
        for (int idx = 0 ; idx < args.length ; ++idx) {
            if (args[idx].startsWith("--url")) {
                checkParameter(args[idx]);
                String[] split = args[idx].split("=");
                params.url = split[1];
            }
            else if (args[idx].startsWith("--connection-string")) {
                checkParameter(args[idx]);
                String[] split = args[idx].split("=");
                params.connectionString = split[1];
            }
            else if (args[idx].startsWith("--axis")) {
                checkParameterAxis(args[idx]);
                String[] split = args[idx].split("=");
                params.axis = split[1];
            }
            else if (args[idx].startsWith("--position")) {
                checkParameterInt(args[idx]);
                String[] split = args[idx].split("=");
                params.position = Integer.parseInt(split[1]);
            }
            else if (args[idx].startsWith("--o_width")) {
                checkParameterInt(args[idx]);
                String[] split = args[idx].split("=");
                params.outputWidth = Integer.parseInt(split[1]);
            }
            else if (args[idx].startsWith("--o_height")) {
                checkParameterInt(args[idx]);
                String[] split = args[idx].split("=");
                params.outputHeight = Integer.parseInt(split[1]);
            }
            else if (args[idx].startsWith("--output")) {
                checkParameterOutput(args[idx]);
                String[] split = args[idx].split("=");
                params.outputFilePath = split[1];
            }
            else if (args[idx].startsWith("--adaptive-tolerance")) {
                checkParameterFloat(args[idx]);
                String[] split = args[idx].split("=");
                params.adaptiveTolerance = Float.parseFloat(split[1]);
                params.adaptiveMode = WaveletAdaptiveMode.Tolerance;
            }
            else if (args[idx].startsWith("--adaptive-ratio")) {
                checkParameterFloat(args[idx]);
                String[] split = args[idx].split("=");
                params.adaptiveRatio = Float.parseFloat(split[1]);
                params.adaptiveMode = WaveletAdaptiveMode.Ratio;
            }
            else {
                throw new IllegalArgumentException("Unknown parameter : " + args[idx]);
            }
        }
        if (params.url == null) {
            throw new IllegalArgumentException("URL must be defined.");
        }
        if (params.outputFilePath == null) {
            throw new IllegalArgumentException("Output file path must be defined.");
        }
        return params;
    }

    private static void checkParameter(String param) throws IllegalArgumentException {
        if (!param.contains("=") || param.split("=").length != 2) {
            throw new IllegalArgumentException("Bad parameter definition " + param + " : expected --param=value");
        }
    }

    private static void checkParameterOutput(String param) throws IllegalArgumentException {
        if (!param.contains("=") || param.split("=").length != 2) {
            throw new IllegalArgumentException("Bad parameter definition " + param + " : expected --param=value");
        }
        else {
            String filePath = param.split("=")[1];
            int posDot = filePath.lastIndexOf(".");
            String suffix =  filePath.substring(posDot + 1);
            String[] knownSuffixes = ImageIO.getReaderFileSuffixes();
            boolean found = Arrays.stream(knownSuffixes).filter(s -> s.toLowerCase(Locale.ROOT).equals(suffix)).findAny().isPresent();
            if (!found) {
                throw new IllegalArgumentException("Bad parameter definition : unknown image suffix (" + suffix + ")");
            }
        }
    }

    private static void checkParameterInt(String param) throws IllegalArgumentException {
        if (!param.contains("=") || param.split("=").length != 2) {
            throw new IllegalArgumentException("Bad parameter definition " + param + " : expected --param=value");
        }
        String[] split = param.split("=");
        try {
            Integer.parseInt(split[1]);
        }
        catch (NumberFormatException nfe) {
            throw new IllegalArgumentException("Bad parameter value for " + param + " Integer value expected.");
        }
    }

    private static void checkParameterFloat(String param) throws IllegalArgumentException {
        if (!param.contains("=") || param.split("=").length != 2) {
            throw new IllegalArgumentException("Bad parameter definition " + param + " : expected --param=value");
        }
        String[] split = param.split("=");
        try {
            Float.parseFloat(split[1]);
        }
        catch (NumberFormatException nfe) {
            throw new IllegalArgumentException("Bad parameter value for " + param + " Float value expected.");
        }
    }

    private static void checkParameterAxis(String param) throws IllegalArgumentException {
        if (!param.contains("=") || param.split("=").length != 2) {
            throw new IllegalArgumentException("Bad parameter definition " + param + " : expected --param=value");
        }
        String[] split = param.split("=");
        try {
            String[] axisStr = split[1].split(",");
            if (axisStr == null || axisStr.length != 3) {
                throw new IllegalArgumentException("Axis must be 3 values comma separated. ex : 0, 1, 2. Got " + param);
            }
            for (String valStr : axisStr) {
                int val = Integer.parseInt(valStr);
                if (val < 0 || val > 2) {
                    throw new IllegalArgumentException("Axis values must be either 0, 1 or 2. Got " + param);
                }
            }
        }
        catch (NumberFormatException nfe) {
            throw new IllegalArgumentException("Bad parameter value for axis " + param + " integer value expected.");
        }
    }

    public static VDS open(SliceDumpParameters parameters) throws IOException {
        if (parameters.connectionString == null) {
            // file
            VDSFileOpenOptions fileOpenOptions = new VDSFileOpenOptions(parameters.url);
            fileOpenOptions.setWaveletAdaptiveMode(parameters.adaptiveMode);
            fileOpenOptions.setWaveletAdaptiveTolerance(parameters.adaptiveTolerance);
            fileOpenOptions.setWaveletAdaptiveRatio(parameters.adaptiveRatio);
            return OpenVDS.open(fileOpenOptions);
        }
        else {
            // connection string, check which adaptive mode must be used
            if (WaveletAdaptiveMode.BestQuality.equals(parameters.adaptiveMode)) {
                return OpenVDS.open(parameters.url, parameters.connectionString);
            }
            else if (WaveletAdaptiveMode.Tolerance.equals(parameters.adaptiveMode)) {
                return OpenVDS.openWithAdaptiveCompressionTolerance(parameters.url, parameters.connectionString, parameters.adaptiveTolerance);
            }
            else /* ratio */{
                return OpenVDS.openWithAdaptiveCompressionRatio(parameters.url, parameters.connectionString, parameters.adaptiveRatio);
            }
        }
    }

    static void process(String[] args) throws Exception {
        SliceDumpParameters parameters = parseParameters(args);

        System.out.println("Library: " + OpenVDS.getOpenVDSName() + " v" + OpenVDS.getOpenVDSVersion() + " rev. " + OpenVDS.getOpenVDSRevision());

        try (VDS generator = open(parameters)) {
            VolumeDataLayout layout = generator.getLayout();
            printLayout(layout);
            try (VolumeDataAccessManager accessManager = generator.getAccessManager()) {
                int[] axis_mapper = getAxisMapping(parameters.axis);
                System.out.println("\nUsing axis mapping " + axis_mapper[0] + "," + axis_mapper[1] + "," + axis_mapper[2]);
                int[] sampleCount = new int[3];
                sampleCount[0] = layout.getDimensionNumSamples(axis_mapper[0]);
                sampleCount[1] = layout.getDimensionNumSamples(axis_mapper[1]);
                sampleCount[2] = layout.getDimensionNumSamples(axis_mapper[2]);

                int axis_position = Math.max(0, parameters.position);
                axis_position = Math.min(sampleCount[0], axis_position);

                System.out.println("\nFound data set with sample count " + sampleCount[0] + "x" + sampleCount[1] + "x" + sampleCount[2]);
                System.out.println("\nCreate request for samples...");

                float x_sample_shift = (float) sampleCount[1] / parameters.outputWidth;
                float y_sample_shift = (float) sampleCount[2] / parameters.outputHeight;
                final int elemSize = VolumeDataLayout.Dimensionality_Max;
                final int elemCount = parameters.outputWidth * parameters.outputHeight;
                float[] samples = new float[elemCount * elemSize];
                for (int y = 0; y < parameters.outputHeight; y++) {
                    float y_pos = y * y_sample_shift + 0.5f;
                    for (int x = 0; x < parameters.outputWidth; x++) {
                        float x_pos = x * x_sample_shift + 0.5f;
                        int offset = (y * parameters.outputWidth + x) * elemSize;
                        samples[offset + axis_mapper[0]] = axis_position + 0.5f;
                        samples[offset + axis_mapper[1]] = x_pos;
                        samples[offset + axis_mapper[2]] = y_pos;
                    }
                }

                System.out.println("Request samples from VolumeDataAccessManager...");

                Instant start = Instant.now();
                try (VolumeDataRequestFloat request = accessManager.requestVolumeSamples(DimensionsND.Dimensions_012, 0, 0, samples, InterpolationMethod.Linear)) {
                    System.out.println("Wait for request completion...");
                    float previousProgress = -1;
                    while (!request.waitForCompletion(1000)) {
                        if (request.isCanceled()) throw new RuntimeException("Cancelled job");

                        // Timeout, so let display progress
                        final float completionFactor = request.getCompletionFactor();
                        if (completionFactor > previousProgress) {
                            previousProgress = completionFactor;
                            System.out.println("Completion : " + completionFactor * 100. + " %");
                        }
                    }
                    Instant finish = Instant.now();
                    long millis = Duration.between(start, finish).toMillis();
                    long seconds = (millis / 1000);
                    System.out.println("Slice query done in " + seconds + "s" + (millis - (seconds * 1000)) + "ms...");

                    System.out.println("Create bitmap " + parameters.outputWidth + "x" + parameters.outputHeight + " from samples...");
                    String outFileName = parameters.outputFilePath;
                    float[] data = request.toArray();
                    writeBitmap(outFileName, layout, data, parameters.outputWidth, parameters.outputHeight);
                    System.out.println("Picture is written to file: " + outFileName);
                }
                System.out.println("Finished");
            }
        }
    }

    private static int[] getAxisMapping(String axisString) throws IllegalArgumentException {
        int[] mapping = Arrays.stream(axisString.split(",")).mapToInt(Integer::parseInt).toArray();
        return mapping;
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
                            break;
                        default:
                            System.out.print("Unhandled key type");
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

    private static double interpolate(double a, double b, double ratio) {
        return b * (ratio) + a * (1. - ratio);
    }

    static void writeBitmap(String fileName, VolumeDataLayout layout,
                            float[] data, int output_width, int output_height) throws Exception {

        DoubleSummaryStatistics dStats = IntStream.range(0, data.length).mapToDouble(i -> data[i]).summaryStatistics();
        float min = (float)dStats.getMin();
        float max = (float)dStats.getMax();

        final int colorComponentsCount = 3;
        int dataSize = output_width * output_height * colorComponentsCount;
        byte[] fileData = new byte[dataSize];

        final BufferedImage image = new BufferedImage(output_width, output_height, BufferedImage.TYPE_INT_RGB);
        for (int y = 0; y < output_height; y++) {
            for (int x = 0; x < output_width; x++) {
                int inOffset = y * output_width + x;
                final float ratio = (data[inOffset] - min) / (max - min);
                byte value = (byte) interpolate(Byte.MIN_VALUE, Byte.MAX_VALUE, ratio);
                int outOffset = inOffset * colorComponentsCount;
                fileData[outOffset] = value;
                fileData[outOffset + 1] = value;
                fileData[outOffset + 2] = value;

                image.setRGB(x, y, new Color(ratio, ratio, ratio).getRGB());
            }
        }

        // get suffix for format and write image
        int posDot = fileName.lastIndexOf(".");
        String suffix =  fileName.substring(posDot + 1);
        final File imageFile = new File(fileName);
        System.out.println("Writing image : " + imageFile.getPath());
        ImageIO.write(image, suffix, imageFile);
    }

    private static class SliceDumpParameters {
        private String connectionString;
        private String url;
        private String axis = "0,1,2";
        private int position;
        private int outputWidth = 500;
        private int outputHeight = 500;
        private String outputFilePath;
        private float adaptiveTolerance = 0.01f;
        private float adaptiveRatio = 1.0f;
        private WaveletAdaptiveMode adaptiveMode = WaveletAdaptiveMode.BestQuality;
    }
}
