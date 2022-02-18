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
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import static org.junit.Assert.*;
import static org.junit.Assume.assumeTrue;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;
import static org.opengroup.openvds.VolumeDataChannelDescriptor.Format.*;
import static org.opengroup.openvds.VolumeDataLayoutDescriptor.BrickSize.BrickSize_32;
import static org.opengroup.openvds.VolumeDataLayoutDescriptor.LODLevels.LODLevels_None;
import static org.opengroup.openvds.VolumeDataChannelDescriptor.Components.*;
public class InMemoryVDSGeneratorTest {

    GlobalState globalState;

    @BeforeClass
    public static void setUpClass() {
    }

    @AfterClass
    public static void tearDownClass() {

    }

    @Before
    public void setUp() {
        this.globalState = OpenVDS.getGlobalState();
    }

    @After
    public void tearDown() {
    }
    
    public InMemoryVDSGeneratorTest() {
    }

    /**
     * Undocumented test
     */
    @org.junit.Test
    public void testLayout() {
        int nXSamples = 60, nYSamples = 60, nZSamples = 60;
        VolumeDataChannelDescriptor.Format format = Format_U8;
        InMemoryVDSGenerator generator = new InMemoryVDSGenerator(nXSamples, nYSamples, nZSamples, format);
        final VolumeDataLayout layout = generator.getLayout();
        assertNotNull(layout);

        int nbChannel = layout.getChannelCount();
        VolumeDataAccessManager accessManager = generator.getAccessManager();
        for (VolumeDataLayoutDescriptor.LODLevels l : VolumeDataLayoutDescriptor.LODLevels.values()) {
            for (int channel = 0; channel < nbChannel; channel++) {
                for (DimensionsND dimGroup : DimensionsND.values()) {
                    VDSProduceStatus vdsProduceStatus = accessManager.getVDSProduceStatus(dimGroup, l.ordinal(), channel);
                    if (channel == 0 && LODLevels_None.equals(l) && DimensionsND.Dimensions_012.equals(dimGroup))
                        assertEquals(VDSProduceStatus.Normal, vdsProduceStatus);
                    else
                        assertEquals(VDSProduceStatus.Unavailable, vdsProduceStatus);
                }
            }
        }
        assertEquals(3, layout.getDimensionality());

        int dimensionIndex = 0;
        assertEquals(60, layout.getDimensionNumSamples(dimensionIndex));
        assertEquals("Sample", layout.getDimensionName(dimensionIndex));
        assertEquals("ms", layout.getDimensionUnit(dimensionIndex));
        assertEquals(0f, layout.getDimensionMin(dimensionIndex), 0f);
        assertEquals(4f, layout.getDimensionMax(dimensionIndex), 0f);

        dimensionIndex = 1;
        assertEquals(60, layout.getDimensionNumSamples(dimensionIndex));
        assertEquals("Crossline", layout.getDimensionName(dimensionIndex));
        assertTrue(layout.getDimensionUnit(dimensionIndex).isEmpty());
        assertEquals(1932.f, layout.getDimensionMin(dimensionIndex), 0f);
        assertEquals(2536.f, layout.getDimensionMax(dimensionIndex), 0f);

        dimensionIndex = 2;
        assertEquals(60, layout.getDimensionNumSamples(dimensionIndex));
        assertEquals("Inline", layout.getDimensionName(dimensionIndex));
        assertTrue(layout.getDimensionUnit(dimensionIndex).isEmpty());
        assertEquals(9985.f, layout.getDimensionMin(dimensionIndex), 0f);
        assertEquals(10369.f, layout.getDimensionMax(dimensionIndex), 0f);

        final VolumeDataLayoutDescriptor descriptor = layout.getLayoutDescriptor();
        assertTrue(descriptor.isValid());
        assertEquals(BrickSize_32, descriptor.getBrickSize());
        assertEquals(4, descriptor.getNegativeMargin());
        assertEquals(4, descriptor.getPositiveMargin());
        assertEquals(4, descriptor.getBrickSizeMultiplier2D());
        assertEquals(LODLevels_None, descriptor.getLODLevels());
        assertTrue(!descriptor.isCreate2DLODs());
        assertTrue(!descriptor.isForceFullResolutionDimension());
        assertEquals(-1, descriptor.getFullResolutionDimension());

        assertEquals(1, layout.getChannelCount());

        int channelIndex = 0;
        String channelName = layout.getChannelName(channelIndex);
        assertEquals("Amplitude", channelName);
        assertTrue(layout.getChannelUnit(channelIndex).isEmpty());
        assertTrue(layout.isChannelAvailable(channelName));
        assertEquals(0, layout.getChannelIndex(channelName));
        assertEquals(Format_U8, layout.getChannelFormat(channelIndex));
        assertEquals(Components_1, layout.getChannelComponents(channelIndex));
        assertEquals(-0.1234f, layout.getChannelValueRangeMin(channelIndex), 0f);
        assertEquals(0.1234f, layout.getChannelValueRangeMax(channelIndex), 0f);
        assertTrue(!layout.isChannelDiscrete(channelIndex));
        assertTrue(layout.isChannelRenderable(channelIndex));
        assertTrue(layout.isChannelAllowingLossyCompression(channelIndex));
        assertTrue(!layout.isChannelUseZipForLosslessCompression(channelIndex));
        assertEquals(VolumeDataMapping.Direct, layout.getChannelMapping(channelIndex));
        assertTrue(layout.isChannelUseNoValue(channelIndex));
        assertEquals(0f, layout.getChannelNoValue(channelIndex), 0f);
        assertEquals(9.7165356E-4f, layout.getChannelIntegerScale(channelIndex), 0f);
        assertEquals(-0.1234f, layout.getChannelIntegerOffset(channelIndex), 0f);

        assertEquals(1, layout.getChannelDescriptor(0).getMappedValueCount());

        final MetadataKey[] metadataKeys = layout.getMetadataKeys();
        assertEquals(14, metadataKeys.length);

        assertTrue(layout.isMetadataIntAvailable("categoryInt", "Int"));
        assertEquals(123, layout.getMetadataInt("categoryInt", "Int"));
        assertTrue(layout.isMetadataIntVector2Available("categoryInt", "IntVector2"));
        assertArrayEquals(new int[]{45, 78}, layout.getMetadataIntVector2("categoryInt", "IntVector2").toArray());
        assertTrue(layout.isMetadataIntVector3Available("categoryInt", "IntVector3"));
        assertArrayEquals(new int[]{45, 78, 72}, layout.getMetadataIntVector3("categoryInt", "IntVector3").toArray());
        assertTrue(layout.isMetadataIntVector4Available("categoryInt", "IntVector4"));
        assertArrayEquals(new int[]{45, 78, 72, 84}, layout.getMetadataIntVector4("categoryInt", "IntVector4").toArray());

        assertTrue(layout.isMetadataFloatAvailable("categoryFloat", "Float"));
        assertEquals(123f, layout.getMetadataFloat("categoryFloat", "Float"), 0f);
        assertTrue(layout.isMetadataFloatVector2Available("categoryFloat", "FloatVector2"));
        assertArrayEquals(new float[]{45.5f, 78.75f}, layout.getMetadataFloatVector2("categoryFloat", "FloatVector2").toArray(), 0f);
        assertTrue(layout.isMetadataFloatVector3Available("categoryFloat", "FloatVector3"));
        assertArrayEquals(new float[]{45.5f, 78.75f, 72.75f}, layout.getMetadataFloatVector3("categoryFloat", "FloatVector3").toArray(), 0f);
        assertTrue(layout.isMetadataFloatVector4Available("categoryFloat", "FloatVector4"));
        assertArrayEquals(new float[]{45.5f, 78.75f, 72.75f, 84.1f}, layout.getMetadataFloatVector4("categoryFloat", "FloatVector4").toArray(), 0f);

        assertTrue(layout.isMetadataDoubleAvailable("categoryDouble", "Double"));
        assertEquals(123., layout.getMetadataDouble("categoryDouble", "Double"), 0f);
        assertTrue(layout.isMetadataDoubleVector2Available("categoryDouble", "DoubleVector2"));
        assertArrayEquals(new double[]{45.5, 78.75}, layout.getMetadataDoubleVector2("categoryDouble", "DoubleVector2").toArray(), 0f);
        assertTrue(layout.isMetadataDoubleVector3Available("categoryDouble", "DoubleVector3"));
        assertArrayEquals(new double[]{45.5, 78.75, 72.75}, layout.getMetadataDoubleVector3("categoryDouble", "DoubleVector3").toArray(), 0f);
        assertTrue(layout.isMetadataDoubleVector4Available("categoryDouble", "DoubleVector4"));
        assertArrayEquals(new double[]{45.5, 78.75, 72.75, 84.1}, layout.getMetadataDoubleVector4("categoryDouble", "DoubleVector4").toArray(), 0f);

        assertTrue(layout.isMetadataStringAvailable("categoryString", "String"));
        assertEquals("Test string", layout.getMetadataString("categoryString", "String"));

        assertTrue(layout.isMetadataBLOBAvailable("categoryBLOB", "BLOB"));
        assertArrayEquals(new byte[]{1,2,3,4}, layout.getMetadataBLOB("categoryBLOB", "BLOB"));

        generator.close();

    }

    @org.junit.Test
    public void testErrors() {
        try (InMemoryVDSGenerator generator = new InMemoryVDSGenerator(100, 100, 100, Format_U8)) {
            UploadError ul = generator.getAccessManager().getCurrentUploadError();
            assertTrue("".equals(ul.ObjectID));
            assertTrue("".equals(ul.ErrorString));
            assertTrue(ul.ErrorCode == 0);

            DownloadError dl = generator.getAccessManager().getCurrentDownloadError();
            assertTrue("".equals(dl.ErrorString));
            assertTrue(dl.ErrorCode == 0);

        }

    }
    
}
