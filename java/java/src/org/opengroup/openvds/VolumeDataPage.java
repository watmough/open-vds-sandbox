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

    private static native float[] cpGetFloatBuffer(long handle, int[] pitch);

    public VolumeDataPage(long handle) {
        super(handle, true);
    }

    public void release() {
        cpRelease(_handle);
    }

    // Called by JniPointer.release()
    @Override
    protected synchronized void deleteHandle() {
        cpRelease(_handle);
    }

    public float[] readFloatBuffer(int[] pitch) {
        if (pitch == null || pitch.length != VolumeDataLayout.Dimensionality_Max) {
            throw new IllegalArgumentException("Wrong pitch array parameter size, expected " + VolumeDataLayout.Dimensionality_Max + ", got " + (pitch == null ? "null" : pitch.length));
        }
        return cpGetFloatBuffer(_handle, pitch);
    }
}
