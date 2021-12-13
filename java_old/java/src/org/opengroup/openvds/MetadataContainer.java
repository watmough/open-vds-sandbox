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

import java.nio.ByteBuffer;

public class MetadataContainer extends MetadataReadAccess {

    private static native long cpCreateMetadataContainerHandle();

    private static native void cpDeleteHandle(long handle);

    private static native void cpSetMetadataInt(long handle, String category, String name, int value);

    private static native void cpSetMetadataIntVector2(long handle, String category, String name, int[] value);

    private static native void cpSetMetadataIntVector3(long handle, String category, String name, int[] value);

    private static native void cpSetMetadataIntVector4(long handle, String category, String name, int[] value);

    private static native void cpSetMetadataFloat(long handle, String category, String name, float value);

    private static native void cpSetMetadataFloatVector2(long handle, String category, String name, float[] value);

    private static native void cpSetMetadataFloatVector3(long handle, String category, String name, float[] value);

    private static native void cpSetMetadataFloatVector4(long handle, String category, String name, float[] value);

    private static native void cpSetMetadataDouble(long handle, String category, String name, double value);

    private static native void cpSetMetadataDoubleVector2(long handle, String category, String name, double[] value);

    private static native void cpSetMetadataDoubleVector3(long handle, String category, String name, double[] value);

    private static native void cpSetMetadataDoubleVector4(long handle, String category, String name, double[] value);

    private static native void cpSetMetadataString(long handle, String category, String name, String value);

    private static native void cpSetMetadataBLOB(long handle, String category, String name, byte[] blobValues);

    /**
     * Constructor around existing JNI object
     * @param handle jni pointer to existing container
     */
    public MetadataContainer(long handle) {
        super(handle);
    }

    /**
     * Creates new MetadataContainer
     */
    public MetadataContainer() {
        super(cpCreateMetadataContainerHandle());
        setOwnHandle(true);
    }

    // Called by JniPointer.release()
    @Override
    protected synchronized void deleteHandle() {
        cpDeleteHandle(_handle);
    }


    /**
     * Set int metadata
     *
     * @param category
     *          metadata category definition
     * @param name
     *          metadata name
     * @param value
     *          metadata value
     */
    public void setMetadataInt(String category, String name, int value) {
        cpSetMetadataInt(_handle, category, name, value);
    }

    /**
     * Set int 2D array metadata
     *
     * @param category
     *          metadata category definition
     * @param name
     *          metadata name
     * @param value
     *          metadata value
     */
    public void setMetadataIntVector2(String category, String name, int[] value) {
        checkArrayArgument(value, 2);
        cpSetMetadataIntVector2(_handle, category, name, value);
    }

    /**
     * Set int 3D array metadata
     *
     * @param category
     *          metadata category definition
     * @param name
     *          metadata name
     * @param value
     *          metadata value
     */
    public void setMetadataIntVector3(String category, String name, int[] value) {
        checkArrayArgument(value, 3);
        cpSetMetadataIntVector3(_handle, category, name, value);
    }

    /**
     * Set int 4D array metadata
     *
     * @param category
     *          metadata category definition
     * @param name
     *          metadata name
     * @param value
     *          metadata value
     */
    public void setMetadataIntVector4(String category, String name, int[] value) {
        checkArrayArgument(value, 4);
        cpSetMetadataIntVector4(_handle, category, name, value);
    }

    /**
     * Set float metadata
     *
     * @param category
     *          metadata category definition
     * @param name
     *          metadata name
     * @param value
     *          metadata value
     */
    public void setMetadataFloat(String category, String name, float value) {
        cpSetMetadataFloat(_handle, category, name, value);
    }

    /**
     * Set float 2D array metadata
     *
     * @param category
     *          metadata category definition
     * @param name
     *          metadata name
     * @param value
     *          metadata value
     */
    public void setMetadataFloatVector2(String category, String name, float[] value) {
        checkArrayArgument(value, 2);
        cpSetMetadataFloatVector2(_handle, category, name, value);
    }

