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

#include "VolumeDataAccessManagerImpl.h"

#include "VDS.h"
#include "VolumeDataPageAccessorImpl.h"
#include "VolumeDataAccessor.h"
#include <OpenVDS/ValueConversion.h>
#include <OpenVDS/VolumeSampler.h>
#include "ParseVDSJson.h"
#include "VolumeDataStore.h"
#include "VolumeDataHash.h"
#include "VolumeDataLayoutImpl.h"
#include "VolumeDataPageAccessorImpl.h"
#include "Logging.h"

#include <algorithm>
#include <inttypes.h>
#include <assert.h>
#include <atomic>
#include <fmt/format.h>

namespace OpenVDS
{

VolumeDataAccessManagerImpl::VolumeDataAccessManagerImpl(VDS &vds)
  : m_refCount(0)
  , m_invalidated(false)
  , m_copyJobIndex(false)
  , m_vds(vds)
  , m_requestProcessor(new VolumeDataRequestProcessor(*this, vds.logger))
{
}

VolumeDataAccessManagerImpl::~VolumeDataAccessManagerImpl()
{
  if (m_uploadErrors.errors.size())
  {
    m_vds.logger.LogWarning("VolumeDataAccessManager destructor: there where upload errors");
  }
}

VolumeDataStore * VolumeDataAccessManagerImpl::GetVolumeDataStore()
{
  return m_vds.volumeDataStore.get();
}

static void
ValidateBuffer(void* buffer, int64_t size, int64_t required_size = 0)
{
  if (buffer == nullptr) 
  {
    throw InvalidArgument("Buffer is null", "buffer");
  }
  if (size < required_size)
  {
    throw InvalidArgument("Buffer size is less than the required size", "size");
  }
}

static int64_t
RaiseInvalidManagerException()
{
  throw InvalidOperation("Invalid Access Manager");
}

void
VolumeDataAccessManagerImpl::ValidateRequest(int64_t requestID)
{
  if (!m_requestProcessor->IsActive(requestID))
  {
    throw InvalidOperation("Request ID is not active.");
  }
}

VolumeDataAccessManagerImpl*  
VolumeDataAccessManagerImpl::Create(VDS &vds)
{
  auto volumeDataAccessManager = new VolumeDataAccessManagerImpl(vds);
  ++volumeDataAccessManager->m_refCount;
  assert(volumeDataAccessManager->m_refCount == 1);
  return volumeDataAccessManager;
}

bool                                  
VolumeDataAccessManagerImpl::IsValid()
{
  if (!m_invalidated)
  {
    if (PrivateGetLayout() == nullptr)
    {
      Invalidate();
    }
  }
  return !m_invalidated;
}

void                                  
VolumeDataAccessManagerImpl::Invalidate()
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_invalidated = true;
  m_requestProcessor.reset();
  while (auto volumeDataPageAccessor = m_volumeDataPageAccessorList.GetFirstItem())
  {
    m_volumeDataPageAccessorList.Remove(volumeDataPageAccessor);
    delete volumeDataPageAccessor;
  }
}

void
VolumeDataAccessManagerImpl::AddRef()
{
  ++m_refCount;
  assert(m_refCount > 0);
}

void
VolumeDataAccessManagerImpl::Release()
{
  assert(m_refCount > 0);
  if (--m_refCount == 0)
  {
    assert(m_invalidated);
    delete this;
  }
}

int
VolumeDataAccessManagerImpl::RefCount() const
{
  return m_refCount;
}

VolumeDataLayoutImpl const*
VolumeDataAccessManagerImpl::PrivateGetLayout()
{
  return m_vds.volumeDataLayout.get();
}

VolumeDataLayer const *
VolumeDataAccessManagerImpl::PrivateGetLayer(DimensionsND dimensionsND, int channel, int LOD)
{
  VolumeDataLayoutImpl const *
    volumeDataLayout = PrivateGetLayout();

  if(!volumeDataLayout)
  {
    throw InvalidOperation("The VDS doesn't have a volume data layout, this is usually because the VDS setup is invalid");
  }

  if(channel > volumeDataLayout->GetChannelCount())
  {
    throw InvalidOperation("Specified channel doesn't exist");
  }

  VolumeDataLayer const *
    volumeDataLayer = volumeDataLayout->GetBaseLayer(DimensionGroupUtil::GetDimensionGroupFromDimensionsND(dimensionsND), channel);

  if(!volumeDataLayer)
  {
    throw InvalidOperation("Specified dimension group doesn't exist");
  }

  while(volumeDataLayer && volumeDataLayer->GetLOD() < LOD)
  {
    volumeDataLayer = volumeDataLayer->GetParentLayer();
  }

  if(!volumeDataLayer || volumeDataLayer->GetLayerType() == VolumeDataLayer::Virtual)
  {
    throw InvalidOperation("Specified LOD doesn't exist");
  }

  assert(volumeDataLayer);
  return volumeDataLayer;
}

