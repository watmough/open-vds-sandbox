/*
 * Copyright 2022 The Open Group
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

import org.opengroup.openvds.*;

import static org.testng.Assert.*;
import org.testng.annotations.*;

import static org.opengroup.openvds.DimensionsND.Dimensions_012;
import static org.opengroup.openvds.VolumeDataFormat.*;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

public class TestVDSQuery {

    GlobalState globalState;

    @BeforeClass
    public void setUpClass() {
        this.globalState = OpenVDS.getGlobalState();
    }

    @AfterClass
    public static void tearDownClass() {
    }

    public TestVDSQuery() {
    }

    @Test
    public void testVDSQuery() {
        int nXSamples = 256, nYSamples = 256, nZSamples = 256;
        VolumeDataFormat format = Format_U8;
        try (InMemoryVDSGenerator generator = new InMemoryVDSGenerator(nZSamples, nYSamples, nXSamples, format)) {
            assertNotNull(generator);
            processWithoutLOD(generator.getVDS(), 40, 50);
        }
        catch (Throwable t) {
            fail();
        }
    }
    
     void processWithoutLOD(String vdsFilePath, int blockSize, int max_iterations) throws Exception {
        VDSFileOpenOptions fileOptions = new VDSFileOpenOptions(vdsFilePath);
        VDS vds = OpenVDS.open(vdsFilePath);
        processWithoutLOD(vds, blockSize, max_iterations);
     }
     
    void processWithoutLOD(VDS vds, int blockSize, int max_iterations) throws Exception {
        assertNotNull(vds);
        int iterations = max_iterations;
        VolumeDataAccessManager accessManager = vds.getAccessManager();
        VolumeDataLayout volumeDataLayout = accessManager.getVolumeDataLayout();
        VolumeDataAxisDescriptor sampleAxis = volumeDataLayout.getAxisDescriptor(0);
        VolumeDataAxisDescriptor jAxis = volumeDataLayout.getAxisDescriptor(1);
        VolumeDataAxisDescriptor iAxis = volumeDataLayout.getAxisDescriptor(2);
        VolumeDataLayoutDescriptor.LODLevels lodLevels = volumeDataLayout.getLayoutDescriptor().getLODLevels();
        VolumeDataFormat channelFormat = volumeDataLayout.getChannelFormat(0);

        int dimK = sampleAxis.getNumSamples();
        int dimJ = jAxis.getNumSamples();
        int dimI = iAxis.getNumSamples();

        System.out.println(String.format("Dimensions : [%d,%d,%d]", dimI, dimJ, dimK));

        // defines size of java brick size
        int blockI = (dimI / blockSize);
        int blockJ = (dimJ / blockSize);
        int blockK = (dimK / blockSize);

        if (blockI * blockSize < dimI) blockI++;
        if (blockJ * blockSize < dimJ) blockJ++;
        if (blockK * blockSize < dimK) blockK++;

        System.out.println(String.format("Brick size %d, Block count : [%d,%d,%d]", blockSize, blockI, blockJ, blockK));

        VolumeDataFormat vdsFormat = channelFormat;

        int cpt = 0;

        // map between brick index and buffer
        Map<BrickIndex, ByteBuffer> allBricksBuffer = new HashMap<>();

        while (max_iterations == 0 || iterations > 0) {
            --iterations;
            int nbComp = 0;
            int nbBadCmp = 0;
            System.out.println("Read #" + ++cpt);
            for (int i = 0; i < blockI; ++i) {
                for (int j = 0; j < blockJ; ++j) {
                    for (int k = 0; k < blockK; ++k) {
                        BrickIndex indexBrick = new BrickIndex(i, j, k);
                        int[] cornerO = new int[]{k * blockSize, j * blockSize, i * blockSize, 0, 0, 0};
                        int[] corner1 = new int[]{
                                Math.min(cornerO[0] + blockSize, dimK),
                                Math.min(cornerO[1] + blockSize, dimJ),
                                Math.min(cornerO[2] + blockSize, dimI),
                                0, 0, 0};

                        VolumeDataRequest request = accessManager.requestVolumeSubset(Dimensions_012, 0, 0, cornerO, corner1, vdsFormat);
                        if (request != null) {
                            checkAndWaitRequest(request, 1000);
                            ByteBuffer resBuffer = request.getBuffer();
                            byte[] resArray = new byte[resBuffer.capacity()];
                            resBuffer.get(resArray);
                            resBuffer.rewind();
                            if (allBricksBuffer.containsKey(indexBrick)) {
                                ByteBuffer oldByteBuffer = allBricksBuffer.get(indexBrick);
                                byte[] oldResArray = new byte[oldByteBuffer.capacity()];
                                oldByteBuffer.get(oldResArray);
                                oldByteBuffer.rewind();
                                ++nbComp;
                                if (!compare(oldResArray, resArray)) {
                                    ++nbBadCmp;
                                }
                            }
                            allBricksBuffer.put(indexBrick, resBuffer);
                        }
                    }
                }
            }

            if (nbBadCmp > 0) {
                System.out.println(" Some bricks failed " + nbBadCmp + "/" + nbComp);
            }
            System.out.println("");
            assertTrue(nbBadCmp == 0);
        }
    }
    
    private boolean compare(byte[] oldResArray, byte[] resArray) {
        if (oldResArray.length != resArray.length) return false;
        for (int c = 0 ; c < oldResArray.length ; ++c) {
            if (oldResArray[c] != resArray[c]) return false;
        }
        return true;
    }

    public VolumeDataRequest requestSubVolume(VolumeDataAccessManager accessManager, VolumeDataFormat vdsFormat, int lod, int channelIndex, DimensionsND dimension, int[] coordMin, int[] coordMax) {
        return accessManager.requestVolumeSubset(dimension, lod, channelIndex, coordMin, coordMax, vdsFormat);
    }

    public void checkAndWaitRequest(VolumeDataRequest request, int timeout) throws IOException {
        while (!request.waitForCompletion(timeout)) {
            if (request.isCanceled()) {
                String errorInfo = request.getErrorMessage();
                StringBuilder detail = new StringBuilder("Error ");
                if (errorInfo != null) {
                    detail.append(" / VDS Error code : ").append(request.getErrorCode()).append(" msg : ").append(errorInfo);
                }
                throw new IOException("Job cancelled " + detail.toString());
            }
        }
    }

    class BrickIndex {
        int i;
        int j;
        int k;

        BrickIndex(int i, int j, int k) {
            this.i = i;
            this.j = j;
            this.k = k;
        }

        @Override
        public boolean equals(Object obj) {
            if (obj == this) return true;
            if (!(obj instanceof BrickIndex)) return false;
            BrickIndex other = (BrickIndex) obj;
            return other.i == i && other.j == j && other.k == k;
        }

        @Override
        public int hashCode() {
            return Objects.hash(i, j, k);
        }
    }
    
}
