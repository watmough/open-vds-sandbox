/*
 * Copyright 2019 The Open Group
 * Copyright 2019 INT, Inc.
 * Copyright 2022 Bluware, Inc.
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

import java.io.IOException;

import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.nio.IntBuffer;
import java.nio.ShortBuffer;
import java.util.LinkedList;

import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.stream.IntStream;

import static org.testng.Assert.*;
import org.testng.annotations.*;

import org.opengroup.openvds.*;

import static org.opengroup.openvds.VolumeDataFormat.*;
import static org.opengroup.openvds.VolumeDataLayoutDescriptor.BrickSize;
import static org.opengroup.openvds.VolumeDataLayoutDescriptor.BrickSize.*;
import static org.opengroup.openvds.VolumeDataLayoutDescriptor.LODLevels;
import static org.opengroup.openvds.VolumeDataLayoutDescriptor.LODLevels.*;
import static org.opengroup.openvds.VolumeDataComponents.*;
import static org.opengroup.openvds.DimensionsND.*;

public class InMemoryVDSGeneratorTest {

    @Test
    public void testOpenClose() throws IOException {
        int nXSamples = 60, nYSamples = 60, nZSamples = 60;
        VolumeDataFormat format = Format_U8;
        InMemoryVDSGenerator generator = new InMemoryVDSGenerator(nXSamples, nYSamples, nZSamples, format);
        assertTrue(!generator.isNull());

        assertTrue(!generator.getLayout().isNull());
        assertTrue(!generator.getAccessManager().isNull());

        generator.close();
        assertTrue(generator.isNull());
    }

    @Test
    public void testVolumeTracesVsSampleRequest() throws IOException {
        int nXSamples = 60, nYSamples = 60, nZSamples = 60;
        VolumeDataFormat format = Format_R32;
        try (InMemoryVDSGenerator generator = new InMemoryVDSGenerator(nZSamples, nYSamples, nXSamples, format)) {
            assertNotNull(generator);
            try (VolumeDataAccessManager accessManager = generator.getAccessManager()) {
                assertNotNull(accessManager);
                try (ManagedBuffer b1 = new ManagedBuffer(accessManager.getVolumeTracesBufferSize(nYSamples, 0));
                     ManagedBuffer b0 = new ManagedBuffer(accessManager.getVolumeSamplesBufferSize(nZSamples * nYSamples))
                ) {
                    assertTrue(b1.getByteBuffer().order() == ByteOrder.nativeOrder());
                    assertTrue(b0.getByteBuffer().order() == ByteOrder.nativeOrder());
                    NDPosArray tracePositions = new NDPosArray(nYSamples);
                    NDPosArray samplePositions = new NDPosArray(nZSamples * nYSamples);
                    for (int j = 0; j < nYSamples; j++) {
                        tracePositions.set(j, 0.5f, j + 0.5f, 0.5f, 0.5f, 0.5f, 0.5f);
                        for (int k = 0; k < nZSamples; k++) {
                            samplePositions.set(j * nYSamples + k, k + 0.5f, j + 0.5f, 0.5f, 0.5f, 0.5f, 0.5f);
                        }
                    }
                    try (VolumeDataRequestFloat r_traces = accessManager.requestVolumeTraces(b1.getByteBuffer(), Dimensions_012, 0, 0, tracePositions, InterpolationMethod.Nearest, 0);
                         VolumeDataRequestFloat r_samples = accessManager.requestVolumeSamples(b0.getByteBuffer(), Dimensions_012, 0, 0, samplePositions, InterpolationMethod.Nearest)
                    ) {
                        r_traces.waitForCompletion();
                        r_samples.waitForCompletion();
                        FloatBuffer floatBuffer0 = b0.asFloatBuffer();
                        FloatBuffer floatBuffer1 = b1.asFloatBuffer();
                        for (int i = 0; i < nZSamples * nYSamples; i++) {
                            final float f1 = floatBuffer0.get(i);
                            final float f2 = floatBuffer1.get(i);
                            assertEquals(0, Float.compare(f1, f2), " value " + i + " is different");
                        }
                    }
                }
            }
        }
    }

    @Test
    public void testVolumeTraces() throws IOException {
        int nXSamples = 60, nYSamples = 60, nZSamples = 60;
        VolumeDataFormat format = Format_R32;
        try (InMemoryVDSGenerator generator = new InMemoryVDSGenerator(nZSamples, nYSamples, nXSamples, format)) {
            assertNotNull(generator);
            try (VolumeDataAccessManager accessManager = generator.getAccessManager()) {
                assertNotNull(accessManager);
                try (ManagedBuffer b1 = new ManagedBuffer(accessManager.getVolumeTracesBufferSize(nYSamples, 0));
                     ManagedBuffer b0 = new ManagedBuffer(accessManager.getVolumeSamplesBufferSize(nZSamples * nYSamples))
                ) {
                    final VolumeDataLayout layout = generator.getLayout();
                    assertTrue(!layout.isNull());

                    NDPosArray tracePositions = new NDPosArray(nYSamples);
                    for (int j = 0; j < nYSamples; j++) {
                        tracePositions.set(j, 0.5f, j + 0.5f, 0.5f, 0.5f, 0.5f, 0.5f);
                    }
                    try (VolumeDataRequestFloat r_traces1 = accessManager.requestVolumeTraces(b1.getByteBuffer(), Dimensions_012, 0, 0, tracePositions, InterpolationMethod.Nearest, 0);
                         VolumeDataRequestFloat r_traces0 = accessManager.requestVolumeTraces(b0.getByteBuffer(), Dimensions_012, 0, 0, tracePositions, InterpolationMethod.Nearest, 0, layout.getChannelNoValue(0));
                    ) {
                        r_traces1.waitForCompletion();
                        r_traces0.waitForCompletion();
                        FloatBuffer floatBuffer0 = b0.asFloatBuffer();
                        FloatBuffer floatBuffer1 = b1.asFloatBuffer();
                        for (int i = 0; i < nZSamples * nYSamples; i++) {
                            final float f1 = floatBuffer0.get(i);
                            final float f2 = floatBuffer1.get(i);
                            assertEquals(0, Float.compare(f1, f2), " value " + i + " is different");
                        }
                    }
                }
            }
        }
    }

    /*
    @Test
    void testVolumeSubsetBufferVsToFloat() throws IOException {
        int nXSamples = 60, nYSamples = 60, nZSamples = 60;
        VolumeDataChannelDescriptor.Format format = FORMAT_R32;
        MemoryVdsGenerator generator = new MemoryVdsGenerator(nZSamples, nYSamples, nXSamples, format);
        assertTrue(!generator.isNull());
        assertTrue(generator.ownHandle());

        final VolumeDataLayout layout = generator.getLayout();
        assertTrue(!layout.isNull());

        final VolumeDataAccessManager accessManager = generator.getAccessManager();
        assertTrue(!accessManager.isNull());

        NDBox box = new NDBox(0, 0, 0, 0, 0, 0, nZSamples, nYSamples, 1, 0, 0, 0);

        final FloatBuffer floatBuffer1 = B.createFloatBuffer(nZSamples * nYSamples);
        final long requestSubsetId1 = accessManager.requestVolumeSubset(floatBuffer1, DimENSIONS_012, 0, 0, box);
        accessManager.waitForCompletion(requestSubsetId1);

        final FloatBuffer floatBuffer0 = B.toBuffer(new float[nZSamples * nYSamples]);
        final long requestSubsetId0 = accessManager.requestVolumeSubset(floatBuffer0, DimENSIONS_012, 0, 0, box);
        accessManager.waitForCompletion(requestSubsetId0);

        for (int i = 0; i < nZSamples * nYSamples; i++) {
            final float f1 = floatBuffer0.get(i);
            final float f2 = floatBuffer1.get(i);
            assertEquals(0, Float.compare(f1, f2), " value " + i + " is different");
        }

        B.release(floatBuffer0, floatBuffer1);

        generator.close();
        assertTrue(generator.isNull());
    }
*/

    /*
        @Test
    void testVolumeSubsetBufferToArrayAndRelease() throws IOException {
        int nXSamples = 60, nYSamples = 60, nZSamples = 60;
        VolumeDataChannelDescriptor.Format format = FORMAT_R32;
        MemoryVdsGenerator generator = new MemoryVdsGenerator(nZSamples, nYSamples, nXSamples, format);
        assertTrue(!generator.isNull());
        assertTrue(generator.ownHandle());

        final VolumeDataLayout layout = generator.getLayout();
        assertTrue(!layout.isNull());

        final VolumeDataAccessManager accessManager = generator.getAccessManager();
        assertTrue(!accessManager.isNull());

        NDBox box = new NDBox(0, 0, 0, 0, 0, 0, nZSamples, nYSamples, 1, 0, 0, 0);

        final FloatBuffer floatBuffer1 = B.createFloatBuffer(nZSamples * nYSamples);
        final long requestSubsetId1 = accessManager.requestVolumeSubset(floatBuffer1, DimENSIONS_012, 0, 0, box);
        accessManager.waitForCompletion(requestSubsetId1);

        final FloatBuffer floatBuffer0 = B.toBuffer(new float[nZSamples * nYSamples]);
        final long requestSubsetId0 = accessManager.requestVolumeSubset(floatBuffer0, DimENSIONS_012, 0, 0, box);
        accessManager.waitForCompletion(requestSubsetId0);
        float[] floatArray = B.toArrayAndRelease(floatBuffer0);

        for (int i = 0; i < nZSamples * nYSamples; i++) {
            final float f1 = floatArray[i];
            final float f2 = floatBuffer1.get(i);
            assertEquals(0, Float.compare(f1, f2), " value " + i + " is different");
        }

        B.release(floatBuffer0, floatBuffer1);

        generator.close();
        assertTrue(generator.isNull());
    }

     */

    @Test
    public void testVolumeSubsetVsSampleRequest() throws IOException {
        int nXSamples = 60, nYSamples = 60, nZSamples = 60;
        VolumeDataFormat format = Format_R32;
        try (InMemoryVDSGenerator generator = new InMemoryVDSGenerator(nZSamples, nYSamples, nXSamples, format)) {
            assertNotNull(generator);
            try (VolumeDataAccessManager accessManager = generator.getAccessManager()) {
                assertNotNull(accessManager);

                final VolumeDataLayout layout = generator.getLayout();
                assertTrue(!layout.isNull());

                NDPosArray samplePositions = new NDPosArray(nYSamples * nZSamples);
                for (int j = 0; j < nYSamples; j++) {
                    for (int k = 0; k < nZSamples; k++) {
                        samplePositions.set(k + j * nYSamples, k + 0.5f, j + 0.5f, 0.5f, 0.5f, 0.5f, 0.5f);
                    }
                }
                int[] min = new int[]{0, 0, 0, 0, 0, 0};
                int[] max = new int[]{nZSamples, nYSamples, 1, 0, 0, 0};
                try (VolumeDataRequest r_subset = accessManager.requestVolumeSubsetFloat(Dimensions_012, 0, 0, min, max);
                     VolumeDataRequest r_samples = accessManager.requestVolumeSamples(Dimensions_012, 0, 0, samplePositions, InterpolationMethod.Nearest)
                ) {
                    r_subset.waitForCompletion();
                    r_samples.waitForCompletion();

                    FloatBuffer floatBuffer1 = r_subset.getBuffer().asFloatBuffer();
                    FloatBuffer floatBuffer0 = r_samples.getBuffer().asFloatBuffer();
                    for (int i = 0; i < nZSamples * nYSamples; i++) {
                        final float f1 = floatBuffer0.get(i);
                        final float f2 = floatBuffer1.get(i);
                        assertEquals(0, Float.compare(f1, f2), " value " + i + " is different");
                    }
                }
            }
        }
    }

