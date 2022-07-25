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

#include "VolumeDataAccessor.h"

#include "VolumeDataAccessManagerImpl.h"
#include "VolumeDataLayoutImpl.h"
#include "VolumeDataRequestProcessor.h"
#include "DataBlock.h"
#include "DimensionGroup.h"
#include <OpenVDS/ValueConversion.h>

#include <inttypes.h>

#include <fmt/format.h>

namespace OpenVDS
{

void VolumeDataAccessorBase::UpdateWrittenRegion()
{
  if(m_writtenRegion.Max[0] != 0)
  {
    assert(m_currentPage);

    int writtenMin[Dimensionality_Max] = { m_writtenRegion.Min[3], m_writtenRegion.Min[2], m_writtenRegion.Min[1], m_writtenRegion.Min[0], 0, 0 };
    int writtenMax[Dimensionality_Max] = { m_writtenRegion.Max[3], m_writtenRegion.Max[2], m_writtenRegion.Max[1], m_writtenRegion.Max[0], 1, 1 };
    m_currentPage->UpdateWrittenRegion(writtenMin, writtenMax);

    m_writtenRegion = AccessorRegion({0, 0, 0, 0}, {0, 0, 0, 0});
  }
}

//-----------------------------------------------------------------------------

void VolumeDataAccessorBase::MakeCurrentPageWritable()
{
  int pitch[Dimensionality_Max];

  m_buffer = m_currentPage->GetWritableBuffer(pitch);
  m_pitch = {pitch[3], pitch[2], pitch[1], pitch[0]};
  m_writable = true;
}

//-----------------------------------------------------------------------------

IVolumeDataAccessor::Manager &VolumeDataAccessorBase::GetManager()
{
  return *static_cast<VolumeDataPageAccessorImpl *>(m_volumeDataPageAccessor)->GetManager();
}

//-----------------------------------------------------------------------------

VolumeDataLayout const *VolumeDataAccessorBase::GetLayout()
{
  return static_cast<VolumeDataPageAccessorImpl *>(m_volumeDataPageAccessor)->GetLayout();
}

//-----------------------------------------------------------------------------

void VolumeDataAccessorBase::Commit()
{
  UpdateWrittenRegion();
  m_volumeDataPageAccessor->Commit();
}

//-----------------------------------------------------------------------------

void VolumeDataAccessorBase::Cancel()
{
  m_canceled = true;
}

//-----------------------------------------------------------------------------

VolumeDataAccessorBase::VolumeDataAccessorBase(VolumeDataPageAccessor &volumeDataPageAccessor)
  : m_volumeDataPageAccessor(&volumeDataPageAccessor)
  , m_currentPage(nullptr)
  , m_canceled(false)
  , m_min{0,0,0,0}
  , m_max{0,0,0,0}
  , m_validRegion(AccessorRegion({0, 0, 0, 0}, {0, 0, 0, 0}))
  , m_writtenRegion(AccessorRegion({0, 0, 0, 0}, {0, 0, 0, 0}))
  , m_writable(false)
  , m_buffer(nullptr)
  , m_pitch{0, 0, 0, 0}
{
  int numSamples[Dimensionality_Max];

  m_volumeDataPageAccessor->GetNumSamples(numSamples);
  m_numSamples = {numSamples[3], numSamples[2], numSamples[1], numSamples[0]};

  int LOD = m_volumeDataPageAccessor->GetLOD();
  auto layoutDescriptor = m_volumeDataPageAccessor->GetLayout()->GetLayoutDescriptor();
  int fullResolutionDimension = layoutDescriptor.IsForceFullResolutionDimension() ? layoutDescriptor.GetFullResolutionDimension() : -1;

  m_LOD = { fullResolutionDimension != 3 ? LOD : 0,
            fullResolutionDimension != 2 ? LOD : 0,
            fullResolutionDimension != 1 ? LOD : 0,
            fullResolutionDimension != 0 ? LOD : 0 };
}

//-----------------------------------------------------------------------------

VolumeDataAccessorBase::~VolumeDataAccessorBase()
{
  if(m_currentPage)
  {
    UpdateWrittenRegion();
    m_currentPage->Release();
    m_currentPage = nullptr;
  }
  if(m_volumeDataPageAccessor)
  {
    if(m_volumeDataPageAccessor->RemoveReference() == 0)
    {
      if(!m_canceled)
      {
        m_volumeDataPageAccessor->Commit();
      }

      static_cast<VolumeDataPageAccessorImpl *>(m_volumeDataPageAccessor)->GetManager()->DestroyVolumeDataPageAccessor(m_volumeDataPageAccessor);
    }
    m_volumeDataPageAccessor = nullptr;
  }
}

void VolumeDataAccessorBase::ReadPageAtPosition(IntVector4 index, bool enableWriting)
{
  if(m_currentPage)
  {
    UpdateWrittenRegion();
    m_currentPage->Release();
    m_currentPage = nullptr;
    m_buffer = nullptr;
    m_min = {0, 0, 0, 0};
    m_max = {0, 0, 0, 0};
    m_pitch = {0, 0, 0, 0};
    m_writable = enableWriting;
  }

  assert(m_writtenRegion.Max[0] == 0);

  int position[Dimensionality_Max] = { index[3], index[2], index[1], index[0], 0, 0 };

  VolumeDataPage *page = m_volumeDataPageAccessor->ReadPageAtPosition(position);

  if(!page)
  {
    m_validRegion = AccessorRegion({0, 0, 0, 0}, {0, 0, 0, 0});

    if(index[0] < 0 || index[0] >= m_numSamples[0] ||
       index[1] < 0 || index[1] >= m_numSamples[1] ||
       index[2] < 0 || index[2] >= m_numSamples[2] ||
       index[3] < 0 || index[3] >= m_numSamples[3])
    {
      std::string message = fmt::format("IndexOutOfRangeException: Index: [{}, {}, {}, {}] is out of the range: [0, 0, 0, 0] - [{}, {}, {}, {}].",
        index[0], index[1], index[2], index[3],
        m_numSamples[0], m_numSamples[1], m_numSamples[2], m_numSamples[3]);
      IndexOutOfRangeException exception(message.c_str());
      throw exception;
    }
    else
    {
      return;
    }
  }
  else
  {
    ReadErrorException
      error = page->GetError();

    if(error.GetErrorCode() == 0)
    {
      m_currentPage = page;
    }
    else
    {
      page->Release();
      m_validRegion = AccessorRegion({0, 0, 0, 0}, {0, 0, 0, 0});

      throw error;
    }
  }

  int32_t min[Dimensionality_Max];
  int32_t max[Dimensionality_Max];

  int32_t minExcludingMargin[Dimensionality_Max];
  int32_t maxExcludingMargin[Dimensionality_Max];

  page->GetMinMax(min, max);
  m_min = {min[3], min[2], min[1], min[0]};
  m_max = {max[3], max[2], max[1], max[0]};

  page->GetMinMaxExcludingMargin(minExcludingMargin, maxExcludingMargin);
  m_validRegion = AccessorRegion({minExcludingMargin[3], minExcludingMargin[2], minExcludingMargin[1], minExcludingMargin[0]}, {maxExcludingMargin[3], maxExcludingMargin[2], maxExcludingMargin[1], maxExcludingMargin[0]});

  int pitch[Dimensionality_Max];

  m_buffer = enableWriting ? page->GetWritableBuffer(pitch) : const_cast<void *>(page->GetBuffer(pitch));
  m_pitch = {pitch[3], pitch[2], pitch[1], pitch[0]};
  m_writable = enableWriting;
}

template <typename INDEX, typename T1, typename T2, bool isUseNoValue>
IVolumeDataReadWriteAccessor<INDEX, T1> *CreateVolumeDataAccessorFormat(VolumeDataPageAccessor *volumeDataPageAccessor, float replacementNoValue)
{
  if (volumeDataPageAccessor->GetLOD() == 0)
  {
    return new ConvertingVolumeDataAccessor<INDEX, T1, T2, isUseNoValue, true>(*volumeDataPageAccessor, replacementNoValue);
  }
  else
  {
    return new ConvertingVolumeDataAccessor<INDEX, T1, T2, isUseNoValue, false>(*volumeDataPageAccessor, replacementNoValue);
  }
}

template <typename INDEX, typename T, bool isUseNoValue>
IVolumeDataReadWriteAccessor<INDEX, T> *CreateVolumeDataAccessorNoValue(VolumeDataPageAccessor *volumeDataPageAccessor, float replacementNoValue)
{
  switch(volumeDataPageAccessor->GetChannelDescriptor().GetFormat())
  {
  default:
    assert(0 && "Unknown voxel format");
    break;
  case VolumeDataChannelDescriptor::Format_R64:
    return CreateVolumeDataAccessorFormat<INDEX, T, double,   isUseNoValue>(volumeDataPageAccessor, replacementNoValue);
  case VolumeDataChannelDescriptor::Format_U64:
    return CreateVolumeDataAccessorFormat<INDEX, T, uint64_t, isUseNoValue>(volumeDataPageAccessor, replacementNoValue);
  case VolumeDataChannelDescriptor::Format_R32:
    return CreateVolumeDataAccessorFormat<INDEX, T, float,    isUseNoValue>(volumeDataPageAccessor, replacementNoValue);
  case VolumeDataChannelDescriptor::Format_U32:
    return CreateVolumeDataAccessorFormat<INDEX, T, uint32_t, isUseNoValue>(volumeDataPageAccessor, replacementNoValue);
  case VolumeDataChannelDescriptor::Format_U16:
    return CreateVolumeDataAccessorFormat<INDEX, T, uint16_t, isUseNoValue>(volumeDataPageAccessor, replacementNoValue);
  case VolumeDataChannelDescriptor::Format_U8:
    return CreateVolumeDataAccessorFormat<INDEX, T, uint8_t,  isUseNoValue>(volumeDataPageAccessor, replacementNoValue);
  case VolumeDataChannelDescriptor::Format_1Bit:
    return CreateVolumeDataAccessorFormat<INDEX, T, bool,     isUseNoValue>(volumeDataPageAccessor, replacementNoValue);
  }
  return nullptr;
}

template <typename INDEX, typename T>
IVolumeDataReadWriteAccessor<INDEX, T> *CreateVolumeDataAccessor(VolumeDataPageAccessor *volumeDataPageAccessor, float replacementNoValue)
{
  if(volumeDataPageAccessor->GetChannelDescriptor().IsUseNoValue())
  {
    return CreateVolumeDataAccessorNoValue<INDEX, T, true>(volumeDataPageAccessor, replacementNoValue);
  }
  else
  {
    return CreateVolumeDataAccessorNoValue<INDEX, T, false>(volumeDataPageAccessor, replacementNoValue);
  }
}

template<typename INDEX, typename T1, typename T2, bool isUseNoValue, int interpolationMethod>
IVolumeDataReadAccessor<INDEX, T1>* CreateInterpolatingVolumeDataAccessorImpl(VolumeDataPageAccessor * volumeDataPageAccessor, float replacementNoValue)
{
  if (volumeDataPageAccessor->GetLOD() == 0)
  {
    return new InterpolatingVolumeDataAccessor<INDEX, T1, T2, isUseNoValue, interpolationMethod, true>(*volumeDataPageAccessor, replacementNoValue);
  }
  else
  {
    return new InterpolatingVolumeDataAccessor<INDEX, T1, T2, isUseNoValue, interpolationMethod, false>(*volumeDataPageAccessor, replacementNoValue);
  }
}

template<typename INDEX, typename T1, typename T2, bool isUseNoValue>
IVolumeDataReadAccessor<INDEX, T1>* CreateInterpolatingVolumeDataAccessorFormat(VolumeDataPageAccessor * volumeDataPageAccessor, float replacementNoValue, InterpolationMethod interpolationMethod)
{
  switch(interpolationMethod)
  {
  default: 
    assert(0 && "Unknown interpolation format");
  case InterpolationMethod::Nearest:
    return CreateInterpolatingVolumeDataAccessorImpl<INDEX, T1, T2, isUseNoValue, (int)InterpolationMethod::Nearest>(volumeDataPageAccessor, replacementNoValue);
  case InterpolationMethod::Linear:
    return CreateInterpolatingVolumeDataAccessorImpl<INDEX, T1, T2, isUseNoValue, (int)InterpolationMethod::Linear> (volumeDataPageAccessor, replacementNoValue);
  case InterpolationMethod::Cubic:
    return CreateInterpolatingVolumeDataAccessorImpl<INDEX, T1, T2, isUseNoValue, (int)InterpolationMethod::Cubic>  (volumeDataPageAccessor, replacementNoValue);
  case InterpolationMethod::Angular:
    return CreateInterpolatingVolumeDataAccessorImpl<INDEX, T1, T2, isUseNoValue, (int)InterpolationMethod::Angular>(volumeDataPageAccessor, replacementNoValue);
  case InterpolationMethod::Triangular:
    return CreateInterpolatingVolumeDataAccessorImpl<INDEX, T1, T2, isUseNoValue, (int)InterpolationMethod::Triangular>(volumeDataPageAccessor, replacementNoValue);
  }

  return nullptr;
}

template <typename INDEX, typename T, bool isUseNoValue>
static IVolumeDataReadAccessor<INDEX, T>* CreateInterpolatingVolumeDataAccessorNoValue(VolumeDataPageAccessor * volumeDataPageAccessor, float replacementNoValue, InterpolationMethod interpolationMethod)
{
  switch(volumeDataPageAccessor->GetChannelDescriptor().GetFormat())
  {
  case VolumeDataChannelDescriptor::Format_R64:
    return CreateInterpolatingVolumeDataAccessorFormat<INDEX, T, double,   isUseNoValue>(volumeDataPageAccessor, replacementNoValue, interpolationMethod);
  case VolumeDataChannelDescriptor::Format_U64:
    return CreateInterpolatingVolumeDataAccessorFormat<INDEX, T, uint64_t, isUseNoValue>(volumeDataPageAccessor, replacementNoValue, interpolationMethod);
  case VolumeDataChannelDescriptor::Format_R32:
    return CreateInterpolatingVolumeDataAccessorFormat<INDEX, T, float,    isUseNoValue>(volumeDataPageAccessor, replacementNoValue, interpolationMethod);
  case VolumeDataChannelDescriptor::Format_U32:
    return CreateInterpolatingVolumeDataAccessorFormat<INDEX, T, uint32_t, isUseNoValue>(volumeDataPageAccessor, replacementNoValue, interpolationMethod);
  case VolumeDataChannelDescriptor::Format_U16:
    return CreateInterpolatingVolumeDataAccessorFormat<INDEX, T, uint16_t, isUseNoValue>(volumeDataPageAccessor, replacementNoValue, interpolationMethod);
  case VolumeDataChannelDescriptor::Format_U8:
    return CreateInterpolatingVolumeDataAccessorFormat<INDEX, T, uint8_t,  isUseNoValue>(volumeDataPageAccessor, replacementNoValue, interpolationMethod);
  case VolumeDataChannelDescriptor::Format_1Bit:
    return CreateInterpolatingVolumeDataAccessorFormat<INDEX, T, bool,     isUseNoValue>(volumeDataPageAccessor, replacementNoValue, interpolationMethod);
  default:
    assert(0 && "Unknown voxel format");
    break;
  }
  return nullptr;
}

template <typename INDEX, typename T>
static IVolumeDataReadAccessor<INDEX, T>* CreateInterpolatingVolumeDataAccessor(VolumeDataPageAccessor * volumeDataPageAccessor, float replacementNoValue, InterpolationMethod interpolationMethod)
{
  if(volumeDataPageAccessor->GetChannelDescriptor().IsUseNoValue())
  {
    return CreateInterpolatingVolumeDataAccessorNoValue<INDEX, T, true>(volumeDataPageAccessor, replacementNoValue, interpolationMethod);
  }
  else
  {
    return CreateInterpolatingVolumeDataAccessorNoValue<INDEX, T, false>(volumeDataPageAccessor, replacementNoValue, interpolationMethod);
  }
}

IVolumeDataReadWriteAccessor<IntVector2, bool>* VolumeDataAccessManagerImpl::Create2DVolumeDataAccessor1Bit(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector2, bool>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector2, uint8_t>* VolumeDataAccessManagerImpl::Create2DVolumeDataAccessorU8(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector2, uint8_t>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector2, uint16_t>* VolumeDataAccessManagerImpl::Create2DVolumeDataAccessorU16(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector2, uint16_t>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector2, uint32_t>* VolumeDataAccessManagerImpl::Create2DVolumeDataAccessorU32(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector2, uint32_t>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector2, uint64_t>* VolumeDataAccessManagerImpl::Create2DVolumeDataAccessorU64(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector2, uint64_t>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector2, float>* VolumeDataAccessManagerImpl::Create2DVolumeDataAccessorR32(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector2, float>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector2, double>* VolumeDataAccessManagerImpl::Create2DVolumeDataAccessorR64(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector2, double>(volumeDataPageAccessor, replacementNoValue);
}

IVolumeDataReadWriteAccessor<IntVector3, bool>* VolumeDataAccessManagerImpl::Create3DVolumeDataAccessor1Bit(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector3, bool>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector3, uint8_t>* VolumeDataAccessManagerImpl::Create3DVolumeDataAccessorU8(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector3, uint8_t>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector3, uint16_t>* VolumeDataAccessManagerImpl::Create3DVolumeDataAccessorU16(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector3, uint16_t>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector3, uint32_t>* VolumeDataAccessManagerImpl::Create3DVolumeDataAccessorU32(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector3, uint32_t>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector3, uint64_t>* VolumeDataAccessManagerImpl::Create3DVolumeDataAccessorU64(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector3, uint64_t>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector3, float>* VolumeDataAccessManagerImpl::Create3DVolumeDataAccessorR32(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector3, float>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector3, double>* VolumeDataAccessManagerImpl::Create3DVolumeDataAccessorR64(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector3, double>(volumeDataPageAccessor, replacementNoValue);
}

IVolumeDataReadWriteAccessor<IntVector4, bool>* VolumeDataAccessManagerImpl::Create4DVolumeDataAccessor1Bit(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector4, bool>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector4, uint8_t>* VolumeDataAccessManagerImpl::Create4DVolumeDataAccessorU8(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector4, uint8_t>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector4, uint16_t>* VolumeDataAccessManagerImpl::Create4DVolumeDataAccessorU16(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector4, uint16_t>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector4, uint32_t>* VolumeDataAccessManagerImpl::Create4DVolumeDataAccessorU32(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector4, uint32_t>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector4, uint64_t>* VolumeDataAccessManagerImpl::Create4DVolumeDataAccessorU64(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector4, uint64_t>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector4, float>* VolumeDataAccessManagerImpl::Create4DVolumeDataAccessorR32(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector4, float>(volumeDataPageAccessor, replacementNoValue);
}
IVolumeDataReadWriteAccessor<IntVector4, double>* VolumeDataAccessManagerImpl::Create4DVolumeDataAccessorR64(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue)
{
  return CreateVolumeDataAccessor<IntVector4, double>(volumeDataPageAccessor, replacementNoValue);
}

IVolumeDataReadAccessor<FloatVector2, float >* VolumeDataAccessManagerImpl::Create2DInterpolatingVolumeDataAccessorR32(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue, InterpolationMethod interpolationMethod)
{
  return CreateInterpolatingVolumeDataAccessor<FloatVector2, float>(volumeDataPageAccessor, replacementNoValue, interpolationMethod);
}
IVolumeDataReadAccessor<FloatVector2, double>* VolumeDataAccessManagerImpl::Create2DInterpolatingVolumeDataAccessorR64(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue, InterpolationMethod interpolationMethod)
{
  return CreateInterpolatingVolumeDataAccessor<FloatVector2, double>(volumeDataPageAccessor, replacementNoValue, interpolationMethod);
}

IVolumeDataReadAccessor<FloatVector3, float >* VolumeDataAccessManagerImpl::Create3DInterpolatingVolumeDataAccessorR32(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue, InterpolationMethod interpolationMethod)
{
  return CreateInterpolatingVolumeDataAccessor<FloatVector3, float >(volumeDataPageAccessor, replacementNoValue, interpolationMethod);
}
IVolumeDataReadAccessor<FloatVector3, double>* VolumeDataAccessManagerImpl::Create3DInterpolatingVolumeDataAccessorR64(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue, InterpolationMethod interpolationMethod)
{
  return CreateInterpolatingVolumeDataAccessor<FloatVector3, double>(volumeDataPageAccessor, replacementNoValue, interpolationMethod);
}

IVolumeDataReadAccessor<FloatVector4, float >* VolumeDataAccessManagerImpl::Create4DInterpolatingVolumeDataAccessorR32(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue, InterpolationMethod interpolationMethod)
{
  return CreateInterpolatingVolumeDataAccessor<FloatVector4, float >(volumeDataPageAccessor, replacementNoValue, interpolationMethod);
}
IVolumeDataReadAccessor<FloatVector4, double>* VolumeDataAccessManagerImpl::Create4DInterpolatingVolumeDataAccessorR64(VolumeDataPageAccessor* volumeDataPageAccessor, float replacementNoValue, InterpolationMethod interpolationMethod)
{
   return CreateInterpolatingVolumeDataAccessor<FloatVector4, double>(volumeDataPageAccessor, replacementNoValue, interpolationMethod);
}

}