VolumeDataLayer const *
VolumeDataAccessManagerImpl::ValidateProduceStatus(VolumeDataLayer const * volumeDataLayer, bool allowLODProduction)
{
  if (volumeDataLayer->GetProduceStatus() == VolumeDataLayer::ProduceStatus_Unavailable)
  {
    bool
      LODProductionPossible = allowLODProduction && volumeDataLayer->GetChildLayer() && volumeDataLayer->GetChildLayer()->GetProduceStatus() == VolumeDataLayer::ProduceStatus_Normal;

    if(!LODProductionPossible)
    {
      throw InvalidOperation("The requested dimension group or channel is unavailable (check VDS produce status before requesting data)");
    }
  }
  return volumeDataLayer;
}

VolumeDataLayer const *
VolumeDataAccessManagerImpl::ValidateVolumeSubset(VolumeDataLayer const * volumeDataLayer, const int (&minVoxelCoordinates)[VolumeDataLayout::Dimensionality_Max], const int (&maxVoxelCoordinates)[VolumeDataLayout::Dimensionality_Max])
{
  const int dimensionality = volumeDataLayer->GetLayout()->GetDimensionality();

  for (int dimension = 0; dimension < dimensionality; dimension++)
  {
    if(minVoxelCoordinates[dimension] < 0 || minVoxelCoordinates[dimension] >= volumeDataLayer->GetDimensionNumSamples(dimension))
    {
      throw InvalidArgument(fmt::format("Illegal volume subset, dimension {} min = {}, max = {}", dimension, minVoxelCoordinates[dimension], maxVoxelCoordinates[dimension]).c_str(), "minVoxelCoordinates");
    }
    if(maxVoxelCoordinates[dimension] <= minVoxelCoordinates[dimension] || maxVoxelCoordinates[dimension] > volumeDataLayer->GetDimensionNumSamples(dimension))
    {
      throw InvalidArgument(fmt::format("Illegal volume subset, dimension {} min = {}, max = {}", dimension, minVoxelCoordinates[dimension], maxVoxelCoordinates[dimension]).c_str(), "maxVoxelCoordinates");
    }
  }
  return volumeDataLayer;
}

VolumeDataLayoutImpl const *
VolumeDataAccessManagerImpl::ValidateChannelIndex(VolumeDataLayoutImpl const * volumeDataLayout, int channel)
{
  if(!volumeDataLayout)
  {
    throw InvalidOperation("The VDS doesn't have a volume data layout, this is usually because the VDS setup is invalid");
  }
  return volumeDataLayout->ValidateChannelIndex(channel);
}

VolumeDataLayer const *
VolumeDataAccessManagerImpl::ValidateChunkIndex(VolumeDataLayer const * volumeDataLayer, int64_t chunkIndex)
{
  if (chunkIndex < 0 || chunkIndex > volumeDataLayer->GetTotalChunkCount())
  {
    throw InvalidArgument("The requested chunk doesn't exist", "chunkIndex");
  }

  return volumeDataLayer;
}

VolumeDataLayer const *
VolumeDataAccessManagerImpl::ValidateVolumeDataStore(VolumeDataLayer const *volumeDataLayer)
{
  return volumeDataLayer;
}

VolumeDataLayer const *
VolumeDataAccessManagerImpl::ValidateTraceDimension(VolumeDataLayer const * volumeDataLayer, int traceDimension)
{
  if(traceDimension < 0 || traceDimension >= volumeDataLayer->GetLayout()->GetDimensionality())
  {
    throw InvalidOperation("The trace dimension must be a valid dimension.");
  }

  return volumeDataLayer;
}

const VolumeDataLayoutImpl *
VolumeDataAccessManagerImpl::GetVolumeDataLayout()
{
  if (!IsValid())
  {
    RaiseInvalidManagerException();
  }

  return PrivateGetLayout();
}

VDSProduceStatus
VolumeDataAccessManagerImpl::GetVDSProduceStatus(DimensionsND dimensionsND, int LOD, int channel)
{
  if(dimensionsND < 0 || dimensionsND > DimensionsND::Dimensions_45)
  {
    throw InvalidArgument("Illegal dimensions", "dimensionsND");
  }
  if(LOD < 0)
  {
    throw InvalidArgument("Illegal LOD level", "LOD");
  }
  if(channel < 0)
  {
    throw InvalidArgument("Illegal channel index", "channel");
  }

  VolumeDataLayoutImpl const *
    volumeDataLayout = PrivateGetLayout();

  VolumeDataLayer const *
    volumeDataLayer = NULL;

  if(volumeDataLayout && channel <= volumeDataLayout->GetChannelCount())
  {
    volumeDataLayer = volumeDataLayout->GetBaseLayer(DimensionGroupUtil::GetDimensionGroupFromDimensionsND(dimensionsND), channel);

    while(volumeDataLayer && volumeDataLayer->GetLOD() < LOD)
    {
      volumeDataLayer = volumeDataLayer->GetParentLayer();
    }
  }

  if(volumeDataLayer)
  {
    switch(volumeDataLayer->GetProduceStatus())
    {
    default: assert(0 && "Illegal produce status");
    case VolumeDataLayer::ProduceStatus_Unavailable: return VDSProduceStatus::Unavailable;
    case VolumeDataLayer::ProduceStatus_Remapped:    return VDSProduceStatus::Remapped;
    case VolumeDataLayer::ProduceStatus_Normal:      return VDSProduceStatus::Normal;
    }
  }

  return VDSProduceStatus::Unavailable;
}

