package org.opengroup.openvds;

public class VolumeIndexer6D extends VolumeIndexerBase {

    private static native long cpCreateVolumeIndexer6D(long pageHandle, int channelIndex, int lod, int dimensions, long layoutHandle);

    public VolumeIndexer6D(VolumeDataPage page, int channelIndex, int lod, int dimensions, VolumeDataLayout layout) {
        super(cpCreateVolumeIndexer6D(page.handle(), channelIndex, lod, dimensions, layout.handle()), 6, true);
    }

    public VolumeIndexer6D(long handle) {
        super(handle, 6, false);
    }

}