/*
    @Test
    void testVolumeProjectedSubsetRequestFloat() throws IOException {
        int nXSamples = 60, nYSamples = 60, nZSamples = 60;
        VolumeDataChannelDescriptor.Format format = FORMAT_R32;
        MemoryVdsGenerator generator = new MemoryVdsGenerator(nZSamples, nYSamples, nXSamples, format);
        assertTrue(!generator.isNull());
        assertTrue(generator.ownHandle());

        final VolumeDataLayout layout = generator.getLayout();
        assertTrue(!layout.isNull());

        final VolumeDataAccessManager accessManager = generator.getAccessManager();
        assertTrue(!accessManager.isNull());

        NDBox wholeCube = new NDBox(0, 0, 0, 0, 0, 0, nZSamples, nYSamples, nXSamples, 0, 0, 0);

        final FloatBuffer floatBuffer1 = B.createFloatBuffer(nZSamples * nYSamples * nXSamples);
        final long requestSubsetId = accessManager.requestProjectedVolumeSubset(floatBuffer1, DimENSIONS_012, 0, 0,
                wholeCube, 0.5f, 0.1f, 0.9f, 1.f, DimENSIONS_01, InterpolationMethod.NEAREST);
        accessManager.waitForCompletion(requestSubsetId);

        final FloatBuffer floatBuffer0 = B.createFloatBuffer(nZSamples * nYSamples * nXSamples);
        final long requestSubsetId0 = accessManager.requestProjectedVolumeSubset(floatBuffer0, DimENSIONS_012, 0, 0,
                wholeCube, 0.5f, 0.1f, 0.9f, 1.f, DimENSIONS_01, InterpolationMethod.NEAREST, layout.getChannelNoValue(0));
        accessManager.waitForCompletion(requestSubsetId0);

        for (int i = 0; i < nZSamples * nYSamples; i++) {
            final float f1 = floatBuffer0.get(i);
            final float f2 = floatBuffer1.get(i);
            assertEquals(0, Float.compare(f1, f2), " value " + i + " is different");
        }

        B.release(floatBuffer0, floatBuffer1);

        generator.close();
        assertTrue(generator.isNull());
    }

 */

    /*
        @Test
    void testVolumeProjectedSubsetVSSubsetRequestFloat() throws IOException {
        int nXSamples = 60, nYSamples = 60, nZSamples = 60;
        VolumeDataChannelDescriptor.Format format = FORMAT_R32;
        MemoryVdsGenerator generator = new MemoryVdsGenerator(nZSamples, nYSamples, nXSamples, format);
        assertTrue(!generator.isNull());
        assertTrue(generator.ownHandle());

        final VolumeDataLayout layout = generator.getLayout();
        assertTrue(!layout.isNull());

        final VolumeDataAccessManager accessManager = generator.getAccessManager();
        assertTrue(!accessManager.isNull());

        NDBox wholeCube = new NDBox(0, 0, 0, 0, 0, 0, nZSamples, nYSamples, nXSamples, 0, 0, 0);

        final long projectedVolumeSubsetBufferSize = accessManager.getProjectedVolumeSubsetBufferSize(wholeCube, DimENSIONS_01, FORMAT_R32, 0, 0);
        assertEquals(nZSamples * nYSamples * Float.BYTES, projectedVolumeSubsetBufferSize);

        final FloatBuffer floatBuffer1 = B.createFloatBuffer(nZSamples * nYSamples * nXSamples);
        final long requestSubsetId = accessManager.requestProjectedVolumeSubset(floatBuffer1, DimENSIONS_012, 0, 0,
                wholeCube, 0.5f, 0.1f, 0.9f, 1.f, DimENSIONS_01, InterpolationMethod.NEAREST);
        accessManager.waitForCompletion(requestSubsetId);

        NDBox boxSlice = new NDBox(0, 0, 0, 0, 0, 0, nZSamples, nYSamples, 1, 0, 0, 0);
        final FloatBuffer floatBuffer0 = B.createFloatBuffer(nZSamples * nYSamples * nXSamples);
        final float channelNoValue = layout.getChannelNoValue(0);
        final long requestID0 = accessManager.requestVolumeSubset(floatBuffer0, DimENSIONS_012, 0, 0, boxSlice, channelNoValue);
        accessManager.waitForCompletion(requestID0);

        for (int i = 0; i < nZSamples * nYSamples; i++) {
            final float f1 = floatBuffer0.get(i);
            final float f2 = floatBuffer1.get(i);
            assertEquals(0, Float.compare(f1, f2), " value " + i + " is different");
        }

        B.release(floatBuffer0, floatBuffer1);

        generator.close();
        assertTrue(generator.isNull());
    }

     */

    @Test
    public void testVolumeSubsetRequestFloatMultiThread() throws IOException {
        VolumeDataFormat format = Format_R32;
        try (InMemoryVDSGenerator generator = new InMemoryVDSGenerator(60, 60, 60, format)) {
            assertNotNull(generator);
            final VolumeDataLayout layout = generator.getLayout();
            LinkedList<Future> jobs = new LinkedList();
            final int NB_TEST = 10000;
            final int NB_THREAD = 16;

            int nZSamples = layout.getDimensionNumSamples(0);
            int nYSamples = layout.getDimensionNumSamples(1);
            int nXSamples = layout.getDimensionNumSamples(2);

            final ExecutorService executorService = Executors.newFixedThreadPool(NB_THREAD);
            IntStream
                    .range(0, NB_TEST)
                    .mapToObj(iTest -> {
                        int iSlice = iTest % nXSamples;
                        return new int[][] {
                                { 0, 0, iSlice, 0, 0, 0} ,
                                { nZSamples, nYSamples, iSlice + 1, 0, 0, 0}};
                    })
                    .forEach(minmax -> {
                                int[] min = minmax[0];
                                int[] max = minmax[1];
                                if (jobs.size() >= NB_THREAD) {
                                    final Future future = jobs.pollFirst();
                                    try {
                                        future.get();
                                    } catch (InterruptedException | ExecutionException e) {
                                        throw new RuntimeException(e);
                                    }
                                }
                                jobs.addLast(executorService.submit(() -> {
                                    try (VolumeDataAccessManager accessManager = generator.getAccessManager()) {
                                        assertTrue(!accessManager.isNull());
                                        try (VolumeDataRequestFloat r_subset1 = accessManager.requestVolumeSubsetFloat(Dimensions_012, 0, 0, min, max);
                                             VolumeDataRequestFloat r_subset0 = accessManager.requestVolumeSubsetFloat(Dimensions_012, 0, 0, min, max, layout.getChannelNoValue(0))
                                        ) {
                                            waitComplationAndDisplayProgress(r_subset1);
                                            waitComplationAndDisplayProgress(r_subset0);
                                            FloatBuffer floatBuffer1 = r_subset1.getFloatBuffer();
                                            FloatBuffer floatBuffer0 = r_subset0.getFloatBuffer();
                                            for (int i = 0; i < nZSamples * nYSamples; i++) {
                                                final float f1 = floatBuffer0.get(i);
                                                final float f2 = floatBuffer1.get(i);
                                                assertEquals(0, Float.compare(f1, f2), " value " + i + " is different");
                                            }
//                                            System.out.println("Done " + r_subset0.requestID() + " " + r_subset1.requestID());
//                                            System.out.flush();
                                        }
                                    }
                                    return null;
                                }));
                            }
                    );
            jobs.forEach(future -> {
                try {
                    future.get();
                } catch (InterruptedException | ExecutionException e) {
                    throw new RuntimeException(e);
                }
            });
            executorService.shutdown();
        }
    }

    private static void waitComplationAndDisplayProgress(VolumeDataRequest request) {
        while (!request.waitForCompletion(1000)) {
            if (request.isCanceled()) throw new RuntimeException("Cancelled job");

            // Timeout, so let display progress
            final float completionFactor = request.getCompletionFactor();
            System.out.println("Completion " + request + " : " + completionFactor * 100. + " %");
        }
    }

    @Test
    public void testVolumeSubsetRequestFloat() throws IOException {
        int nXSamples = 60, nYSamples = 60, nZSamples = 60;
        VolumeDataFormat format = Format_R32;
        try (InMemoryVDSGenerator generator = new InMemoryVDSGenerator(nZSamples, nYSamples, nXSamples, format)) {
            assertNotNull(generator);
            final VolumeDataLayout layout = generator.getLayout();
            assertTrue(!layout.isNull());
            try (VolumeDataAccessManager accessManager = generator.getAccessManager()) {
                assertNotNull(accessManager);
                int[] min = new int[]{ 0,0,0,0,0,0 };
                int[] max = new int[]{ nZSamples, nYSamples, 1, 0, 0, 0 };
                long bufferSize = accessManager.getVolumeSubsetBufferSize(min, max, format, 0, 0);
                try (ManagedBuffer buffer1 = new ManagedBuffer(bufferSize);
                     ManagedBuffer buffer0 = new ManagedBuffer(bufferSize)
                ) {
                    final float channelNoValue = layout.getChannelNoValue(0);
                    try (VolumeDataRequestFloat r_subset1 = accessManager.requestVolumeSubsetFloat(buffer1.getByteBuffer(), Dimensions_012, 0, 0, min, max);
                         VolumeDataRequestFloat r_subset0 = accessManager.requestVolumeSubsetFloat(buffer0.getByteBuffer(), Dimensions_012, 0, 0, min, max, channelNoValue)
                    ) {
                        r_subset1.waitForCompletion();
                        r_subset0.waitForCompletion();
                        FloatBuffer floatBuffer0 = buffer0.asFloatBuffer();
                        FloatBuffer floatBuffer1 = buffer1.asFloatBuffer();
                        for (int i = 0; i < nZSamples * nYSamples; i++) {
                            final float f1 = floatBuffer0.get(i);
                            final float f2 = floatBuffer1.get(i);
                            assertEquals(0, Float.compare(f1, f2), " value " + i + " is different");
                        }
                    }
                }
            }
        }
    }

    @Test
    public void testVolumeSubsetRequestByte() throws IOException {
        int nXSamples = 60, nYSamples = 60, nZSamples = 60;
        VolumeDataFormat format = Format_U8;
        try (InMemoryVDSGenerator generator = new InMemoryVDSGenerator(nZSamples, nYSamples, nXSamples, format)) {
            assertNotNull(generator);
            final VolumeDataLayout layout = generator.getLayout();
            assertTrue(!layout.isNull());
            try (VolumeDataAccessManager accessManager = generator.getAccessManager()) {
                assertNotNull(accessManager);
                int[] min = new int[]{ 0,0,0,0,0,0 };
                int[] max = new int[]{ nZSamples, nYSamples, 1, 0, 0, 0 };
                long bufferSize = accessManager.getVolumeSubsetBufferSize(min, max, format, 0, 0);
                try (ManagedBuffer buffer1 = new ManagedBuffer(bufferSize);
                     ManagedBuffer buffer0 = new ManagedBuffer(bufferSize)
                ) {
                    final float channelNoValue = layout.getChannelNoValue(0);
                    try (VolumeDataRequest r_subset1 = accessManager.requestVolumeSubset(buffer1.getByteBuffer(), Dimensions_012, 0, 0, min, max, format);
                         VolumeDataRequest r_subset0 = accessManager.requestVolumeSubset(buffer0.getByteBuffer(), Dimensions_012, 0, 0, min, max, format, channelNoValue)
                    ) {
                        r_subset1.waitForCompletion();
                        r_subset0.waitForCompletion();
                        for (int i = 0; i < nZSamples * nYSamples; i++) {
                            final float f1 = buffer0.get(i);
                            final float f2 = buffer1.get(i);
                            assertEquals(0, Float.compare(f1, f2), " value " + i + " is different");
                        }
                    }
                }
            }
        }
    }

    @Test
    public void testVolumeSubsetRequestInteger() throws IOException {
        int nXSamples = 60, nYSamples = 60, nZSamples = 60;
        VolumeDataFormat format = Format_U32;
        try (InMemoryVDSGenerator generator = new InMemoryVDSGenerator(nZSamples, nYSamples, nXSamples, format)) {
            assertNotNull(generator);
            final VolumeDataLayout layout = generator.getLayout();
            assertTrue(!layout.isNull());
            try (VolumeDataAccessManager accessManager = generator.getAccessManager()) {
                assertNotNull(accessManager);
                int[] min = new int[]{ 0,0,0,0,0,0 };
                int[] max = new int[]{ nZSamples, nYSamples, 1, 0, 0, 0 };
                long bufferSize = accessManager.getVolumeSubsetBufferSize(min, max, format, 0, 0);
                try (ManagedBuffer buffer1 = new ManagedBuffer(bufferSize);
                     ManagedBuffer buffer0 = new ManagedBuffer(bufferSize)
                ) {
                    final float channelNoValue = layout.getChannelNoValue(0);
                    try (VolumeDataRequest r_subset1 = accessManager.requestVolumeSubset(buffer1.getByteBuffer(), Dimensions_012, 0, 0, min, max, format);
                         VolumeDataRequest r_subset0 = accessManager.requestVolumeSubset(buffer0.getByteBuffer(), Dimensions_012, 0, 0, min, max, format, channelNoValue)
                    ) {
                        r_subset1.waitForCompletion();
                        r_subset0.waitForCompletion();
                        IntBuffer typedBuffer0 = buffer0.asIntBuffer();
                        IntBuffer typedBuffer1 = buffer1.asIntBuffer();
                        for (int i = 0; i < nZSamples * nYSamples; i++) {
                            final float f1 = typedBuffer0.get(i);
                            final float f2 = typedBuffer1.get(i);
                            assertEquals(0, Float.compare(f1, f2), " value " + i + " is different");
                        }
                    }
                }
            }
        }
    }

    @Test
    public void testVolumeSubsetRequestShort() throws IOException {
        int nXSamples = 60, nYSamples = 60, nZSamples = 60;
        VolumeDataFormat format = Format_U16;
        try (InMemoryVDSGenerator generator = new InMemoryVDSGenerator(nZSamples, nYSamples, nXSamples, format)) {
            assertNotNull(generator);
            final VolumeDataLayout layout = generator.getLayout();
            assertTrue(!layout.isNull());
            try (VolumeDataAccessManager accessManager = generator.getAccessManager()) {
                assertNotNull(accessManager);
                int[] min = new int[]{ 0,0,0,0,0,0 };
                int[] max = new int[]{ nZSamples, nYSamples, 1, 0, 0, 0 };
                long bufferSize = accessManager.getVolumeSubsetBufferSize(min, max, format, 0, 0);
                try (ManagedBuffer buffer1 = new ManagedBuffer(bufferSize);
                     ManagedBuffer buffer0 = new ManagedBuffer(bufferSize)
                ) {
                    final float channelNoValue = layout.getChannelNoValue(0);
                    try (VolumeDataRequest r_subset1 = accessManager.requestVolumeSubset(buffer1.getByteBuffer(), Dimensions_012, 0, 0, min, max, format);
                         VolumeDataRequest r_subset0 = accessManager.requestVolumeSubset(buffer0.getByteBuffer(), Dimensions_012, 0, 0, min, max, format, channelNoValue)
                    ) {
                        r_subset1.waitForCompletion();
                        r_subset0.waitForCompletion();
                        ShortBuffer typedBuffer0 = buffer0.asShortBuffer();
                        ShortBuffer typedBuffer1 = buffer1.asShortBuffer();
                        for (int i = 0; i < nZSamples * nYSamples; i++) {
                            final float f1 = typedBuffer0.get(i);
                            final float f2 = typedBuffer1.get(i);
                            assertEquals(0, Float.compare(f1, f2), " value " + i + " is different");
                        }
                    }
                }
            }
        }
    }

    @Test
    public void testLayout() {
        int nXSamples = 60, nYSamples = 60, nZSamples = 60;
        VolumeDataFormat format = Format_U8;
        InMemoryVDSGenerator generator = new InMemoryVDSGenerator(nXSamples, nYSamples, nZSamples, format);
        final VolumeDataLayout layout = generator.getLayout();
        assertNotNull(layout);

        int nbChannel = layout.getChannelCount();
        VolumeDataAccessManager accessManager = generator.getAccessManager();
        for (VolumeDataLayoutDescriptor.LODLevels l : VolumeDataLayoutDescriptor.LODLevels.values()) {
            for (int channel = 0; channel < nbChannel; channel++) {
                for (DimensionsND dimGroup : DimensionsND.values()) {
                    VDSProduceStatus vdsProduceStatus = accessManager.getVDSProduceStatus(dimGroup, l.ordinal(), channel);
                    if (channel == 0 && LODLevels_None.equals(l) && Dimensions_012.equals(dimGroup)) {
                        assertEquals(VDSProduceStatus.Normal, vdsProduceStatus);
                    } else if (channel == 0 && LODLevels_None.equals(l) && (
                            Dimensions_01.equals(dimGroup) ||
                            Dimensions_02.equals(dimGroup) ||
                            Dimensions_12.equals(dimGroup)
                            )
                    ) {
                        assertEquals(VDSProduceStatus.Remapped, vdsProduceStatus);
                    } else {
                        assertEquals(VDSProduceStatus.Unavailable, vdsProduceStatus);
                    }
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
        assertEquals(new int[]{45, 78}, layout.getMetadataIntVector2("categoryInt", "IntVector2").toArray());
        assertTrue(layout.isMetadataIntVector3Available("categoryInt", "IntVector3"));
        assertEquals(new int[]{45, 78, 72}, layout.getMetadataIntVector3("categoryInt", "IntVector3").toArray());
        assertTrue(layout.isMetadataIntVector4Available("categoryInt", "IntVector4"));
        assertEquals(new int[]{45, 78, 72, 84}, layout.getMetadataIntVector4("categoryInt", "IntVector4").toArray());

        assertTrue(layout.isMetadataFloatAvailable("categoryFloat", "Float"));
        assertEquals(123f, layout.getMetadataFloat("categoryFloat", "Float"), 0f);
        assertTrue(layout.isMetadataFloatVector2Available("categoryFloat", "FloatVector2"));
        assertEquals(new float[]{45.5f, 78.75f}, layout.getMetadataFloatVector2("categoryFloat", "FloatVector2").toArray());
        assertTrue(layout.isMetadataFloatVector3Available("categoryFloat", "FloatVector3"));
        assertEquals(new float[]{45.5f, 78.75f, 72.75f}, layout.getMetadataFloatVector3("categoryFloat", "FloatVector3").toArray());
        assertTrue(layout.isMetadataFloatVector4Available("categoryFloat", "FloatVector4"));
        assertEquals(new float[]{45.5f, 78.75f, 72.75f, 84.1f}, layout.getMetadataFloatVector4("categoryFloat", "FloatVector4").toArray());

        assertTrue(layout.isMetadataDoubleAvailable("categoryDouble", "Double"));
        assertEquals(123., layout.getMetadataDouble("categoryDouble", "Double"), 0f);
        assertTrue(layout.isMetadataDoubleVector2Available("categoryDouble", "DoubleVector2"));
        assertEquals(new double[]{45.5, 78.75}, layout.getMetadataDoubleVector2("categoryDouble", "DoubleVector2").toArray());
        assertTrue(layout.isMetadataDoubleVector3Available("categoryDouble", "DoubleVector3"));
        assertEquals(new double[]{45.5, 78.75, 72.75}, layout.getMetadataDoubleVector3("categoryDouble", "DoubleVector3").toArray());
        assertTrue(layout.isMetadataDoubleVector4Available("categoryDouble", "DoubleVector4"));
        assertEquals(new double[]{45.5, 78.75, 72.75, 84.1}, layout.getMetadataDoubleVector4("categoryDouble", "DoubleVector4").toArray());

        assertTrue(layout.isMetadataStringAvailable("categoryString", "String"));
        assertEquals("Test string", layout.getMetadataString("categoryString", "String"));

        assertTrue(layout.isMetadataBLOBAvailable("categoryBLOB", "BLOB"));
        assertEquals(new byte[]{1,2,3,4}, layout.getMetadataBLOB("categoryBLOB", "BLOB"));

        generator.close();
    }

    @Test
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
