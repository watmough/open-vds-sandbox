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
        vds = new MemoryVdsGenerator(100, 100, 100, VolumeDataChannelDescriptor.Format.FORMAT_R32);
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

    @Test
    public void testVolumeIndexerCreationDeletion() {
        try {
            // TODO : create fils in tmp dir
            System.getProperty("java.io.tmpdir");
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
            VDSFileOpenOptions options = new VDSFileOpenOptions("/tmp/testPageAccessorFloat100_100_100.vds");
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
            System.out.println("Nombre de chunks : " + chunkCount);
            for (int i = 0; i < chunkCount; i++) {
                VolumeDataPage page = pageAccessor.createPage(i);
                System.out.println("Page created : " + i);
                VolumeIndexer3D outputIndexer = new  VolumeIndexer3D(page, 0, 0, DimensionsND.DIMENSIONS_012.ordinal(), layout);
                System.out.println("\tOutput indexer created : " + i);

                int[] numSamples = new int[3];
                for (int j = 0; j < 3; j++) {
                    numSamples[j] = outputIndexer.getDataBlockNumSamples(j);
                }
                System.out.println("\tChunk dim (" + i + ") : I " + numSamples[0] + " - X " + numSamples[1] + " - Z " + numSamples[2]);

                int[] pitch = new int[VolumeDataLayout.Dimensionality_Max];
                int[] chunkMin = new int[VolumeDataLayout.Dimensionality_Max];
                int[] chunkMax = new int[VolumeDataLayout.Dimensionality_Max];

                page.getMinMax(chunkMin, chunkMax);
                int chunkSizeI = chunkMax[2] - chunkMin[2];
                int chunkSizeX = chunkMax[1] - chunkMin[1];
                int chunkSizeZ = chunkMax[0] - chunkMin[0];
                int nbElem = chunkSizeI * chunkSizeX * chunkSizeZ;
                float[] buffer = new float[nbElem];
                System.out.println("\tGot min max : " + i);
                System.out.println("\tChunk coords (" + i + ") : I " + chunkMin[2] + "/" + chunkMax[2] + " - X " + chunkMin[1] + "/" + chunkMax[1] + " - Z " + chunkMin[0] + "/" + chunkMax[0]);

                //float[] buffer = page.readFloatBuffer(pitch);
                //double[] buffer = page.readDoubleBuffer(pitch);
                //byte[] buffer = page.readByteBuffer(pitch);

                for (int idx = 0 ; idx < buffer.length ; ++idx) {
                    buffer[idx] = (float) (idx % 256);
                }

                int smp = 0;
                for (int iDim2 = 0; iDim2 < chunkSizeI; iDim2++) {
                    System.out.print(".");
                    for (int iDim1 = 0; iDim1 < chunkSizeX; iDim1++) {
                        for (int iDim0 = 0; iDim0 < chunkSizeZ; iDim0++) {
                            int[] localOutIndex = new int[]{iDim0, iDim1, iDim2};

                            int[] voxelIndex = outputIndexer.localIndexToVoxelIndex(localOutIndex);

                            int pos[] = new int[] {
                                    voxelIndex[0],
                                    voxelIndex[1],
                                    voxelIndex[2]
                            };

                            float value = pos[0];
                            //double value = pos[0];
                            //byte value = (byte) (pos[0] % Byte.MAX_VALUE);
                            int dataIndex = outputIndexer.localIndexToDataIndex(pos);
                            if (dataIndex < buffer.length) {
                                buffer[dataIndex] = value;
                                ++smp;
                            }
                        }
                    }
                }
                System.out.println("");
                System.out.println("\t" + smp + " samples will be written");
                System.out.println("\tWill write buffer " + i);
                page.writeFloatBuffer(buffer, pitch, layout.getDimensionality());
                System.out.println("\tDone write buffer " + i);
                //page.writeDoubleBuffer(buffer);
                //page.writeByteBuffer(buffer);
                page.release();
                System.out.println("Page writed : " + i);
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
