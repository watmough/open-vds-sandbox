package org.opengroup.openvds;

public class VolumeIndexer5D extends VolumeIndexerBase {

    private static native long cpCreateVolumeIndexer5D(long pageHandle, int channelIndex, int lod, int dimensions, long layoutHandle);

    private static native void cpLocalIndexToVoxelIndex(long handle, int[] voxelIndexRes, int i, int j, int k, int l, int n);
    private static native void cpLocalIndexToLocalChunkIndex(long handle, int[] localChunkIndexRes, int i, int j, int k, int l, int n);

    private static native void cpVoxelIndexToLocalIndex(long handle, int[] localIndexRes, int i, int j, int k, int l, int n);
    private static native void cpVoxelIndexToLocalChunkIndex(long handle, int[] localChunkIndexRes, int i, int j, int k, int l, int n);

    private static native void cpLocalChunkIndexToLocalIndex(long handle, int[] localIndexRes, int i, int j, int k, int l, int n);
    private static native void cpLocalChunkIndexToVoxelIndex(long handle, int[] voxelIndexRes, int i, int j, int k, int l, int n);

    private static native int cpLocalIndexToDataIndex(long handle, int i, int j, int k, int l, int n);
    private static native int cpVoxelIndexToDataIndex(long handle, int i, int j, int k, int l, int n);
    private static native int cpLocalChunkIndexToDataIndex(long handle, int i, int j, int k, int l, int n);

    private static native boolean cpVoxelIndexInProcessArea(long handle, int i, int j, int k, int l, int n);
    private static native boolean cpLocalIndexInProcessArea(long handle, int i, int j, int k, int l, int n);
    private static native boolean cpLocalChunkIndexInProcessArea(long handle, int i, int j, int k, int l, int n);

    /**
     * Constructor
     * @param page
     * @param channelIndex
     * @param lod
     * @param dimensions
     * @param layout
     */
    public VolumeIndexer5D(VolumeDataPage page, int channelIndex, int lod, int dimensions, VolumeDataLayout layout) {
        super(cpCreateVolumeIndexer5D(page.handle(), channelIndex, lod, dimensions, layout.handle()), 5, true);
    }

    /**
     * Constructor
     * @param handle
     */
    public VolumeIndexer5D(long handle) {
        super(handle, 5, false);
    }

    @Override
    public int[] localIndexToVoxelIndex(int[] localIndex) {
        checkIndexArgument(localIndex);
        int[] res = new int[localIndex.length];
        cpLocalIndexToVoxelIndex(_handle, res, localIndex[0], localIndex[1], localIndex[2], localIndex[3], localIndex[4]);
        return res;
    }

    @Override
    public int[] localIndexToLocalChunkIndex(int[] localIndex) {
        checkIndexArgument(localIndex);
        int[] res = new int[localIndex.length];
        cpLocalIndexToLocalChunkIndex(_handle, res, localIndex[0], localIndex[1], localIndex[2], localIndex[3], localIndex[4]);
        return res;
    }

    @Override
    public int[] voxelIndexToLocalIndex(int[] voxelIndex) {
        checkIndexArgument(voxelIndex);
        int[] res = new int[voxelIndex.length];
        cpVoxelIndexToLocalIndex(_handle, res, voxelIndex[0], voxelIndex[1], voxelIndex[2], voxelIndex[3], voxelIndex[4]);
        return res;
    }

    @Override
    public int[] voxelIndexToLocalChunkIndex(int[] voxelIndex) {
        checkIndexArgument(voxelIndex);
        int[] res = new int[voxelIndex.length];
        cpVoxelIndexToLocalChunkIndex(_handle, res, voxelIndex[0], voxelIndex[1], voxelIndex[2], voxelIndex[3], voxelIndex[4]);
        return res;
    }

    @Override
    public int[] localChunkIndexToLocalIndex(int[] localChunkIndex) {
        checkIndexArgument(localChunkIndex);
        int[] res = new int[localChunkIndex.length];
        cpLocalChunkIndexToLocalIndex(_handle, res, localChunkIndex[0], localChunkIndex[1], localChunkIndex[2], localChunkIndex[3], localChunkIndex[4]);
        return res;
    }

    @Override
    public int[] localChunkIndexToVoxelIndex(int[] localChunkIndex) {
        checkIndexArgument(localChunkIndex);
        int[] res = new int[localChunkIndex.length];
        cpLocalChunkIndexToVoxelIndex(_handle, res, localChunkIndex[0], localChunkIndex[1], localChunkIndex[2], localChunkIndex[3], localChunkIndex[4]);
        return res;
    }

    @Override
    public int localIndexToDataIndex(int[] localIndex) {
        checkIndexArgument(localIndex);
        return cpLocalIndexToDataIndex(_handle, localIndex[0], localIndex[1], localIndex[2], localIndex[3], localIndex[4]);
    }

    @Override
    public int voxelIndexToDataIndex(int[] voxelIndex) {
        checkIndexArgument(voxelIndex);
        return cpVoxelIndexToDataIndex(_handle, voxelIndex[0], voxelIndex[1], voxelIndex[2], voxelIndex[3], voxelIndex[4]);
    }

    @Override
    public int localChunkIndexToDataIndex(int[] localChunkIndex) {
        checkIndexArgument(localChunkIndex);
        return cpLocalChunkIndexToDataIndex(_handle, localChunkIndex[0], localChunkIndex[1], localChunkIndex[2], localChunkIndex[3], localChunkIndex[4]);
    }

    @Override
    public boolean voxelIndexInProcessArea(int[] voxelIndex) {
        checkIndexArgument(voxelIndex);
        return cpVoxelIndexInProcessArea(_handle, voxelIndex[0], voxelIndex[1], voxelIndex[2], voxelIndex[3], voxelIndex[4]);
    }

    @Override
    public boolean localIndexInProcessArea(int[] localIndex) {
        checkIndexArgument(localIndex);
        return cpLocalIndexInProcessArea(_handle, localIndex[0], localIndex[1], localIndex[2], localIndex[3], localIndex[4]);
    }

    @Override
    public boolean localChunkIndexInProcessArea(int[] localChunkIndex) {
        checkIndexArgument(localChunkIndex);
        return cpLocalChunkIndexInProcessArea(_handle, localChunkIndex[0], localChunkIndex[1], localChunkIndex[2], localChunkIndex[3], localChunkIndex[4]);
    }
}
