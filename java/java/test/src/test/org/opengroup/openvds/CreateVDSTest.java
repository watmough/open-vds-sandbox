/*
 * Copyright 2020 The Open Group
 * Copyright 2020 INT, Inc.
 * Copyright 2020 Bluware, Inc.
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

import java.util.ArrayList;
import java.util.EnumSet;

import static org.testng.Assert.*;
import org.testng.annotations.*;

import static org.opengroup.openvds.VolumeDataFormat.*;
import static org.opengroup.openvds.VolumeDataComponents.*;
import static org.opengroup.openvds.VolumeDataLayoutDescriptor.BrickSize;
import static org.opengroup.openvds.VolumeDataLayoutDescriptor.BrickSize.*;
import static org.opengroup.openvds.VolumeDataLayoutDescriptor.LODLevels;
import static org.opengroup.openvds.VolumeDataLayoutDescriptor.LODLevels.*;

public class CreateVDSTest {

    public static class FloatScaleAndOffset {
        public float scale = 1.0f;
        public float offset = 0.0f;

        public FloatScaleAndOffset() {
        }

        public FloatScaleAndOffset(float scale, float offset) {
            this.scale = scale;
            this.offset = offset;
        }
    }

    public static FloatScaleAndOffset getScaleOffsetForFormat(float min, float max, boolean usingNovalue, VolumeDataFormat format)
    {
        float novalue = usingNovalue ? 1.0f : 0.0f;
        float scale = 1.0f;
        float offset = 0.0f;
        switch (format) {
            case Format_U8:
            scale = 1.f / (255.f - novalue) * (max - min);
                offset = min;
                break;
            case Format_U16:
            scale = 1.f/(65535.f - novalue) * (max - min);
                offset = min;
                break;
            case Format_R32:
            case Format_U32:
            case Format_R64:
            case Format_U64:
            case Format_1Bit:
            case Format_Any:
                scale = 1.0f;
                offset = 0.0f;
        }
        return new FloatScaleAndOffset(scale, offset);
    }

    public static VolumeDataChannelDescriptor[] createDefaultChannelDescriptors(String[] channelNames, VolumeDataFormat format) {
        ArrayList<VolumeDataChannelDescriptor> channelDescriptors = new ArrayList<>();
        for (String channel: channelNames) {
            float rangeMin = -0.1234f;
            float rangeMax = 0.1234f;
            EnumSet<VolumeDataChannelDescriptor.Flags> channelFlags = EnumSet.noneOf(VolumeDataChannelDescriptor.Flags.class);
            FloatScaleAndOffset scaleAndOffset = getScaleOffsetForFormat(rangeMin, rangeMax, true, format);
            channelDescriptors.add(new VolumeDataChannelDescriptor(format, Components_1,  channel, "", rangeMin, rangeMax, VolumeDataMapping.Direct, 1, channelFlags, 0.f, scaleAndOffset.scale, scaleAndOffset.offset));
        }
        return channelDescriptors.toArray(new VolumeDataChannelDescriptor[channelDescriptors.size()]);
    }

    public static VolumeDataChannelDescriptor[] createDefaultChannelDescriptors(String channelName, VolumeDataFormat format) {
        return createDefaultChannelDescriptors(new String[] { channelName }, format);
    }

    public static VDS createVDS(int samplesX, int samplesY, int samplesZ, VolumeDataFormat format, OpenOptions options, VolumeDataChannelDescriptor[] channelDescriptors,  VDSError error) {
        if (options == null) {
            options = new InMemoryOpenOptions();
        }
        if (error == null) {
            error = new VDSError();
        }
        if (channelDescriptors == null) {
            channelDescriptors = createDefaultChannelDescriptors("Amplitude", Format_R32);
        }
        LODLevels lodLevels = LODLevels_None;
        BrickSize brickSize = (samplesZ == 0) ? BrickSize_1024 : BrickSize_128;
        EnumSet<VolumeDataLayoutDescriptor.Options> layoutOptions = EnumSet.noneOf(VolumeDataLayoutDescriptor.Options.class);
        int negativeMargin = 4;
        int positiveMargin = 4;
        int brickSize2DMultiplier = 4;

        VolumeDataLayoutDescriptor layoutDescriptor = new VolumeDataLayoutDescriptor(brickSize, negativeMargin, positiveMargin, brickSize2DMultiplier, lodLevels, layoutOptions, 0);

        ArrayList<VolumeDataAxisDescriptor> axisDescriptors = new ArrayList<>();
        axisDescriptors.add(new VolumeDataAxisDescriptor(samplesX, KnownAxisNames.sample(), "ms", 0.0f, 4.f));
        axisDescriptors.add(new VolumeDataAxisDescriptor(samplesY, KnownAxisNames.crossline(), "", 1932.f, 2536.f));
        if (samplesZ != 0) {
            axisDescriptors.add(new VolumeDataAxisDescriptor(samplesZ, KnownAxisNames.inline(), "", 9985.f, 10369.f));
        }

        MetadataContainer metadataContainer = new MetadataContainer();
        metadataContainer.setMetadataInt( "categoryInt", "Int", 123 );
        metadataContainer.setMetadataIntVector2( "categoryInt", "IntVector2", new IntVector2( 45, 78 ) );
        metadataContainer.setMetadataIntVector3( "categoryInt", "IntVector3", new IntVector3( 45, 78 , 72) );
        metadataContainer.setMetadataIntVector4( "categoryInt", "IntVector4", new IntVector4( 45, 78 , 72,84 ));
        metadataContainer.setMetadataFloat( "categoryFloat", "Float", 123.f );
        metadataContainer.setMetadataFloatVector2( "categoryFloat", "FloatVector2", new FloatVector2( 45.5f, 78.75f ) );
        metadataContainer.setMetadataFloatVector3( "categoryFloat", "FloatVector3", new FloatVector3( 45.5f, 78.75f , 72.75f) );
        metadataContainer.setMetadataFloatVector4( "categoryFloat", "FloatVector4", new FloatVector4( 45.5f, 78.75f , 72.75f,84.1f) );
        metadataContainer.setMetadataDouble( "categoryDouble", "Double", 123.);
        metadataContainer.setMetadataDoubleVector2( "categoryDouble", "DoubleVector2", new DoubleVector2( 45.5, 78.75 ) );
        metadataContainer.setMetadataDoubleVector3( "categoryDouble", "DoubleVector3", new DoubleVector3( 45.5, 78.75 , 72.75) );
        metadataContainer.setMetadataDoubleVector4( "categoryDouble", "DoubleVector4", new DoubleVector4( 45.5, 78.75 , 72.75,84.1) );
        metadataContainer.setMetadataString( "categoryString", "String", "Test string" );
        byte[] data = new byte[] { 1,2,3,4 };
        metadataContainer.setMetadataBLOB("categoryBLOB", "BLOB", data);

        return OpenVDS.create(options, layoutDescriptor, axisDescriptors.toArray(new VolumeDataAxisDescriptor[axisDescriptors.size()]), channelDescriptors, metadataContainer, error);
    }

    public static VDS createVDS(int samplesX, int samplesY, VolumeDataFormat format, OpenOptions options, VolumeDataChannelDescriptor[] channelDescriptors, VDSError error) {
        return createVDS(samplesX, samplesY, 0, format, options, channelDescriptors, error);
    }

    public static VDS createVDS(int samplesX, int samplesY, VolumeDataFormat format, OpenOptions options, VDSError error) {
        return createVDS(samplesX, samplesY, 0, format, options, createDefaultChannelDescriptors("Amplitude", format), error);
    }

    @BeforeClass
    public void init() {
        vds = new InMemoryVDSGenerator(16, 16, 16, Format_U8);
        url = "inmemory://create_test";
        o = new AzureOpenOptions();
        error = new VDSError();
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


    @Test
    public void testCreateVDS() {
        VDS openvds1 = OpenVDS.create(url, "", ld, vda, vdc, md, error);
        openvds1.getAccessManager().flush(error);
        assertEquals(error.getCode(), 0);

        VDS openvds2 = OpenVDS.open(url, "", error);

        VolumeDataLayout layout = openvds2.getLayout();

        assertEquals(layout.getDimensionality(), 3);
        assertEquals(layout.getChannelCount(), 1);
        assertEquals(layout.getChannelFormat(0), Format_U8);
        assertEquals(layout.getDimensionName(1), openvds1.getLayout().getDimensionName(1));
    }


    @Test(expectedExceptions = IllegalArgumentException.class)
    public void testException1() {
        try {
            AWSOpenOptions o = null;
            OpenVDS.create(o, ld, vda, vdc, md);
        } catch (java.io.IOException e) {
            fail();
        }
    }


    @Test(expectedExceptions = IllegalArgumentException.class)
     public void testException2() {
        try {
            OpenVDS.create(o, null, vda, vdc, md);
        } catch (java.io.IOException e) {
            fail();
        }
    }


    @Test(expectedExceptions = IllegalArgumentException.class)
    public void testException3() {
        try {
            OpenVDS.create(o, ld, null, vdc, md);
        } catch (java.io.IOException e) {
            fail();
        }
    }


    @Test(expectedExceptions = IllegalArgumentException.class)
    public void testException4() {
        try {
            OpenVDS.create(o, ld, vda, null, md);
        } catch (java.io.IOException e) {
            fail();
        }
    }


    @Test(expectedExceptions = IllegalArgumentException.class)
    public void testException5() {
        try {
            OpenVDS.create(o, ld, vda, vdc, null);
        } catch (java.io.IOException e) {
            fail();
        }
    }

    public AzureOpenOptions o;
    public String url;
    public VolumeDataLayoutDescriptor ld;
    public VolumeDataAxisDescriptor[] vda;
    public VolumeDataChannelDescriptor[] vdc;
    public MetadataReadAccess md;
    public InMemoryVDSGenerator vds;
    public VDSError error;
}
