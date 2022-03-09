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

import java.io.File;
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.util.Objects;
import java.util.Random;
import org.opengroup.openvds.*;
import org.junit.Test;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.opengroup.openvds.Error;

import static org.junit.Assert.*;

public class MetaDataContainerTest {

    private static String TEMP_FILE_NAME = "vdsTestMetadata.vds";

    public String url;
    public VolumeDataLayoutDescriptor ld;
    public VolumeDataAxisDescriptor[] vda;
    public VolumeDataChannelDescriptor[] vdc;
    public MetadataReadAccess md;
    public InMemoryVDSGenerator vds;

    @Before
    public void init() {
        vds = new InMemoryVDSGenerator(16, 16, 16, VolumeDataChannelDescriptor.Format.Format_U8);
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

        vda = new VolumeDataAxisDescriptor[] {volumeDataLayout.getAxisDescriptor(0),
                volumeDataLayout.getAxisDescriptor(1),
                volumeDataLayout.getAxisDescriptor(2)};
        vdc = new VolumeDataChannelDescriptor[] {volumeDataLayout.getChannelDescriptor(0)};

        md = volumeDataLayout;
        ld = volumeDataLayout.getLayoutDescriptor();
    }

    @After
    public void cleanTempFile() {
        String tempDir = System.getProperty("java.io.tmpdir");
        String tempFilePath = tempDir + File.separator + TEMP_FILE_NAME;
        File fileTemp = new File(tempFilePath);
        if (fileTemp.exists()) {
            fileTemp.delete();
        }
    }

    @Test
    public void testMetadataContainer() {
        // creates object
        MetadataContainer container = new MetadataContainer();

        // set values
        int singleInt = 1337;
        container.setMetadataInt("CategoryInt", "intMetaData", singleInt);

        // get values
        int readSingleInt = container.getMetadataInt("CategoryInt", "intMetaData");

        // check equality
        assertEquals(singleInt, readSingleInt);
    }

    @Test
    public void testNullCheckIntV2() {
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataIntVector2("categoryArray", "nullArray", (int[])null));
    }

    @Test
    public void testNullCheckIntV3() {
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataIntVector3("categoryArray", "nullArray", (int[])null));
    }

    @Test
    public void testNullCheckIntV4() {
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataIntVector4("categoryArray", "nullArray", (int[])null));
    }

    @Test
    public void testNullCheckFloatV2() {
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataFloatVector2("categoryArray", "nullArray", (float[])null));
    }

    @Test
    public void testNullCheckFloatV3() {
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataFloatVector3("categoryArray", "nullArray", (float[])null));
    }

    @Test
    public void testNullCheckFloatV4() {
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataFloatVector4("categoryArray", "nullArray", (float[])null));
    }

    @Test
    public void testNullCheckDoubleV2() {
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataDoubleVector2("categoryArray", "nullArray", (double[])null));
    }

    @Test
    public void testNullCheckDoubleV3() {
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataDoubleVector3("categoryArray", "nullArray", (double[])null));
    }

    @Test
    public void testNullCheckDoubleV4() {
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataDoubleVector4("categoryArray", "nullArray", (double[])null));
    }

    @Test
    public void testSizeCheckIntV2() {
        int[] vec3i = new int[] {54, 76, 99};
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataIntVector2("categoryInt", "IntV2", vec3i));
    }

    @Test
    public void testSizeCheckIntV3() {
        int[] vec2i = new int[] {54, 76};
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataIntVector3("categoryInt", "IntV3", vec2i));
    }

    @Test
    public void testSizeCheckIntV4() {
        int[] vec2i = new int[] {54, 76};
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataIntVector4("categoryInt", "IntV3", vec2i));
    }

    @Test
    public void testSizeCheckFloatV2() {
        float[] vec3f = new float[] {54f, 76f, 99f};
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataFloatVector2("categoryFloat", "FloatV2", vec3f));
    }

    @Test
    public void testSizeCheckFloatV3() {
        float[] vec2f = new float[] {54f, 76f};
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataFloatVector3("categoryFloat", "FloatV3", vec2f));
    }

    @Test
    public void testSizeCheckFloatV4() {
        float[] vec5f = new float[] {54f, 76f, 55f, 50f, 88f};
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataFloatVector4("categoryFloat", "FloatV4", vec5f));
    }

    @Test
    public void testSizeCheckDoubleV2() {
        double[] vec3d = new double[] {54d, 76d, 99d};
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataDoubleVector2("categoryDouble", "DoubleV2", vec3d));
    }

    @Test
    public void testSizeCheckDoubleV3() {
        double[] vec2f = new double[] {54d, 76d};
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataDoubleVector3("categoryDouble", "DoubleV3", vec2f));
    }

    @Test
    public void testSizeCheckDoubleV4() {
        double[] vec5d = new double[] {54d, 76d, 55d, 50d, 88d};
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataDoubleVector4("categoryDouble", "DoubleV4", vec5d));
    }

    @Test
    public void testNullMetadataString() {
        MetadataContainer metaData = new MetadataContainer();
        assertThrows(IllegalArgumentException.class, () -> metaData.setMetadataString("categoryString", "nullMetaData", null));
    }