int64_t
VolumeDataAccessManagerImpl::GetVDSChunkCount(DimensionsND dimensionsND, int LOD, int channel)
{
  if(dimensionsND < 0 || dimensionsND > DimensionsND::Dimensions_45)
  {
    throw InvalidArgument("Illegal dimensions", "dimensionsND");
  }
  if(LOD < 0)
  {
    throw InvalidArgument("Illegal LOD level", "LOD");
  }
  if(channel < 0)
  {
    throw InvalidArgument("Illegal channel index", "channel");
  }

  VolumeDataLayoutImpl const *
    volumeDataLayout = PrivateGetLayout();

  VolumeDataLayer const *
    volumeDataLayer = NULL;

  if(volumeDataLayout && channel <= volumeDataLayout->GetChannelCount())
  {
    volumeDataLayer = volumeDataLayout->GetBaseLayer(DimensionGroupUtil::GetDimensionGroupFromDimensionsND(dimensionsND), channel);

    while(volumeDataLayer && volumeDataLayer->GetLOD() < LOD)
    {
      volumeDataLayer = volumeDataLayer->GetParentLayer();
    }
  }

  if(volumeDataLayer)
  {
    return volumeDataLayer->GetTotalChunkCount();
  }
  return 0;
}

VolumeDataPageAccessorImpl *
VolumeDataAccessManagerImpl::CreateVolumeDataPageAccessor(VolumeDataLayer const *volumeDataLayer, int maxPages, VolumeDataPageAccessor::AccessMode accessMode)
{
  VolumeDataPageAccessorImpl *parentVolumeDataPageAccessor = nullptr;

  if(accessMode != VolumeDataPageAccessor::AccessMode_ReadOnly && accessMode != VolumeDataPageAccessor::AccessMode_CreateWithoutLODGeneration)
  {
    if(volumeDataLayer->GetParentLayer() && volumeDataLayer->GetParentLayer()->GetLayerType() != VolumeDataLayer::Virtual)
    {
      parentVolumeDataPageAccessor = CreateVolumeDataPageAccessor(volumeDataLayer->GetParentLayer(), (maxPages + 1) / 2, accessMode);
    }
  }

  VolumeDataPageAccessorImpl *pageAccessor = new VolumeDataPageAccessorImpl(this, parentVolumeDataPageAccessor, volumeDataLayer, maxPages, accessMode, m_vds.logger);
  return pageAccessor;
}

VolumeDataPageAccessor *
VolumeDataAccessManagerImpl::CreateVolumeDataPageAccessor(DimensionsND dimensionsND, int LOD, int channel, int maxPages, VolumeDataAccessManager::AccessMode accessMode, int chunkMetadataPageSize)
{
  std::unique_lock<std::mutex> lock(m_mutex);

  VolumeDataLayer *volumeDataLayer = const_cast<VolumeDataLayer *>(PrivateGetLayer(dimensionsND, channel, LOD));

  if((accessMode == VolumeDataPageAccessor::AccessMode_Create || accessMode == VolumeDataPageAccessor::AccessMode_ReadWrite) && LOD > 0)
  {
    throw InvalidOperation("LODs can only be automatically created/updated when accessing LOD 0, use AccessMode_CreateWithoutLODGeneration to write LODs directly");
  }

  if(accessMode != VolumeDataPageAccessor::AccessMode_ReadOnly)
  {
    assert(DimensionGroupUtil::GetDimensionality(DimensionGroupUtil::GetDimensionGroupFromDimensionsND(dimensionsND)) == 2
      || DimensionGroupUtil::GetDimensionality(DimensionGroupUtil::GetDimensionGroupFromDimensionsND(dimensionsND)) == 3);

    bool success = GetVolumeDataStore()->AddLayer(volumeDataLayer, chunkMetadataPageSize);
    if(!success)
    {
      throw InvalidOperation("Failed to create layer");
    }
    volumeDataLayer->SetProduceStatus(VolumeDataLayer::ProduceStatus_Normal);

    if(accessMode == VolumeDataPageAccessor::AccessMode_Create)
    {
      VolumeDataLayer *LODLayer = volumeDataLayer->GetParentLayer();
      while(LODLayer && LODLayer->GetLayerType() != VolumeDataLayer::Virtual)
      {
        bool success = GetVolumeDataStore()->AddLayer(LODLayer, chunkMetadataPageSize);
        if(!success)
        {
          throw InvalidOperation("Failed to create LOD layer");
        }
        LODLayer->SetProduceStatus(VolumeDataLayer::ProduceStatus_Normal);
        LODLayer = LODLayer->GetParentLayer();
      }
    }
  }

  VolumeDataPageAccessorImpl *pageAccessor = CreateVolumeDataPageAccessor(ValidateProduceStatus(volumeDataLayer), maxPages, accessMode);
  m_volumeDataPageAccessorList.InsertLast(pageAccessor);
  return pageAccessor;
}

