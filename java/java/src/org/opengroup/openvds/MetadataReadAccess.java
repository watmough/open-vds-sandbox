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

public class MetadataReadAccess extends JniPointerWithoutDeletion {

    private static native boolean cpIsMetadataIntAvailable(long handle, String category, String name);

    private static native boolean cpIsMetadataIntVector2Available(long handle, String category, String name);

    private static native boolean cpIsMetadataIntVector3Available(long handle, String category, String name);

    private static native boolean cpIsMetadataIntVector4Available(long handle, String category, String name);

    private static native boolean cpIsMetadataFloatAvailable(long handle, String category, String name);

    private static native boolean cpIsMetadataFloatVector2Available(long handle, String category, String name);

    private static native boolean cpIsMetadataFloatVector3Available(long handle, String category, String name);

    private static native boolean cpIsMetadataFloatVector4Available(long handle, String category, String name);

    private static native boolean cpIsMetadataDoubleAvailable(long handle, String category, String name);

    private static native boolean cpIsMetadataDoubleVector2Available(long handle, String category, String name);

    private static native boolean cpIsMetadataDoubleVector3Available(long handle, String category, String name);

    private static native boolean cpIsMetadataDoubleVector4Available(long handle, String category, String name);

    private static native boolean cpIsMetadataStringAvailable(long handle, String category, String name);

    private static native boolean cpIsMetadataBLOBAvailable(long handle, String category, String name);

    private static native int cpGetMetadataInt(long handle, String category, String name);

    private static native int[] cpGetMetadataIntVector2(long handle, String category, String name);

    private static native int[] cpGetMetadataIntVector3(long handle, String category, String name);

    private static native int[] cpGetMetadataIntVector4(long handle, String category, String name);

    private static native float cpGetMetadataFloat(long handle, String category, String name);

    private static native float[] cpGetMetadataFloatVector2(long handle, String category, String name);

    private static native float[] cpGetMetadataFloatVector3(long handle, String category, String name);

    private static native float[] cpGetMetadataFloatVector4(long handle, String category, String name);

    private static native double cpGetMetadataDouble(long handle, String category, String name);

    private static native double[] cpGetMetadataDoubleVector2(long handle, String category, String name);

    private static native double[] cpGetMetadataDoubleVector3(long handle, String category, String name);

    private static native double[] cpGetMetadataDoubleVector4(long handle, String category, String name);

    private static native String cpGetMetadataString(long handle, String category, String name);

    private static native byte[] cpGetMetadataBLOB(long handle, String category, String name);

    private static native MetadataKey[] cpGetMetadataKeys(long handle);

    public MetadataReadAccess(long handle) {
        super(handle);
    }

    /**
     * Check if the int metadata defined by the given category and name is available.
     * 
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return true if a metadata int with the given category and name is available
     */
    public boolean isMetadataIntAvailable(String category, String name) {
        return cpIsMetadataIntAvailable(_handle, category, name);
    }

    /**
     * Check if the int 2 sized array metadata defined by the given category and name is available.
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return true if a metadata IntVector2 with the given category and name is
     * available
     */
    public boolean isMetadataIntVector2Available(String category, String name) {
        return cpIsMetadataIntVector2Available(_handle, category, name);
    }

    /**
     * Check if the int 3 sized array metadata defined by the given category and name is available.
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return true if a metadata IntVector3 with the given category and name is
     * available
     */
    public boolean isMetadataIntVector3Available(String category, String name) {
        return cpIsMetadataIntVector3Available(_handle, category, name);
    }

    /**
     * Check if the int 4 sized array metadata defined by the given category and name is available.
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return true if a metadata IntVector4 with the given category and name is
     * available
     */
    public boolean isMetadataIntVector4Available(String category, String name) {
        return cpIsMetadataIntVector4Available(_handle, category, name);
    }

    /**
     * Check if the float metadata defined by the given category and name is available.
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return true if a metadata float with the given category and name is available
     */
    public boolean isMetadataFloatAvailable(String category, String name) {
        return cpIsMetadataFloatAvailable(_handle, category, name);
    }

    /**
     * Check if the float 2 sized array metadata defined by the given category and name is available.
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return true if a metadata FloatVector2 with the given category and name is available
     */
    public boolean isMetadataFloatVector2Available(String category, String name) {
        return cpIsMetadataFloatVector2Available(_handle, category, name);
    }

    /**
     * Check if the float 3 sized metadata defined by the given category and name is available.
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return true if a metadata FloatVector3 with the given category and name is available
     */
    public boolean isMetadataFloatVector3Available(String category, String name) {
        return cpIsMetadataFloatVector3Available(_handle, category, name);
    }

    /**
     * Check if the float 4 sized metadata defined by the given category and name is available.
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return true if a metadata FloatVector4 with the given category and name is available
     */
    public boolean isMetadataFloatVector4Available(String category, String name) {
        return cpIsMetadataFloatVector4Available(_handle, category, name);
    }

    /**
     * Check if the double metadata defined by the given category and name is available.
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return true if a metadata double with the given category and name is available
     */
    public boolean isMetadataDoubleAvailable(String category, String name) {
        return cpIsMetadataDoubleAvailable(_handle, category, name);
    }

    /**
     * Check if the double 2 sized metadata defined by the given category and name is available.
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return true if a metadata DoubleVector2 with the given category and name is available
     */
    public boolean isMetadataDoubleVector2Available(String category, String name) {
        return cpIsMetadataDoubleVector2Available(_handle, category, name);
    }

