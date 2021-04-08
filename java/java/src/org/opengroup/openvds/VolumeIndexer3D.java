package org.opengroup.openvds;

public class VolumeIndexer3D extends VolumeIndexerBase {

    private static native long cpCreateVolumeIndexer3D(long pageHandle, int channelIndex, int lod, int dimensions, long layoutHandle);

    public VolumeIndexer3D(VolumeDataPage page, int channelIndex, int lod, int dimensions, VolumeDataLayout layout) {
        super(cpCreateVolumeIndexer3D(page.handle(), channelIndex, lod, dimensions, layout.handle()), 3, true);
    }

    public VolumeIndexer3D(long handle) {
        super(handle, 3, false);
    }

}