void
VolumeDataAccessManagerImpl::DestroyVolumeDataPageAccessor(VolumeDataPageAccessor *volumeDataPageAccessor)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  VolumeDataPageAccessorImpl *pageAccessor = static_cast<VolumeDataPageAccessorImpl *>(volumeDataPageAccessor);
  m_volumeDataPageAccessorList.Remove(pageAccessor);
  delete pageAccessor;
}

int64_t 
VolumeDataAccessManagerImpl::GetVolumeSubsetBufferSize(const int (&minVoxelCoordinates)[VolumeDataLayout::Dimensionality_Max], const int (&maxVoxelCoordinates)[VolumeDataLayout::Dimensionality_Max], VolumeDataChannelDescriptor::Format format, int LOD, int channel)
{
  if (IsValid())
  {
    return VolumeDataRequestProcessor::StaticGetVolumeSubsetBufferSize(ValidateChannelIndex(PrivateGetLayout(), channel)->ValidateVoxelCoordinates(minVoxelCoordinates, maxVoxelCoordinates), minVoxelCoordinates, maxVoxelCoordinates, format, LOD, channel);
  }
  else
  {
    return RaiseInvalidManagerException();
  }
}

int64_t 
VolumeDataAccessManagerImpl::RequestVolumeSubset(void *buffer, int64_t bufferByteSize, DimensionsND dimensionsND, int LOD, int channel, const int (&minVoxelCoordinates)[VolumeDataLayout::Dimensionality_Max], const int (&maxVoxelCoordinates)[VolumeDataLayout::Dimensionality_Max], VolumeDataChannelDescriptor::Format format, optional<float> replacementNoValue)
{
  if (IsValid())
  {
    auto requiredBufferSize = GetVolumeSubsetBufferSize(minVoxelCoordinates, maxVoxelCoordinates, format, LOD, channel);
    ValidateBuffer(buffer, bufferByteSize, requiredBufferSize);
    return m_requestProcessor->RequestVolumeSubset(buffer, ValidateVolumeSubset(ValidateProduceStatus(PrivateGetLayer(dimensionsND, channel, LOD)), minVoxelCoordinates, maxVoxelCoordinates), minVoxelCoordinates, maxVoxelCoordinates, LOD, format, replacementNoValue.has_value(), replacementNoValue.value_or(0));
  }
  else
  {
    return RaiseInvalidManagerException();
  }
}

int64_t 
VolumeDataAccessManagerImpl::GetProjectedVolumeSubsetBufferSize(const int (&minVoxelCoordinates)[VolumeDataLayout::Dimensionality_Max], const int (&maxVoxelCoordinates)[VolumeDataLayout::Dimensionality_Max], DimensionsND projectedDimensions, VolumeDataChannelDescriptor::Format format, int LOD, int channel)
{
  if (IsValid())
  {
    return VolumeDataRequestProcessor::StaticGetProjectedVolumeSubsetBufferSize(ValidateChannelIndex(PrivateGetLayout(), channel)->ValidateVoxelCoordinates(minVoxelCoordinates, maxVoxelCoordinates), minVoxelCoordinates, maxVoxelCoordinates, DimensionGroupUtil::GetDimensionGroupFromDimensionsND(projectedDimensions), format, LOD, channel);
  }
  else
  {
    return RaiseInvalidManagerException();
  }
}

int64_t 
VolumeDataAccessManagerImpl::RequestProjectedVolumeSubset(void *buffer, int64_t bufferByteSize, DimensionsND dimensionsND, int LOD, int channel, const int (&minVoxelCoordinates)[VolumeDataLayout::Dimensionality_Max], const int (&maxVoxelCoordinates)[VolumeDataLayout::Dimensionality_Max], FloatVector4 const &voxelPlane, DimensionsND projectedDimensions, VolumeDataChannelDescriptor::Format format, InterpolationMethod interpolationMethod, optional<float> replacementNoValue)
{
  if (IsValid())
  {
    auto requiredBufferSize = GetProjectedVolumeSubsetBufferSize(minVoxelCoordinates, maxVoxelCoordinates, projectedDimensions, format, LOD, channel);
    ValidateBuffer(buffer, bufferByteSize, requiredBufferSize);
    return m_requestProcessor->RequestProjectedVolumeSubset(buffer, ValidateVolumeSubset(ValidateProduceStatus(PrivateGetLayer(dimensionsND, channel, LOD)), minVoxelCoordinates, maxVoxelCoordinates), minVoxelCoordinates, maxVoxelCoordinates, voxelPlane, DimensionGroupUtil::GetDimensionGroupFromDimensionsND(projectedDimensions), LOD, format, interpolationMethod, replacementNoValue.has_value(), replacementNoValue.value_or(0));
  }
  else
  {
    return RaiseInvalidManagerException();
  }
}

