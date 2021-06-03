/*
 * Copyright 2019 The Open Group
 * Copyright 2019 INT, Inc.
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

import java.util.Arrays;
import java.util.Optional;

public class VolumeDataPageAccessor extends JniPointerWithoutDeletion {

    public enum AccessMode {
        ReadOnly(0),
        ReadWrite(1),
        Create(2);

        private final int code;

        AccessMode(int code) {
            this.code = code;
        }

        public int getCode() {
            return code;
        }

        public static AccessMode fromCode(int codeParam) {
            Optional<AccessMode> first = Arrays
                    .stream(values())
                    .filter(e -> e.getCode() == codeParam)
                    .findFirst();
            if (!first.isPresent())
                throw new IllegalArgumentException("invalid code");
            return first.get();
        }
    }

    private static native long cpGetLayout( long handle );
    private static native int cpGetLOD( long handle );
    private static native int cpGetChannelIndex(long handle);
    private static native int[] cpGetNumSamples(long handle);
    private static native long cpGetChunkCount( long handle );
    private static native void cpGetChunkMinMax( long handle, int chunk, int[] chunkMin, int[] chunkMax);
    private static native void cpGetChunkMinMaxExcludingMargin( long handle, int chunk, int[] chunkMin, int[] chunkMax);
    private static native long cpGetChunkIndex(long handle, int[] position);
    private static native long cpGetMappedChunkIndex(long handle, long primaryChannelChunkIndex);
    private static native long cpGetPrimaryChannelChunkIndex(long handle, long chunkIndex);
    private static native long cpCreatePage( long handle, long chunkIndex);
    private static native long cpReadPage( long handle, long chunkIndex);
    private static native void cpCommit(long handle);
    private static native void cpSetMaxPage(long handle, int maxPage);

    VolumeDataPageAccessor(long handle) {
        super(handle);
    }

    /**
     * Get volume layout
     * @return
     */
    public VolumeDataLayout getLayout() {
        return new VolumeDataLayout( cpGetLayout(_handle) );
    }

    /**
     * @return the LOD value for this page accessor
     */
    public int getLOD() {
        return cpGetLOD(_handle);
    }

    /**
     * @return the channel index for this page accessor
     */
    public int getChannelIndex() {
        return cpGetChannelIndex(_handle);
    }

    /**
     * Get the num samples for this page accessor on each dimension
     * @return an array of size Dimensionality_Max (or null if error)
     */
    public int[] getNumSamples() {
        return cpGetNumSamples(_handle);
    }


    /**
     * @return the chunk count
     */
    public long getChunkCount() {
        return cpGetChunkCount(_handle);
    }

    /**
     * Get chunk dimension
     * @param chunk chunk index
     * @param chunkMin array that will receive min values
     * @param chunkMax array that will receive max values
     * @throws IllegalArgumentException if chunk min and max array don't have the correct size, or if chunk index is incorrect
     */
    public void getChunkMinMax(int chunk, int[] chunkMin, int[] chunkMax) {
        checkChunkAndArraySize(chunk, chunkMin, chunkMax);
        cpGetChunkMinMax(_handle, chunk, chunkMin, chunkMax);
    }

    /**
     * Get chunk dimension (without margin)
     * @param chunk chunk index
     * @param chunkMin array that will receive min values
     * @param chunkMax array that will receive max values
     * @throws IllegalArgumentException if chunk min and max array don't have the correct size, or if chunk index is incorrect
     */
    public void getChunkMinMaxExcludingMargin(int chunk, int[] chunkMin, int[] chunkMax) {
        checkChunkAndArraySize(chunk, chunkMin, chunkMax);
        cpGetChunkMinMaxExcludingMargin(_handle, chunk, chunkMin, chunkMax);
    }

    /**
     * Get index of chunk for a given position
     * @param position voxel position (array must be VolumeDataLayout.Dimensionality_Max size)
     * @return chunk position
     */
    public long getChunkIndex(int[] position) {
        checkPosition(position);
        return cpGetChunkIndex(_handle, position);
    }

    /**
     * Get the chunk index for this VolumeDataPageAccessor corresponding to the given chunk index in the primary channel.
     * Because some channels can have mappings (e.g. one value per trace), the number of chunks can be less than in the primary
     * channel and we need to have a mapping to figure out the chunk index in each channel that is produced together.
     * @param primaryChannelChunkIndex
     * @return The chunk index for this VolumeDataPageAccessor corresponding to the given chunk index in the primary channel.
     */
    public long getMappedChunkIndex(long primaryChannelChunkIndex) {
        return cpGetMappedChunkIndex(_handle, primaryChannelChunkIndex);
    }

    /**
     * Get the primary channel chunk index corresponding to the given chunk index of this VolumeDataPageAccessor.
     * In order to avoid creating duplicates requests when a channel is mapped, we need to know which primary channel chunk index is representative of
     * a particular mapped chunk index.
     * @param chunkIndex
     * @return The primary channel chunk index corresponding to the given chunk index for this VolumeDataPageAccessor.
     */
    public long getPrimaryChannelChunkIndex(long chunkIndex) {
        return cpGetPrimaryChannelChunkIndex(_handle, chunkIndex);
    }

    /**
     * Create page for given chunk index
     * @param chunkIndex
     * @return A VolumeDataPage
     * @throws IllegalArgumentException if chunkIndex is incorrect
     */
    public VolumeDataPage createPage(long chunkIndex) {
        if (chunkIndex < 0 || chunkIndex >= getChunkCount()) {
            throw new IllegalArgumentException("VolumeDataPageAccessor::createPage : wrong chunk index");
        }
        return new VolumeDataPage(cpCreatePage(_handle, chunkIndex), getLayout().getDimensionality(), getLOD());
    }

    /**
     * Reads page for given chunk index
     * @param chunkIndex
     * @return A VolumeDataPage
     * @throws IllegalArgumentException if chunkIndex is incorrect
     */
    public VolumeDataPage readPage(long chunkIndex) {
        if (chunkIndex < 0 || chunkIndex >= getChunkCount()) {
            throw new IllegalArgumentException("VolumeDataPageAccessor::readPage : wrong chunk index");
        }
        return new VolumeDataPage(cpReadPage(_handle, chunkIndex), getLayout().getDimensionality(), getLOD());
    }

    /**
     * Commit modification
     */
    public void commit() {
        cpCommit(_handle);
    }

    /**
     * Set maximum number of pages
     * @param maxPages page count
     */
    public void setMaxPages(int maxPages) {
        cpSetMaxPage(_handle, maxPages);
    }

    private void checkPosition(int[] position) {
        if (position == null || position.length != VolumeDataLayout.Dimensionality_Max) {
            throw new IllegalArgumentException("VolumeDataPageAccessor::getChunkIndex : wrong position vector");
        }
    }

    private void checkChunkAndArraySize(int chunk, int[] chunkMin, int[] chunkMax) {
        if (chunk < 0 || chunk >= getChunkCount()) {
            throw new IllegalArgumentException("VolumeDataPageAccessor::getChunkMinMax : wrong chunk index");
        }
        if (chunkMin == null || chunkMin.length != VolumeDataLayout.Dimensionality_Max) {
            throw new IllegalArgumentException("VolumeDataPageAccessor::getChunkMinMax : wrong chunkMin array, expected non null or size 6");
        }
        if (chunkMax == null || chunkMax.length != VolumeDataLayout.Dimensionality_Max) {
            throw new IllegalArgumentException("VolumeDataPageAccessor::getChunkMinMax : wrong chunkMax array, expected non null or size 6");
        }
    }
  /*
  virtual int   GetLOD() const = 0;
  virtual int   GetChannelIndex() const = 0;
  virtual VolumeDataChannelDescriptor const &GetChannelDescriptor() const = 0;
  virtual void  GetNumSamples(int (&numSamples)[Dimensionality_Max]) const = 0;

  virtual int64_t GetChunkCount() const = 0;
  virtual void  GetChunkMinMax(int64_t chunk, int (&min)[Dimensionality_Max], int (&max)[Dimensionality_Max]) const = 0;
  virtual void  GetChunkMinMaxExcludingMargin(int64_t chunk, int (&minExcludingMargin)[Dimensionality_Max], int (&maxExcludingMargin)[Dimensionality_Max]) const = 0;
  virtual int64_t GetChunkIndex(const int (&position)[Dimensionality_Max]) const = 0;
  virtual int64_t GetMappedChunkIndex(int64_t primaryChannelChunkIndex) const = 0;
  virtual int64_t GetPrimaryChannelChunkIndex(int64_t chunkIndex) const = 0;

  virtual int   AddReference() = 0;
  virtual int   RemoveReference() = 0;

  virtual int   GetMaxPages() = 0;
  virtual void  SetMaxPages(int maxPages) = 0;

  virtual VolumeDataPage *CreatePage(int64_t chunkIndex) = 0;
  virtual VolumeDataPage *ReadPage(int64_t chunkIndex) = 0;

  VolumeDataPage *ReadPageAtPosition(const int (&position)[Dimensionality_Max]) { return ReadPage(GetChunkIndex(position)); }

  virtual void  Commit() = 0;*/
}
