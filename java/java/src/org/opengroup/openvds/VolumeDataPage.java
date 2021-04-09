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

    private static native  void cpGetMinMax(long handle, int[] min, int[] max);

    private static native float[] cpGetFloatBuffer(long handle, int[] pitch);

    private static native float[] cpSetFloatBuffer(long handle, float[] buffer,int[] pitch);

    public VolumeDataPage(long handle) {
        super(handle, true);
    }

    public VolumeDataPage(long handle, boolean ownHandle) {
        super(handle, ownHandle);
    }

    /**
     * Release page
     */
    public void release() {
        cpRelease(_handle);
    }

    // Called by JniPointer.release()
    @Override
    protected synchronized void deleteHandle() {
        cpRelease(_handle);
    }

    /**
     * Get min max extent, and set results in array parameters
     * @param min arrays that will receive min values
     * @param max arrays that will receive max values
     */
    public void getMinMax(int[] min, int[] max) {
        if (min == null || min.length != VolumeDataLayout.Dimensionality_Max) {
            throw new IllegalArgumentException("Wrong min array parameter size, expected " + VolumeDataLayout.Dimensionality_Max + ", got " + (min == null ? "null" : min.length));
        }
        if (max == null || max.length != VolumeDataLayout.Dimensionality_Max) {
            throw new IllegalArgumentException("Wrong max array parameter size, expected " + VolumeDataLayout.Dimensionality_Max + ", got " + (max == null ? "null" : max.length));
        }
        cpGetMinMax(_handle, min, max);
    }

    /**
     * Read float array of page
     * @param pitch will receive pitch values for this page
     * @return the float array of page data
     */
    public float[] readFloatBuffer(int[] pitch) {
        if (pitch == null || pitch.length != VolumeDataLayout.Dimensionality_Max) {
            throw new IllegalArgumentException("Wrong pitch array parameter size, expected " + VolumeDataLayout.Dimensionality_Max + ", got " + (pitch == null ? "null" : pitch.length));
        }
        return cpGetFloatBuffer(_handle, pitch);
    }

    /**
     * Set float array int page
     * @param pitch pitch values
     */
    public void writeFloatBuffer(float[] buffer, int[] pitch) {
        if (pitch == null || pitch.length != VolumeDataLayout.Dimensionality_Max) {
            throw new IllegalArgumentException("Wrong pitch array parameter size, expected " + VolumeDataLayout.Dimensionality_Max + ", got " + (pitch == null ? "null" : pitch.length));
        }
        int[] chunkMin = new int[VolumeDataLayout.Dimensionality_Max];
        int[] chunkMax = new int[VolumeDataLayout.Dimensionality_Max];
        getMinMax(chunkMin, chunkMax);
        int elementCount = getElementCount(chunkMin, chunkMax);
        if (buffer == null || buffer.length != elementCount) {
            throw new IllegalArgumentException("Wrong buffer size, expected " + elementCount + ", got " + (buffer == null ? "null" : buffer.length));
        }
        cpSetFloatBuffer(_handle, buffer, pitch);
    }

    private int getElementCount(int[] min, int[] max) {
        int chunkSizeI = max[2] - min[2];
        int chunkSizeX = max[1] - min[1];
        int chunkSizeZ = max[0] - min[0];
        return chunkSizeI * chunkSizeX * chunkSizeZ;
    }
}
