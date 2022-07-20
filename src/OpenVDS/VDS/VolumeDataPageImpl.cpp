/****************************************************************************
** Copyright 2019 The Open Group
** Copyright 2019 Bluware, Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
****************************************************************************/

#include "VolumeDataPageImpl.h"
#include "VolumeDataPageAccessorImpl.h"
#include "VolumeDataChunk.h"
#include "VolumeDataLayer.h"
#include "VolumeDataAccessManagerImpl.h"
#include "VolumeDataStore.h"
#include "VolumeDataLayoutImpl.h"
#include <OpenVDS/VolumeDataChannelDescriptor.h>

#include <algorithm>

namespace OpenVDS
{


template <bool isTargetDim0Contigous, bool isSourceDim0Contigous, typename T>
static void dataBlock_BlockCopyWithExplicitContiguity(T * __restrict ptTarget, const T * __restrict ptSource, int32_t iTargetIndex, int32_t iSourceIndex, int32_t (&targetPitch)[4], int32_t (&sourcePitch)[4], int32_t (&size)[4])
{
  assert(!isTargetDim0Contigous || targetPitch[0] == 1);
  assert(!isSourceDim0Contigous || sourcePitch[0] == 1);

  int32_t
    iSourceIndex3 = iSourceIndex,
    iTargetIndex3 = iTargetIndex;

  for(int32_t iDim3 = 0; iDim3 < size[3]; iDim3++, iSourceIndex3 += sourcePitch[3], iTargetIndex3 += targetPitch[3])
  {
    int32_t
      iSourceIndex2 = iSourceIndex3,
      iTargetIndex2 = iTargetIndex3;

    for(int32_t iDim2 = 0; iDim2 < size[2]; iDim2++, iSourceIndex2 += sourcePitch[2], iTargetIndex2 += targetPitch[2])
    {
      int32_t
        iSourceIndex1 = iSourceIndex2,
        iTargetIndex1 = iTargetIndex2;

      for(int32_t iDim1 = 0; iDim1 < size[1]; iDim1++, iSourceIndex1 += sourcePitch[1], iTargetIndex1 += targetPitch[1])
      {
        int32_t
          iSourceIndex0 = iSourceIndex1,
          iTargetIndex0 = iTargetIndex1;

        if(isTargetDim0Contigous && isSourceDim0Contigous)
        {
          for(int32_t iDim0 = 0; iDim0 < size[0]; iDim0++, iSourceIndex0++, iTargetIndex0++)
          {
            WriteElement(ptTarget, iTargetIndex0, ReadElement(ptSource, iSourceIndex0));
          }
        }
        else if(isTargetDim0Contigous)
        {
          for(int32_t iDim0 = 0; iDim0 < size[0]; iDim0++, iSourceIndex0 += sourcePitch[0], iTargetIndex0++)
          {
            WriteElement(ptTarget, iTargetIndex0, ReadElement(ptSource, iSourceIndex0));
          }
        }
        else if(isSourceDim0Contigous)
        {
          for(int32_t iDim0 = 0; iDim0 < size[0]; iDim0++, iSourceIndex0++, iTargetIndex0 += targetPitch[0])
          {
            WriteElement(ptTarget, iTargetIndex0, ReadElement(ptSource, iSourceIndex0));
          }
        }
        else
        {
          for(int32_t iDim0 = 0; iDim0 < size[0]; iDim0++, iSourceIndex0 += sourcePitch[0], iTargetIndex0 += targetPitch[0])
          {
            WriteElement(ptTarget, iTargetIndex0, ReadElement(ptSource, iSourceIndex0));
          }
        }
      }
    }
  }
}

VolumeDataPageImpl::VolumeDataPageImpl(VolumeDataPageAccessorImpl* volumeDataPageAccessor, int64_t chunk, VolumeDataPageImpl *parentPage)
  : m_volumeDataPageAccessor(volumeDataPageAccessor)
  , m_chunk(chunk)
  , m_parentPage(parentPage)
  , m_blob()
  , m_hash(VolumeDataHash::UNKNOWN)
  , m_pins(1)
  , m_settingData(0)
  , m_isReadWrite(false)
  , m_isDirty(false)
  , m_requestPrepared(true)
  , m_jobID(-1)
  , m_chunksCopiedTo(0)
{
  for (int32_t dimension = 0; dimension < Dimensionality_Max; dimension++)
  {
    m_sizeND[dimension] = 0;
    m_pitchND[dimension] = 0;
    m_writtenMin[dimension] = 0;
    m_writtenMax[dimension] = 0;
  }

  memset(m_copiedToChunkIndexes, 0, sizeof(m_copiedToChunkIndexes));
}

VolumeDataPageImpl::~VolumeDataPageImpl()
{
  if(m_parentPage)
  {
    m_parentPage->Release();
  }  
}

