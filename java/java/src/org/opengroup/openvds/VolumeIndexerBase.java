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

package org.opengroup.openvds;

public abstract class VolumeIndexerBase extends JniPointer {

    private static native void cpDeleteHandle(long handle, int volumeDimension);
    private static native float cpGetValueRangeMin(long handle, int volumeDimension);
    private static native float cpGetValueRangeMax(long handle, int volumeDimension);

    private static native int cpGetDataBlockNumSamples(long handle, int volumeDimension, int dim);
    private static native int cpGetLocalChunkNumSamples(long handle, int volumeDimension, int dim);

    protected final int dimensionVolume;

    public VolumeIndexerBase(long handle, int dimension, boolean ownHandle) {
        super(handle, ownHandle);
        this.dimensionVolume = dimension;
    }

    protected int getVolumeDimension() {
        return dimensionVolume;
    }

    @Override
    protected void deleteHandle() {
        cpDeleteHandle(_handle, dimensionVolume);
    }

    public float getValueRangeMin() {
        return cpGetValueRangeMin(_handle, dimensionVolume);
    }

    public float getValueRangeMax() {
        return cpGetValueRangeMax(_handle, dimensionVolume);
    }

    /**
     * Gets the number of samples for a dimension in the DataBlock
     * @param dimension the volume dimension
     * @return the number of samples in the dimension
     */
    public int getDataBlockNumSamples(int dimension) {
        return cpGetDataBlockNumSamples(_handle, dimensionVolume, dimension);
    }

    /**
     * Get the number of samples for a dimension in the volume
     * @param dimension the volume dimension
     * @return the number of samples in the dimension
     */
    public int getLocalChunkNumSamples(int dimension) { return cpGetLocalChunkNumSamples(_handle, dimensionVolume, dimension); };

    /**
     * Converts a local index to a voxel index
     * @param localIndex the local index to convert
     * @return the voxel index
     */
    public abstract int[] localIndexToVoxelIndex(int[] localIndex);

    /**
     * Converts a local index to a local chunk index
     * @param localIndex the local index to convert
     * @return the local chunk index
     */
    public abstract int[] localIndexToLocalChunkIndex(int[] localIndex);

    /**
     * Converts a voxel index to a local index
     * @param voxelIndex the voxel index to convert
     * @return the local index
     */
    public abstract int[] voxelIndexToLocalIndex(int[] voxelIndex);

    /**
     * Converts a voxel index to a local chunk index
     * @param voxelIndex the voxel index to convert
     * @return the local index
     */
    public abstract int[] voxelIndexToLocalChunkIndex(int[] voxelIndex);

    /**
     * Converts a local chunk index to a local index
     * @param localChunkIndex the local chunk index to convert
     * @return the local index
     */
    public abstract int[] localChunkIndexToLocalIndex(int[] localChunkIndex);

    /**
     * Converts a local chunk index to a voxel index
     * @param localChunkIndex
     * @return the local voxel index
     */
    public abstract int[] localChunkIndexToVoxelIndex(int[] localChunkIndex);

    /**
     * Converts a local index to a data index
     * @param localIndex the local index to convert
     * @return the buffer offset (position) for the local index
     */
    public abstract int localIndexToDataIndex(int[] localIndex);

    /**
     * Converts a voxel index to a data index
     * @param voxelIndex the voxel index to convert
     * @return the buffer offset for the voxel index
     */
    public abstract int voxelIndexToDataIndex(int[] voxelIndex);

    /**
     * Converts a voxel index to a data index
     * @param localChunkIndex the local chunk index to convert
     * @return the buffer offset for the local chunk index
     */
    public abstract int localChunkIndexToDataIndex(int[] localChunkIndex);

    /**
     * Checks if a voxel index is within the chunk this indexer was created with
     * @param voxelIndex the voxel index to check
     * @return true if the index is within this chunk, false otherwise
     */
    public abstract boolean voxelIndexInProcessArea(int[] voxelIndex);

    /**
     * Checks if a local index is within the chunk this indexer was created with
     * @param localIndex the local index to check
     * @return true if the index is within this chunk, false otherwise
     */
    public abstract boolean localIndexInProcessArea(int[] localIndex);

    /**
     * Checks if a local chunk index is within the chunk this indexer was created with
     * @param localChunkIndex the local chunk index to check
     * @return true if the index is within this chunk, false otherwise
     */
    public abstract boolean localChunkIndexInProcessArea(int[] localChunkIndex);


    protected void checkIndexArgument(int[] localIndex) {
        if (localIndex == null || localIndex.length != dimensionVolume) {
            throw new IllegalArgumentException("Local index size must match indexer dimension. Expected " + dimensionVolume + ", got " + (localIndex != null ? localIndex.length : "null"));
        }
    }
}
