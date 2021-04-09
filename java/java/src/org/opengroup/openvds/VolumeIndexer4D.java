package org.opengroup.openvds;

public class VolumeIndexer4D extends VolumeIndexerBase {

    private static native long cpCreateVolumeIndexer4D(long pageHandle, int channelIndex, int lod, int dimensions, long layoutHandle);

    public VolumeIndexer4D(VolumeDataPage page, int channelIndex, int lod, int dimensions, VolumeDataLayout layout) {
        super(cpCreateVolumeIndexer4D(page.handle(), channelIndex, lod, dimensions, layout.handle()), 4, true);
    }

    public VolumeIndexer4D(long handle) {
        super(handle, 4, false);
    }

}
