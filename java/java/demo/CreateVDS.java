/*
 * Copyright 2021 The Open Group
 * Copyright 2021 INT, Inc.
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

import java.io.File;
import java.io.IOException;
import java.nio.FloatBuffer;
import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.EnumSet;
import java.util.List;

import static org.opengroup.openvds.DimensionsND.*;
import static org.opengroup.openvds.VolumeDataPageAccessor.AccessMode.*;
import static org.opengroup.openvds.VolumeDataLayout.Dimensionality_Max;

public class CreateVDS {

    // usage : CreateVDSWithLOD n /path/to/file
    // where n is the LOD level wanted
    public static void main(String[] args) {
        try {
            if (!checkParams(args)) {
                System.out.println("Bad params, usage  : <CreateVDS> n /path/to/file dimX dimY dimZ");
                System.out.println("n is the LOD level wanted (0 <= lod <= 12), path must be a valid non existing file path");
                System.out.println("dims are cube dimension (must be > 0)");
            }
            else {
                Instant start = Instant.now();
                process(Integer.parseInt(args[0]), args[1], Integer.parseInt(args[2]), Integer.parseInt(args[3]), Integer.parseInt(args[4]));
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
        if (args == null || args.length != 5) {
            return false;
        }
        try {
            Integer lod = Integer.parseInt(args[0]);
            if (lod < 0 || lod > 12) {
                System.err.println("Invalid LOD value (must be between 0 and 12) !");
                return false;
            }
        }
        catch (NumberFormatException nfe) {
            return false;
        }
        String path = args[1];
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
            file.delete();
//            System.err.println("VDS File already exists !"); return false;
        }
        try {
            Integer dimX = Integer.parseInt(args[2]);
            if (dimX < 0) {
                System.err.println("Invalid x dimension !");
                return false;
            }
            Integer dimY = Integer.parseInt(args[3]);
            if (dimX < 0) {
                System.err.println("Invalid y dimension !");
                return false;
            }
            Integer dimZ = Integer.parseInt(args[4]);
            if (dimX < 0) {
                System.err.println("Invalid Z dimension !");
                return false;
            }
        }
        catch (NumberFormatException nfe) {
            return false;
        }
        return true;
    }

    static int localIndexToDataIndex(int[] localIndex, int[] pitch) {
        assert(localIndex.length <= pitch.length);
        int dataIndex = 0;
        for (int i = 0; i < localIndex.length; i++) {
            dataIndex += localIndex[i] * pitch[i];
        }
        return dataIndex;
    }

    static void localIndexToGlobalIndex(int[] globalIndex, int[] localIndex, int[] chunkMin, int lod) {
        assert(globalIndex.length == localIndex.length);
        assert(localIndex.length <= chunkMin.length);
        for (int i = 0; i < localIndex.length; i++) {
            globalIndex[i] = chunkMin[i] + (localIndex[i] << lod);
        }
    }

    static void process(int lodParam, String vdsFilePath, int dimX, int dimY, int dimZ) throws Exception {
        int samplesX = dimZ; // time
        int samplesY = dimY; // XL
        int samplesZ = dimX; // IL
        VolumeDataFormat format = VolumeDataFormat.Format_R32;

        double sizeX = samplesX;
        double sizeY = samplesY;
        double sizeZ = samplesZ;

        double distMax = distance3D(0, 0, 0, sizeX, sizeY, sizeZ);
        double cycles = Math.PI * 2 * 20;
        double midX = samplesX / 2f;
        double midY = samplesY / 2f;
        double midZ = samplesZ / 2f;

        VolumeDataLayoutDescriptor.BrickSize brickSize = VolumeDataLayoutDescriptor.BrickSize.BrickSize_64;
        int negativeMargin = 4;
        int positiveMargin = 4;
        int brickSize2DMultiplier = 4;

        VolumeDataLayoutDescriptor.LODLevels lodLevels = VolumeDataLayoutDescriptor.LODLevels.values()[lodParam];

        boolean create2DLODs = false; // 2D LODs are in general only useful for 2D VDSs (e.g. horizons)
        EnumSet<VolumeDataLayoutDescriptor.Options> layoutOptions = create2DLODs ? EnumSet.of(VolumeDataLayoutDescriptor.Options.Options_Create2DLODs) : EnumSet.of(VolumeDataLayoutDescriptor.Options.Options_None);

        VolumeDataLayoutDescriptor layoutDescriptor = new VolumeDataLayoutDescriptor( brickSize, negativeMargin, positiveMargin,
              brickSize2DMultiplier, lodLevels, layoutOptions,0);

        List<VolumeDataAxisDescriptor> axisDescriptors = new ArrayList<>();
        axisDescriptors.add(new VolumeDataAxisDescriptor(samplesX, "Sample", "s", 0.0f,6.0f));
        axisDescriptors.add(new VolumeDataAxisDescriptor(samplesY, "Crossline", "",0f, 0f + samplesY - 1f));
        axisDescriptors.add(new VolumeDataAxisDescriptor(samplesZ, "Inline", "", 0f, 0f + samplesZ - 1f));

        List<VolumeDataChannelDescriptor> channelDescriptors = new ArrayList<>();

        float rangeMin = -1f;
        float rangeMax = 1f;
        float[] scaleOffset = getScaleOffsetForFormat(rangeMin, rangeMax, true, format);

        VolumeDataChannelDescriptor channelDescriptor = new VolumeDataChannelDescriptor(format, VolumeDataComponents.Components_1,
              "Amplitude",
              "",
              rangeMin,
              rangeMax,
              VolumeDataMapping.Direct,
              1,
              EnumSet.of(VolumeDataChannelDescriptor.Flags.Default),
              scaleOffset[0], // integer scale
              scaleOffset[1]); // integer offset
        channelDescriptors.add(channelDescriptor);

        // file options
        VDSFileOpenOptions options = new VDSFileOpenOptions(vdsFilePath);

        MetadataContainer metadataContainer = new MetadataContainer();
        StringBuilder builder = new StringBuilder();
        builder.append("CreateVDS Demo ");
        builder.append("dimX: ");
        builder.append(dimX);
        builder.append(" dimY: ");
        builder.append(dimY);
        builder.append(" dimZ: ");
        builder.append(dimZ);

        metadataContainer.setMetadataString("General Info", "Cube layout", builder.toString());
        metadataContainer.setMetadataInt("General Info", "LOD", lodLevels.ordinal());
        metadataContainer.setMetadataString("General Info", "Brick Size", "BRICK_SIZE_64");

        float compressionTolerance = 0.01f;
        CompressionMethod compressionMethod = OpenVDS.isCompressionMethodSupported(CompressionMethod.Wavelet) ? CompressionMethod.Wavelet : CompressionMethod.None;
        VDS vds = OpenVDS.create(options, layoutDescriptor, axisDescriptors.toArray(new VolumeDataAxisDescriptor[0]), channelDescriptors.toArray(new VolumeDataChannelDescriptor[0]), metadataContainer, compressionMethod, compressionTolerance);

        VolumeDataLayout layout = vds.getLayout();
        VolumeDataAccessManager accessManager = vds.getAccessManager();

        int channel = 0;

        int lodCount = lodLevels.ordinal();

        System.out.println("\nCreate 3D dimension group (012)");
        createDimensionGroup(distMax, cycles, midX, midY, midZ, layout, accessManager, channel, lodCount, Dimensions_012);

        // Adds slice data : additional layers
        System.out.println("\nCreate 2D dimension group (01)");
        createDimensionGroup(distMax, cycles, midX, midY, midZ, layout, accessManager, channel, create2DLODs ? lodCount : 0, Dimensions_01);

        System.out.println("\nCreate 2D dimension group (02)");
        createDimensionGroup(distMax, cycles, midX, midY, midZ, layout, accessManager, channel, create2DLODs ? lodCount : 0, Dimensions_02);

        System.out.println("\nCreate 2D dimension group (12)");
        createDimensionGroup(distMax, cycles, midX, midY, midZ, layout, accessManager, channel, create2DLODs ? lodCount : 0, Dimensions_12);

        vds.close();
        System.out.println("VDS closed");
    }

    private static void createDimensionGroup(double distMax, double cycles, double midX, double midY, double midZ, VolumeDataLayout layout, VolumeDataAccessManager accessManager, int channel, int lodCount, DimensionsND dimensionGroup) {
        int[] chunkMin = new int[Dimensionality_Max];
        int[] chunkMax = new int[Dimensionality_Max];
        int[] chunkSize = new int[Dimensionality_Max];
        int[] chunkPitch = new int[Dimensionality_Max];
        String header = dimensionGroup.toString();
        for (int lod = 0 ; lod <= lodCount ; ++lod) {
            VolumeDataPageAccessor pageAccessor = accessManager.createVolumeDataPageAccessor(dimensionGroup, lod, channel, 200, AccessMode_Create);
            int chunkCount = (int) pageAccessor.getChunkCount();
            System.out.println(header + " LOD : " + lod + " Chunk count :  " + chunkCount);
            for (int i = 0; i < chunkCount; i++) {
                try (VolumeDataPage page = pageAccessor.createPage(i)) {
                    FloatBuffer buffer = page.getWritableBuffer(chunkSize, chunkPitch).asFloatBuffer();
                    System.out.print(header + " LOD : " + lod + " / Page " + (i + 1) + " / " + chunkCount);
                    float valueRangeScale = pageAccessor.getLayout().getChannelValueRangeMax(channel) - pageAccessor.getLayout().getChannelValueRangeMin(channel);
                    page.getMinMax(chunkMin, chunkMax);
                    System.out.print("\t" + header + " Chunk : [" + chunkSize[0] + ", " + chunkSize[1] + ", " + chunkSize[2] + "]");

                    System.out.print("\tCoords page : " + chunkMin[0] + "/" + chunkMax[0] + " " + chunkMin[1] + "/" + chunkMax[1] + "/" + chunkMin[2] + "/" + chunkMax[2]);

                    float[] output = new float[buffer.capacity()];

                    System.out.print(" Page Buffer Size : " + output.length);// + " / [" + stats.getMin() + "," + stats.getMax() + "]");

                    // Buffer dimensions
                    System.out.println(" Size : [" + chunkSize[0] + ", " + chunkSize[1] + ", " + chunkSize[2] + "]");
                    System.out.println(" Pitch : [" + chunkPitch[0] + ", " + chunkPitch[1] + "," + chunkPitch[2] + "]");

                    long time_2 = System.currentTimeMillis();

                    fillChunk(distMax, cycles, midX, midY, midZ, chunkMin, chunkSize, chunkPitch, lod, output);

                    // write buffer then release page
                    buffer.put(output);
                }
            }
            pageAccessor.commit();
            accessManager.flushUploadQueue();
            accessManager.destroyVolumeDataPageAccessor(pageAccessor);
        }
    }

    private static void fillChunk(double distMax, double cycles, double midX, double midY, double midZ,  int[] chunkMin, int[] chunkSize, int[] chunkPitch, int lod, float[] output) {
        int[] voxelPos = new int[3];
        int[] localIndex = new int[3];
        for (int iDim2 = 0; iDim2 < chunkSize[2]; iDim2++) {
            localIndex[2] = iDim2;
            for (int iDim1 = 0; iDim1 < chunkSize[1]; iDim1++) {
                localIndex[1] = iDim1;
                for (int iDim0 = 0; iDim0 < chunkSize[0]; iDim0++) {
                    localIndex[0] = iDim0;
                    localIndexToGlobalIndex(voxelPos, localIndex, chunkMin, lod);
                    float value = 0f;
                    double dist = 0;
                    if (voxelPos[0] >= midX) {
                        dist = distance2D(midY, midZ, voxelPos[1], voxelPos[2]);
                    } else {
                        dist = distance3D(midX, midY, midZ, voxelPos[0], voxelPos[1], voxelPos[2]);
                    }
                    value = (float) Math.sin((dist * cycles) / distMax);
                    int iPos = localIndexToDataIndex(localIndex, chunkPitch);
                    output[iPos] = value;
                }
            }
        }
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

    static float[] getScaleOffsetForFormat(float min, float max, boolean useNoValue, VolumeDataFormat format) {
        float res[] = new float[] {1f, 0f};
        float noValueCmp = useNoValue ? 1f : 0f;
        switch (format) {
            case Format_U8:
                res[0] = 1.f / (255.f - noValueCmp) * (max - min);
                res[1] = min;
                break;
            case Format_U16:
                res[0] = 1.f / (65535.f - noValueCmp) * (max - min);
                res[1] = min;
                break;
            case Format_R32:
            case Format_U32:
            case Format_R64:
            case Format_U64:
            case Format_1Bit:
            case Format_Any:
                res[0] = 1.0f;
                res[1] = 0.0f;
        }
        return res;
    }

}
