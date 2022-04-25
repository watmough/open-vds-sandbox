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
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.EnumSet;
import java.util.List;
import java.util.Objects;

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
            System.err.println("VDS File already exists !");
            return false;
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

    static void getLocalChunkNumSamples(int[] numSamples, int[] chunkMin, int[] chunkMax, int lod) {
        assert(numSamples != null);
        assert(chunkMin != null);
        assert(chunkMax != null);
        assert(chunkMin.length == chunkMax.length);
        assert(numSamples.length <= chunkMin.length);
// From "VolumeDataIndexer.h" :
// static_cast<VolumeIndexerData &>(newIndexer).localChunkSamples[iDimension] = (anVoxelMax[iDimension] - anVoxelMin[iDimension] + (1 << iLOD) - 1) >> iLOD;
        for(int i = 0; i < numSamples.length; i++) {
            numSamples[i] = (chunkMax[i] - chunkMin[i] + (1 << lod) - 1) >> lod;
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

      EnumSet<VolumeDataLayoutDescriptor.Options> layoutOptions = EnumSet.of(VolumeDataLayoutDescriptor.Options.Options_None);
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

      VDS vds = OpenVDS.create(options, layoutDescriptor,
              axisDescriptors.toArray(new VolumeDataAxisDescriptor[0]),
              channelDescriptors.toArray(new VolumeDataChannelDescriptor[0]), metadataContainer);

//      // for compression use
//        float compressionTolerance = 0f; //
//        VDS vds = OpenVDS.create(options, layoutDescriptor,
//                axisDescriptors.toArray(new VolumeDataAxisDescriptor[0]),
//                channelDescriptors.toArray(new VolumeDataChannelDescriptor[0]), metadataContainer, CompressionMethod.Zip, compressionTolerance);

      VolumeDataLayout layout = vds.getLayout();
      VolumeDataAccessManager accessManager = vds.getAccessManager();

      int channel = 0;

      int lodCount = lodLevels.ordinal();

      int[] localIndex = new int[3];
      int[] voxelPos = new int[3];

      int[] pitch = new int[Dimensionality_Max];
      int[] chunkMin = new int[Dimensionality_Max];
      int[] chunkMax = new int[Dimensionality_Max];

      for (int lod = 0 ; lod <= lodCount ; ++lod) {
          VolumeDataPageAccessor pageAccessor = accessManager.createVolumeDataPageAccessor(
                  Dimensions_012, // dimension ND
                  lod, // lod
                  channel, // channel
                  100, // max pages
                  AccessMode_Create); // access mode

          //ASSERT_TRUE(pageAccessor);
          int paLOD = pageAccessor.getLOD();
          int paCI = pageAccessor.getChannelIndex();
//          int[] paNumSamples = pageAccessor.getNumSamples(); // SteinFIXME!!! Fix overload to return int[]
          int[] paNumSamples = new int[Dimensionality_Max];
          pageAccessor.getNumSamples(paNumSamples);

          int chunkCount = (int) pageAccessor.getChunkCount();
          System.out.println("LOD : " + lod + " Chunk count :  " + chunkCount);

          for (int i = 0; i < chunkCount; i++) {

              long time_1 = System.currentTimeMillis();

              VolumeDataPage page = pageAccessor.createPage(i);
              System.out.print("LOD : " + lod + " / Page " + (i + 1) + " / " + chunkCount);

//              VolumeIndexer3D outputIndexer = new VolumeIndexer3D(page, 0, lod, Dimensions_012, layout);

              int[] numSamplesChunk = new int[3];
//              int[] numSamplesDB = new int[3];

              page.getMinMax(chunkMin, chunkMax);
              getLocalChunkNumSamples(numSamplesChunk, chunkMin, chunkMax, lod);

              System.out.print("\tDimensions Chunk : [" + numSamplesChunk[0] + ", " + numSamplesChunk[1] + ", " + numSamplesChunk[2] + "]");
//              System.out.print("\tDimensions DataBlock : [" + numSamplesDB[0] + ", " + numSamplesDB[1] + ", " + numSamplesDB[2] + "]");


              System.out.print("\tCoords page : " + chunkMin[0] + "/" + chunkMax[0] + " " + chunkMin[1] + "/" + chunkMax[1] + " " + chunkMin[2] + "/" + chunkMax[2]);
              int chunkSizeY = chunkMax[2] - chunkMin[2];
              int chunkSizeX = chunkMax[1] - chunkMin[1];
              int chunkSizeZ = chunkMax[0] - chunkMin[0];

              FloatBuffer buffer = page.getWritableBuffer(pitch).asFloatBuffer();
              float[] output = new float[buffer.capacity()];

              System.out.print( " Page Buffer Size : " + buffer.capacity() + " ");/// [" + stats.getMin() + "," + stats.getMax() + "]");
              // dimension of buffer
              System.out.println(" Pitch : [" + pitch[0] + ", " + pitch[1] + ", " + pitch[2] + "]");

              int[] numSamples = numSamplesChunk;

              for (int iDim2 = 0; iDim2 < numSamples[2]; iDim2++) {
                  localIndex[2] = iDim2;
                  for (int iDim1 = 0; iDim1 < numSamples[1]; iDim1++) {
                      localIndex[1] = iDim1;
                      for (int iDim0 = 0; iDim0 < numSamples[0]; iDim0++) {
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

                          int iPos = localIndexToDataIndex(localIndex, pitch);
//                          buffer.put(iPos, value);
                          output[iPos] = value;
                      }
                  }
              }
              buffer.put(output);
              page.close();
          }

          pageAccessor.commit();
          pageAccessor.setMaxPages(0);
          accessManager.flushUploadQueue();
          accessManager.destroyVolumeDataPageAccessor(pageAccessor);
      }

        // Adds slice data : additional layouts
//        System.out.println("\nCreate 01 Layout");
//        create2DLayout(distMax, cycles, midX, midY, midZ, layout, accessManager, channel, lodCount, DimensionsND.DIMENSIONS_01);
//
//        System.out.println("\nCreate 02 Layout");
//        create2DLayout(distMax, cycles, midX, midY, midZ, layout, accessManager, channel, lodCount, DimensionsND.DIMENSIONS_02);
//
//        System.out.println("\nCreate 12 Layout");
//        create2DLayout(distMax, cycles, midX, midY, midZ, layout, accessManager, channel, lodCount, DimensionsND.DIMENSIONS_12);


        vds.close();
        System.out.println("VDS closed");
    }


//    private static void create2DLayout(double distMax, double cycles, double midX, double midY, double midZ, VolumeDataLayout layout, VolumeDataAccessManager accessManager,
//                                       int channel, int lodCount, DimensionsND dimLYT) {
//
//        int[] localIndex = new int[3];
//
//        int[] pitch = new int[Dimensionality_Max];
//        int[] chunkMin = new int[Dimensionality_Max];
//        int[] chunkMax = new int[Dimensionality_Max];
//        for (int lod = 0 ; lod <= lodCount ; ++lod) {
//            VolumeDataPageAccessor pageAccessor = accessManager.createVolumeDataPageAccessor(
//                    dimLYT, // dimension ND
//                    lod, // lod
//                    channel, // channel
//                    200, // max pages
//                    AccessMode_Create); // access mode
//
//            int chunkCount = (int) pageAccessor.getChunkCount();
//            System.out.println("2D Layout LOD : " + lod + " Chunk count :  " + chunkCount);
//
//            for (int i = 0; i < chunkCount; i++) {
//                VolumeDataPage page = pageAccessor.createPage(i);
//                System.out.print("2D Layout LOD : " + lod + " / Page " + (i + 1) + " / " + chunkCount);
//                float valueRangeScale = pageAccessor.getLayout().getChannelValueRangeMax(channel) - pageAccessor.getLayout().getChannelValueRangeMin(channel);
//
//                int[] numSamplesChunk = new int[3];
////                int[] numSamplesDB = new int[3];
//
//                page.getMinMax(chunkMin, chunkMax);
//
//                getLocalChunkNumSamples(numSamplesChunk, chunkMin, chunkMax, lod);
//                System.out.print("\t2D Layout Dimensions Chunk : [" + numSamplesChunk[0] + ", " + numSamplesChunk[1] + ", " + numSamplesChunk[2] + "]");
////                System.out.print("\t2D Layout Dimensions DataBlock : [" + numSamplesDB[0] + ", " + numSamplesDB[1] + ", " + numSamplesDB[2] + "]");
//
//                // determine fixed position index and value
//                int fixedCoordIndex = -1;
//                int fixedCoordValue = -1;
//                for (int pos = 0; pos < 3 && fixedCoordIndex == -1 ; ++pos) {
//                    if (Math.abs(chunkMax[pos] - chunkMin[pos]) == 1) {
//                        fixedCoordIndex = pos;
//                        fixedCoordValue = chunkMin[pos];
//                    }
//                }
//
//                System.out.print("\tCoords page : " + chunkMin[0] + "/" + chunkMax[0] + " " + chunkMin[1] + "/" + chunkMax[1] + "/" + chunkMin[2] + "/" + chunkMax[2]);
//                int chunkSizeY = chunkMax[2] - chunkMin[2];
//                int chunkSizeX = chunkMax[1] - chunkMin[1];
//                int chunkSizeZ = chunkMax[0] - chunkMin[0];
//
//                FloatBuffer buffer = page.getWritableBuffer(pitch).asFloatBuffer();
//                float[] output = new float[buffer.capacity()];
//
//                System.out.print( " Page Buffer Size : " + output.length);// + " / [" + stats.getMin() + "," + stats.getMax() + "]");
//                // dimension of buffer
//                System.out.println(" Pitch : [" + pitch[0] + ", " + pitch[1] + "," + pitch[2] + "]");
//
//                long time_2 = System.currentTimeMillis();
//
//                int[] numSamples = numSamplesChunk;
//
//                for (int iDim2 = 0; iDim2 < numSamples[2]; iDim2++) {
//                    localIndex[2] = iDim2;
//                    for (int iDim1 = 0; iDim1 < numSamples[1]; iDim1++) {
//                        localIndex[1] = iDim1;
//                        for (int iDim0 = 0; iDim0 < numSamples[0]; iDim0++) {
//                            localIndex[0] = iDim0;
//                            int[] local2DIndex = convertTo2DLocalIndex(localIndex, fixedCoordIndex);
//                            int[] voxelIndex = outputIndexer.localChunkIndexToLocalIndex(local2DIndex);
//                            int[] voxelPos = convertTo3DCoord(voxelIndex, fixedCoordIndex, fixedCoordValue);
//
//                            float value = 0f;
//                            double dist = 0;
//                            if (voxelPos[0] >= midX) {
//                                dist = distance2D(midY, midZ, voxelPos[1], voxelPos[2]);
//                            } else {
//                                dist = distance3D(midX, midY, midZ, voxelPos[0], voxelPos[1], voxelPos[2]);
//                            }
//                            value = (float) Math.sin((dist * cycles) / distMax);
//
//                            int iPos = outputIndexer.localIndexToDataIndex(local2DIndex);
//                            output[iPos] = value;
//                        }
//                    }
//                }
//
//                // write buffer then release page
//                page.writeFloatBuffer(output, pitch);
//                page.pageRelease();
//            }
//
//            pageAccessor.commit();
//            accessManager.flushUploadQueue();
//            accessManager.destroyVolumeDataPageAccessor(pageAccessor);
//        }
//    }


    /**
     * Convert 3D index to 2D index
     */
    private static int[] convertTo2DLocalIndex(int[] localIndex, int fixedCoordIndex) {
        int[] local2DIndex = new int[2];
        if (fixedCoordIndex == 0) {
            local2DIndex[0] = localIndex[1];
            local2DIndex[1] = localIndex[2];
        }
        else if (fixedCoordIndex == 1) {
            local2DIndex[0] = localIndex[0];
            local2DIndex[1] = localIndex[2];
        }
        else if (fixedCoordIndex == 2) {
            local2DIndex[0] = localIndex[0];
            local2DIndex[1] = localIndex[1];
        }
        return local2DIndex;
    }

    /**
     * Convert 2D index to 3D index
     * @param voxelIndex
     * @param fixedCoordIndex
     * @param fixedCoordValue
     */
    private static int[] convertTo3DCoord(int[] voxelIndex, int fixedCoordIndex, int fixedCoordValue) {
        int[] voxelPos = new int[3];
        voxelPos[fixedCoordIndex] = fixedCoordValue;
        if (fixedCoordIndex == 0) {
            voxelPos[1] = voxelIndex[0];
            voxelPos[2] = voxelIndex[1];
        }
        else if (fixedCoordIndex == 1) {
            voxelPos[0] = voxelIndex[0];
            voxelPos[2] = voxelIndex[1];
        }
        else if (fixedCoordIndex == 2) {
            voxelPos[0] = voxelIndex[0];
            voxelPos[1] = voxelIndex[1];
        }
        return voxelPos;
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
