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

package test.org.opengroup.openvds;
import org.opengroup.openvds.*;

import static org.testng.Assert.*;

import org.testng.Assert;
import org.testng.annotations.*;

import java.io.File;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import static org.opengroup.openvds.VolumeDataFormat.*;
import static org.opengroup.openvds.VolumeDataPageAccessor.AccessMode.*;
import static org.opengroup.openvds.DimensionsND.*;

public class PageAccessorFloatTest {

    private static String TEMP_FILE_NAME_VOL_INDEX = "volIndexer";
    private static String TEMP_FILE_NAME_COPY = "vdsFloatCopy";

    public String url;
    public VolumeDataLayoutDescriptor ld;
    public VolumeDataAxisDescriptor[] vda;
    public VolumeDataChannelDescriptor[] vdc;
    public MetadataReadAccess md;
    public InMemoryVDSGenerator vds;
    public MetadataContainer metadataContainer;
    public VDSError vdserror;

    private String tempVolIndexerFileName;
    private String tempVdsCopyFileName;

    static void assertWithinRange(float[] values, float min, float max) {
        for (float val : values) {
            assertFalse(Float.isInfinite(val));
            assertFalse(Float.isNaN(val));
            assertTrue(val >= min && val <= max);
        }
    }

    @BeforeClass
    public void init() {
        vds = new InMemoryVDSGenerator(200, 200, 200, Format_R32);
        url = "inmemory://create_test";
        VolumeDataLayout volumeDataLayout = vds.getLayout();

        int nbChannel = volumeDataLayout.getChannelCount();
        VolumeDataAccessManager accessManager = vds.getAccessManager();

        for (VolumeDataLayoutDescriptor.LODLevels l : VolumeDataLayoutDescriptor.LODLevels.values()) {
            for (int channel = 0; channel < nbChannel; channel++) {
                for (DimensionsND dimGroup : DimensionsND.values()) {
                    VDSProduceStatus vdsProduceStatus = accessManager.getVDSProduceStatus(dimGroup, l.value(), channel);
                }
            }
        }

        vda = new VolumeDataAxisDescriptor[] {
                volumeDataLayout.getAxisDescriptor(0),
                volumeDataLayout.getAxisDescriptor(1),
                volumeDataLayout.getAxisDescriptor(2)};
        vdc = new VolumeDataChannelDescriptor[] {volumeDataLayout.getChannelDescriptor(0)};

        md = volumeDataLayout;
        ld = volumeDataLayout.getLayoutDescriptor();

        metadataContainer = new MetadataContainer();

        vdserror = new VDSError();

        long ms = System.currentTimeMillis();
        tempVolIndexerFileName = TEMP_FILE_NAME_VOL_INDEX + "_" + ms + ".vds";
        tempVdsCopyFileName = TEMP_FILE_NAME_COPY + "_" + ms + ".vds";
    }

    @AfterClass
    public void cleanFiles() {
        String tempDir = System.getProperty("java.io.tmpdir");
        String fileVolIndexPath = tempDir + File.separator + tempVolIndexerFileName;
        File fileVolIndex = new File(fileVolIndexPath);
        if (fileVolIndex.exists()) {
            fileVolIndex.delete();
        }

        String fileCopyPath = tempDir + File.separator + tempVdsCopyFileName;
        File fileCopy = new File(fileCopyPath);
        if (fileCopy.exists()) {
            fileCopy.delete();
        }
    }

    @Test
    public void testVolumeIndexerCreationDeletion() {
        // create file in tmp dir
        String tmpDir = System.getProperty("java.io.tmpdir");
        String volIndexPath = tmpDir + File.separator + tempVolIndexerFileName;
        VDSFileOpenOptions options = new VDSFileOpenOptions(volIndexPath);
        VDSError error = new VDSError();
        VDS vdsTest = OpenVDS.create(options, ld,
                vda,
                vdc, metadataContainer, error);
        VolumeDataAccessManager accessManager = vdsTest.getAccessManager();
        //ASSERT_TRUE(accessManager);

        int channel = 0;
        VolumeDataLayout layout = vdsTest.getLayout();
        VolumeDataPageAccessor pageAccessor = accessManager.createVolumeDataPageAccessor(
                Dimensions_012, // dimension ND
                0, // lod
                channel, // channel
                100, // max pages
                AccessMode_Create); // access mode

        VolumeDataPage page = pageAccessor.createPage(0);
/*
        VolumeIndexer3D outputIndexer = new  VolumeIndexer3D(page, 0, 0, DimENSIONS_012.ordinal(), layout);
        outputIndexer.finalize();
        */

        vdsTest.close();
    }

