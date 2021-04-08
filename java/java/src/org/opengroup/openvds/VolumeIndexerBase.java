package org.opengroup.openvds;

public abstract class VolumeIndexerBase extends JniPointer {

    private static native void cpDeleteHandle(long handle, int volumeDimension);
    private static native float cpGetValueRangeMin(long handle, int volumeDimension);
    private static native float cpGetValueRangeMax(long handle, int volumeDimension);
    private static native int cpGetDataBlockNumSamples(long handle, int volumeDimension, int dim);

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
}
