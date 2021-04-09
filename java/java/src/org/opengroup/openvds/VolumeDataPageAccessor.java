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

public class VolumeDataPageAccessor extends JniPointerWithoutDeletion {

    private static native long cpGetLayout( long handle );
    private static native long cpGetChunkCount( long handle );
    private static native void cpGetChunkMinMax( long handle, int chunk, int[] chunkMin, int[] chunkMax);
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
        if (chunk < 0 || chunk >= getChunkCount()) {
            throw new IllegalArgumentException("VolumeDataPageAccessor::getChunkMinMax : wrong chunk index");
        }
        if (chunkMin == null || chunkMin.length != VolumeDataLayout.Dimensionality_Max) {
            throw new IllegalArgumentException("VolumeDataPageAccessor::getChunkMinMax : wrong chunkMin array, expected non null or size 6");
        }
        if (chunkMax == null || chunkMax.length != VolumeDataLayout.Dimensionality_Max) {
            throw new IllegalArgumentException("VolumeDataPageAccessor::getChunkMinMax : wrong chunkMax array, expected non null or size 6");
        }
        cpGetChunkMinMax(_handle, chunk, chunkMin, chunkMax);
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
        return new VolumeDataPage(cpCreatePage(_handle, chunkIndex));
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
        return new VolumeDataPage(cpReadPage(_handle, chunkIndex));
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