int64_t 
VolumeDataAccessManagerImpl::GetVolumeSamplesBufferSize(int nSampleCount, int channel)
{
  if (IsValid())
  {
    return VolumeDataRequestProcessor::StaticGetVolumeSamplesBufferSize(ValidateChannelIndex(PrivateGetLayout(), channel), nSampleCount, channel);
  }
  else
  {
    return RaiseInvalidManagerException();
  }
}

int64_t 
VolumeDataAccessManagerImpl::RequestVolumeSamples(float *buffer, int64_t bufferByteSize, DimensionsND dimensionsND, int LOD, int channel, const float (*SamplePositions)[VolumeDataLayout::Dimensionality_Max], int nSampleCount, InterpolationMethod interpolationMethod, optional<float> replacementNoValue)
{
  if (IsValid())
  {
    auto requiredBufferSize = GetVolumeSamplesBufferSize(nSampleCount, channel);
    ValidateBuffer(buffer, bufferByteSize, requiredBufferSize);
    return m_requestProcessor->RequestVolumeSamples(buffer, ValidateProduceStatus(PrivateGetLayer(dimensionsND, channel, LOD)), SamplePositions, nSampleCount, interpolationMethod, replacementNoValue.has_value(), replacementNoValue.value_or(0));
  }
  else
  {
    return RaiseInvalidManagerException();
  }
}

int64_t 
VolumeDataAccessManagerImpl::GetVolumeTracesBufferSize(int traceCount, int traceDimension, int LOD, int channel)
{
  if (IsValid())
  {
    return VolumeDataRequestProcessor::StaticGetVolumeTracesBufferSize(ValidateChannelIndex(PrivateGetLayout(), channel)->ValidateTraceDimension(traceDimension), traceCount, traceDimension, LOD, channel);
  }
  else
  {
    return RaiseInvalidManagerException();
  }
}

int64_t 
VolumeDataAccessManagerImpl::RequestVolumeTraces(float *buffer, int64_t bufferByteSize, DimensionsND dimensionsND, int LOD, int channel, const float(*tracePositions)[VolumeDataLayout::Dimensionality_Max], int traceCount, InterpolationMethod interpolationMethod, int traceDimension, optional<float> replacementNoValue)
{
  if (IsValid())
  {
    auto requiredBufferSize = GetVolumeTracesBufferSize(traceCount, traceDimension, LOD, channel);
    ValidateBuffer(buffer, bufferByteSize, requiredBufferSize);
    return m_requestProcessor->RequestVolumeTraces(buffer, ValidateTraceDimension(ValidateProduceStatus(PrivateGetLayer(dimensionsND, channel, LOD)), traceDimension), tracePositions, traceCount, LOD, interpolationMethod, traceDimension, replacementNoValue.has_value(), replacementNoValue.value_or(0));
  }
  else
  {
    return RaiseInvalidManagerException();
  }
}

int64_t 
VolumeDataAccessManagerImpl::PrefetchVolumeChunk(DimensionsND dimensionsND, int LOD, int channel, int64_t chunkIndex)
{
  if (IsValid())
  {
    return m_requestProcessor->PrefetchVolumeChunk(ValidateVolumeDataStore(ValidateChunkIndex(ValidateProduceStatus(PrivateGetLayer(dimensionsND, channel, LOD), true), chunkIndex)), chunkIndex);
  }
  else
  {
    return RaiseInvalidManagerException();
  }
}

bool    
VolumeDataAccessManagerImpl::IsCompleted(int64_t requestID)
{
  ValidateRequest(requestID);
  return m_requestProcessor->IsCompleted(requestID);
}

bool
VolumeDataAccessManagerImpl::IsCanceled(int64_t requestID)
{
  ValidateRequest(requestID);
  Error error;
  return m_requestProcessor->IsCanceled(requestID, error);
}

bool
VolumeDataAccessManagerImpl::IsCanceled(int64_t requestID, ReadErrorException* readErrorException)
{
  ValidateRequest(requestID);
  Error error;
  bool isCanceled = m_requestProcessor->IsCanceled(requestID, error);
  if(error.code != 0 && readErrorException)
  {
    *readErrorException = ReadErrorException(error.string.c_str(), error.code);
  }
  return isCanceled;
}

bool    
VolumeDataAccessManagerImpl::WaitForCompletion(int64_t requestID, int millisecondsBeforeTimeout)
{
  ValidateRequest(requestID);
  return m_requestProcessor->WaitForCompletion(requestID, millisecondsBeforeTimeout);
}

void    
VolumeDataAccessManagerImpl::Cancel(int64_t requestID)
{
  ValidateRequest(requestID);
  return m_requestProcessor->Cancel(requestID);
}

void    
VolumeDataAccessManagerImpl::CancelAndWaitForCompletion(int64_t requestID)
{
  if (m_invalidated)
    return;
  ValidateRequest(requestID);
  Error error;
  if(!m_requestProcessor->IsCanceled(requestID, error))
  {
    Cancel(requestID);

    while(!WaitForCompletion(requestID, 0))
    {
      if(m_requestProcessor->IsCanceled(requestID, error))
      {
        return;
      }
    }
  }
}