    /**
     * Check if the double 3 sized array metadata defined by the given category and name is available.
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return true if a metadata DoubleVector3 with the given category and name is available
     */
    public boolean isMetadataDoubleVector3Available(String category, String name) {
        return cpIsMetadataDoubleVector3Available(_handle, category, name);
    }

    /**
     * Check if the double 4 sized array metadata defined by the given category and name is available.
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return true if a metadata DoubleVector4 with the given category and name is available
     */
    public boolean isMetadataDoubleVector4Available(String category, String name) {
        return cpIsMetadataDoubleVector4Available(_handle, category, name);
    }

    /**
     * Check if the String metadata defined by the given category and name is available.
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return true if a metadata string with the given category and name is available
     */
    public boolean isMetadataStringAvailable(String category, String name) {
        return cpIsMetadataStringAvailable(_handle, category, name);
    }

    /**
     * Check if the blob metadata defined by the given category and name is available.
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return true if a metadata BLOB with the given category and name is available
     */
    public boolean isMetadataBLOBAvailable(String category, String name) {
        return cpIsMetadataBLOBAvailable(_handle, category, name);
    }

    /**
     * Get int metadata defined by category and name
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return the metadata int with the given category and name
     */
    public int getMetadataInt(String category, String name) {
        return cpGetMetadataInt(_handle, category, name);
    }

    /**
     * Get int 2 sized array metadata defined by category and name
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return the metadata IntVector2 (2 sized int array) with the given category and name
     */
    public int[] getMetadataIntVector2(String category, String name) {
        return cpGetMetadataIntVector2(_handle, category, name);
    }

    /**
     * Get int 3 sized array metadata defined by category and name
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return the metadata IntVector3 (3 sized int array) with the given category and name
     */
    public int[] getMetadataIntVector3(String category, String name) {
        return cpGetMetadataIntVector3(_handle, category, name);
    }

    /**
     * Get int 4 sized array metadata defined by category and name
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return the metadata IntVector4 (4 sized int array) with the given category and name
     */
    public int[] getMetadataIntVector4(String category, String name) {
        return cpGetMetadataIntVector4(_handle, category, name);
    }

    /**
     * Get float metadata defined by category and name
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return the metadata float with the given category and name
     */
    public float getMetadataFloat(String category, String name) {
        return cpGetMetadataFloat(_handle, category, name);
    }

    /**
     * Get float 2 sized array metadata defined by category and name
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return the metadata FloatVector2 (2 sized float array) with the given category and name
     */
    public float[] getMetadataFloatVector2(String category, String name) {
        return cpGetMetadataFloatVector2(_handle, category, name);
    }

    /**
     * Get float 3 sized array metadata defined by category and name
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return the metadata FloatVector3 (3 sized float array)) with the given category and name
     */
    public float[] getMetadataFloatVector3(String category, String name) {
        return cpGetMetadataFloatVector3(_handle, category, name);
    }

    /**
     * Get float 4 sized array metadata defined by category and name
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return the metadata FloatVector4 (4 sized float array)) with the given category and name
     */
    public float[] getMetadataFloatVector4(String category, String name) {
        return cpGetMetadataFloatVector4(_handle, category, name);
    }

    /**
     * Get double metadata defined by category and name
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return the metadata double with the given category and name
     */
    public double getMetadataDouble(String category, String name) {
        return cpGetMetadataDouble(_handle, category, name);
    }

    /**
     * Get double 2 sized array metadata defined by category and name
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return the metadata DoubleVector2 (2 sized double array) with the given category and name
     */
    public double[] getMetadataDoubleVector2(String category, String name) {
        return cpGetMetadataDoubleVector2(_handle, category, name);
    }

    /**
     * Get double 3 sized array metadata defined by category and name
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return the metadata DoubleVector3 (3 sized double array) with the given category and name
     */
    public double[] getMetadataDoubleVector3(String category, String name) {
        return cpGetMetadataDoubleVector3(_handle, category, name);
    }

    /**
     * Get double 4 sized array metadata defined by category and name
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return the metadata DoubleVector4 (4 sized double array) with the given category and name
     */
    public double[] getMetadataDoubleVector4(String category, String name) {
        return cpGetMetadataDoubleVector4(_handle, category, name);
    }

    /**
     * Get String metadata defined by the given category and name
     *
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return the metadata string with the given category and name
     */
    public String getMetadataString(String category, String name) {
        return cpGetMetadataString(_handle, category, name);
    }

    /**
     * get Metadata blob as byte array
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return a byte array with the blob data
     */
    public byte[] getMetadataBLOB(String category, String name) {
        return cpGetMetadataBLOB(_handle, category, name);
    }

    /**
     * get Metadata blob as byte buffer
     * @param category
     *          metadata category
     * @param name
     *          metadata name
     * @return a byte buffer of the blob metadata
     */
    public ByteBuffer getMetadataBLOBAsBuffer(String category, String name) {
        byte[] blobArray = cpGetMetadataBLOB(_handle, category, name);
        return ByteBuffer.wrap(blobArray);
    }

    /**
     * @return an array of metadata keys
     */
    public MetadataKey[] getMetadataKeys() {
        return cpGetMetadataKeys(_handle);
    }
}
