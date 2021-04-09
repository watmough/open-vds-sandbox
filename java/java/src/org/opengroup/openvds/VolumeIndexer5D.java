package org.opengroup.openvds;

public class VolumeIndexer5D extends VolumeIndexerBase {

    private static native long cpCreateVolumeIndexer5D(long pageHandle, int channelIndex, int lod, int dimensions, long layoutHandle);

    public VolumeIndexer5D(VolumeDataPage page, int channelIndex, int lod, int dimensions, VolumeDataLayout layout) {
        super(cpCreateVolumeIndexer5D(page.handle(), channelIndex, lod, dimensions, layout.handle()), 5, true);
    }

    public VolumeIndexer5D(long handle) {
        super(handle, 5, false);
    }

}