float
VolumeDataAccessManagerImpl::GetCompletionFactor(int64_t requestID)
{
  ValidateRequest(requestID);
  return m_requestProcessor->GetCompletionFactor(requestID);
}

// The following methods intentionally violates our coding standard to make bulk editing easier.
//IVolumeDataReadWriteAccessor<IntVector2, bool>    * VolumeDataAccessManagerImpl::Create2DVolumeDataAccessor1Bit(VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector2, bool>    (pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector2, uint8_t> * VolumeDataAccessManagerImpl::Create2DVolumeDataAccessorU8  (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector2, uint8_t> (pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector2, uint16_t>* VolumeDataAccessManagerImpl::Create2DVolumeDataAccessorU16 (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector2, uint16_t>(pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector2, uint32_t>* VolumeDataAccessManagerImpl::Create2DVolumeDataAccessorU32 (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector2, uint32_t>(pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector2, uint64_t>* VolumeDataAccessManagerImpl::Create2DVolumeDataAccessorU64 (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector2, uint64_t>(pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector2, float>   * VolumeDataAccessManagerImpl::Create2DVolumeDataAccessorR32 (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector2, float>   (pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector2, double>  * VolumeDataAccessManagerImpl::Create2DVolumeDataAccessorR64 (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector2, double>  (pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector3, bool>    * VolumeDataAccessManagerImpl::Create3DVolumeDataAccessor1Bit(VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector3, bool>    (pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector3, uint8_t> * VolumeDataAccessManagerImpl::Create3DVolumeDataAccessorU8  (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector3, uint8_t> (pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector3, uint16_t>* VolumeDataAccessManagerImpl::Create3DVolumeDataAccessorU16 (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector3, uint16_t>(pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector3, uint32_t>* VolumeDataAccessManagerImpl::Create3DVolumeDataAccessorU32 (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector3, uint32_t>(pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector3, uint64_t>* VolumeDataAccessManagerImpl::Create3DVolumeDataAccessorU64 (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector3, uint64_t>(pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector3, float>   * VolumeDataAccessManagerImpl::Create3DVolumeDataAccessorR32 (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector3, float>   (pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector3, double>  * VolumeDataAccessManagerImpl::Create3DVolumeDataAccessorR64 (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector3, double>  (pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector4, bool>    * VolumeDataAccessManagerImpl::Create4DVolumeDataAccessor1Bit(VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector4, bool>    (pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector4, uint8_t> * VolumeDataAccessManagerImpl::Create4DVolumeDataAccessorU8  (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector4, uint8_t> (pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector4, uint16_t>* VolumeDataAccessManagerImpl::Create4DVolumeDataAccessorU16 (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector4, uint16_t>(pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector4, uint32_t>* VolumeDataAccessManagerImpl::Create4DVolumeDataAccessorU32 (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector4, uint32_t>(pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector4, uint64_t>* VolumeDataAccessManagerImpl::Create4DVolumeDataAccessorU64 (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector4, uint64_t>(pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector4, float>   * VolumeDataAccessManagerImpl::Create4DVolumeDataAccessorR32 (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector4, float>   (pVolumeDataPageAccessor, replacementNoValue); }
//IVolumeDataReadWriteAccessor<IntVector4, double>  * VolumeDataAccessManagerImpl::Create4DVolumeDataAccessorR64 (VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue) { return CreateVolumeDataAccessor<IntVector4, double>  (pVolumeDataPageAccessor, replacementNoValue); }
//
//IVolumeDataReadAccessor<FloatVector2, float >* VolumeDataAccessManagerImpl::Create2DInterpolatingVolumeDataAccessorR32(VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue, InterpolationMethod interpolationMethod) { return CreateInterpolatingVolumeDataAccessor<FloatVector2, float >(pVolumeDataPageAccessor, replacementNoValue, interpolationMethod); }
//IVolumeDataReadAccessor<FloatVector2, double>* VolumeDataAccessManagerImpl::Create2DInterpolatingVolumeDataAccessorR64(VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue, InterpolationMethod interpolationMethod) { return CreateInterpolatingVolumeDataAccessor<FloatVector2, double>(pVolumeDataPageAccessor, replacementNoValue, interpolationMethod); }
//IVolumeDataReadAccessor<FloatVector3, float >* VolumeDataAccessManagerImpl::Create3DInterpolatingVolumeDataAccessorR32(VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue, InterpolationMethod interpolationMethod) { return CreateInterpolatingVolumeDataAccessor<FloatVector3, float >(pVolumeDataPageAccessor, replacementNoValue, interpolationMethod); }
//IVolumeDataReadAccessor<FloatVector3, double>* VolumeDataAccessManagerImpl::Create3DInterpolatingVolumeDataAccessorR64(VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue, InterpolationMethod interpolationMethod) { return CreateInterpolatingVolumeDataAccessor<FloatVector3, double>(pVolumeDataPageAccessor, replacementNoValue, interpolationMethod); }
//IVolumeDataReadAccessor<FloatVector4, float >* VolumeDataAccessManagerImpl::Create4DInterpolatingVolumeDataAccessorR32(VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue, InterpolationMethod interpolationMethod) { return CreateInterpolatingVolumeDataAccessor<FloatVector4, float >(pVolumeDataPageAccessor, replacementNoValue, interpolationMethod); }
//IVolumeDataReadAccessor<FloatVector4, double>* VolumeDataAccessManagerImpl::Create4DInterpolatingVolumeDataAccessorR64(VolumeDataPageAccessor* pVolumeDataPageAccessor, float replacementNoValue, InterpolationMethod interpolationMethod) { return CreateInterpolatingVolumeDataAccessor<FloatVector4, double>(pVolumeDataPageAccessor, replacementNoValue, interpolationMethod); }

