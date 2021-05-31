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

public class VolumeIndexer4D extends VolumeIndexerBase {

    private static native long cpCreateVolumeIndexer4D(long pageHandle, int channelIndex, int lod, int dimensions, long layoutHandle);

    private static native void cpLocalIndexToVoxelIndex(long handle, int[] voxelIndexRes, int i, int j, int k, int l);
    private static native void cpLocalIndexToLocalChunkIndex(long handle, int[] localChunkIndexRes, int i, int j, int k, int l);

    private static native void cpVoxelIndexToLocalIndex(long handle, int[] localIndexRes, int i, int j, int k, int l);
    private static native void cpVoxelIndexToLocalChunkIndex(long handle, int[] localChunkIndexRes, int i, int j, int k, int l);

    private static native void cpLocalChunkIndexToLocalIndex(long handle, int[] localIndexRes, int i, int j, int k, int l);
    private static native void cpLocalChunkIndexToVoxelIndex(long handle, int[] voxelIndexRes, int i, int j, int k, int l);

    private static native int cpLocalIndexToDataIndex(long handle, int i, int j, int k, int l);
    private static native int cpVoxelIndexToDataIndex(long handle, int i, int j, int k, int l);
    private static native int cpLocalChunkIndexToDataIndex(long handle, int i, int j, int k, int l);

    private static native boolean cpVoxelIndexInProcessArea(long handle, int i, int j, int k, int l);
    private static native boolean cpLocalIndexInProcessArea(long handle, int i, int j, int k, int l);
    private static native boolean cpLocalChunkIndexInProcessArea(long handle, int i, int j, int k, int l);

    /**
     * Constructor
     * @param page
     * @param channelIndex
     * @param lod
     * @param dimensions
     * @param layout
     */
    public VolumeIndexer4D(VolumeDataPage page, int channelIndex, int lod, int dimensions, VolumeDataLayout layout) {
        super(cpCreateVolumeIndexer4D(page.handle(), channelIndex, lod, dimensions, layout.handle()), 4, true);
    }

    /**
     * Constructor
     * @param handle
     */
    public VolumeIndexer4D(long handle) {
        super(handle, 4, false);
    }

    @Override
    public int[] localIndexToVoxelIndex(int[] localIndex) {
        checkIndexArgument(localIndex);
        int[] res = new int[localIndex.length];
        cpLocalIndexToVoxelIndex(_handle, res, localIndex[0], localIndex[1], localIndex[2], localIndex[3]);
        return res;
    }

    @Override
    public int[] localIndexToLocalChunkIndex(int[] localIndex) {
        checkIndexArgument(localIndex);
        int[] res = new int[localIndex.length];
        cpLocalIndexToLocalChunkIndex(_handle, res, localIndex[0], localIndex[1], localIndex[2], localIndex[3]);
        return res;
    }

    @Override
    public int[] voxelIndexToLocalIndex(int[] voxelIndex) {
        checkIndexArgument(voxelIndex);
        int[] res = new int[voxelIndex.length];
        cpVoxelIndexToLocalIndex(_handle, res, voxelIndex[0], voxelIndex[1], voxelIndex[2], voxelIndex[3]);
        return res;
    }

    @Override
    public int[] voxelIndexToLocalChunkIndex(int[] voxelIndex) {
        checkIndexArgument(voxelIndex);
        int[] res = new int[voxelIndex.length];
        cpVoxelIndexToLocalChunkIndex(_handle, res, voxelIndex[0], voxelIndex[1], voxelIndex[2], voxelIndex[3]);
        return res;
    }

    @Override
    public int[] localChunkIndexToLocalIndex(int[] localChunkIndex) {
        checkIndexArgument(localChunkIndex);
        int[] res = new int[localChunkIndex.length];
        cpLocalChunkIndexToLocalIndex(_handle, res, localChunkIndex[0], localChunkIndex[1], localChunkIndex[2], localChunkIndex[3]);
        return res;
    }

    @Override
    public int[] localChunkIndexToVoxelIndex(int[] localChunkIndex) {
        checkIndexArgument(localChunkIndex);
        int[] res = new int[localChunkIndex.length];
        cpLocalChunkIndexToVoxelIndex(_handle, res, localChunkIndex[0], localChunkIndex[1], localChunkIndex[2], localChunkIndex[3]);
        return res;
    }

    @Override
    public int localIndexToDataIndex(int[] localIndex) {
        checkIndexArgument(localIndex);
        return cpLocalIndexToDataIndex(_handle, localIndex[0], localIndex[1], localIndex[2], localIndex[3]);
    }

    @Override
    public int voxelIndexToDataIndex(int[] voxelIndex) {
        checkIndexArgument(voxelIndex);
        return cpVoxelIndexToDataIndex(_handle, voxelIndex[0], voxelIndex[1], voxelIndex[2], voxelIndex[3]);
    }

    @Override
    public int localChunkIndexToDataIndex(int[] localChunkIndex) {
        checkIndexArgument(localChunkIndex);
        return cpLocalChunkIndexToDataIndex(_handle, localChunkIndex[0], localChunkIndex[1], localChunkIndex[2], localChunkIndex[3]);
    }

    @Override
    public boolean voxelIndexInProcessArea(int[] voxelIndex) {
        checkIndexArgument(voxelIndex);
        return cpVoxelIndexInProcessArea(_handle, voxelIndex[0], voxelIndex[1], voxelIndex[2], voxelIndex[3]);
    }

    @Override
    public boolean localIndexInProcessArea(int[] localIndex) {
        checkIndexArgument(localIndex);
        return cpLocalIndexInProcessArea(_handle, localIndex[0], localIndex[1], localIndex[2], localIndex[3]);
    }

    @Override
    public boolean localChunkIndexInProcessArea(int[] localChunkIndex) {
        checkIndexArgument(localChunkIndex);
        return cpLocalChunkIndexInProcessArea(_handle, localChunkIndex[0], localChunkIndex[1], localChunkIndex[2], localChunkIndex[3]);
    }
}
