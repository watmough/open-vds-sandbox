package org.opengroup.openvds;

import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import static org.testng.Assert.assertEquals;
import static org.testng.Assert.fail;

public class PageAccessorTest {
    public String url;
    public VolumeDataLayoutDescriptor ld;
    public VolumeDataAxisDescriptor[] vda;
    public VolumeDataChannelDescriptor[] vdc;
    public MetadataReadAccess md;
    public MemoryVdsGenerator vds;
    private MetadataContainer metadataContainer;

    @BeforeClass
    public void init() {
        vds = new MemoryVdsGenerator(16, 16, 16, VolumeDataChannelDescriptor.Format.FORMAT_R32);
        url = "inmemory://create_test";
        VolumeDataLayout volumeDataLayout = vds.getLayout();

        int nbChannel = volumeDataLayout.getChannelCount();
        VolumeDataAccessManager accessManager = vds.getAccessManager();

        for (VolumeDataLayoutDescriptor.LODLevels l : VolumeDataLayoutDescriptor.LODLevels.values()) {
            for (int channel = 0; channel < nbChannel; channel++) {
                for (DimensionsND dimGroup : DimensionsND.values()) {
                    VDSProduceStatus vdsProduceStatus = accessManager.getVDSProduceStatus(volumeDataLayout, dimGroup, l.ordinal(), channel);
                }
            }
        }

        vda = new VolumeDataAxisDescriptor[] {volumeDataLayout.getAxisDescriptor(0),
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

    @Test
    public void testVolumeIndexerCreationDeletion() {
        try {
            VDSFileOpenOptions options = new VDSFileOpenOptions("/tmp/testVolumeIndexer.vds");
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
                    VolumeDataAccessManager.AccessMode.Create.getCode()); // access mode

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
    public void testCreatePageAccessor() {
        try {
            VDSFileOpenOptions options = new VDSFileOpenOptions("/tmp/testPageAccessor.vds");
            VdsHandle vdsPageAccessor = OpenVDS.create(options, ld,
                    vda,
                    vdc, metadataContainer);
            VolumeDataAccessManager accessManager = vdsPageAccessor.getAccessManager();
            //ASSERT_TRUE(accessManager);

            int channel = 0;
            VolumeDataLayout layout = vdsPageAccessor.getLayout();
            VolumeDataPageAccessor pageAccessor = accessManager.createVolumeDataPageAccessor(
                    layout, // layout
                    DimensionsND.DIMENSIONS_012.ordinal(), // dimension ND
                    0, // lod
                    channel, // channel
                    100, // max pages
                    VolumeDataAccessManager.AccessMode.Create.getCode()); // access mode

            int chunkCount = (int) pageAccessor.getChunkCount();

            for (int i = 0; i < chunkCount; i++) {
                VolumeDataPage page = pageAccessor.createPage(i);
                VolumeIndexer3D outputIndexer = new  VolumeIndexer3D(page, 0, 0, DimensionsND.DIMENSIONS_012.ordinal(), layout);

                int[] numSamples = new int[3];

                for (int j = 0; j < 3; j++) {
                    numSamples[j] = outputIndexer.getDataBlockNumSamples(j);
                }

                int[] pitch = new int[VolumeDataLayout.Dimensionality_Max];
                int[] chunkMin = new int[VolumeDataLayout.Dimensionality_Max];
                int[] chunkMax = new int[VolumeDataLayout.Dimensionality_Max];

                pageAccessor.getChunkMinMax(i, chunkMin, chunkMax);

                float[] floatBuffer = page.readFloatBuffer(pitch);

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
                            int dataIndex = outputIndexer.localIndexToDataIndex(localOutIndex);
                            floatBuffer[dataIndex] = value;
                        }
                page.writeFloatBuffer(floatBuffer, pitch);
                page.release();
                System.out.println("Create float buffer succes");
            }

            pageAccessor.commit();
            pageAccessor.setMaxPages(0);
            accessManager.flushUploadQueue();
            accessManager.destroyVolumeDataPageAccessor(pageAccessor);

            vdsPageAccessor.close();
        }
        catch (java.io.IOException e) {
            System.out.println(e.getMessage());
            fail();
        }
    }
}