  // All these methods require the caller to hold a lock
bool VolumeDataPageImpl::IsPinned()
{
  //assert(m_volumeDataPageAccessor->m_pageListMutex.isLockedByCurrentThread());
  assert(m_pins >= 0);
  return m_pins > 0;
}

void VolumeDataPageImpl::Pin()
{
  //assert(m_volumeDataPageAccessor->m_pageListMutex.isLockedByCurrentThread());
  assert(m_pins >= 0);
  m_pins++;
}
void VolumeDataPageImpl::UnPin()
{
  //assert(m_volumeDataPageAccessor->m_pageListMutex.isLockedByCurrentThread());
  m_pins--;
  assert(m_pins >= 0);
}

bool VolumeDataPageImpl::IsEmpty()
{
  //assert(m_volumeDataPageAccessor->m_pageListMutex.isLockedByCurrentThread());
  return m_blob.empty();
}

bool VolumeDataPageImpl::IsCanceled()
{
  //assert(m_volumeDataPageAccessor->m_pageListMutex.isLockedByCurrentThread());
  return m_error.code != 0;
}

bool VolumeDataPageImpl::IsDirty()
{
  //assert(m_volumeDataPageAccessor->m_pageListMutex.isLockedByCurrentThread());
  return m_isDirty;
}

bool VolumeDataPageImpl::IsWritten()
{
  //assert(m_volumeDataPageAccessor->m_pageListMutex.isLockedByCurrentThread());
  return !(m_writtenMin[0] == 0 && m_writtenMax[0] == 0);
}

void VolumeDataPageImpl::MakeDirty()
{
  //assert(m_volumeDataPageAccessor->m_pageListMutex.isLockedByCurrentThread());
  m_isDirty = true;
}

void VolumeDataPageImpl::SetBufferData(const DataBlock &dataBlock, DimensionGroup chunkDimensionGroup, std::vector<uint8_t>&& blob, uint64_t hash)
{
  //assert(m_volumeDataPageAccessor->m_pageListMutex.isLockedByCurrentThread());
  m_dataBlock = dataBlock;

  for (int32_t dimension = 0; dimension < Dimensionality_Max; dimension++)
  {
    m_sizeND[dimension] = 1;
    m_pitchND[dimension] = 0;
  }

  const int chunkDimensionality =  DimensionGroupUtil::GetDimensionality(chunkDimensionGroup);

  for(int chunkDimension = 0; chunkDimension < chunkDimensionality; chunkDimension++)
  {
    int dimension = DimensionGroupUtil::GetDimension(chunkDimensionGroup, chunkDimension);

    assert(dimension >= 0 && dimension < Dimensionality_Max);
    m_sizeND[dimension] = dataBlock.Size[chunkDimension];

    if(dataBlock.Format == VolumeDataFormat::Format_1Bit && chunkDimension > 0)
    {
      // Convert pitch to bitpitch for 1-bit data
      m_pitchND[dimension] = dataBlock.Pitch[chunkDimension] * 8;
    }
    else
    {
      m_pitchND[dimension] = dataBlock.Pitch[chunkDimension];
    }
  }

  m_blob = std::move(blob);
  m_hash = hash;
}

void VolumeDataPageImpl::WriteBack(VolumeDataLayer const* volumeDataLayer, std::unique_lock<std::mutex>& pageListMutexLock)
{
  assert(m_isDirty);
  if(m_parentPage)
  {
    WriteIntoLOD();
  }
  m_volumeDataPageAccessor->RequestWritePage(m_chunk, m_dataBlock, m_blob, m_hash);
  auto layout = const_cast<VolumeDataLayoutImpl*>(static_cast<VolumeDataLayoutImpl const*>(m_volumeDataPageAccessor->GetLayout()));
  m_isDirty = false;
  layout->CompletePendingWriteChunkRequests(16);
}

void* VolumeDataPageImpl::GetBufferInternal(int(&sizeND)[Dimensionality_Max], int(&pitchND)[Dimensionality_Max], bool isReadWrite)
{
  //assert(m_volumeDataPageAccessor->m_pageListMutex.isLockedByCurrentThread());

  for(int32_t dimension = 0; dimension < Dimensionality_Max; dimension++)
  {
    sizeND[dimension] = m_sizeND[dimension];
    pitchND[dimension] = m_pitchND[dimension];
  }

  if(isReadWrite)
  {
    m_isReadWrite = true;
  }

  return m_blob.data();
}

bool VolumeDataPageImpl::IsCopyMarginNeeded(VolumeDataPageImpl* targetPage)
{
  if(targetPage == this) return false;

  int32_t targetMin[Dimensionality_Max];
  int32_t targetMax[Dimensionality_Max];

  targetPage->GetMinMax(targetMin, targetMax);

  int32_t overlapMin[Dimensionality_Max];
  int32_t overlapMax[Dimensionality_Max];
  int32_t overlapSize[Dimensionality_Max];

  // Check if there is any overlap between the written region and the target page
  for(int32_t dimension = 0; dimension < Dimensionality_Max; dimension++)
  {
    overlapMin[dimension] = m_writtenMin[dimension] >= targetMin[dimension] ? m_writtenMin[dimension] : targetMin[dimension];
    overlapMax[dimension] = m_writtenMax[dimension] <= targetMax[dimension] ? m_writtenMax[dimension] : targetMax[dimension];
    overlapSize[dimension] = overlapMax[dimension] - overlapMin[dimension];
    if(overlapSize[dimension] <= 0)
    {
      return false;
    }
  }

  // Check if the margins have already been copied
  for(int32_t copiedToChunk = 0; copiedToChunk < m_chunksCopiedTo; copiedToChunk++)
  {
    if(targetPage->GetChunkIndex() == m_copiedToChunkIndexes[copiedToChunk])
    {
      return false;
    }
  }

  return true;
}

void VolumeDataPageImpl::CopyMargin(VolumeDataPageImpl* targetPage)
{
  //assert(m_volumeDataPageAccessor->m_pageListMutex.isLockedByCurrentThread());
  assert(IsDirty());
  assert(!targetPage->IsEmpty() && "The caller have to ensure the target page is finished reading");

  int32_t sourceMin[Dimensionality_Max];
  int32_t sourceMax[Dimensionality_Max];

  GetMinMax(sourceMin, sourceMax);

  int32_t targetMin[Dimensionality_Max];
  int32_t targetMax[Dimensionality_Max];

  targetPage->GetMinMax(targetMin, targetMax);

  int32_t overlapMin[Dimensionality_Max];
  int32_t overlapMax[Dimensionality_Max];
  int32_t overlapSize[Dimensionality_Max];

  for(int32_t dimension = 0; dimension < Dimensionality_Max; dimension++)
  {
    overlapMin[dimension] = m_writtenMin[dimension] >= targetMin[dimension] ? m_writtenMin[dimension] : targetMin[dimension];
    overlapMax[dimension] = m_writtenMax[dimension] <= targetMax[dimension] ? m_writtenMax[dimension] : targetMax[dimension];
    overlapSize[dimension] = overlapMax[dimension] - overlapMin[dimension];
  }

  int32_t sourceSize[Dimensionality_Max];
  int32_t sourcePitch[Dimensionality_Max];
  int32_t targetSize[Dimensionality_Max];
  int32_t targetPitch[Dimensionality_Max];

  const void * sourceBuffer = GetBufferInternal(sourceSize, sourcePitch, false);

  void *targetBuffer = static_cast<VolumeDataPageImpl*>(targetPage)->GetBufferInternal(targetSize, targetPitch, true);

  int32_t iSourceIndex = 0;
  int32_t iTargetIndex = 0;

  for(int32_t dimension = 0; dimension < Dimensionality_Max; dimension++)
  {
    iSourceIndex += (overlapMin[dimension] - sourceMin[dimension]) * sourcePitch[dimension];
    iTargetIndex += (overlapMin[dimension] - targetMin[dimension]) * targetPitch[dimension];
  }

  int32_t remappedSourcePitch[4];
  int32_t remappedTargetPitch[4];
  int32_t remappedOverlapSize[4];

  for(int32_t iChunkDimension = 0; iChunkDimension < 4; iChunkDimension++)
  {
    int32_t dimension = m_volumeDataPageAccessor->GetLayer()->GetChunkDimension(iChunkDimension);

    if(dimension == -1)
    {
      remappedSourcePitch[iChunkDimension] = 0;
      remappedTargetPitch[iChunkDimension] = 0;
      remappedOverlapSize[iChunkDimension] = 1;
    }
    else
    {
      remappedSourcePitch[iChunkDimension] = sourcePitch[dimension];
      remappedTargetPitch[iChunkDimension] = targetPitch[dimension];
      remappedOverlapSize[iChunkDimension] = overlapSize[dimension];
    }
  }

  switch(m_volumeDataPageAccessor->GetChannelDescriptor().GetFormat())
  {
  default:
    assert(0 && "Unsupported format");
    break;
  case VolumeDataChannelDescriptor::Format_1Bit:
    dataBlock_BlockCopyWithExplicitContiguity<true, true>((bool *)targetBuffer, (bool *)sourceBuffer, iTargetIndex, iSourceIndex, remappedTargetPitch, remappedSourcePitch, remappedOverlapSize);
    break;

  case VolumeDataChannelDescriptor::Format_U8:
    dataBlock_BlockCopyWithExplicitContiguity<true, true>((uint8_t *)targetBuffer, (uint8_t *)sourceBuffer, iTargetIndex, iSourceIndex, remappedTargetPitch, remappedSourcePitch, remappedOverlapSize);
    break;

  case VolumeDataChannelDescriptor::Format_U16:
    dataBlock_BlockCopyWithExplicitContiguity<true, true>((uint16_t *)targetBuffer, (uint16_t *)sourceBuffer, iTargetIndex, iSourceIndex, remappedTargetPitch, remappedSourcePitch, remappedOverlapSize);
    break;

  case VolumeDataChannelDescriptor::Format_R32:
  case VolumeDataChannelDescriptor::Format_U32:
    dataBlock_BlockCopyWithExplicitContiguity<true, true>((uint32_t *)targetBuffer, (uint32_t *)sourceBuffer, iTargetIndex, iSourceIndex, remappedTargetPitch, remappedSourcePitch, remappedOverlapSize);
    break;

  case VolumeDataChannelDescriptor::Format_R64:
  case VolumeDataChannelDescriptor::Format_U64:
    dataBlock_BlockCopyWithExplicitContiguity<true, true>((uint64_t *)targetBuffer, (uint64_t *)sourceBuffer, iTargetIndex, iSourceIndex, remappedTargetPitch, remappedSourcePitch, remappedOverlapSize);
    break;
  }

  targetPage->MakeDirty();

  assert(m_chunksCopiedTo < ArraySize(m_copiedToChunkIndexes));
  m_copiedToChunkIndexes[m_chunksCopiedTo++] = targetPage->GetChunkIndex();
}

void VolumeDataPageImpl::WriteIntoLOD()
{
  if(m_parentPage == nullptr) return;

  if(VolumeDataHash_IsConstant(m_hash) && m_hash == m_parentPage->m_hash) return;

  VolumeDataLayer const *childLayer = m_volumeDataPageAccessor->GetLayer();
  VolumeDataLayer const *parentLayer = m_parentPage->m_volumeDataPageAccessor->GetLayer();

  int
    parentLOD = parentLayer->GetLOD(),
    childLOD  = childLayer->GetLOD();

  int
    parentMin[Dimensionality_Max],
    parentMax[Dimensionality_Max],
    parentMinExcludingMargin[Dimensionality_Max],
    parentMaxExcludingMargin[Dimensionality_Max],
    childMin[Dimensionality_Max],
    childMax[Dimensionality_Max],
    childMinExcludingMargin[Dimensionality_Max],
    childMaxExcludingMargin[Dimensionality_Max];

  m_parentPage->GetMinMax(parentMin, parentMax);
  m_parentPage->GetMinMaxExcludingMargin(parentMinExcludingMargin, parentMaxExcludingMargin);

  GetMinMax(childMin, childMax);
  GetMinMaxExcludingMargin(childMinExcludingMargin, childMaxExcludingMargin);

  int
    targetOffset[3],
    targetSize[3],
    sourceOffset[3];

  int
    fullResolutionDataBlockDimension = -1;

  for(int dataBlockDimension = 0; dataBlockDimension < 3; dataBlockDimension++)
  {
    int dimension = childLayer->GetChunkDimension(dataBlockDimension);

    assert(dimension == parentLayer->GetChunkDimension(dataBlockDimension));

    if(dimension == -1)
    {
      targetOffset[dataBlockDimension] = 0;
      targetSize[dataBlockDimension] = 1;
      sourceOffset[dataBlockDimension] = 0;
    }
    else
    {
      int
        sourceMin, sourceMax;

      if(childMin[dimension] < parentMinExcludingMargin[dimension])
      {
        sourceMin = childMin[dimension];
      }
      else
      {
        sourceMin = childMinExcludingMargin[dimension];
      }

      if (parentLayer->IsDimensionLODDecimated(dimension))
      {
        targetOffset[dataBlockDimension] = (sourceMin - parentMin[dimension]) >> parentLOD;
        sourceOffset[dataBlockDimension] = (sourceMin - childMin[dimension]) >> childLOD;

        if (childMax[dimension] >= parentMaxExcludingMargin[dimension])
        {
          sourceMax = childMax[dimension];
          targetSize[dataBlockDimension] = GetLODSize(sourceMin, sourceMax, parentLOD, true);
        }
        else
        {
          sourceMax = childMaxExcludingMargin[dimension];
          targetSize[dataBlockDimension] = GetLODSize(sourceMin, sourceMax, parentLOD, false);
        }
      }
      else
      {
        fullResolutionDataBlockDimension = dataBlockDimension;

        targetOffset[dataBlockDimension] = sourceMin - parentMin[dimension];
        sourceOffset[dataBlockDimension] = sourceMin - childMin[dimension];

        if (childMax[dimension] >= parentMaxExcludingMargin[dimension])
        {
          sourceMax = childMax[dimension];
          targetSize[dataBlockDimension] = sourceMax - sourceMin;
        }
        else
        {
          sourceMax = childMaxExcludingMargin[dimension];
          targetSize[dataBlockDimension] = sourceMax - sourceMin;
        }
      }
    }
  }

  DownSampleAndCopyRegion(m_parentPage->GetDataBlock(), GetDataBlock(), m_parentPage->GetRawBufferInternal(), GetRawBufferInternal(),
                          targetOffset[0], targetOffset[1], targetOffset[2],
                          targetSize[0], targetSize[1], targetSize[2],
                          sourceOffset[0], sourceOffset[1], sourceOffset[2], parentLayer->GetNoValue(), fullResolutionDataBlockDimension);
}

// Implementation of Hue::HueSpaceLib::VolumeDataPage interface, these methods aquire a lock (except the GetMinMax methods which don't need to)
VolumeDataPageAccessor &VolumeDataPageImpl::GetVolumeDataPageAccessor() const
{
  return *m_volumeDataPageAccessor;
}
void  VolumeDataPageImpl::GetMinMax(int(&min)[Dimensionality_Max], int(&max)[Dimensionality_Max]) const
{
  m_volumeDataPageAccessor->GetLayer()->GetChunkMinMax(m_chunk, min, max, true);
}
void  VolumeDataPageImpl::GetMinMaxExcludingMargin(int(&minExcludingMargin)[Dimensionality_Max], int(&maxExcludingMargin)[Dimensionality_Max]) const
{
  m_volumeDataPageAccessor->GetLayer()->GetChunkMinMax(m_chunk, minExcludingMargin, maxExcludingMargin, false);
}
ReadErrorException VolumeDataPageImpl::GetError() const
{
  return ReadErrorException(m_error.string.c_str(), m_error.code);
}
const void* VolumeDataPageImpl::GetBuffer(int(&size)[Dimensionality_Max], int(&pitch)[Dimensionality_Max])
{
  std::unique_lock<std::mutex>  pageListMutexLock(const_cast<VolumeDataPageAccessorImpl *>(m_volumeDataPageAccessor)->m_pagesMutex);

  return GetBufferInternal(size, pitch, m_volumeDataPageAccessor->IsReadWrite());
}
void* VolumeDataPageImpl::GetWritableBuffer(int(&size)[Dimensionality_Max], int(&pitch)[Dimensionality_Max])
{
 std::unique_lock<std::mutex>  pageListMutexLock(const_cast<VolumeDataPageAccessorImpl *>(m_volumeDataPageAccessor)->m_pagesMutex);

  if(!m_volumeDataPageAccessor->IsReadWrite())
  {
    throw std::runtime_error("Trying to get a writable buffer from a read-only volume data page accessor");
  }

  // This will throw if we can't acquire the write lock
  m_volumeDataPageAccessor->AcquireLayerWriteLock();

  return GetBufferInternal(size, pitch, true);
}

void VolumeDataPageImpl::UpdateWrittenRegion(const int(&writtenMin)[Dimensionality_Max], const int(&writtenMax)[Dimensionality_Max])
{
 std::unique_lock<std::mutex>  pageListMutexLock(const_cast<VolumeDataPageAccessorImpl *>(m_volumeDataPageAccessor)->m_pagesMutex);

  if(!m_isReadWrite)
  {
    throw std::runtime_error("Cannot update the written area of a volume data page which isn't writable");
  }

  // Expand written area
  if(m_writtenMin[0] == 0 && m_writtenMax[0] == 0)
  {
    for(int32_t dimension = 0; dimension < Dimensionality_Max; dimension++)
    {
      m_writtenMin[dimension] = writtenMin[dimension];
      m_writtenMax[dimension] = writtenMax[dimension];
    }
  }
  else
  {
    for(int32_t dimension = 0; dimension < Dimensionality_Max; dimension++)
    {
      m_writtenMin[dimension] = std::min(m_writtenMin[dimension], writtenMin[dimension]);
      m_writtenMax[dimension] = std::max(m_writtenMax[dimension], writtenMax[dimension]);
    }
  }

  // Clear the copied to chunk list
  memset(m_copiedToChunkIndexes, 0, sizeof(m_copiedToChunkIndexes));
  m_chunksCopiedTo = 0;

  MakeDirty();
}

void VolumeDataPageImpl::Release()
{
 std::unique_lock<std::mutex>  pageListMutexLock(const_cast<VolumeDataPageAccessorImpl *>(m_volumeDataPageAccessor)->m_pagesMutex);
 if(m_isDirty)
 {
   m_hash = VolumeDataHash::GetUniqueHash();
 }
 UnPin();
}

}
