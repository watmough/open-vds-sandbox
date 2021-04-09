package org.opengroup.openvds;

public class VolumeIndexer2D extends VolumeIndexerBase {

    private static native long cpCreateVolumeIndexer2D(long pageHandle, int channelIndex, int lod, int dimensions, long layoutHandle);

    public VolumeIndexer2D(VolumeDataPage page, int channelIndex, int lod, int dimensions, VolumeDataLayout layout) {
        super(cpCreateVolumeIndexer2D(page.handle(), channelIndex, lod, dimensions, layout.handle()), 2, true);
    }

    public VolumeIndexer2D(long handle) {
        super(handle, 2, false);
    }

}
