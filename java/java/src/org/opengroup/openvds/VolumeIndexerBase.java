package org.opengroup.openvds;

public abstract class VolumeIndexerBase extends JniPointer {

    private static native void cpDeleteHandle(long handle, int volumeDimension);
    private static native float cpGetValueRangeMin(long handle, int volumeDimension);
    private static native float cpGetValueRangeMax(long handle, int volumeDimension);

    private static native int cpGetDataBlockNumSamples(long handle, int volumeDimension, int dim);
    private static native int cpGetLocalChunkNumSamples(long handle, int volumeDimension, int dim);

    private static native int[] cpLocalIndexToVoxelIndex(long handle, int volumeDimension, int[] localIndex);
    private static native int[] cpLocalIndexToLocalChunkIndex(long handle, int volumeDimension, int[] localIndex);

    private static native int[] cpVoxelIndexToLocalIndex(long handle, int volumeDimension, int[] voxelIndex);
    private static native int[] cpVoxelIndexToLocalChunkIndex(long handle, int volumeDimension, int[] voxelIndex);

    private static native int[] cpLocalChunkIndexToLocalIndex(long handle, int volumeDimension, int[] voxelIndex);
    private static native int[] cpLocalChunkIndexToVoxelIndex(long handle, int volumeDimension, int[] voxelIndex);

    private static native int cpLocalIndexToDataIndex(long handle, int volumeDimension, int[] localIndex);
    private static native int cpVoxelIndexToDataIndex(long handle, int volumeDimension, int[] voxelIndex);
    private static native int cpLocalChunkIndexToDataIndex(long handle, int volumeDimension, int[] localChunkIndex);

    private final int dimensionVolume;

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
    public int[] localIndexToVoxelIndex(int[] localIndex) {
        checkLocalIndexArgument(localIndex);
        return cpLocalIndexToVoxelIndex(_handle, dimensionVolume, localIndex);
    }

    /**
     * Converts a local index to a local chunk index
     * @param localIndex the local index to convert
     * @return the local chunk index
     */
    public int[] localIndexToLocalChunkIndex(int[] localIndex) {
        checkLocalIndexArgument(localIndex);
        return cpLocalIndexToLocalChunkIndex(_handle, dimensionVolume, localIndex);
    }

    /**
     * Converts a voxel index to a local index
     * @param voxelIndex the voxel index to convert
     * @return the local index
     */
    public int[] voxelIndexToLocalIndex(int[] voxelIndex) {
        checkLocalIndexArgument(voxelIndex);
        return cpVoxelIndexToLocalIndex(_handle, dimensionVolume, voxelIndex);
    }

    /**
     * Converts a voxel index to a local chunk index
     * @param voxelIndex the voxel index to convert
     * @return the local index
     */
    public int[] voxelIndexToLocalChunkIndex(int[] voxelIndex) {
        checkLocalIndexArgument(voxelIndex);
        return cpVoxelIndexToLocalChunkIndex(_handle, dimensionVolume, voxelIndex);
    }

    /**
     * Converts a local chunk index to a local index
     * @param localChunkIndex the local chunk index to convert
     * @return the local index
     */
    public int[] localChunkIndexToLocalIndex(int[] localChunkIndex) {
        checkLocalIndexArgument(localChunkIndex);
        return cpLocalChunkIndexToLocalIndex(_handle, dimensionVolume, localChunkIndex);
    }

    /**
     * Converts a local chunk index to a voxel index
     * @param localChunkIndex
     * @return the local voxel index
     */
    public int[] localChunkIndexToVoxelIndex(int[] localChunkIndex) {
        checkLocalIndexArgument(localChunkIndex);
        return cpLocalChunkIndexToVoxelIndex(_handle, dimensionVolume, localChunkIndex);
    }

    /**
     * Converts a local index to a data index
     * @param localIndex the local index to convert
     * @return the buffer offset (position) for the local index
     */
    public int localIndexToDataIndex(int[] localIndex) {
        checkLocalIndexArgument(localIndex);
        return cpLocalIndexToDataIndex(_handle, dimensionVolume, localIndex);
    }

    /**
     * Converts a voxel index to a data index
     * @param voxelIndex the voxel index to convert
     * @return the buffer offset for the voxel index
     */
    public int voxelIndexToDataIndex(int[] voxelIndex) {
        checkLocalIndexArgument(voxelIndex);
        return cpVoxelIndexToDataIndex(_handle, dimensionVolume, voxelIndex);
    }

    /**
     * Converts a voxel index to a data index
     * @param localChunkIndex the local chunk index to convert
     * @return the buffer offset for the local chunk index
     */
    public int localChunkIndexToDataIndex(int[] localChunkIndex) {
        checkLocalIndexArgument(localChunkIndex);
        return cpLocalChunkIndexToDataIndex(_handle, dimensionVolume, localChunkIndex);
    }


    private void checkLocalIndexArgument(int[] localIndex) {
        if (localIndex == null || localIndex.length != dimensionVolume) {
            throw new IllegalArgumentException("Local index size must match indexer dimension. Expected " + dimensionVolume + ", got " + (localIndex != null ? localIndex.length : "null"));
        }
    }
}
