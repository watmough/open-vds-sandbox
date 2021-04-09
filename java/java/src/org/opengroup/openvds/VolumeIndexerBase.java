package org.opengroup.openvds;

public abstract class VolumeIndexerBase extends JniPointer {

    private static native void cpDeleteHandle(long handle, int volumeDimension);
    private static native float cpGetValueRangeMin(long handle, int volumeDimension);
    private static native float cpGetValueRangeMax(long handle, int volumeDimension);
    private static native int cpGetDataBlockNumSamples(long handle, int volumeDimension, int dim);
    private static native int[] cpLocalIndexToVoxelIndex(long handle, int volumeDimension, int[] localIndex);
    private static native int cpLocalIndexToDataIndex(long handle, int volumeDimension, int[] localIndex);

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

    public int getDataBlockNumSamples(int dimension) {
        return cpGetDataBlockNumSamples(_handle, dimensionVolume, dimension);
    }

    public int[] localIndexToVoxelIndex(int[] localIndex) {
        checkLocalIndexArgument(localIndex);
        return cpLocalIndexToVoxelIndex(_handle, dimensionVolume, localIndex);
    }

    public int localIndexToDataIndex(int[] localIndex) {
        checkLocalIndexArgument(localIndex);
        return cpLocalIndexToDataIndex(_handle, dimensionVolume, localIndex);
    }

    private void checkLocalIndexArgument(int[] localIndex) {
        if (localIndex == null || localIndex.length != dimensionVolume) {
            throw new IllegalArgumentException("Local index size must match indexer dimension. Expected " + dimensionVolume + ", got " + (localIndex != null ? localIndex.length : "null"));
        }
    }
}