    @Test
    public void testCopyPageAccessor() {
            String tmpDir = System.getProperty("java.io.tmpdir");
            String vdsPath = tmpDir + File.separator + tempVdsCopyFileName;
            VDSFileOpenOptions options = new VDSFileOpenOptions(vdsPath);

            // copy information from input VDS
            VolumeDataLayoutDescriptor cpLd = new VolumeDataLayoutDescriptor(
                    ld.getBrickSize(),
                    ld.getNegativeMargin(),
                    ld.getPositiveMargin(),
                    ld.getBrickSizeMultiplier2D(),
                    ld.getLODLevels(),
                    ld.getOptions(),
                    ld.getFullResolutionDimension()
            );

            VolumeDataAxisDescriptor[] cpVda = new VolumeDataAxisDescriptor[3];
            for (int i = 0 ; i < 3 ; ++i) {
                cpVda[i] = new VolumeDataAxisDescriptor(vda[i].getNumSamples(),
                        vda[i].getName(), vda[i].getUnit(),
                        vda[i].getCoordinateMin(), vda[i].getCoordinateMax());
            }


            VolumeDataChannelDescriptor[] cpVdc = new VolumeDataChannelDescriptor[1];
            cpVdc[0] = new VolumeDataChannelDescriptor(
                    vdc[0].getFormat(),
                    vdc[0].getComponents(),
                    vdc[0].getName(),
                    vdc[0].getUnit(),
                    vdc[0].getValueRangeMin(), vdc[0].getValueRangeMax(),
                    vdc[0].getMapping(),
                    vdc[0].getMappedValueCount(),
                    vdc[0].getFlags(),
                    vdc[0].getNoValue(),
                    vdc[0].getIntegerScale(),
                    vdc[0].getIntegerOffset()
            );
            MetadataContainer cpMetadataContainer = new MetadataContainer();
            VDS vdsCopy = OpenVDS.create(options, cpLd,
                    cpVda,
                    cpVdc,
                    cpMetadataContainer,
                    vdserror);

            VolumeDataAccessManager accessManager = vdsCopy.getAccessManager();
            //ASSERT_TRUE(accessManager);

            int channel = 0;
            VolumeDataPageAccessor pageAccessor = accessManager.createVolumeDataPageAccessor(
                    Dimensions_012, // dimension ND
                    0, // lod
                    channel, // channel
                    100, // max pages
                    AccessMode_Create); // access mode

            // get input manager
            VolumeDataAccessManager inputAM = vds.getAccessManager();
            VolumeDataPageAccessor pageAccessorInput = inputAM.createVolumeDataPageAccessor(
                    Dimensions_012, // dimension ND
                    0, // lod
                    channel, // channel
                    100, // max pages
                    AccessMode_ReadOnly); // access mode

            // copy file
            int[] pitch = new int[VolumeDataLayout.Dimensionality_Max];
            long chunkCount = pageAccessorInput.getChunkCount();
            for (long chunk = 0 ; chunk < chunkCount ; ++chunk) {
                VolumeDataPage inputPage = pageAccessorInput.readPage(chunk);
                VolumeDataPage page = pageAccessor.createPage(chunk);
                ByteBuffer readBuffer = inputPage.getBuffer(pitch);
                assertTrue(readBuffer.order() == ByteOrder.nativeOrder());
//                float[] data = inputPage.readFloatBuffer(pitch);
                 ByteBuffer writeBuffer = page.getWritableBuffer(pitch);
                assertTrue(writeBuffer.order() == ByteOrder.nativeOrder());
//                page.writeFloatBuffer(data, pitch);
                writeBuffer.put(readBuffer);
                inputPage.release();
                page.release();
            }
            pageAccessor.close();
            vdsCopy.close();
    }

    /**
     * Will test that copied file is the same as the input
     */