//    @Test
    public void testCreateVDSWithMetaData() {
        try {
            int i1 = 1337;
            int[] vec2i = new int[] {54, 76};
            int[] vec3i = new int[] {33, 66, 99};
            int[] vec4i = new int[] {11, 22, 44, 88};

            float f1 = 13.37f;
            float[] vec2f = new float[] {54.4f, 76.5f};
            float[] vec3f = new float[] {33.3f, 66.6f, 99.9f};
            float[] vec4f = new float[] {11.1f, 22.2f, 44.4f, 88.8f};

            double d1 = 26.74d;
            double[] vec2d = new double[] {54.45d, 76.67d};
            double[] vec3d = new double[] {33.33d, 666.666d, 999.999d};
            double[] vec4d = new double[] {11.11d, 22.222d, 44.4444d, 88.88888d};

            String mdString = "Char sequence metadata";

            MetadataContainer metaData = new MetadataContainer();
            metaData.setMetadataInt("categoryInt", "singleInt", i1);
            metaData.setMetadataIntVector2("categoryInt", "IntV2", vec2i);
            metaData.setMetadataIntVector3("categoryInt", "IntV3", vec3i);
            metaData.setMetadataIntVector4("categoryInt", "IntV4", vec4i);

            metaData.setMetadataFloat("categoryFloat", "singleFloat", f1);
            metaData.setMetadataFloatVector2("categoryFloat", "FloatV2", vec2f);
            metaData.setMetadataFloatVector3("categoryFloat", "FloatV3", vec3f);
            metaData.setMetadataFloatVector4("categoryFloat", "FloatV4", vec4f);

            metaData.setMetadataDouble("categoryDouble", "singleDouble", d1);
            metaData.setMetadataDoubleVector2("categoryDouble", "DoubleV2", vec2d);
            metaData.setMetadataDoubleVector3("categoryDouble", "DoubleV3", vec3d);
            metaData.setMetadataDoubleVector4("categoryDouble", "DoubleV4", vec4d);

            metaData.setMetadataString("categoryString", "String", mdString);

            // create file in tmp dir
            String tempDir = System.getProperty("java.io.tmpdir");
            String tempFilePath = tempDir + File.separator + TEMP_FILE_NAME;

            VDSFileOpenOptions options = new VDSFileOpenOptions(tempFilePath);
            VDSError vdsError = new VDSError();
            VDS createdVDS = OpenVDS.create(options, ld, vda, vdc, metaData, vdsError);

            // test meta data
            // int
            int mdInt = createdVDS.getLayout().getMetadataInt("categoryInt", "singleInt");
            assertEquals(mdInt, i1);

            int[] mdIntVector2 = createdVDS.getLayout().getMetadataIntVector2("categoryInt", "IntV2").toArray();
            assertEquals(vec2i, mdIntVector2);

            int[] mdIntVector3 = createdVDS.getLayout().getMetadataIntVector3("categoryInt", "IntV3").toArray();
            assertEquals(vec3i, mdIntVector3);

            int[] mdIntVector4 = createdVDS.getLayout().getMetadataIntVector4("categoryInt", "IntV4").toArray();
            assertEquals(vec4i, mdIntVector4);

            // float
            float mdFloat = createdVDS.getLayout().getMetadataFloat("categoryFloat", "singleFloat");
            assertEquals(mdFloat, f1);

            float[] mdFloatVector2 = createdVDS.getLayout().getMetadataFloatVector2("categoryFloat", "FloatV2").toArray();
            assertEquals(vec2f, mdFloatVector2);

            float[] mdFloatVector3 = createdVDS.getLayout().getMetadataFloatVector3("categoryFloat", "FloatV3").toArray();
            assertEquals(vec3f, mdFloatVector3);

            float[] mdFloatVector4 = createdVDS.getLayout().getMetadataFloatVector4("categoryFloat", "FloatV4").toArray();
            assertEquals(vec4f, mdFloatVector4);

            // double
            double mdDouble = createdVDS.getLayout().getMetadataDouble("categoryDouble", "singleDouble");
            assertEquals(mdDouble, d1);

            double[] mdDoubleVector2 = createdVDS.getLayout().getMetadataDoubleVector2("categoryDouble", "DoubleV2").toArray();
            assertEquals(vec2d, mdDoubleVector2);

            double[] mdDoubleVector3 = createdVDS.getLayout().getMetadataDoubleVector3("categoryDouble", "DoubleV3").toArray();
            assertEquals(vec3d, mdDoubleVector3);

            double[] mdDoubleVector4 = createdVDS.getLayout().getMetadataDoubleVector4("categoryDouble", "DoubleV4").toArray();
            assertEquals(vec4d, mdDoubleVector4);

            // String
            String mdS = createdVDS.getLayout().getMetadataString("categoryString", "String");
            assertEquals(mdS, mdString);
        } catch (Exception e) {
            System.out.println(e.getMessage());
            fail();
        }
    }

    @Test
    public void testMetaDataBlob() {
        MetadataContainer metaData = new MetadataContainer();

        byte[] blobArray = new byte[3200];
        Random rand = new Random(System.currentTimeMillis());
        rand.nextBytes(blobArray);

        metaData.setMetadataBLOB("Blob data", "blob array", blobArray);

        assertTrue(metaData.isMetadataBLOBAvailable("Blob data", "blob array"));
        byte[] metadataBLOBArray = metaData.getMetadataBLOB("Blob data", "blob array");
        assertArrayEquals(blobArray, metadataBLOBArray);
    }

    @Test
    public void testMetaDataBlobBuffer() {
        MetadataContainer metaData = new MetadataContainer();

        byte[] blobArray = new byte[3200];
        Random rand = new Random(System.currentTimeMillis());
        rand.nextBytes(blobArray);

        ByteBuffer blobBuffer = ByteBuffer.wrap(blobArray);
        metaData.setMetadataBLOB("Blob data", "blob array", blobBuffer);

        assertTrue(metaData.isMetadataBLOBAvailable("Blob data", "blob array"));
        ByteBuffer metadataBLOBBuffer = metaData.getMetadataBLOBAsBuffer("Blob data", "blob array");

        // compares buffer
        assertTrue(metadataBLOBBuffer.equals(blobBuffer));
    }

    @Test
    public void testMetaDataBlobFloatBuffer() {
        MetadataContainer metaData = new MetadataContainer();

        // put floats in a byte buffer
        float[] blobFloatArray = new float[2000];
        Random rand = new Random(System.currentTimeMillis());
        for (int i = 0 ; i < blobFloatArray.length ; ++i) {
            blobFloatArray[i] = rand.nextFloat();
        }
        ByteBuffer byteBuffer = ByteBuffer.allocate(Float.BYTES * blobFloatArray.length);
        byteBuffer.asFloatBuffer().put(blobFloatArray);

        // writes them
        metaData.setMetadataBLOB("Blob data", "blob array", byteBuffer);

        // reread and compare
        assertTrue(metaData.isMetadataBLOBAvailable("Blob data", "blob array"));
        ByteBuffer metadataBLOBBuffer = metaData.getMetadataBLOBAsBuffer("Blob data", "blob array");
        FloatBuffer metaDataFB = metadataBLOBBuffer.asFloatBuffer();
        int size = metaDataFB.remaining();
        float[] readFloats = new float[size];
        metaDataFB.get(readFloats);

        // compares buffer
        assertArrayEquals(blobFloatArray, readFloats, 0.0f);
    }
}