void VolumeDataAccessManagerImpl::DestroyVolumeDataAccessor(IVolumeDataAccessor* accessor)
{
  VolumeDataAccessorBase *volumeDataAccessorBase = dynamic_cast<VolumeDataAccessorBase *>(accessor);
  if(!volumeDataAccessorBase)
  {
    throw InvalidOperation("Cast from IVolumeDataAccessor to VolumeDataAccessorBase failed.");
  }
  delete volumeDataAccessorBase;
}

IVolumeDataAccessor* VolumeDataAccessManagerImpl::CloneVolumeDataAccessor(IVolumeDataAccessor const& accessor)
{
  VolumeDataAccessorBase *volumeDataAccessorBase = dynamic_cast<VolumeDataAccessorBase*>(const_cast<IVolumeDataAccessor *>(&accessor));
  if(!volumeDataAccessorBase)
  {
    throw InvalidOperation("Cast from IVolumeDataAccessor to VolumeDataAccessorBase failed.");
  }
  return volumeDataAccessorBase->Clone(*volumeDataAccessorBase->GetVolumeDataPageAccessor());
}

void VolumeDataAccessManagerImpl::FlushUploadQueue(bool writeUpdatedLayerStatus, ErrorHandler errorHandler, Error* error)
{
  ErrorGuard errorGuard(errorHandler, error);
  GetVolumeDataStore()->Flush(writeUpdatedLayerStatus, errorGuard);
}

static bool isPureCopy(const VolumeDataChunk &a, const VolumeDataChunk &b)
{
  auto sourceCompressionMethod = a.layer->GetEffectiveCompressionMethod();
  if (sourceCompressionMethod == b.layer->GetEffectiveCompressionMethod())
  {
    if (CompressionMethod_IsWavelet(sourceCompressionMethod))
    {
      return a.layer->GetEffectiveCompressionTolerance() == b.layer->GetEffectiveCompressionTolerance();
    }
    return true;
  }
  return false;
}

void VolumeDataAccessManagerImpl::AddCopyPageJob(VolumeDataChunk& chunk, VolumeDataPageAccessorImpl &destination, VolumeDataPageAccessorImpl const &source)
{
  Error error;
  VolumeDataChunk sourceChunk = { source.GetLayer(), chunk.index };

  if (source.GetChunkVolumeDataHash(chunk.index) == destination.GetChunkVolumeDataHash(chunk.index))
  {
    return;
  }

  source.GetManager()->GetVolumeDataStore()->PrepareReadChunk(sourceChunk, sourceChunk.layer->GetEffectiveWaveletAdaptiveLoadLevel(), error);
  std::unique_lock<std::mutex> lock(m_mutex);
  if (error.code)
  {
    std::string url = fmt::format("Chunk: {}, Channel: {} LOD: {}", chunk.index, chunk.layer->GetChannelIndex(), chunk.layer->GetLOD());
    AddUploadError(error, url);
    return;
  }
  auto threadCount = m_requestProcessor->GetThreadPool().ThreadCount();
  m_copyJobs[m_copyJobIndex].emplace_back(chunk, m_requestProcessor->GetThreadPool().Enqueue([this, chunk, sourceChunk, threadCount, &destination, &source]
    {
      Error error;
      std::vector<uint8_t> serializedData;
      std::vector<uint8_t> metadata;
      CompressionInfo compressionInfo;

      auto sourceDataStore = source.GetManager()->GetVolumeDataStore();
      auto destDataStore = GetVolumeDataStore();
      sourceDataStore->ReadChunk(sourceChunk, sourceChunk.layer->GetEffectiveWaveletAdaptiveLoadLevel(), serializedData, metadata, compressionInfo, error);
      if (error.code)
      {
        return error;
      }

      uint64_t hash = VolumeDataHash::UNKNOWN;
      if(metadata.size() >= sizeof(uint64_t))
      {
        memcpy(&hash, metadata.data(), sizeof(uint64_t));
        if(hash == VolumeDataHash::UNKNOWN)
        {
          // We don't copy chunks that have not been written in the source
          return error;
        }
      }

      if (isPureCopy(chunk, sourceChunk))
      {
        destDataStore->WriteChunk(chunk, serializedData, metadata);
      }
      else
      {
        auto destFormat = PrivateGetLayout()->GetChannelFormat(chunk.layer->GetChannelIndex());
        DataBlock destDataBlock;
        std::vector<uint8_t> deserializedData;
        uint64_t hash = VolumeDataHash::UNKNOWN;
        sourceDataStore->DeserializeVolumeData(sourceChunk, serializedData, metadata, compressionInfo.GetCompressionMethod(), sourceChunk.layer->GetEffectiveWaveletAdaptiveLoadLevel(), destFormat, destDataBlock, deserializedData, hash, error);
        if (error.code)
        {
          return error;
        }

        destination.RequestWritePage(chunk.index, destDataBlock, deserializedData, hash);
      }
      destination.GetManager()->GetVolumeDataLayout()->CompletePendingWriteChunkRequests(int32_t(threadCount));
      return error;
    }));

  if (m_copyJobs[m_copyJobIndex].size() == m_requestProcessor->GetThreadPool().ThreadCount())
  {
    auto& otherjobs = m_copyJobs[!m_copyJobIndex];
    for (auto& job : otherjobs)
    {
      auto error = job.second.get();
      if (error.code)
      {
        auto& jobChunk = job.first;
        std::string url = fmt::format("Chunk: {}, Channel: {} LOD: {}", jobChunk.index, jobChunk.layer->GetChannelIndex(), jobChunk.layer->GetLOD());
        AddUploadError(error, url);
      }
    }
    otherjobs.clear();
    m_copyJobIndex = !m_copyJobIndex;
  }
}