    @Test
    public void testCopyPageAccessorValidation() {
//        testCopyPageAccessor(); // Uncomment this line if running separately
            String tmpDir = System.getProperty("java.io.tmpdir");
            String vdsPath = tmpDir + File.separator + tempVdsCopyFileName;
            VDSFileOpenOptions options = new VDSFileOpenOptions(vdsPath);
            VDS vdsCopy = OpenVDS.open(options, vdserror);
            VolumeDataAccessManager accessManager = vdsCopy.getAccessManager();

            int channel = 0;
            VolumeDataLayout layout = vdsCopy.getLayout();
            VolumeDataPageAccessor pageAccessor = accessManager.createVolumeDataPageAccessor(
                    Dimensions_012, // dimension ND
                    0, // lod
                    channel, // channel
                    20, // max pages
                    AccessMode_ReadOnly); // access mode

            // get input manager
            VolumeDataAccessManager inputAM = vds.getAccessManager();
            VolumeDataPageAccessor pageAccessorInput = inputAM.createVolumeDataPageAccessor(
                    Dimensions_012, // dimension ND
                    0, // lod
                    channel, // channel
                    20, // max pages
                    AccessMode_ReadOnly); // access mode

            // compares block data
            int[] pitchInput = new int[VolumeDataLayout.Dimensionality_Max];
            int[] pitchOutput = new int[VolumeDataLayout.Dimensionality_Max];

            long chunkCount = pageAccessorInput.getChunkCount();
            for (long chunk = 0 ; chunk < chunkCount ; ++chunk) {
                VolumeDataPage inputPage = pageAccessorInput.readPage(chunk);
                VolumeDataPage page = pageAccessor.readPage(chunk);
                FloatBuffer dataInB = inputPage.getBuffer(pitchInput).asFloatBuffer();
                FloatBuffer dataOutB = page.getBuffer(pitchOutput).asFloatBuffer();
                float[] dataIn = new float[dataInB.remaining()];
                float[] dataOut = new float[dataOutB.remaining()];
                dataInB.get(dataIn);
                dataOutB.get(dataOut);

                inputPage.release();
                page.release();

                Assert.assertEquals(pitchInput, pitchOutput);
                Assert.assertEquals(dataIn, dataOut);
                assertWithinRange(dataIn, -1.0f, 1.0f);
                assertWithinRange(dataOut, -1.0f, 1.0f);
            }
            pageAccessor.close();
            vdsCopy.close();
    }

    /**
     * Will test that copied file has the same layout as original (test that 3D positions are in the same chunk index)
     */

    @Test
    public void testCopyPageAccessorValidationChunkIndex() {
//        testCopyPageAccessor(); // Uncomment this line if running separately
            String tmpDir = System.getProperty("java.io.tmpdir");
            String vdsPath = tmpDir + File.separator + tempVdsCopyFileName;
            VDSFileOpenOptions options = new VDSFileOpenOptions(vdsPath);
            VDS vdsCopy = OpenVDS.open(options, vdserror);
            VolumeDataAccessManager accessManager = vdsCopy.getAccessManager();

            int channel = 0;
            VolumeDataLayout layout = vdsCopy.getLayout();
            VolumeDataPageAccessor pageAccessor = accessManager.createVolumeDataPageAccessor(
                    Dimensions_012, // dimension ND
                    0, // lod
                    channel, // channel
                    20, // max pages
                    AccessMode_ReadOnly); // access mode

            // compares block data
            int[] chunkMin = new int[VolumeDataLayout.Dimensionality_Max];
            int[] chunkMax = new int[VolumeDataLayout.Dimensionality_Max];
            int[] chunkMaxPos = new int[VolumeDataLayout.Dimensionality_Max];

            long chunkCount = pageAccessor.getChunkCount();
            for (long chunk = 0 ; chunk < chunkCount ; ++chunk) {
                VolumeDataPage inputPage = pageAccessor.readPage(chunk);

                // check that chunk index matches current index
                inputPage.getMinMaxExcludingMargin(chunkMin, chunkMax);
                for (int i = 0 ; i < VolumeDataLayout.Dimensionality_Max ; ++i) {
                    chunkMaxPos[i] = chunkMax[i] != 0 ? chunkMax[i] - 1 : chunkMax[i];
                }
                long idxChMin = pageAccessor.getChunkIndex(chunkMin);
                long idxChMax = pageAccessor.getChunkIndex(chunkMaxPos);

                Assert.assertEquals(chunk, idxChMin);
                Assert.assertEquals(idxChMin, idxChMax);

                inputPage.release();
            }
            pageAccessor.close();
            vdsCopy.close();
    }
}
 