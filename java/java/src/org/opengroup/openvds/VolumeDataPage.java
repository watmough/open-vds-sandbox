/*
 * Copyright 2019 The Open Group
 * Copyright 2019 INT, Inc.
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

public class VolumeDataPage extends JniPointer {

    private static native void cpRelease(long handle);

    private static native void cpGetMinMax(long handle, int[] min, int[] max);

    private static native void cpGetMinMaxExcludingMargin(long handle, int[] min, int[] max);

    private static native int cpGetAllocatedSize(long handle);

    private static native byte[] cpGetByteBuffer(long handle, int[] pitch, int lod);

    private static native void cpSetByteBuffer(long handle, byte[] buffer);

    private static native float[] cpGetFloatBuffer(long handle, int[] pitch, int lod);

    private static native void cpSetFloatBuffer(long handle, float[] buffer);

    private static native double[] cpGetDoubleBuffer(long handle, int[] pitch, int lod);

    private static native void cpSetDoubleBuffer(long handle, double[] buffer);

    private final int dimensionality;
    private final int lod;

    public VolumeDataPage(long handle, int dimensionality, int lod) {
        super(handle, true);
        this.dimensionality = dimensionality;
        this.lod = lod;
    }

    public VolumeDataPage(long handle, int dimensionality, int lod, boolean ownHandle) {
        super(handle, ownHandle);
        this.dimensionality = dimensionality;
        this.lod = lod;
    }

    public void pageRelease() {
        cpRelease(_handle);
    }

    // Called by JniPointer.release()
    @Override
    protected synchronized void deleteHandle() {
        // nothing to do. Release page is done by its own method and must be called by the main code, not the finalizer
        // volume page release is handled by the page accessor and manager
    }

    /**
     * Get min max extent, and set results in array parameters
     * @param min arrays that will receive min values
     * @param max arrays that will receive max values
     */
    public void getMinMax(int[] min, int[] max) {
        checkDimParamArray(min, "Wrong min array parameter size, expected ");
        checkDimParamArray(max, "Wrong max array parameter size, expected ");
        cpGetMinMax(_handle, min, max);
    }

    /**
     * Get min max extent, and set results in array parameters
     * @param min arrays that will receive min values
     * @param max arrays that will receive max values
     */
    public void getMinMaxExcludingMargin(int[] min, int[] max) {
        checkDimParamArray(min, "Wrong min array parameter size, expected ");
        checkDimParamArray(max, "Wrong max array parameter size, expected ");
        cpGetMinMaxExcludingMargin(_handle, min, max);
    }

    /**
     * @return the buffer size of this page
     */
    public int getAllocatedBufferSize() {
        return cpGetAllocatedSize(_handle);
    }

    /**
     * Read byte array of page
     * @param pitch will receive pitch values for this page
     * @return the float array of page data
     */
    public byte[] readByteBuffer(int[] pitch) {
        checkDimParamArray(pitch, "Wrong pitch array parameter size, expected ");
        return cpGetByteBuffer(_handle, pitch, lod);
    }

    /**
     * Set byte array int page
     * @param buffer values to set. Size must match page sample size
     * @param pitch chunk pitch (got by a read)
     */
    public void writeByteBuffer(byte[] buffer, int[] pitch) {
        checkBufferSize(buffer, pitch, dimensionality, lod);
        cpSetByteBuffer(_handle, buffer);
    }

    /**
     * Read float array of page
     * @param pitch will receive pitch values for this page
     * @return the float array of page data
     */
    public float[] readFloatBuffer(int[] pitch) {
        checkDimParamArray(pitch, "Wrong pitch array parameter size, expected ");
        return cpGetFloatBuffer(_handle, pitch, lod);
    }

    /**
     * Set float array int page
     * @param buffer values to set. Size must match page sample size
     * @param pitch chunk pitch (got by a read)
     */
    public void writeFloatBuffer(float[] buffer, int[] pitch) {
        checkBufferSize(buffer, pitch, dimensionality, lod);
        cpSetFloatBuffer(_handle, buffer);
    }

    /**
     * Read double array of page
     * @param pitch will receive pitch values for this page
     * @return the double array of page data
     */
    public double[] readDoubleBuffer(int[] pitch) {
        checkDimParamArray(pitch, "Wrong pitch array parameter size, expected ");
        return cpGetDoubleBuffer(_handle, pitch, lod);
    }

    /**
     * Set double array int page
     * @param buffer values to set. Size must match page sample size
     * @param pitch chunk pitch (got by a read)
     */
    public void writeDoubleBuffer(double[] buffer, int[] pitch) {
        checkBufferSize(buffer, pitch, dimensionality, lod);
        cpSetDoubleBuffer(_handle, buffer);
    }

    private int getElementCount() {
        int[] chunkMin = new int[VolumeDataLayout.Dimensionality_Max];
        int[] chunkMax = new int[VolumeDataLayout.Dimensionality_Max];
        getMinMax(chunkMin, chunkMax);
        return getElementCount(chunkMin, chunkMax);
    }

    private void checkBufferSize(byte[] buffer, int[] pitch, int dim, int lod) {
        if (buffer == null) {
            throw new IllegalArgumentException("Wrong buffer size, got NULL");
        }
        checkBufferSize(buffer.length, pitch, dimensionality, lod);
    }

    private void checkBufferSize(float[] buffer, int[] pitch, int dim, int lod) {
        if (buffer == null) {
            throw new IllegalArgumentException("Wrong buffer size, got NULL");
        }
        checkBufferSize(buffer.length, pitch, dimensionality, lod);
    }

    private void checkBufferSize(double[] buffer, int[] pitch, int dim, int lod) {
        if (buffer == null) {
            throw new IllegalArgumentException("Wrong buffer size, got NULL");
        }
        checkBufferSize(buffer.length, pitch, dimensionality, lod);
    }

    private void checkBufferSize(int sizeInputBuffer, int[] pitch, int dim, int lod) {
        int pageBufferSize = getAllocatedBufferSize();
        if (sizeInputBuffer != pageBufferSize) {
            throw new IllegalArgumentException("Wrong buffer size, expected " + pageBufferSize + ", got " + sizeInputBuffer);
        }
    }

    private void checkDimParamArray(int[] pitch, String s) {
        if (pitch == null || pitch.length != VolumeDataLayout.Dimensionality_Max) {
            throw new IllegalArgumentException(s + VolumeDataLayout.Dimensionality_Max + ", got " + (pitch == null ? "null" : pitch.length));
        }
    }

    private int getElementCount(int[] min, int[] max) {
        int chunkSizeI = max[2] - min[2];
        int chunkSizeX = max[1] - min[1];
        int chunkSizeZ = max[0] - min[0];
        return chunkSizeI * chunkSizeX * chunkSizeZ;
    }
}