    /**
     * Set float 3D array metadata
     *
     * @param category
     *          metadata category definition
     * @param name
     *          metadata name
     * @param value
     *          metadata value
     */
    public void setMetadataFloatVector3(String category, String name, float[] value) {
        checkArrayArgument(value, 3);
        cpSetMetadataFloatVector3(_handle, category, name, value);
    }

    /**
     * Set float 4D array metadata
     *
     * @param category
     *          metadata category definition
     * @param name
     *          metadata name
     * @param value
     *          metadata value
     */
    public void setMetadataFloatVector4(String category, String name, float[] value) {
        checkArrayArgument(value, 4);
        cpSetMetadataFloatVector4(_handle, category, name, value);
    }

    /**
     * Set double metadata
     *
     * @param category
     *          metadata category definition
     * @param name
     *          metadata name
     * @param value
     *          metadata value
     */
    public void setMetadataDouble(String category, String name, double value) {
         cpSetMetadataDouble(_handle, category, name, value);
    }

    /**
     * Set double 2D array metadata
     *
     * @param category
     *          metadata category definition
     * @param name
     *          metadata name
     * @param value
     *          metadata value
     */
    public void setMetadataDoubleVector2(String category, String name, double[] value) {
        checkArrayArgument(value, 2);
        cpSetMetadataDoubleVector2(_handle, category, name, value);
    }

    /**
     * Set double 3D array metadata
     *
     * @param category
     *          metadata category definition
     * @param name
     *          metadata name
     * @param value
     *          metadata value
     */
    public void setMetadataDoubleVector3(String category, String name, double[] value) {
        checkArrayArgument(value, 3);
        cpSetMetadataDoubleVector3(_handle, category, name, value);
    }

    /**
     * Set double 4D array metadata
     *
     * @param category
     *          metadata category definition
     * @param name
     *          metadata name
     * @param value
     *          metadata value
     */
    public void setMetadataDoubleVector4(String category, String name, double[] value) {
        checkArrayArgument(value, 4);
        cpSetMetadataDoubleVector4(_handle, category, name, value);
    }

    /**
     * Set String metadata
     *
     * @param category
     *          metadata category definition
     * @param name
     *          metadata name
     * @param value
     *          metadata value
     */
    public void setMetadataString(String category, String name, String value) {
        if (value == null) {
            throw new IllegalArgumentException("Null String Metadata.");
        }
        cpSetMetadataString(_handle, category, name, value);
    }

    /**
     * Set metadata blob using a byte array
     *
     * @param category
     *          metadata category definition
     * @param name
     *          metadata name
     * @param blobValues
     *          metadata value as byte array
     */
    public void setMetadataBLOB(String category, String name, byte[] blobValues) {
        if (blobValues == null) {
            throw new IllegalArgumentException("Blob values array is null.");
        }
        cpSetMetadataBLOB(_handle, category, name, blobValues);
    }

    /**
     * Set metadata blob using a byte buffer
     *
     * @param category
     *          metadata category definition
     * @param name
     *          metadata name
     * @param blobValues
     *          metadata value as byte buffer
     */
    public void setMetadataBLOB(String category, String name, ByteBuffer blobValues) {
        if (blobValues == null) {
            throw new IllegalArgumentException("Blob values buffer is null.");
        }
        cpSetMetadataBLOB(_handle, category, name, blobValues.array());
    }

    private void checkArrayArgument(int[] array, int expectedSize) {
        if (array == null || array.length != expectedSize) {
            throw new IllegalArgumentException("Wrong IntVector parameter size, expected " + expectedSize + " got " + (array == null ? "null" : array.length));
        }
    }

    private void checkArrayArgument(float[] array, int expectedSize) {
        if (array == null || array.length != expectedSize) {
            throw new IllegalArgumentException("Wrong FloatVector parameter size, expected 2, got " + (array == null ? "null" : array.length));
        }
    }

    private void checkArrayArgument(double[] array, int expectedSize) {
        if (array == null || array.length != expectedSize) {
            throw new IllegalArgumentException("Wrong DoubleVector parameter size, expected 2, got " + (array == null ? "null" : array.length));
        }
    }
}