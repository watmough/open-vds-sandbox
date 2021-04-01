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

import java.util.HashMap;
import java.util.Map;

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

    private static native void cpSetMetadataKeys(long handle, MetadataKey[] keys);

    public MetadataContainer(long handle) {
        super(handle);
    }

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
     * @param category
     * @param name
     * @param value
     */
    public void setMetadataInt(String category, String name, int value) {
        cpSetMetadataInt(_handle, category, name, value);
    }

    /**
     * @param category
     * @param name
     * @param value
     */
    public void setMetadataIntVector2(String category, String name, int[] value) {
        cpSetMetadataIntVector2(_handle, category, name, value);
    }

    /**
     * @param category
     * @param name
     * @param value
     */
    public void setMetadataIntVector3(String category, String name, int[] value) {
        cpSetMetadataIntVector3(_handle, category, name, value);
    }

    /**
     * @param category
     * @param name
     * @param value
     */
    public void setMetadataIntVector4(String category, String name, int[] value) {
        cpSetMetadataIntVector4(_handle, category, name, value);
    }

    /**
     * @param category
     * @param name
     * @param value
     */
    public void setMetadataFloat(String category, String name, float value) {
        cpSetMetadataFloat(_handle, category, name, value);
    }

    /**
     * @param category
     * @param name
     * @param value
     */
    public void setMetadataFloatVector2(String category, String name, float[] value) {
        cpSetMetadataFloatVector2(_handle, category, name, value);
    }

    /**
     * @param category
     * @param name
     * @param value
     */
    public void setMetadataFloatVector3(String category, String name, float[] value) {
        cpSetMetadataFloatVector3(_handle, category, name, value);
    }

    /**
     * @param category
     * @param name
     * @param value
     */
    public void setMetadataFloatVector4(String category, String name, float[] value) {
        cpSetMetadataFloatVector4(_handle, category, name, value);
    }

    /**
     * @param category
     * @param name
     * @param value
     */
    public void setMetadataDouble(String category, String name, double value) {
         cpSetMetadataDouble(_handle, category, name, value);
    }

    /**
     * @param category
     * @param name
     * @param value
     */
    public void setMetadataDoubleVector2(String category, String name, double[] value) {
        cpSetMetadataDoubleVector2(_handle, category, name, value);
    }

    /**
     * @param category
     * @param name
     * @param value
     */
    public void setMetadataDoubleVector3(String category, String name, double[] value) {
        cpSetMetadataDoubleVector3(_handle, category, name, value);
    }

    /**
     * @param category
     * @param name
     * @param value
     */
    public void setMetadataDoubleVector4(String category, String name, double[] value) {
        cpSetMetadataDoubleVector4(_handle, category, name, value);
    }

    /**
     * @param category
     * @param name
     * @param value
     */
    public void setMetadataString(String category, String name, String value) {
        cpSetMetadataString(_handle, category, name, value);
    }

    /**
     * Sets an array of metadata keys
     * @param keys
     */
    public void setMetadataKeys(MetadataKey[] keys) {
        cpSetMetadataKeys(_handle, keys);
    }

}