void VolumeDataAccessManagerImpl::FlushCopyPageJobs()
{
  std::unique_lock<std::mutex> lock(m_mutex);
  for (auto& jobsMap : m_copyJobs)
  {
    for (auto& job : jobsMap)
    {
      auto error = job.second.get();
      if (error.code)
      {
        auto& jobChunk = job.first;
        std::string url = fmt::format("Chunk: {}, Channel: {} LOD: {}", jobChunk.index, jobChunk.layer->GetChannelIndex(), jobChunk.layer->GetLOD());
        AddUploadError(error, url);
      }
    }
    jobsMap.clear();
  }

}

int64_t VolumeDataAccessManagerImpl::AddRemapJob(VolumeDataPageImpl &targetPage, std::vector<VolumeDataChunk> const &sourceChunks)
{
  return m_requestProcessor->RequestRemap(targetPage, sourceChunks);
}

void VolumeDataAccessManagerImpl::AddUploadError(Error const &error, const std::string &url)
{
  std::unique_lock<std::mutex> lock(m_uploadErrors.mutex);
  m_uploadErrors.errors.emplace_back(new UploadError(error, url));
}

void VolumeDataAccessManagerImpl::ClearUploadErrors()
{
  std::unique_lock<std::mutex> lock(m_uploadErrors.mutex);
  m_uploadErrors.errors.erase(m_uploadErrors.errors.begin(), m_uploadErrors.errors.begin() + m_uploadErrors.currentErrorIndex);
  m_uploadErrors.currentErrorIndex = 0;
}

void VolumeDataAccessManagerImpl::ForceClearAllUploadErrors()
{
  std::unique_lock<std::mutex> lock(m_uploadErrors.mutex);
  m_uploadErrors.errors.erase(m_uploadErrors.errors.begin(), m_uploadErrors.errors.begin() + m_uploadErrors.currentErrorIndex);
  m_uploadErrors.errors.clear();
  m_uploadErrors.currentErrorIndex = 0;
}


int32_t VolumeDataAccessManagerImpl::UploadErrorCount()
{
  std::unique_lock<std::mutex> lock(m_uploadErrors.mutex);
  return int32_t(m_uploadErrors.errors.size() - m_uploadErrors.currentErrorIndex);
}

void VolumeDataAccessManagerImpl::GetCurrentUploadError(const char** objectId, int32_t* errorCode, const char** errorString)
{
  std::unique_lock<std::mutex> lock(m_uploadErrors.mutex);
  if (m_uploadErrors.currentErrorIndex >= m_uploadErrors.errors.size())
  {
    if (objectId)
      *objectId = "";
    if (errorCode)
      *errorCode = 0;
    if (errorString)
      *errorString = "";
    return;
  }

  const auto &error = m_uploadErrors.errors[m_uploadErrors.currentErrorIndex];
  m_uploadErrors.currentErrorIndex++;
  if (objectId)
    *objectId = error->urlObject.c_str();
  if (errorCode)
    *errorCode = error->error.code;
  if (errorString)
    *errorString = error->error.string.c_str();
}

void VolumeDataAccessManagerImpl::GetCurrentDownloadError(int* errorCode, const char** errorString)
{
  if (errorCode)
    *errorCode = m_currentDownloadError.code;
  if (errorString)
    *errorString = m_currentDownloadError.string.c_str();
}

}
