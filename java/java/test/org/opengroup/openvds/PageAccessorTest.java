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

package org.opengroup.openvds;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;


import java.io.File;

import static org.testng.Assert.fail;

public class PageAccessorTest {

    private static String TEMP_FILE_NAME_VOL_INDEX = "volIndexer.vds";
    private static String TEMP_FILE_NAME_COPY = "vdsCopy.vds";

    public String url;
    public VolumeDataLayoutDescriptor ld;
    public VolumeDataAxisDescriptor[] vda;
    public VolumeDataChannelDescriptor[] vdc;
    public MetadataReadAccess md;
    public MemoryVdsGenerator vds;
    public MetadataContainer metadataContainer;

    @BeforeClass
    public void init() {
        vds = new MemoryVdsGenerator(200, 300, 300, VolumeDataChannelDescriptor.Format.FORMAT_R32);
        url = "inmemory://create_test";
        VolumeDataLayout volumeDataLayout = vds.getLayout();

        int nbChannel = volumeDataLayout.getChannelCount();
        VolumeDataAccessManager accessManager = vds.getAccessManager();

        for (VolumeDataLayoutDescriptor.LODLevels l : VolumeDataLayoutDescriptor.LODLevels.values()) {
            for (int channel = 0; channel < nbChannel; channel++) {
                for (DimensionsND dimGroup : DimensionsND.values()) {
                    VDSProduceStatus vdsProduceStatus = accessManager.getVDSProduceStatus(dimGroup, l.ordinal(), channel);
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
    }

    @AfterClass
    public void cleanFiles() {
        String tempDir = System.getProperty("java.io.tmpdir");
        String fileVolIndexPath = tempDir + File.separator + TEMP_FILE_NAME_VOL_INDEX;
        File fileVolIndex = new File(fileVolIndexPath);
        if (fileVolIndex.exists()) {
            fileVolIndex.delete();
        }

        String fileCopyPath = tempDir + File.separator + TEMP_FILE_NAME_COPY;
        File fileCopy = new File(fileCopyPath);
        if (fileCopy.exists()) {
            fileCopy.delete();
        }
    }

    @Test
    public void testVolumeIndexerCreationDeletion() {
        try {
            // create file in tmp dir
            String tmpDir = System.getProperty("java.io.tmpdir");
            String volIndexPath = tmpDir + File.separator + TEMP_FILE_NAME_VOL_INDEX;
            VDSFileOpenOptions options = new VDSFileOpenOptions(volIndexPath);
            VdsHandle vdsTest = OpenVDS.create(options, ld,
                    vda,
                    vdc, metadataContainer);
            VolumeDataAccessManager accessManager = vdsTest.getAccessManager();
            //ASSERT_TRUE(accessManager);

            int channel = 0;
            VolumeDataLayout layout = vdsTest.getLayout();
            VolumeDataPageAccessor pageAccessor = accessManager.createVolumeDataPageAccessor(
                    layout, // layout
                    DimensionsND.DIMENSIONS_012.ordinal(), // dimension ND
                    0, // lod
                    channel, // channel
                    100, // max pages
                    VolumeDataPageAccessor.AccessMode.Create.getCode()); // access mode

            VolumeDataPage page = pageAccessor.createPage(0);
            VolumeIndexer3D outputIndexer = new  VolumeIndexer3D(page, 0, 0, DimensionsND.DIMENSIONS_012.ordinal(), layout);
            outputIndexer.finalize();

            vdsTest.close();
        }
        catch (java.io.IOException e) {
            System.out.println(e.getMessage());
            fail();
        }
    }
    
    @Test
    public void testCopyPageAccessor() {
        try {
            String tmpDir = System.getProperty("java.io.tmpdir");
            String vdsPath = tmpDir + File.separator + TEMP_FILE_NAME_COPY;
            VDSFileOpenOptions options = new VDSFileOpenOptions(vdsPath);
            VdsHandle vdsCopy = OpenVDS.create(options, ld,
                    vda,
                    vdc, metadataContainer);
            VolumeDataAccessManager accessManager = vdsCopy.getAccessManager();
            //ASSERT_TRUE(accessManager);

            int channel = 0;
            VolumeDataLayout layout = vdsCopy.getLayout();
            VolumeDataPageAccessor pageAccessor = accessManager.createVolumeDataPageAccessor(
                    layout, // layout
                    DimensionsND.DIMENSIONS_012.ordinal(), // dimension ND
                    0, // lod
                    channel, // channel
                    20, // max pages
                    VolumeDataPageAccessor.AccessMode.Create.getCode()); // access mode

            // get input manager
            VolumeDataAccessManager inputAM = vds.getAccessManager();
            VolumeDataPageAccessor pageAccessorInput = inputAM.createVolumeDataPageAccessor(
                    layout, // layout
                    DimensionsND.DIMENSIONS_012.ordinal(), // dimension ND
                    0, // lod
                    channel, // channel
                    20, // max pages
                    VolumeDataPageAccessor.AccessMode.ReadOnly.getCode()); // access mode

            // copy file
            int[] pitch = new int[VolumeDataLayout.Dimensionality_Max];
            long chunkCount = pageAccessorInput.getChunkCount();
            for (long chunk = 0 ; chunk < chunkCount ; ++chunk) {
                VolumeDataPage inputPage = pageAccessorInput.readPage(chunk);
                VolumeDataPage page = pageAccessor.createPage(chunk);
                float[] data = inputPage.readFloatBuffer(pitch);
                page.writeFloatBuffer(data, pitch);

                inputPage.pageRelease();
                page.pageRelease();
            }

            pageAccessor.commit();
            pageAccessor.setMaxPages(0);
            accessManager.flushUploadQueue();
            accessManager.destroyVolumeDataPageAccessor(pageAccessor);

            vdsCopy.close();
        }
        catch (java.io.IOException e) {
            System.out.println(e.getMessage());
            fail();
        }
    }

    /**
     * Will test that copied file is the same as the input
     * Name of method is the same + Suffix so that it's executed after the copy test
     */
    @Test
    public void testCopyPageAccessorValidation() {
        try {
            String tmpDir = System.getProperty("java.io.tmpdir");
            String vdsPath = tmpDir + File.separator + TEMP_FILE_NAME_COPY;
            VDSFileOpenOptions options = new VDSFileOpenOptions(vdsPath);
            VdsHandle vdsCopy = OpenVDS.open(options);
            VolumeDataAccessManager accessManager = vdsCopy.getAccessManager();

            int channel = 0;
            VolumeDataLayout layout = vdsCopy.getLayout();
            VolumeDataPageAccessor pageAccessor = accessManager.createVolumeDataPageAccessor(
                    layout, // layout
                    DimensionsND.DIMENSIONS_012.ordinal(), // dimension ND
                    0, // lod
                    channel, // channel
                    20, // max pages
                    VolumeDataPageAccessor.AccessMode.ReadOnly.getCode()); // access mode

            // get input manager
            VolumeDataAccessManager inputAM = vds.getAccessManager();
            VolumeDataPageAccessor pageAccessorInput = inputAM.createVolumeDataPageAccessor(
                    layout, // layout
                    DimensionsND.DIMENSIONS_012.ordinal(), // dimension ND
                    0, // lod
                    channel, // channel
                    20, // max pages
                    VolumeDataPageAccessor.AccessMode.ReadOnly.getCode()); // access mode

            // compares block data
            int[] pitchInput = new int[VolumeDataLayout.Dimensionality_Max];
            int[] pitchOutput = new int[VolumeDataLayout.Dimensionality_Max];

            long chunkCount = pageAccessorInput.getChunkCount();
            for (long chunk = 0 ; chunk < chunkCount ; ++chunk) {
                VolumeDataPage inputPage = pageAccessorInput.readPage(chunk);
                VolumeDataPage page = pageAccessor.readPage(chunk);
                float[] dataIn = inputPage.readFloatBuffer(pitchInput);
                float[] dataOut = page.readFloatBuffer(pitchOutput);

                Assert.assertEquals(pitchInput, pitchOutput);
                Assert.assertEquals(dataIn, dataOut);

                inputPage.pageRelease();
                page.pageRelease();
            }

            accessManager.destroyVolumeDataPageAccessor(pageAccessor);

            vdsCopy.close();
        }
        catch (java.io.IOException e) {
            System.out.println(e.getMessage());
            fail();
        }
    }

    /**
     * Will test that copied file has the same layout than original (test that 3D positions are in the same chunk index)
     * Name of method is the same + Suffix so that it's executed after the copy test
     */
    @Test
    public void testCopyPageAccessorValidationChunkIndex() {
        try {
            String tmpDir = System.getProperty("java.io.tmpdir");
            String vdsPath = tmpDir + File.separator + TEMP_FILE_NAME_COPY;
            VDSFileOpenOptions options = new VDSFileOpenOptions(vdsPath);
            VdsHandle vdsCopy = OpenVDS.open(options);
            VolumeDataAccessManager accessManager = vdsCopy.getAccessManager();

            int channel = 0;
            VolumeDataLayout layout = vdsCopy.getLayout();
            VolumeDataPageAccessor pageAccessor = accessManager.createVolumeDataPageAccessor(
                    layout, // layout
                    DimensionsND.DIMENSIONS_012.ordinal(), // dimension ND
                    0, // lod
                    channel, // channel
                    20, // max pages
                    VolumeDataPageAccessor.AccessMode.ReadOnly.getCode()); // access mode

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

                inputPage.pageRelease();
            }

            accessManager.destroyVolumeDataPageAccessor(pageAccessor);

            vdsCopy.close();
        }
        catch (java.io.IOException e) {
        System.out.println(e.getMessage());
        fail();
    }
}
}
