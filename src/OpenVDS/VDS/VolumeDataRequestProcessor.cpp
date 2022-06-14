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

#include "VolumeDataRequestProcessor.h"

#include "VolumeDataChunk.h"
#include "VolumeDataChannelMapping.h"
#include "VolumeDataAccessManagerImpl.h"
#include "VolumeDataLayoutImpl.h"
#include "VolumeDataPageImpl.h"
#include "DataBlock.h"
#include "DimensionGroup.h"
#include "CompilerDefines.h"
#include <OpenVDS/ValueConversion.h>
#include <OpenVDS/VolumeSampler.h>
#include "VDS.h"
#include "Env.h"

#include <cstdint>
#include <algorithm>
#include <atomic>
#include <thread>
#include <chrono>

#include <inttypes.h>

#include <fmt/format.h>
#include <exception>

namespace OpenVDS
{

const int MIN_MEMCPY = 4096;
template <unsigned int IVAL> struct FIND_SHIFT    { enum { RET = 1 + FIND_SHIFT<IVAL/2>::RET }; };
template <>                  struct FIND_SHIFT<1> { enum { RET = 0 }; };
template <>                  struct FIND_SHIFT<0> { enum { RET = 0 }; };

struct ConversionParameters
{
    float valueRangeMin;
    float valueRangeMax;
    float integerScale;
    float integerOffset;
    float noValue;
    float replacementNoValue;
    bool hasReplacementNoValue;
};

static ConversionParameters makeConversionParameters(VolumeDataLayer const *layer, bool hasReplacementNoValue, float replacementNoValue)
{
  ConversionParameters ret;
  ret.valueRangeMin = layer->GetValueRange().Min;
  ret.valueRangeMax = layer->GetValueRange().Max;
  ret.integerScale = layer->GetIntegerScale();
  ret.integerOffset = layer->GetIntegerOffset();
  ret.noValue = layer->GetNoValue();
  ret.replacementNoValue = replacementNoValue;
  ret.hasReplacementNoValue = hasReplacementNoValue;
  return ret;
}

static int32_t CombineAndReduceDimensions (int32_t (&sourceSize  )[DataBlock::Dimensionality_Max],
                                           int32_t (&sourceOffset)[DataBlock::Dimensionality_Max],
                                           int32_t (&targetSize  )[DataBlock::Dimensionality_Max],
                                           int32_t (&targetOffset)[DataBlock::Dimensionality_Max],
                                           int32_t (&overlapSize )[DataBlock::Dimensionality_Max],
                                           const int32_t (&origSourceSize  )[Dimensionality_Max],
                                           const int32_t (&origSourceOffset)[Dimensionality_Max],
                                           const int32_t (&origTargetSize  )[Dimensionality_Max],
                                           const int32_t (&origTargetOffset)[Dimensionality_Max],
                                           const int32_t (&origOverlapSize )[Dimensionality_Max])
{
    int32_t tmpSourceSize  [Dimensionality_Max] = {origSourceSize  [0],origSourceSize  [1],origSourceSize  [2],origSourceSize  [3],origSourceSize  [4],origSourceSize  [5]};
    int32_t tmpSourceOffset[Dimensionality_Max] = {origSourceOffset[0],origSourceOffset[1],origSourceOffset[2],origSourceOffset[3],origSourceOffset[4],origSourceOffset[5]};
    int32_t tmpTargetSize  [Dimensionality_Max] = {origTargetSize  [0],origTargetSize  [1],origTargetSize  [2],origTargetSize  [3],origTargetSize  [4],origTargetSize  [5]};
    int32_t tmpTargetOffset[Dimensionality_Max] = {origTargetOffset[0],origTargetOffset[1],origTargetOffset[2],origTargetOffset[3],origTargetOffset[4],origTargetOffset[5]};
    int32_t tmpOverlapSize [Dimensionality_Max] = {origOverlapSize [0],origOverlapSize [1],origOverlapSize [2],origOverlapSize [3],origOverlapSize [4],origOverlapSize [5]};

  // Combine dimensions where the overlap size is 1
  for (int32_t iCopyDimension = Dimensionality_Max - 1; iCopyDimension > 0; iCopyDimension--)
  {
    if (tmpOverlapSize[iCopyDimension] == 1)
    {
      tmpSourceOffset[iCopyDimension-1] += tmpSourceOffset[iCopyDimension] * tmpSourceSize[iCopyDimension-1];
      tmpTargetOffset[iCopyDimension-1] += tmpTargetOffset[iCopyDimension] * tmpTargetSize[iCopyDimension-1];
      tmpSourceOffset[iCopyDimension  ]  = 0;
      tmpTargetOffset[iCopyDimension  ]  = 0;

      tmpSourceSize[iCopyDimension-1] *= tmpSourceSize[iCopyDimension];
      tmpTargetSize[iCopyDimension-1] *= tmpTargetSize[iCopyDimension];
      tmpSourceSize[iCopyDimension  ]  = 1;
      tmpTargetSize[iCopyDimension  ]  = 1;
    }
  }

  if((tmpOverlapSize[0] + tmpSourceOffset[0] > tmpSourceSize[0]) ||
     (tmpOverlapSize[1] + tmpSourceOffset[1] > tmpSourceSize[1]) ||
     (tmpOverlapSize[2] + tmpSourceOffset[2] > tmpSourceSize[2]) ||
     (tmpOverlapSize[3] + tmpSourceOffset[3] > tmpSourceSize[3]) ||
     (tmpOverlapSize[4] + tmpSourceOffset[4] > tmpSourceSize[4]) ||
     (tmpOverlapSize[5] + tmpSourceOffset[5] > tmpSourceSize[5]) ||
     (tmpOverlapSize[0] + tmpTargetOffset[0] > tmpTargetSize[0]) ||
     (tmpOverlapSize[1] + tmpTargetOffset[1] > tmpTargetSize[1]) ||
     (tmpOverlapSize[2] + tmpTargetOffset[2] > tmpTargetSize[2]) ||
     (tmpOverlapSize[3] + tmpTargetOffset[3] > tmpTargetSize[3]) ||
     (tmpOverlapSize[4] + tmpTargetOffset[4] > tmpTargetSize[4]) ||
     (tmpOverlapSize[5] + tmpTargetOffset[5] > tmpTargetSize[5]))
  {
    assert(0 && "Invalid Copy Parameters #1");
  }

  int32_t nCopyDimensions = 0;

  // Reduce dimensions where the source and target size is 1 (and the offset is 0)
  for (int32_t dimension = 0; dimension < DataBlock::Dimensionality_Max; dimension++)
  {
    if(tmpSourceSize[dimension] == 1 && tmpTargetSize[dimension] == 1)
    {
      assert(tmpSourceOffset[dimension] == 0);
      assert(tmpTargetOffset[dimension] == 0);
      assert(tmpOverlapSize [dimension] == 1);
      continue;
    }

    sourceOffset[nCopyDimensions] = tmpSourceOffset[dimension];
    targetOffset[nCopyDimensions] = tmpTargetOffset[dimension];

    sourceSize[nCopyDimensions] = tmpSourceSize[dimension];
    targetSize[nCopyDimensions] = tmpTargetSize[dimension];

    overlapSize[nCopyDimensions] = tmpOverlapSize[dimension];

    nCopyDimensions++;
  }

  assert(nCopyDimensions <= DataBlock::Dimensionality_Max && "Invalid Copy Parameters #4");

  // Further combine inner dimensions if possible, to minimize number of copy invocations
  int32_t nCombineDimensions = nCopyDimensions - 1;

  for (int iCombineDimension = 0; iCombineDimension < nCombineDimensions; iCombineDimension++)
  {
    if (overlapSize[0] != sourceSize[0] || sourceSize[0] != targetSize[0])
    {
      break;
    }

    assert(sourceOffset[0] == 0 && "Invalid Copy Parameters #2");
    assert(targetOffset[0] == 0 && "Invalid Copy Parameters #3");

    sourceOffset[0] = sourceOffset[1] * sourceSize[0];
    targetOffset[0] = targetOffset[1] * targetSize[0];

    sourceSize [0] *= sourceSize [1];
    targetSize [0] *= targetSize [1];
    overlapSize[0] *= overlapSize[1];

    for (int dimension = 1; dimension < nCopyDimensions - 1; dimension++)
    {
      sourceOffset[dimension] = sourceOffset[dimension+1];
      targetOffset[dimension] = targetOffset[dimension+1];

      sourceSize [dimension] = sourceSize [dimension+1];
      targetSize [dimension] = targetSize [dimension+1];
      overlapSize[dimension] = overlapSize[dimension+1];
    }

    nCopyDimensions -= 1;
  }

  // Reset remaining dimensions
  for(int32_t dimension = nCopyDimensions; dimension < DataBlock::Dimensionality_Max; dimension++)
  {
    sourceOffset[dimension] = 0;
    targetOffset[dimension] = 0;

    sourceSize [dimension] = 1;
    targetSize [dimension] = 1;
    overlapSize[dimension] = 1;
  }

  return nCopyDimensions;
}

template <typename T, bool isUseNoValue>
static void CopyTo1Bit(uint8_t * __restrict target, int64_t targetBit, const QuantizingValueConverterWithNoValue<float, T, isUseNoValue> &valueConverter, const T * __restrict source, float noValue, int32_t count)
{
    target += targetBit / 8;
    uint8_t bits = *target;

    int32_t mask = 1;

    for(int32_t voxel = 0; voxel < count; voxel++)
    {
      float value = valueConverter.ConvertValue(*source++);
      if (!isUseNoValue || value != noValue)
      {
        bits |= (value != 0.0f) ? mask : 0;
      }

      mask <<= 1;
      if(mask == 0x100)
      {
        *target++ = bits;
        bits = 0;
        mask = 1;
      }
    }

    if(mask != 1)
    {
      if(bits & (mask >> 1))
      {
        while(mask != 0x100)
        {
          bits |= mask;
          mask <<= 1;
        }
      }
      *target++ = bits;
    }
}

template <typename T>
static void CopyFrom1Bit(T * __restrict target, const uint8_t * __restrict source, uint64_t bitIndex, int32_t count)
{
  source += bitIndex / 8;
  uint8_t bits = *source;
  int32_t mask = 1 << (bitIndex %  8);
  for (int i = 0; i < count; i++)
  {
    *target = (bits & mask)? T(1) : T(0);
    target++;
    mask <<= 1;
    if (mask == 0x100)
    {
      source++;
      bits = *source;
      mask = 1;
    }
  }
}

static force_inline void CopyBits(void* target, int64_t targetBit, const void* source, int64_t sourceBit, int32_t bits)
{
  while(bits--)
  {
    WriteElement(reinterpret_cast<bool *>(target), targetBit++, ReadElement(reinterpret_cast<const bool *>(source), sourceBit++));
  }
}

template <typename T>
static force_inline void CopyBytesT(T* __restrict target, const T* __restrict source, int32_t size)
{
  if (size >= MIN_MEMCPY)
  {
    memcpy (target, source, size_t(size));
  }
  else
  {
    int32_t nBigElements = size >> CALC_BIT_SHIFT(sizeof(T));
    for (int32_t iBigElement = 0; iBigElement < nBigElements; iBigElement++)
    {
      target [iBigElement] = source [iBigElement];
    }
    int32_t nTail = size & ((int32_t) sizeof (T) - 1);
    if (nTail)
    {
      assert(nTail <= 7 && "Invalid Sample Size Remainder\n");
      const uint8_t *sourceTail = (const uint8_t *) (source + nBigElements);
      uint8_t *targetTail = (uint8_t *) (target + nBigElements);

      int32_t iTail = 0;
      switch (nTail)
      {
      case 7: targetTail[iTail] = sourceTail[iTail]; iTail++; FALLTHROUGH;
      case 6: targetTail[iTail] = sourceTail[iTail]; iTail++; FALLTHROUGH;
      case 5: targetTail[iTail] = sourceTail[iTail]; iTail++; FALLTHROUGH;
      case 4: targetTail[iTail] = sourceTail[iTail]; iTail++; FALLTHROUGH;
      case 3: targetTail[iTail] = sourceTail[iTail]; iTail++; FALLTHROUGH;
      case 2: targetTail[iTail] = sourceTail[iTail]; iTail++; FALLTHROUGH;
      case 1: targetTail[iTail] = sourceTail[iTail]; 
      }
    }
  }
}

static force_inline void CopyBytes(void* target, const void* source, int32_t size)
{
  if (size >= int32_t(sizeof (int64_t)) && !((intptr_t) source & (sizeof (int64_t)-1)) && !((intptr_t) target & (sizeof (int64_t)-1)))
    CopyBytesT ((int64_t*) target, (int64_t*) source, size);
  else if (size >= int32_t(sizeof (int32_t)) && !((intptr_t) source & (sizeof (int32_t)-1)) && !((intptr_t) target & (sizeof (int32_t)-1)))
    CopyBytesT ((int32_t*) target, (int32_t*) source, size);
  else if (size >= int32_t(sizeof (int16_t)) && !((intptr_t) source & (sizeof (int16_t)-1)) && !((intptr_t) target & (sizeof (int16_t)-1)))
    CopyBytesT ((int16_t*) target, (int16_t*) source, size);
  else
    CopyBytesT ((int8_t*) target, (int8_t*) source, size);
}

template<typename T, typename S, bool noValue>
static void ConvertAndCopy(T * __restrict target, const S * __restrict source, const QuantizingValueConverterWithNoValue<T, S, noValue> &valueConverter, int32_t count)
{
  for (int i = 0; i < count; i++)
  {
    target[i] = valueConverter.ConvertValue(source[i]);
  }
}

template<typename T, typename S, bool noValue>
QuantizingValueConverterWithNoValue<T,S,noValue> createValueConverter(const ConversionParameters &cp)
{
  return QuantizingValueConverterWithNoValue<T, S, noValue>(cp.valueRangeMin, cp.valueRangeMax, cp.integerScale, cp.integerOffset, cp.noValue, cp.replacementNoValue);
}

template<typename T, bool targetOneBit, typename S, bool sourceOneBit, bool noValue>
struct BlockCopy
{
  static void Do(void       *target, const int32_t (&targetOffset)[DataBlock::Dimensionality_Max], const int32_t (&targetSize)[DataBlock::Dimensionality_Max],
                 void const *source, const int32_t (&sourceOffset)[DataBlock::Dimensionality_Max], const int32_t (&sourceSize)[DataBlock::Dimensionality_Max],
                 const int32_t (&overlapSize) [DataBlock::Dimensionality_Max], const ConversionParameters &conversionParamters)
  {
    int64_t sourceLocalBaseSize = ((((int64_t)sourceOffset[3] * sourceSize[2] + sourceOffset[2]) * sourceSize[1] + sourceOffset[1]) * sourceSize[0] + sourceOffset[0]) * (int64_t)sizeof(S);
    int64_t targetLocalBaseSize = ((((int64_t)targetOffset[3] * targetSize[2] + targetOffset[2]) * targetSize[1] + targetOffset[1]) * targetSize[0] + targetOffset[0]) * (int64_t)sizeof(T);

    const uint8_t *sourceLocalBase = reinterpret_cast<const uint8_t *>(source) + sourceLocalBaseSize;
    uint8_t *targetLocalBase = reinterpret_cast<uint8_t *>(target) + targetLocalBaseSize;

    QuantizingValueConverterWithNoValue<T, S, noValue> valueConverter = createValueConverter<T,S,noValue>(conversionParamters);
    QuantizingValueConverterWithNoValue<float, S, noValue> floatValueConverter = createValueConverter<float,S,noValue>(conversionParamters);

    for (int dimension3 = 0; dimension3 < overlapSize[3]; dimension3++)
    {
      for (int dimension2 = 0; dimension2 < overlapSize[2]; dimension2++)
      {
        for (int dimension1 = 0; dimension1 < overlapSize[1]; dimension1++)
        {
          int64_t sourceLocal = (((int64_t)dimension3 * sourceSize[2] + dimension2) * sourceSize[1] + dimension1) * (int64_t)sourceSize[0] * (int64_t)sizeof(S);
          int64_t targetLocal = (((int64_t)dimension3 * targetSize[2] + dimension2) * targetSize[1] + dimension1) * (int64_t)targetSize[0] * (int64_t)sizeof(T);
          if (targetOneBit)
          {
            if (sourceOneBit)
            {
              //should not reach this path
              assert(false);
            }
            else
            {
              CopyTo1Bit(static_cast<uint8_t *>(target), targetLocalBaseSize + targetLocal, floatValueConverter, reinterpret_cast<const S *>(sourceLocalBase + sourceLocal), conversionParamters.noValue, overlapSize[0]);
            }
          }
          else if(sourceOneBit)
          {
            CopyFrom1Bit(reinterpret_cast<T *>(targetLocalBase + targetLocal), static_cast<const uint8_t *>(source), sourceLocalBaseSize + sourceLocal, overlapSize[0]);
          } else
          {
            ConvertAndCopy(reinterpret_cast<T *>(targetLocalBase + targetLocal), reinterpret_cast<const S *>(sourceLocalBase + sourceLocal), valueConverter, overlapSize[0]);
          }
        }
      }
    }
  }
};

template<typename T, bool is1Bit>
struct BlockCopy<T, is1Bit, T, is1Bit, false>
{
  static void Do(void       *target, const int32_t (&targetOffset)[DataBlock::Dimensionality_Max], const int32_t (&targetSize)[DataBlock::Dimensionality_Max],
                 void const *source, const int32_t (&sourceOffset)[DataBlock::Dimensionality_Max], const int32_t (&sourceSize)[DataBlock::Dimensionality_Max],
                 const int32_t (&overlapSize) [DataBlock::Dimensionality_Max], const ConversionParameters &conversionParamters)
  {
    int64_t sourceLocalBaseSize = ((((int64_t)sourceOffset[3] * sourceSize[2] + sourceOffset[2]) * sourceSize[1] + sourceOffset[1]) * sourceSize[0] + sourceOffset[0]) * (int64_t)sizeof(T);
    int64_t targetLocalBaseSize = ((((int64_t)targetOffset[3] * targetSize[2] + targetOffset[2]) * targetSize[1] + targetOffset[1]) * targetSize[0] + targetOffset[0]) * (int64_t)sizeof(T);
    const uint8_t *sourceLocalBase = reinterpret_cast<const uint8_t *>(source) + sourceLocalBaseSize;
    uint8_t *targetLocalBase = reinterpret_cast<uint8_t *>(target) + targetLocalBaseSize;

    for (int dimension3 = 0; dimension3 < overlapSize[3]; dimension3++)
    {
      for (int dimension2 = 0; dimension2 < overlapSize[2]; dimension2++)
      {
        for (int dimension1 = 0; dimension1 < overlapSize[1]; dimension1++)
        {
          int64_t sourceLocal = (((int64_t)dimension3 * sourceSize[2] + dimension2) * sourceSize[1] + dimension1) * (int64_t)sourceSize[0] * (int64_t)sizeof(T);
          int64_t targetLocal = (((int64_t)dimension3 * targetSize[2] + dimension2) * targetSize[1] + dimension1) * (int64_t)targetSize[0] * (int64_t)sizeof(T);
          if (is1Bit)
          {
            CopyBits(target, targetLocalBaseSize + targetLocal, source, sourceLocalBaseSize + sourceLocal, overlapSize[0]);
          }
          else
          {
            CopyBytes(targetLocalBase + targetLocal, sourceLocalBase + sourceLocal, overlapSize[0] * (int32_t)sizeof(T));
          }
        }
      }
    }
  }
};

template<typename T, bool targetOneBit, typename S, bool sourceOneBit>
static void DispatchBlockCopy3(void *target, const int32_t (&targetOffset)[DataBlock::Dimensionality_Max], const int32_t (&targetSize)[DataBlock::Dimensionality_Max],
                              void const *source, const int32_t (&sourceOffset)[DataBlock::Dimensionality_Max], const int32_t (&sourceSize)[DataBlock::Dimensionality_Max],
                              const int32_t (&overlapSize) [DataBlock::Dimensionality_Max], const ConversionParameters &conversionParameters)
{
  if (conversionParameters.hasReplacementNoValue && !(targetOneBit && sourceOneBit))
    BlockCopy<T, targetOneBit, S, sourceOneBit, true>::Do(target, targetOffset, targetSize, source, sourceOffset, sourceSize, overlapSize, conversionParameters);
  else
    BlockCopy<T, targetOneBit, S, sourceOneBit, false>::Do(target, targetOffset, targetSize, source, sourceOffset, sourceSize, overlapSize, conversionParameters);
}
template<typename T, bool targetOneBit>
static void DispatchBlockCopy2(void *target, const int32_t (&targetOffset)[DataBlock::Dimensionality_Max], const int32_t (&targetSize)[DataBlock::Dimensionality_Max],
                              VolumeDataChannelDescriptor::Format sourceFormat,
                              void const *source, const int32_t (&sourceOffset)[DataBlock::Dimensionality_Max], const int32_t (&sourceSize)[DataBlock::Dimensionality_Max],
                              const int32_t (&overlapSize) [DataBlock::Dimensionality_Max], const ConversionParameters &conversionParameters)
{
  switch(sourceFormat)
  {
  case VolumeDataChannelDescriptor::Format_1Bit:
    return DispatchBlockCopy3<T, targetOneBit, uint8_t, true>(target, targetOffset, targetSize, source, sourceOffset, sourceSize, overlapSize, conversionParameters);
  case VolumeDataChannelDescriptor::Format_U8:
    return DispatchBlockCopy3<T, targetOneBit, uint8_t, false>(target, targetOffset, targetSize, source, sourceOffset, sourceSize, overlapSize, conversionParameters);
  case VolumeDataChannelDescriptor::Format_U16:
    return DispatchBlockCopy3<T, targetOneBit, uint16_t, false>(target, targetOffset, targetSize, source, sourceOffset, sourceSize, overlapSize, conversionParameters);
  case VolumeDataChannelDescriptor::Format_R32:
    return DispatchBlockCopy3<T, targetOneBit, float, false>(target, targetOffset, targetSize, source, sourceOffset, sourceSize, overlapSize, conversionParameters);
  case VolumeDataChannelDescriptor::Format_U32:
    return DispatchBlockCopy3<T, targetOneBit, uint32_t, false>(target, targetOffset, targetSize, source, sourceOffset, sourceSize, overlapSize, conversionParameters);
  case VolumeDataChannelDescriptor::Format_R64:
    return DispatchBlockCopy3<T, targetOneBit, double, false>(target, targetOffset, targetSize, source, sourceOffset, sourceSize, overlapSize, conversionParameters);
  case VolumeDataChannelDescriptor::Format_U64:
    return DispatchBlockCopy3<T, targetOneBit, uint64_t, false>(target, targetOffset, targetSize, source, sourceOffset, sourceSize, overlapSize, conversionParameters);
  case VolumeDataChannelDescriptor::Format_Any:
    return DispatchBlockCopy3<T, targetOneBit, uint8_t, false>(target, targetOffset, targetSize, source, sourceOffset, sourceSize, overlapSize, conversionParameters);
  }
}

static void DispatchBlockCopy(VolumeDataChannelDescriptor::Format destinationFormat,
                              void       *target, const int32_t (&targetOffset)[DataBlock::Dimensionality_Max], const int32_t (&targetSize)[DataBlock::Dimensionality_Max],
                              VolumeDataChannelDescriptor::Format sourceFormat,
                              void const *source, const int32_t (&sourceOffset)[DataBlock::Dimensionality_Max], const int32_t (&sourceSize)[DataBlock::Dimensionality_Max],
                              const int32_t (&overlapSize) [DataBlock::Dimensionality_Max], const ConversionParameters &conversionParamters)
{
  switch(destinationFormat)
  {
  case VolumeDataChannelDescriptor::Format_1Bit:
    return DispatchBlockCopy2<uint8_t, true>(target, targetOffset, targetSize, sourceFormat, source, sourceOffset, sourceSize, overlapSize, conversionParamters);
  case VolumeDataChannelDescriptor::Format_U8:
    return DispatchBlockCopy2<uint8_t, false>(target, targetOffset, targetSize, sourceFormat, source, sourceOffset, sourceSize, overlapSize, conversionParamters);
  case VolumeDataChannelDescriptor::Format_U16:
    return DispatchBlockCopy2<uint16_t, false>(target, targetOffset, targetSize, sourceFormat, source, sourceOffset, sourceSize, overlapSize, conversionParamters);
  case VolumeDataChannelDescriptor::Format_R32:
    return DispatchBlockCopy2<float, false>(target, targetOffset, targetSize, sourceFormat, source, sourceOffset, sourceSize, overlapSize, conversionParamters);
  case VolumeDataChannelDescriptor::Format_U32:
    return DispatchBlockCopy2<uint32_t, false>(target, targetOffset, targetSize, sourceFormat, source, sourceOffset, sourceSize, overlapSize, conversionParamters);
  case VolumeDataChannelDescriptor::Format_R64:
    return DispatchBlockCopy2<double, false>(target, targetOffset, targetSize, sourceFormat, source, sourceOffset, sourceSize, overlapSize, conversionParamters);
  case VolumeDataChannelDescriptor::Format_U64:
    return DispatchBlockCopy2<uint64_t, false>(target, targetOffset, targetSize, sourceFormat, source, sourceOffset, sourceSize, overlapSize, conversionParamters);
  case VolumeDataChannelDescriptor::Format_Any:
    return DispatchBlockCopy2<uint8_t, false>(target, targetOffset, targetSize, sourceFormat, source, sourceOffset, sourceSize, overlapSize, conversionParamters);
  }
}

template <class TYPE>
void
FixupBorder(TYPE * buffer, int offset, const int (&pitch)[DataBlock::Dimensionality_Max], BorderMode borderMode, const int (&borderMin)[DataBlock::Dimensionality_Max], const int (&borderSize)[DataBlock::Dimensionality_Max], const int (&fixupSize)[DataBlock::Dimensionality_Max])
{
  for (int dim3 = 0; dim3 < borderSize[3]; dim3++)
  {
    for (int dim2 = 0; dim2 < borderSize[2]; dim2++)
    {
      for (int dim1 = 0; dim1 < borderSize[1]; dim1++)
      {
        for (int dim0 = 0; dim0 < borderSize[0]; dim0++)
        {
          int
            writePos[DataBlock::Dimensionality_Max],
            readPos[DataBlock::Dimensionality_Max];

          writePos[0] = borderMin[0] + dim0;
          writePos[1] = borderMin[1] + dim1;
          writePos[2] = borderMin[2] + dim2;
          writePos[3] = borderMin[3] + dim3;

          int
            writeOffset = dim0 * pitch[0] +
                          dim1 * pitch[1] + 
                          dim2 * pitch[2] + 
                          dim3 * pitch[3] + offset;

          if(borderMode == BorderMode::Clear)
          {
            WriteElement(buffer, writeOffset, TYPE());
            continue;
          }
          else if(borderMode == BorderMode::Mirror)
          {
            for (int i = 0; i < DataBlock::Dimensionality_Max; i++)
            {
              int
                newPos = abs(writePos[i]) % (2 * fixupSize[i]);

              if (newPos >= fixupSize[i])
              {
                newPos = 2 * fixupSize[i] - newPos - 1;
              }

              readPos[i] = newPos;
            }
          }
          else if(borderMode == BorderMode::Repeat)
          {
            for (int i = 0; i < DataBlock::Dimensionality_Max; i++)
            {
              int
                newPos = writePos[i];

              if (newPos >= fixupSize[i])
              {
                newPos = fixupSize[i] - 1;
              }
              else if (newPos < 0)
              {
                newPos = 0;
              }

              readPos[i] = newPos;
            }
          }

          int
            readOffset = (readPos[0] - borderMin[0]) * pitch[0] +
                         (readPos[1] - borderMin[1]) * pitch[1] + 
                         (readPos[2] - borderMin[2]) * pitch[2] + 
                         (readPos[3] - borderMin[3]) * pitch[3] + offset;

          WriteElement(buffer, writeOffset, ReadElement(buffer, readOffset));
        }
      }
    }
  }
}

static void
FixupBorder(DataBlock const &dataBlock, void *buffer, VolumeDataFormat format, VolumeDataComponents components, BorderMode borderMode, const int (&borderNegativeRadius)[6], const int (&borderPositiveRadius)[6], const int (&layoutMin)[6], const int (&layoutSize)[6], const int (&layoutDimension)[DataBlock::Dimensionality_Max])
{
  int validOffset = 0; // Offset in elements of valid area inside datablock

  int pitch[DataBlock::Dimensionality_Max];
  int fixupMin[DataBlock::Dimensionality_Max];
  int fixupSize[DataBlock::Dimensionality_Max];
  int validMin[DataBlock::Dimensionality_Max];
  int validSize[DataBlock::Dimensionality_Max];

  for(int dataBlockDimension = 0; dataBlockDimension < DataBlock::Dimensionality_Max; dataBlockDimension++)
  {
    pitch[dataBlockDimension] = dataBlock.Pitch[dataBlockDimension];
    if(format == VolumeDataFormat::Format_1Bit && dataBlockDimension > 0)
    {
      pitch[dataBlockDimension] *= 8;
    }

    // Get fixup min/size
    if(layoutDimension[dataBlockDimension] >= 0)
    {
      assert(layoutDimension[dataBlockDimension] < 6);
      fixupMin[dataBlockDimension] = layoutMin[layoutDimension[dataBlockDimension]];
      fixupSize[dataBlockDimension] = layoutSize[layoutDimension[dataBlockDimension]];
    }
    else
    {
      fixupMin[dataBlockDimension] = 0;
      fixupSize[dataBlockDimension] = 1;
    }

    // Calculate valid min/size
    if(fixupMin[dataBlockDimension] < 0)
    {
      validMin[dataBlockDimension] = 0;
      validOffset += -layoutMin[layoutDimension[dataBlockDimension]] * pitch[dataBlockDimension];
    }
    else
    {
      validMin[dataBlockDimension] = fixupMin[dataBlockDimension];
    }

    validSize[dataBlockDimension] = std::min(fixupSize[dataBlockDimension], fixupMin[dataBlockDimension] + dataBlock.Size[dataBlockDimension]) - validMin[dataBlockDimension];
  }

  for(int fixupDimension = 0; fixupDimension < DataBlock::Dimensionality_Max; /* fixupDimension is increased at the end if no fixup was needed for this dimension */)
  {
    bool
      needsFixup = false;

    int borderMin[DataBlock::Dimensionality_Max];
    int borderSize[DataBlock::Dimensionality_Max];

    if(fixupMin[fixupDimension] < validMin[fixupDimension])
    {
      for(int dimension = 0; dimension < DataBlock::Dimensionality_Max; dimension++)
      {
        if(dimension == fixupDimension)
        {
          borderMin[dimension] = fixupMin[dimension];
          borderSize[dimension] = validMin[dimension] - fixupMin[dimension];
        }
        else
        {
          borderMin[dimension] = validMin[dimension];
          borderSize[dimension] = validSize[dimension];
        }
      }

      needsFixup = true;
    }
    else if(dataBlock.Size[fixupDimension] > validSize[fixupDimension])
    {
      assert(fixupMin[fixupDimension] == validMin[fixupDimension] && "We should already have fixed up the negative side");
      for(int dimension = 0; dimension < DataBlock::Dimensionality_Max; dimension++)
      {
        if(dimension == fixupDimension)
        {
          borderMin[dimension] = fixupSize[fixupDimension];
          borderSize[dimension] = dataBlock.Size[fixupDimension] - validSize[fixupDimension];
        }
        else
        {
          borderMin[dimension] = validMin[dimension];
          borderSize[dimension] = validSize[dimension];
        }
      }

      needsFixup = true;
    }

    if(needsFixup)
    {
      int borderOffset = validOffset + (borderMin[fixupDimension] - validMin[fixupDimension]) * pitch[fixupDimension];

      switch(components)
      {
      case VolumeDataComponents::Components_1:
        switch(format)
        {
        case VolumeDataFormat::Format_1Bit:
          FixupBorder((bool *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        case VolumeDataFormat::Format_U8:
          FixupBorder((uint8_t *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        case VolumeDataFormat::Format_U16:
          FixupBorder((uint16_t *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        case VolumeDataFormat::Format_R32:
          FixupBorder((float *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        case VolumeDataFormat::Format_U32:
          FixupBorder((uint32_t *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        case VolumeDataFormat::Format_R64:
          FixupBorder((double *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        case VolumeDataFormat::Format_U64:
          FixupBorder((uint64_t *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        default:
          throw std::runtime_error("Illegal datablock format");
        }
        break;

      case VolumeDataComponents::Components_2:
        switch(format)
        {
        case VolumeDataFormat::Format_1Bit:
          assert(0 && "Fixup is not implemented for multi-component 1-bit data");
          break;
        case VolumeDataFormat::Format_U8:
          FixupBorder((Vector<uint8_t,2> *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        case VolumeDataFormat::Format_U16:
          FixupBorder((Vector<uint16_t,2> *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        case VolumeDataFormat::Format_R32:
          FixupBorder((Vector<float,2> *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        case VolumeDataFormat::Format_U32:
          FixupBorder((Vector<uint32_t,2> *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        case VolumeDataFormat::Format_R64:
          FixupBorder((Vector<double,2> *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        case VolumeDataFormat::Format_U64:
          FixupBorder((Vector<uint64_t,2> *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        default:
          throw std::runtime_error("Illegal datablock format");
        }
        break;

      case VolumeDataComponents::Components_4:
        switch(format)
        {
        case VolumeDataFormat::Format_1Bit:
          assert(0 && "Fixup is not implemented for multi-component 1-bit data");
          break;
        case VolumeDataFormat::Format_U8:
          FixupBorder((Vector<uint8_t,4> *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        case VolumeDataFormat::Format_U16:
          FixupBorder((Vector<uint16_t,4> *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        case VolumeDataFormat::Format_R32:
          FixupBorder((Vector<float,4> *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        case VolumeDataFormat::Format_U32:
          FixupBorder((Vector<uint32_t,4> *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        case VolumeDataFormat::Format_R64:
          FixupBorder((Vector<double,4> *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        case VolumeDataFormat::Format_U64:
          FixupBorder((Vector<uint64_t,4> *)buffer, borderOffset, pitch, borderMode, borderMin, borderSize, fixupSize);
          break;
        default:
          throw std::runtime_error("Illegal datablock format");
        }
        break;

      default:
        throw std::runtime_error("Illegal number of datablock components");
      }

      // Increase valid size
      if(validMin[fixupDimension] > borderMin[fixupDimension])
      {
        assert(validMin[fixupDimension] - borderMin[fixupDimension] == borderSize[fixupDimension]);
        validMin[fixupDimension] = borderMin[fixupDimension];
        validOffset -= borderSize[fixupDimension] * pitch[fixupDimension];
      }
      validSize[fixupDimension] += borderSize[fixupDimension];
    }
    else
    {
      // This dimension is valid, go to next dimension
      assert(validSize[fixupDimension] == dataBlock.Size[fixupDimension]);
      fixupDimension++;
    }
  }
}

static bool RequestSubsetProcessPage(VolumeDataPageImpl* page, const VolumeDataChunk &chunk, const int32_t (&destMin)[Dimensionality_Max], const int32_t (&destMax)[Dimensionality_Max], VolumeDataChannelDescriptor::Format destinationFormat, const ConversionParameters &conversionParameters, void *destBuffer, Error &error)
{
  int32_t sourceMin[Dimensionality_Max];
  int32_t sourceMax[Dimensionality_Max];
  int32_t sourceMinExcludingMargin[Dimensionality_Max];
  int32_t sourceMaxExcludingMargin[Dimensionality_Max];

  page->GetMinMax(sourceMin, sourceMax);
  page->GetMinMaxExcludingMargin(sourceMinExcludingMargin, sourceMaxExcludingMargin);

  int32_t LOD = chunk.layer->GetLOD();

  VolumeDataLayoutImpl *volumeDataLayout = chunk.layer->GetLayout();

  int32_t overlapMin[Dimensionality_Max];
  int32_t overlapMax[Dimensionality_Max];

  int32_t sizeThisLOD[Dimensionality_Max];

  for (int32_t dimension = 0; dimension < Dimensionality_Max; dimension++)
  {
    overlapMin[dimension] = std::max(sourceMinExcludingMargin[dimension], destMin[dimension]);
    overlapMax[dimension] = std::min(sourceMaxExcludingMargin[dimension], destMax[dimension]);
    if (volumeDataLayout->IsDimensionLODDecimated(dimension))
    {
      sizeThisLOD[dimension] = GetLODSize(destMin[dimension], destMax[dimension], LOD);
    }
    else
    {
      sizeThisLOD[dimension] = destMax[dimension] - destMin[dimension];
    }
  }

  DimensionGroup sourceDimensionGroup = chunk.layer->GetChunkDimensionGroup();

  VolumeDataChannelDescriptor::Format sourceFormat = chunk.layer->GetFormat();
  bool sourceIs1Bit = (sourceFormat == VolumeDataChannelDescriptor::Format_1Bit);

  int32_t globalSourceSize[Dimensionality_Max];
  int32_t globalSourceOffset[Dimensionality_Max];
  int32_t globalTargetSize[Dimensionality_Max];
  int32_t globalTargetOffset[Dimensionality_Max];
  int32_t globalOverlapSize[Dimensionality_Max];

  for (int32_t dimension = 0; dimension < Dimensionality_Max; dimension++)
  {
    globalSourceSize[dimension] = 1;
    for (int iCopyDimension = 0; iCopyDimension < DataBlock::Dimensionality_Max; iCopyDimension++)
    {
      if (dimension == DimensionGroupUtil::GetDimension(sourceDimensionGroup, iCopyDimension))
      {

        globalSourceSize[dimension] = page->GetDataBlock().AllocatedSize[iCopyDimension];
        if (sourceIs1Bit && iCopyDimension == 0)
        {
          globalSourceSize[dimension] *= 8;
        }
        break;
      }
    }
    globalTargetSize[dimension] = sizeThisLOD[dimension];

    if (volumeDataLayout->IsDimensionLODDecimated(dimension))
    {
      globalSourceOffset[dimension] = (overlapMin[dimension] - sourceMin[dimension]) >> LOD;
      globalTargetOffset[dimension] = (overlapMin[dimension] - destMin[dimension]) >> LOD;
      globalOverlapSize[dimension] = GetLODSize(overlapMin[dimension], overlapMax[dimension], LOD, overlapMax[dimension] == destMax[dimension]);
    }
    else
    {
      globalSourceOffset[dimension] = (overlapMin[dimension] - sourceMin[dimension]);
      globalTargetOffset[dimension] = (overlapMin[dimension] - destMin[dimension]);
      globalOverlapSize[dimension] = (overlapMax[dimension] - overlapMin[dimension]);
    }
  }

  int32_t sourceSize[DataBlock::Dimensionality_Max];
  int32_t sourceOffset[DataBlock::Dimensionality_Max];
  int32_t targetSize[DataBlock::Dimensionality_Max];
  int32_t targetOffset[DataBlock::Dimensionality_Max];
  int32_t overlapSize[DataBlock::Dimensionality_Max];

  int32_t copyDimensions = CombineAndReduceDimensions(sourceSize, sourceOffset, targetSize, targetOffset, overlapSize, globalSourceSize, globalSourceOffset, globalTargetSize, globalTargetOffset, globalOverlapSize);
  (void) copyDimensions;

  // Multiply sizes and offsets by number of components since BlockCopy is not component-aware
  if(chunk.layer->GetComponents() > 1)
  {
    sourceSize  [0] *= int(chunk.layer->GetComponents());
    sourceOffset[0] *= int(chunk.layer->GetComponents());
    targetSize  [0] *= int(chunk.layer->GetComponents());
    targetOffset[0] *= int(chunk.layer->GetComponents());
    overlapSize [0] *= int(chunk.layer->GetComponents());
  }

  void *source = page->GetRawBufferInternal();

  DispatchBlockCopy(destinationFormat, destBuffer, targetOffset, targetSize,
    sourceFormat, source, sourceOffset, sourceSize,
    overlapSize, conversionParameters);

  return true;
}

struct Box
{
  int32_t min[Dimensionality_Max];
  int32_t max[Dimensionality_Max];
};

int64_t VolumeDataRequestProcessor::RequestVolumeSubset(void *buffer, VolumeDataLayer const *volumeDataLayer, const int32_t(&minRequested)[Dimensionality_Max], const int32_t (&maxRequested)[Dimensionality_Max], int32_t LOD, VolumeDataChannelDescriptor::Format format, bool isReplaceNoValue, float replacementNoValue)
{
  Box boxRequested;
  memcpy(boxRequested.min, minRequested, sizeof(boxRequested.min));
  memcpy(boxRequested.max, maxRequested, sizeof(boxRequested.max));

  // Initialized unused dimensions
  for (int32_t dimension = volumeDataLayer->GetLayout()->GetDimensionality(); dimension < Dimensionality_Max; dimension++)
  {
    boxRequested.min[dimension] = 0;
    boxRequested.max[dimension] = 1;
  }

  std::vector<VolumeDataChunk> chunksInRegion;

  volumeDataLayer->GetChunksInRegion(boxRequested.min, boxRequested.max, &chunksInRegion);

  if (chunksInRegion.size() == 0)
  {
    throw std::runtime_error("Requested volume subset does not contain any data");
  }

  ConversionParameters conversionParameters = makeConversionParameters(volumeDataLayer, isReplaceNoValue, replacementNoValue);

  return AddJob(chunksInRegion, [boxRequested, buffer, format, conversionParameters](VolumeDataPageImpl* page, VolumeDataChunk dataChunk, Error &error) {return RequestSubsetProcessPage(page, dataChunk, boxRequested.min, boxRequested.max, format, conversionParameters, buffer, error);}, format == VolumeDataChannelDescriptor::Format_1Bit);
}

int64_t VolumeDataRequestProcessor::RequestRemap(VolumeDataPageImpl& targetPage, std::vector<VolumeDataChunk> const &sourceChunks)
{
  class SharedData
  {
  public:
    std::mutex m_mutex;
    uint64_t   m_volumeDataHash;
    uint64_t   m_constantValueHash;
    int        m_chunksProcessed;
    const int  m_totalChunks;
    std::vector<uint8_t>
               m_buffer;
    SharedData(int totalChunks) : m_volumeDataHash(VolumeDataHash::UNKNOWN), m_constantValueHash(VolumeDataHash::UNKNOWN), m_chunksProcessed(0), m_totalChunks(totalChunks) {}
  };

  VolumeDataChunk targetDataChunk = static_cast<VolumeDataPageAccessorImpl&>(targetPage.GetVolumeDataPageAccessor()).GetLayer()->GetChunkFromIndex(targetPage.GetChunkIndex());

  int32_t size[4];
  targetDataChunk.layer->GetChunkVoxelSize(targetDataChunk.index, size);
  DataBlock targetDataBlock;
  Error error;

  if (!InitializeDataBlock(targetDataChunk.layer->GetFormat(), targetDataChunk.layer->GetComponents(), (enum DataBlock::Dimensionality)(targetDataChunk.layer->GetChunkDimensionality()), size, targetDataBlock, error))
  {
    throw std::runtime_error(error.string);
  }

  int32_t allocatedSize = GetAllocatedByteSize(targetDataBlock);
  auto sharedData = std::make_shared<SharedData>(int(sourceChunks.size()));
  sharedData->m_buffer.resize(allocatedSize);

  return AddJob(sourceChunks, [&targetPage, targetDataChunk, targetDataBlock, sharedData](VolumeDataPageImpl* sourcePage, VolumeDataChunk sourceDataChunk, Error &error) -> bool
  {
    VolumeDataLayer const *sourceLayer = sourceDataChunk.layer;
    VolumeDataLayer const *targetLayer = targetDataChunk.layer;
    assert(sourceLayer->GetFormat() == targetLayer->GetFormat() && "Cannot remap between layers with different format");
    assert(sourceLayer->GetComponents() == targetLayer->GetComponents() && "Cannot remap between layers with different number of components");
    assert(sourceLayer->GetVolumeDataChannelMapping() == targetLayer->GetVolumeDataChannelMapping() && "Cannot remap between layers with different mappings");

    ConversionParameters conversionParameters = makeConversionParameters(sourceLayer, false, 0.0f);

    int LOD = targetLayer->GetLOD();

    int targetMin[Dimensionality_Max];
    int targetMax[Dimensionality_Max];
    int targetMinExcludingMargin[Dimensionality_Max];
    int targetMaxExcludingMargin[Dimensionality_Max];

    targetPage.GetMinMax(targetMin, targetMax);
    targetPage.GetMinMaxExcludingMargin(targetMinExcludingMargin, targetMaxExcludingMargin);

    // Determine the region required to cover the target including margin
    int regionMin[Dimensionality_Max];
    int regionMax[Dimensionality_Max];

    for(int dimension = 0; dimension < Dimensionality_Max; dimension++)
    {
      int neededExtraValidVoxelsNegative = 0;
      int neededExtraValidVoxelsPositive = 0;

      // Do we have a render margin in this dimension?
      if(DimensionGroupUtil::IsDimensionInGroup(targetLayer->GetOriginalDimensionGroup(), dimension))
      {
        // Can we copy from source's render margin in this dimension?
        if(DimensionGroupUtil::IsDimensionInGroup(sourceLayer->GetOriginalDimensionGroup(), dimension))
        {
          neededExtraValidVoxelsNegative = std::max(0, targetLayer->GetNegativeRenderMargin() - sourceLayer->GetNegativeRenderMargin());
          neededExtraValidVoxelsPositive = std::max(0, targetLayer->GetPositiveRenderMargin() - sourceLayer->GetPositiveRenderMargin());
        }
        else
        {
          neededExtraValidVoxelsNegative = targetLayer->GetNegativeRenderMargin();
          neededExtraValidVoxelsPositive = targetLayer->GetPositiveRenderMargin();
        }
      }

      neededExtraValidVoxelsNegative += targetLayer->GetNegativeMargin(dimension) - sourceLayer->GetNegativeMargin(dimension),
      neededExtraValidVoxelsPositive += targetLayer->GetPositiveMargin(dimension) - sourceLayer->GetPositiveMargin(dimension);

      regionMin[dimension] = targetMinExcludingMargin[dimension] - std::max(0, neededExtraValidVoxelsNegative),
      regionMax[dimension] = targetMaxExcludingMargin[dimension] + std::max(0, neededExtraValidVoxelsPositive);

      // Limit the valid area so it doesn't extend into the border
      regionMin[dimension] = std::max(regionMin[dimension], targetLayer->GetDimensionFirstSample(dimension));
      regionMax[dimension] = std::min(regionMax[dimension], targetLayer->GetDimensionFirstSample(dimension) + targetLayer->GetDimensionNumSamples(dimension));
    }

    // Calculate overlapping region
    int sourceMin[Dimensionality_Max];
    int sourceMax[Dimensionality_Max];
    int sourceMinExcludingMargin[Dimensionality_Max];
    int sourceMaxExcludingMargin[Dimensionality_Max];

    int overlapMin[Dimensionality_Max];
    int overlapMax[Dimensionality_Max];

    sourcePage->GetMinMax(sourceMin, sourceMax);
    sourcePage->GetMinMaxExcludingMargin(sourceMinExcludingMargin, sourceMaxExcludingMargin);

    bool valid = true;

    for(int dimension = 0; dimension < Dimensionality_Max; dimension++)
    {
      int sourceMinOverlap = sourceMin[dimension];
      int sourceMaxOverlap = sourceMax[dimension];

      // Only copy margins from source at TARGETS margins
      if (sourceMinExcludingMargin[dimension] > regionMin[dimension])
      {
        sourceMinOverlap = sourceMinExcludingMargin[dimension];
      }

      if (sourceMaxExcludingMargin[dimension] < regionMax[dimension])
      {
        sourceMaxOverlap = sourceMaxExcludingMargin[dimension];
      }

      overlapMin[dimension] = std::max(targetMin[dimension], sourceMinOverlap);
      overlapMax[dimension] = std::min(targetMax[dimension], sourceMaxOverlap);

      if(overlapMin[dimension] >= overlapMax[dimension])
      {
        valid = false;
      }
    }

    int offsetTarget[Dimensionality_Max];
    int offsetSource[Dimensionality_Max];
    int globalOverlapSize[Dimensionality_Max];

    for(int dimension = 0; dimension < Dimensionality_Max; dimension++)
    {
      if (targetLayer->IsDimensionLODDecimated(dimension))
      {
        bool
          includePartialUpperVoxel = (sourceMaxExcludingMargin[dimension] >= regionMax[dimension]);

        offsetTarget[dimension] = (overlapMin[dimension] - targetMin[dimension]) >> LOD;
        offsetSource[dimension] = (overlapMin[dimension] - sourceMin[dimension]) >> LOD;
        globalOverlapSize[dimension]  = GetLODSize(overlapMin[dimension], overlapMax[dimension], LOD, includePartialUpperVoxel);
      }
      else
      {
        offsetTarget[dimension] = (overlapMin[dimension] - targetMin[dimension]);
        offsetSource[dimension] = (overlapMin[dimension] - sourceMin[dimension]);
        globalOverlapSize[dimension]  = (overlapMax[dimension] - overlapMin[dimension]);
      }
      assert(offsetTarget[dimension] >= 0);
      assert(offsetSource[dimension] >= 0);
      assert(globalOverlapSize[dimension] > 0 && "If there is no overlap it should have been caught earlier");
    }

    assert(valid && "Don't try to copy datablock area if there is no overlap");
    if(!valid) return true;

    // Update hash
    VolumeDataHash sourceHash(sourcePage->GetVolumeDataHash());
    HashCombiner hashCombiner(sourceHash);

    for(int dimension = 0; dimension < Dimensionality_Max; dimension++)
    {
      hashCombiner.Add(offsetSource[dimension]);
      hashCombiner.Add(offsetTarget[dimension]);
      hashCombiner.Add(globalOverlapSize[dimension]);
    }

    std::unique_lock<std::mutex> sharedDataLock(sharedData->m_mutex);

    sharedData->m_volumeDataHash ^= hashCombiner.GetCombinedHash();

    // Check if all items are constant
    if(sourceHash.IsConstant() && sharedData->m_chunksProcessed == 0)
    {
      sharedData->m_constantValueHash = uint64_t(sourceHash);
    }
    else if(!sourceHash.IsConstant() || sharedData->m_constantValueHash != uint64_t(sourceHash))
    {
      sharedData->m_constantValueHash = VolumeDataHash::UNKNOWN;
      sharedDataLock.unlock();
    }

    int globalSourceSize[Dimensionality_Max];

    for(int dimension = 0, chunkDimension = 0; dimension < Dimensionality_Max; dimension++)
    {
      if(dimension == DimensionGroupUtil::GetDimension(sourceLayer->GetChunkDimensionGroup(), chunkDimension))
      {
        globalSourceSize[dimension] = sourcePage->GetDataBlock().AllocatedSize[chunkDimension++];
      }
      else
      {
        globalSourceSize[dimension] = 1;
      }
    }

    int globalTargetSize[Dimensionality_Max];

    for(int dimension = 0, chunkDimension = 0; dimension < Dimensionality_Max; dimension++)
    {
      if(dimension == DimensionGroupUtil::GetDimension(targetLayer->GetChunkDimensionGroup(), chunkDimension))
      {
        globalTargetSize[dimension] = targetDataBlock.AllocatedSize[chunkDimension++];
      }
      else
      {
        globalTargetSize[dimension] = 1;
      }
    }

    // Convert byte size to bitsize for 1-bit data
    if(sourceLayer->GetFormat() == VolumeDataChannelDescriptor::Format_1Bit)
    {
      globalSourceSize[0] *= 8;
      globalTargetSize[0] *= 8;
    }

    int32_t sourceSize[DataBlock::Dimensionality_Max];
    int32_t sourceOffset[DataBlock::Dimensionality_Max];
    int32_t targetSize[DataBlock::Dimensionality_Max];
    int32_t targetOffset[DataBlock::Dimensionality_Max];
    int32_t overlapSize[DataBlock::Dimensionality_Max];

    int32_t copyDimensions = CombineAndReduceDimensions(sourceSize, sourceOffset, targetSize, targetOffset, overlapSize, globalSourceSize, offsetSource, globalTargetSize, offsetTarget, globalOverlapSize);
    (void) copyDimensions;

    // Multiply sizes and offsets by number of components since BlockCopy is not component-aware
    if(sourceLayer->GetComponents() > 1)
    {
      sourceSize  [0] *= int(targetLayer->GetComponents());
      sourceOffset[0] *= int(targetLayer->GetComponents());
      targetSize  [0] *= int(targetLayer->GetComponents());
      targetOffset[0] *= int(targetLayer->GetComponents());
      overlapSize [0] *= int(targetLayer->GetComponents());
    }

    DispatchBlockCopy(targetLayer->GetFormat(), sharedData->m_buffer.data(), targetOffset, targetSize,
                      sourceLayer->GetFormat(), sourcePage->GetRawBufferInternal(), sourceOffset, sourceSize,
                      overlapSize, conversionParameters);

    sharedDataLock.lock();
    if(++sharedData->m_chunksProcessed == sharedData->m_totalChunks)
    {
      sharedDataLock.unlock(); // At this point we have exclusive access to the sharedData since this is the last page

      bool
        needsBorderFixup = false;

      HashCombiner
        borderHashCombiner;

      if(targetLayer->GetBorderMode() != BorderMode::None)
      {
        borderHashCombiner.Add(targetLayer->GetBorderMode());
        for (int i = 0; i < Dimensionality_Max; i++)
        {
          int firstSample = targetLayer->GetDimensionFirstSample(i);

          if(targetMin[i] - firstSample < 0)
          {
            borderHashCombiner.Add(targetMin[i] - firstSample);
            needsBorderFixup = true;
          }
          if(targetMax[i] - firstSample > targetLayer->GetDimensionNumSamples(i))
          {
            borderHashCombiner.Add(targetMax[i] - firstSample - targetLayer->GetDimensionNumSamples(i));
            needsBorderFixup = true;
          }
          borderHashCombiner.Add(i);
        }
      }

      if(needsBorderFixup)
      {
        sharedData->m_volumeDataHash = sharedData->m_volumeDataHash ^ borderHashCombiner.GetCombinedHash();

        int borderNegativeRadius[Dimensionality_Max];
        int borderPositiveRadius[Dimensionality_Max];
        int layoutMin[Dimensionality_Max];
        int layoutSize[Dimensionality_Max];

        // get correct values based on LOD to send to the datablock, because datablock doesn't know about LOD
        for (int i = 0; i < Dimensionality_Max; i++)
        {
          int windowLODFactor = 1;

          if (targetLayer->IsDimensionLODDecimated(i))
          {
            windowLODFactor = 1 << LOD;
          }

          borderNegativeRadius[i] = targetLayer->GetNegativeBorder(i) / windowLODFactor;
          borderPositiveRadius[i] = targetLayer->GetPositiveBorder(i) / windowLODFactor;

          int minWithoutBorder = targetMin[i] - targetLayer->GetDimensionFirstSample(i) + targetLayer->GetNegativeBorder(i);
          assert(minWithoutBorder >= 0);

          if (targetLayer->GetLayout()->IsDimensionLODDecimated(i))
          {
            layoutMin[i] = (minWithoutBorder >> LOD) - borderNegativeRadius[i];
            layoutSize[i] = (targetLayer->GetDimensionNumSamples(i) + (1 << LOD) - 1) >> LOD;
          }
          else
          {
            layoutMin[i] = minWithoutBorder - borderNegativeRadius[i];
            layoutSize[i] = targetLayer->GetDimensionNumSamples(i);
          }
        }

        int layoutDimension[DataBlock::Dimensionality_Max];

        for (int i = 0; i < DataBlock::Dimensionality_Max; i++)
        {
          layoutDimension[i] = targetLayer->GetChunkDimension(i);
        }

        FixupBorder(targetDataBlock, sharedData->m_buffer.data(), targetLayer->GetFormat(), targetLayer->GetComponents(), targetLayer->GetBorderMode(), borderNegativeRadius, borderPositiveRadius, layoutMin, layoutSize, layoutDimension);
      }

      targetPage.SetBufferData(targetDataBlock, targetLayer->GetChunkDimensionGroup(), targetLayer->GetFormat() == VolumeDataFormat::Format_1Bit, std::move(sharedData->m_buffer), VolumeDataHash(sharedData->m_constantValueHash).IsConstant() ? sharedData->m_constantValueHash : sharedData->m_volumeDataHash);
    }
    return true;
  });
}

struct ProjectVars
{
  FloatVector4 voxelPlane;

  int requestedMin[Dimensionality_Max];
  int requestedMax[Dimensionality_Max];
  int requestedPitch[Dimensionality_Max];
  int requestedSizeThisLOD[Dimensionality_Max];

  int LOD;
  int projectionDimension;
  int projectedDimensions[2];

  int DataIndex(const int32_t (&voxelIndex)[Dimensionality_Max]) const
  {
    int dataIndex = 0;

    for (int i = 0; i < 6; i++)
    {
      int localChunkIndex = voxelIndex[i] - requestedMin[i];
      localChunkIndex >>= LOD;
      dataIndex += localChunkIndex * requestedPitch[i];
    }

    return dataIndex;
  }

  void VoxelIndex(const int32_t (&localChunkIndex)[Dimensionality_Max], int32_t (&voxelIndexR)[Dimensionality_Max]) const
  {
    for (int i = 0; i < 6; i++)
      voxelIndexR[i] = requestedMin[i] + (localChunkIndex[i] << LOD);
  }
};

struct IndexValues
{
  float valueRangeMin;
  float valueRangeMax;
  int32_t LOD;
  int32_t voxelMin[Dimensionality_Max];
  int32_t voxelMax[Dimensionality_Max];
  int32_t localChunkSamples[Dimensionality_Max];
  int32_t localChunkAllocatedSize[Dimensionality_Max];
  int32_t pitch[Dimensionality_Max];
  int32_t bitPitch[Dimensionality_Max];
  int32_t axisNumSamples[Dimensionality_Max];

  int32_t dataBlockSamples[DataBlock::Dimensionality_Max];
  int32_t dataBlockAllocatedSize[DataBlock::Dimensionality_Max];
  int32_t dataBlockPitch[DataBlock::Dimensionality_Max];
  int32_t dataBlockBitPitch[DataBlock::Dimensionality_Max];
  int32_t dimensionMap[DataBlock::Dimensionality_Max];


  float coordinateMin[Dimensionality_Max];
  float coordinateMax[Dimensionality_Max];

  bool isDimensionLODDecimated[Dimensionality_Max];

  void Initialize(const VolumeDataChunk &dataChunk, const DataBlock &dataBlock)
  {
    const VolumeDataLayout *dataLayout = dataChunk.layer->GetLayout();

    valueRangeMin = dataLayout->GetChannelDescriptor(dataChunk.layer->GetChannelIndex()).GetValueRangeMin();
    valueRangeMax = dataLayout->GetChannelDescriptor(dataChunk.layer->GetChannelIndex()).GetValueRangeMax();

    LOD = dataChunk.layer->GetLOD();
    dataChunk.layer->GetChunkMinMax(dataChunk.index, voxelMin, voxelMax, true);

    for (int dimension = 0; dimension < Dimensionality_Max; dimension++)
    {
      pitch[dimension] = 0;
      bitPitch[dimension] = 0;

      axisNumSamples[dimension] = dataLayout->GetDimensionNumSamples(dimension);
      coordinateMin[dimension] = (dimension < dataLayout->GetDimensionality()) ? dataLayout->GetDimensionMin(dimension) : 0;
      coordinateMax[dimension] = (dimension < dataLayout->GetDimensionality()) ? dataLayout->GetDimensionMax(dimension) : 0;

      localChunkSamples[dimension] = 1;
      isDimensionLODDecimated[dimension] = false;

      localChunkAllocatedSize[dimension] = 1;
    }

    for (int dimension = 0; dimension < DataBlock::Dimensionality_Max; dimension++)
    {
      dataBlockPitch[dimension] = dataBlock.Pitch[dimension];
      dataBlockAllocatedSize[dimension] = dataBlock.AllocatedSize[dimension];
      dataBlockSamples[dimension] = dataBlock.Size[dimension];

      for (int iDataBlockDim = 0; iDataBlockDim < DataBlock::Dimensionality_Max; iDataBlockDim++)
      {
        dataBlockBitPitch[iDataBlockDim] = dataBlockPitch[iDataBlockDim] * (iDataBlockDim == 0 ? 1 : 8);

        int dimension = DimensionGroupUtil::GetDimension(dataChunk.layer->GetChunkDimensionGroup(), iDataBlockDim);
        dimensionMap[iDataBlockDim] = dimension;
        if (dimension >= 0 && dimension < Dimensionality_Max)
        {
          pitch[dimension] = dataBlockPitch[iDataBlockDim];
          bitPitch[dimension] = dataBlockBitPitch[iDataBlockDim];
          localChunkAllocatedSize[dimension] = dataBlockAllocatedSize[iDataBlockDim];

          isDimensionLODDecimated[dimension] = (dataBlockSamples[iDataBlockDim] < voxelMax[dimension] - voxelMin[dimension]);
          localChunkSamples[dimension] = dataBlockSamples[iDataBlockDim];
        }
      }
    }
  }
};

static bool VoxelIndexInProcessArea(const IndexValues &indexValues, const int32_t (&iVoxelIndex)[Dimensionality_Max])
{
  bool ret = true;

  for (int i = 0; i < Dimensionality_Max; i++)
  {
    ret = ret && (iVoxelIndex[i] < indexValues.voxelMax[i]) && (iVoxelIndex[i] >= indexValues.voxelMin[i]);
  }

  return ret;
}

static void VoxelIndexToLocalIndexFloat(const IndexValues &indexValues, const float (&iVoxelIndex)[Dimensionality_Max], float (&localIndex)[Dimensionality_Max] )
  {
    for (int i = 0; i < Dimensionality_Max; i++)
      localIndex[i] = 0.0f;

    for (int i = 0; i < DataBlock::Dimensionality_Max; i++)
    {
      if (indexValues.dimensionMap[i] >= 0)
      {
        localIndex[i] = iVoxelIndex[indexValues.dimensionMap[i]] - indexValues.voxelMin[indexValues.dimensionMap[i]];
        localIndex[i] /= (1 << (indexValues.isDimensionLODDecimated[indexValues.dimensionMap[i]] ? indexValues.LOD : 0));
      }
    }
}

template <typename T, typename S, InterpolationMethod INTERPMETHOD, bool isUseNoValue>
void ProjectValuesKernelInner(T *output, const S *input, const ProjectVars &projectVars, const IndexValues &inputIndexer, const int32_t (&voxelOutIndex)[Dimensionality_Max], VolumeSampler<S, INTERPMETHOD, isUseNoValue> &sampler, QuantizingValueConverterWithNoValue<T, typename InterpolatedRealType<S>::type, isUseNoValue> &converter, float voxelCenterOffset)
{
  float zValue = (projectVars.voxelPlane.X * (voxelOutIndex[projectVars.projectedDimensions[0]] + voxelCenterOffset) + projectVars.voxelPlane.Y * (voxelOutIndex[projectVars.projectedDimensions[1]] + voxelCenterOffset) + projectVars.voxelPlane.T) / (-projectVars.voxelPlane.Z);

  //clamp so it's inside the volume
  if (zValue < 0) zValue = 0;
  else if (zValue >= inputIndexer.axisNumSamples[projectVars.projectionDimension]) zValue = (float)(inputIndexer.axisNumSamples[projectVars.projectionDimension] - 1);

  float voxelCenterInIndex[Dimensionality_Max];
  voxelCenterInIndex[0] = (float)voxelOutIndex[0] + voxelCenterOffset;
  voxelCenterInIndex[1] = (float)voxelOutIndex[1] + voxelCenterOffset;
  voxelCenterInIndex[2] = (float)voxelOutIndex[2] + voxelCenterOffset;
  voxelCenterInIndex[3] = (float)voxelOutIndex[3] + voxelCenterOffset;
  voxelCenterInIndex[4] = (float)voxelOutIndex[4] + voxelCenterOffset;
  voxelCenterInIndex[5] = (float)voxelOutIndex[5] + voxelCenterOffset;

  voxelCenterInIndex[projectVars.projectionDimension] = zValue;

  int32_t voxelInIndexInt[Dimensionality_Max];
  voxelInIndexInt[0] = voxelOutIndex[0];
  voxelInIndexInt[1] = voxelOutIndex[1];
  voxelInIndexInt[2] = voxelOutIndex[2];
  voxelInIndexInt[3] = voxelOutIndex[3];
  voxelInIndexInt[4] = voxelOutIndex[4];
  voxelInIndexInt[5] = voxelOutIndex[5];

  voxelInIndexInt[projectVars.projectionDimension] = (int)zValue;

  if (VoxelIndexInProcessArea(inputIndexer, voxelInIndexInt))
  {
    float localInIndex[Dimensionality_Max];
    VoxelIndexToLocalIndexFloat(inputIndexer, voxelCenterInIndex, localInIndex);
    FloatVector3 localInIndex3D(localInIndex[0], localInIndex[1], localInIndex[2]);

    typedef typename InterpolatedRealType<S>::type TREAL;
    TREAL value = sampler.Sample3D(input, localInIndex3D);

    WriteElement(output, projectVars.DataIndex(voxelOutIndex), converter.ConvertValue(value));
  }
}

template <typename T, typename S, InterpolationMethod INTERPMETHOD, bool isUseNoValue>
void ProjectValuesKernel(T *pxOutput, const S *pxInput, const ProjectVars &projectVars, const IndexValues &indexValues, float scale, float offset, float noValue)
{
  VolumeSampler<S, INTERPMETHOD, isUseNoValue> sampler(indexValues.dataBlockSamples, indexValues.dataBlockPitch, indexValues.valueRangeMin, indexValues.valueRangeMax, scale, offset, noValue, noValue);
  QuantizingValueConverterWithNoValue<T, typename InterpolatedRealType<S>::type, isUseNoValue> converter(indexValues.valueRangeMin, indexValues.valueRangeMax, scale, offset, noValue, noValue, false);

  int32_t numSamples[2];
  int32_t offsetPair[2];

  for (int i = 0; i < 2; i++)
  {
    int nMin = std::max(projectVars.requestedMin[projectVars.projectedDimensions[i]], indexValues.voxelMin[projectVars.projectedDimensions[i]]);
    int nMax = std::min(projectVars.requestedMax[projectVars.projectedDimensions[i]], indexValues.voxelMax[projectVars.projectedDimensions[i]]);

    numSamples[i] = GetLODSize(nMin, nMax, projectVars.LOD, nMax == projectVars.requestedMax[projectVars.projectedDimensions[i]]);
    offsetPair[i] = (nMin - projectVars.requestedMin[projectVars.projectedDimensions[i]]) >> projectVars.LOD;
  }

  float voxelCenterOffset = (1 << projectVars.LOD) / 2.0f;

  // we can keep this to two dimensions because we know the input chunk is 3D
//#pragma omp parallel for
  for (int dimension1 = 0; dimension1 < numSamples[1]; dimension1++)
  for (int dimension0 = 0; dimension0 < numSamples[0]; dimension0++)
  {
    // this looks really strange, but since we know that the chunk dimension group for the input is always the projected and projection dimensions, this works
    int32_t localChunkIndex[Dimensionality_Max];
    for (int i = 0; i < 6; i++)
      localChunkIndex[i] = (indexValues.voxelMin[i] - projectVars.requestedMin[i]) >> projectVars.LOD;

    localChunkIndex[projectVars.projectedDimensions[0]] = dimension0 + offsetPair[0];
    localChunkIndex[projectVars.projectedDimensions[1]] = dimension1 + offsetPair[1];
    localChunkIndex[projectVars.projectionDimension] = 0;

    int32_t voxelIndex[Dimensionality_Max];
    projectVars.VoxelIndex(localChunkIndex, voxelIndex);
    ProjectValuesKernelInner<T, S, INTERPMETHOD, isUseNoValue>(pxOutput, pxInput, projectVars, indexValues, voxelIndex, sampler, converter, voxelCenterOffset);
  }
}

template <typename T, typename S, bool isUseNoValue>
static void DispatchProjectValues3(void *output, const void *input, const ProjectVars &projectVars, const IndexValues &indexValues, InterpolationMethod interpolationMethod, float scale, float offset, float noValue)
{
  switch(interpolationMethod)
  {
  case InterpolationMethod::Nearest: ProjectValuesKernel<T, S, InterpolationMethod::Nearest, isUseNoValue>(static_cast<T *>(output), static_cast<const S *>(input), projectVars, indexValues, scale, offset, noValue); break;
  case InterpolationMethod::Linear:  ProjectValuesKernel<T, S, InterpolationMethod::Linear,  isUseNoValue>(static_cast<T *>(output), static_cast<const S *>(input), projectVars, indexValues, scale, offset, noValue); break;
  case InterpolationMethod::Cubic:   ProjectValuesKernel<T, S, InterpolationMethod::Cubic,   isUseNoValue>(static_cast<T *>(output), static_cast<const S *>(input), projectVars, indexValues, scale, offset, noValue); break;
  case InterpolationMethod::Angular: ProjectValuesKernel<T, S, InterpolationMethod::Angular, isUseNoValue>(static_cast<T *>(output), static_cast<const S *>(input), projectVars, indexValues, scale, offset, noValue); break;
  case InterpolationMethod::Triangular: ProjectValuesKernel<T, S, InterpolationMethod::Triangular, isUseNoValue>(static_cast<T *>(output), static_cast<const S *>(input), projectVars, indexValues, scale, offset, noValue); break;
  }
}

template<typename T, bool isUseNoValue>
static void DispatchProjectValues2(void *output, const void *input, VolumeDataChannelDescriptor::Format sourceFormat, const ProjectVars &projectVars, const IndexValues &indexValues, InterpolationMethod interpolationMethod, float scale, float offset, float noValue)
{
  switch(sourceFormat)
  {
  case VolumeDataChannelDescriptor::Format_1Bit: DispatchProjectValues3<T, bool, isUseNoValue>(output, input, projectVars, indexValues, interpolationMethod, scale, offset, isUseNoValue); break;
  case VolumeDataChannelDescriptor::Format_U8: DispatchProjectValues3<T, uint8_t, isUseNoValue>(output, input, projectVars, indexValues, interpolationMethod, scale, offset, isUseNoValue); break;
  case VolumeDataChannelDescriptor::Format_U16: DispatchProjectValues3<T, uint16_t, isUseNoValue>(output, input, projectVars, indexValues, interpolationMethod, scale, offset, isUseNoValue); break;
  case VolumeDataChannelDescriptor::Format_R32: DispatchProjectValues3<T, float, isUseNoValue>(output, input, projectVars, indexValues, interpolationMethod, scale, offset, isUseNoValue); break;
  case VolumeDataChannelDescriptor::Format_U32: DispatchProjectValues3<T, uint32_t, isUseNoValue>(output, input, projectVars, indexValues, interpolationMethod, scale, offset, isUseNoValue); break;
  case VolumeDataChannelDescriptor::Format_R64: DispatchProjectValues3<T, double, isUseNoValue>(output, input, projectVars, indexValues, interpolationMethod, scale, offset, isUseNoValue); break;
  case VolumeDataChannelDescriptor::Format_U64: DispatchProjectValues3<T, uint32_t, isUseNoValue>(output, input, projectVars, indexValues, interpolationMethod, scale, offset, isUseNoValue); break;
  case VolumeDataChannelDescriptor::Format_Any: return;
  }
}

template<bool isUseNoValue>
static void DispatchProjectValues1(void *output, VolumeDataChannelDescriptor::Format targetFormat, const void *input, VolumeDataChannelDescriptor::Format sourceFormat, const ProjectVars &projectVars, const IndexValues &indexValues, InterpolationMethod interpolationMethod, float scale, float offset, float noValue)
{
  switch(targetFormat)
  {
  case VolumeDataChannelDescriptor::Format_1Bit: DispatchProjectValues2<bool, isUseNoValue>(output, input, sourceFormat, projectVars, indexValues, interpolationMethod, scale, offset, isUseNoValue); break;
  case VolumeDataChannelDescriptor::Format_U8: DispatchProjectValues2<uint8_t, isUseNoValue>(output, input, sourceFormat, projectVars, indexValues, interpolationMethod, scale, offset, isUseNoValue); break;
  case VolumeDataChannelDescriptor::Format_U16: DispatchProjectValues2<uint16_t, isUseNoValue>(output, input, sourceFormat, projectVars, indexValues, interpolationMethod, scale, offset, isUseNoValue); break;
  case VolumeDataChannelDescriptor::Format_R32: DispatchProjectValues2<float, isUseNoValue>(output, input, sourceFormat, projectVars, indexValues, interpolationMethod, scale, offset, isUseNoValue); break;
  case VolumeDataChannelDescriptor::Format_U32: DispatchProjectValues2<uint32_t, isUseNoValue>(output, input, sourceFormat, projectVars, indexValues, interpolationMethod, scale, offset, isUseNoValue); break;
  case VolumeDataChannelDescriptor::Format_R64: DispatchProjectValues2<double, isUseNoValue>(output, input, sourceFormat, projectVars, indexValues, interpolationMethod, scale, offset, isUseNoValue); break;
  case VolumeDataChannelDescriptor::Format_U64: DispatchProjectValues2<uint32_t, isUseNoValue>(output, input, sourceFormat, projectVars, indexValues, interpolationMethod, scale, offset, isUseNoValue); break;
  case VolumeDataChannelDescriptor::Format_Any: break;
  }
}

static void DispatchProjectValues(void *output, VolumeDataChannelDescriptor::Format targetFormat, const void *input, VolumeDataChannelDescriptor::Format sourceFormat, const ProjectVars &projectVars, const IndexValues &indexValues, InterpolationMethod interpolationMethod, float scale, float offset, bool isUseNoValue, float noValue)
{
  if (isUseNoValue)
    DispatchProjectValues1<true>(output, targetFormat, input, sourceFormat, projectVars, indexValues, interpolationMethod, scale, offset, noValue);
  else
    DispatchProjectValues1<false>(output, targetFormat, input, sourceFormat, projectVars, indexValues, interpolationMethod, scale, offset, noValue);
}

static bool RequestProjectedVolumeSubsetProcessPage(VolumeDataPageImpl* page, const VolumeDataChunk &chunk, const int32_t (&destMin)[Dimensionality_Max], const int32_t (&destMax)[Dimensionality_Max], DimensionGroup projectedDimensionsEnum, FloatVector4 voxelPlane, VolumeDataChannelDescriptor::Format targetFormat,  InterpolationMethod interpolationMethod, bool useNoValue, float noValue, void *targetBuffer, Error &error)
{
  VolumeDataChannelDescriptor::Format sourceFormat = chunk.layer->GetFormat();

  VolumeDataLayer const *volumeDataLayer = chunk.layer;

  DataBlock const &dataBlock = page->GetDataBlock();

  if (dataBlock.Components != VolumeDataChannelDescriptor::Components_1)
  {
    error.string = "Cannot request volume subset from multi component VDSs";
    error.code = -1;
    return false;
  }

  int32_t LOD = volumeDataLayer->GetLOD();

  int32_t projectionDimension = -1;
  int32_t projectedDimensions[2] = { -1, -1 };

  if (DimensionGroupUtil::GetDimensionality(volumeDataLayer->GetChunkDimensionGroup()) < 3)
  {
    error.string = "The requested dimension group must contain at least 3 dimensions.";
    error.code = -1;
    return false;
  }

  if (DimensionGroupUtil::GetDimensionality(projectedDimensionsEnum) != 2)
  {
    error.string = "The projected dimension group must contain 2 dimensions.";
    error.code = -1;
    return false;
  }

  for (int dimensionIndex = 0; dimensionIndex < DimensionGroupUtil::GetDimensionality(volumeDataLayer->GetChunkDimensionGroup()); dimensionIndex++)
  {
    int32_t dimension = DimensionGroupUtil::GetDimension(volumeDataLayer->GetChunkDimensionGroup(), dimensionIndex);

    if (!DimensionGroupUtil::IsDimensionInGroup(projectedDimensionsEnum, dimension))
    {
      projectionDimension = dimension;
    }
  }

  assert(projectionDimension != -1);

  for (int32_t dimensionIndex = 0, projectionDimensionality = DimensionGroupUtil::GetDimensionality(projectedDimensionsEnum); dimensionIndex < projectionDimensionality; dimensionIndex++)
  {
    int32_t dimension = DimensionGroupUtil::GetDimension(projectedDimensionsEnum, dimensionIndex);
    projectedDimensions[dimensionIndex] = dimension;

    if (!DimensionGroupUtil::IsDimensionInGroup(volumeDataLayer->GetChunkDimensionGroup(), dimension))
    {
      error.string = "The requested dimension group must contain the dimensions of the projected dimension group.";
      error.code = -1;
      return false;
    }
  }

  int32_t sizeThisLOD[Dimensionality_Max];
  for (int32_t dimension = 0; dimension < Dimensionality_Max; dimension++)
  {
    if (chunk.layer->GetLayout()->IsDimensionLODDecimated(dimension))
    {
      sizeThisLOD[dimension] = GetLODSize(destMin[dimension], destMax[dimension], LOD);
    }
    else
    {
      sizeThisLOD[dimension] = destMax[dimension] - destMin[dimension];
    }
  }

  ProjectVars projectVars;
  for (int dimension = 0; dimension < Dimensionality_Max; dimension++)
  {
    projectVars.requestedMin[dimension] = destMin[dimension];
    projectVars.requestedMax[dimension] = destMax[dimension];
    projectVars.requestedSizeThisLOD[dimension] = sizeThisLOD[dimension];
    projectVars.requestedPitch[dimension] = (dimension == 0) ? 1 : projectVars.requestedPitch[dimension - 1] * projectVars.requestedSizeThisLOD[dimension - 1];
  }

  projectVars.LOD = LOD;
  projectVars.voxelPlane = FloatVector4(voxelPlane.X, voxelPlane.Y, voxelPlane.Z, voxelPlane.T);
  projectVars.projectionDimension = projectionDimension;
  projectVars.projectedDimensions[0] = projectedDimensions[0];
  projectVars.projectedDimensions[1] = projectedDimensions[1];

  const void* sourceBuffer = page->GetRawBufferInternal();

  IndexValues indexValues;
  indexValues.Initialize(chunk, page->GetDataBlock());

  DispatchProjectValues(targetBuffer, targetFormat, sourceBuffer, sourceFormat, projectVars, indexValues, interpolationMethod, volumeDataLayer->GetIntegerScale(), volumeDataLayer->GetIntegerOffset(), useNoValue, noValue);
  return true;
}

int64_t
VolumeDataRequestProcessor::RequestProjectedVolumeSubset(void *buffer, VolumeDataLayer const *volumeDataLayer, const int32_t (&minRequested)[Dimensionality_Max], const int32_t (&maxRequested)[Dimensionality_Max], FloatVector4 const &voxelPlane, DimensionGroup projectedDimensions, int32_t LOD, VolumeDataChannelDescriptor::Format format, InterpolationMethod interpolationMethod, bool isReplaceNoValue, float replacementNoValue)
{
  Box boxRequested;
  memcpy(boxRequested.min, minRequested, sizeof(boxRequested.min));
  memcpy(boxRequested.max, maxRequested, sizeof(boxRequested.max));

  // Initialized unused dimensions
  for (int32_t dimension = volumeDataLayer->GetLayout()->GetDimensionality(); dimension < Dimensionality_Max; dimension++)
  {
    boxRequested.min[dimension] = 0;
    boxRequested.max[dimension] = 1;
  }

  std::vector<VolumeDataChunk> chunksInRegion;

  int32_t projectionDimension = -1;
  int32_t projectionDimensionPosition = -1;
  int32_t projectedDimensionsPair[2] = { -1, -1 };

  int32_t layerDimensionGroup = DimensionGroupUtil::GetDimensionality(volumeDataLayer->GetChunkDimensionGroup());
  if (layerDimensionGroup < 3)
  {
    throw std::runtime_error("The requested dimension group must contain at least 3 dimensions.");
  }

  if (DimensionGroupUtil::GetDimensionality(projectedDimensions) != 2)
  {
    throw std::runtime_error("The projected dimension group must contain 2 dimensions.");
  }

  for (int dimensionIndex = 0; dimensionIndex < layerDimensionGroup; dimensionIndex++)
  {
    int32_t dimension = DimensionGroupUtil::GetDimension(volumeDataLayer->GetChunkDimensionGroup(), dimensionIndex);

    if (!DimensionGroupUtil::IsDimensionInGroup(projectedDimensions, dimension))
    {
      projectionDimension = dimension;
      projectionDimensionPosition = dimensionIndex;

      // min/max in the projection dimension is not used
      boxRequested.min[dimension] = 0;
      boxRequested.max[dimension] = 1;
    }
  }

  assert(projectionDimension != -1);

  for (int dimensionIndex = 0; dimensionIndex < DimensionGroupUtil::GetDimensionality(projectedDimensions); dimensionIndex++)
  {
    int32_t dimension = DimensionGroupUtil::GetDimension(projectedDimensions, dimensionIndex);
    projectedDimensionsPair[dimensionIndex] = dimension;

    if (!DimensionGroupUtil::IsDimensionInGroup(volumeDataLayer->GetChunkDimensionGroup(), dimension))
    {
      throw std::runtime_error("The requested dimension group must contain the dimensions of the projected dimension group.");
    }

    if (!volumeDataLayer->IsDimensionLODDecimated(dimension) && LOD > 0)
    {
      throw std::runtime_error("Cannot project subsets with a full-resolution dimension at LOD > 0.");
    }
  }

  //Swap components of VoxelPlane based on projection dimension
  FloatVector4 voxelPlaneSwapped(voxelPlane);

  assert(projectionDimensionPosition != -1);
  if (projectionDimensionPosition < 2)
  {
    //need to swap
    if (projectionDimensionPosition == 1)
    {
      voxelPlaneSwapped.Y = voxelPlane.Z;
      voxelPlaneSwapped.Z = voxelPlane.Y;
    }
    else
    {
      voxelPlaneSwapped.X = voxelPlane.Y;
      voxelPlaneSwapped.Y = voxelPlane.Z;
      voxelPlaneSwapped.Z = voxelPlane.X;
    }
  }

  if (voxelPlaneSwapped.Z == 0)
  {
    throw std::runtime_error("The Voxel plane cannot be perpendicular to the projected dimensions.");
  }

  std::vector<VolumeDataChunk> chunksInProjectedRegion;
  std::vector<VolumeDataChunk> chunksIntersectingPlane;

  volumeDataLayer->GetChunksInRegion(boxRequested.min,
                                     boxRequested.max,
                                     &chunksInProjectedRegion);

  for (int iChunk = 0; iChunk < int(chunksInProjectedRegion.size()); iChunk++)
  {
    int32_t min[Dimensionality_Max];
    int32_t max[Dimensionality_Max];

    chunksInProjectedRegion[iChunk].layer->GetChunkMinMax(chunksInProjectedRegion[iChunk].index, min, max, true);

    for (int dimensionIndex = 0; dimensionIndex < 2; dimensionIndex++)
    {
      int32_t dimension = projectedDimensionsPair[dimensionIndex];

      if (min[dimension] < minRequested[dimension]) min[dimension] = minRequested[dimension];
      if (max[dimension] > maxRequested[dimension]) max[dimension] = maxRequested[dimension];
    }

    int32_t corners[4];

    corners[0] = (int32_t)(((voxelPlaneSwapped.X * min[projectedDimensionsPair[0]] + voxelPlaneSwapped.Y * min[projectedDimensionsPair[1]] + voxelPlaneSwapped.T) / (-voxelPlaneSwapped.Z)) + 0.5f);
    corners[1] = (int32_t)(((voxelPlaneSwapped.X * min[projectedDimensionsPair[0]] + voxelPlaneSwapped.Y * max[projectedDimensionsPair[1]] + voxelPlaneSwapped.T) / (-voxelPlaneSwapped.Z)) + 0.5f);
    corners[2] = (int32_t)(((voxelPlaneSwapped.X * max[projectedDimensionsPair[0]] + voxelPlaneSwapped.Y * min[projectedDimensionsPair[1]] + voxelPlaneSwapped.T) / (-voxelPlaneSwapped.Z)) + 0.5f);
    corners[3] = (int32_t)(((voxelPlaneSwapped.X * max[projectedDimensionsPair[0]] + voxelPlaneSwapped.Y * max[projectedDimensionsPair[1]] + voxelPlaneSwapped.T) / (-voxelPlaneSwapped.Z)) + 0.5f);

    int32_t nMin = corners[0];
    int32_t nMax = corners[0] + 1;

    for (int i = 1; i < 4; i++)
    {
      if (corners[i] < nMin) nMin = corners[i];
      if (corners[i] + 1 > nMax) nMax = corners[i] + 1;
    }

    Box boxProjected = boxRequested;

    boxProjected.min[projectionDimension] = nMin;
    boxProjected.max[projectionDimension] = nMax;

    boxProjected.min[projectedDimensionsPair[0]] = min[projectedDimensionsPair[0]];
    boxProjected.max[projectedDimensionsPair[0]] = max[projectedDimensionsPair[0]];

    boxProjected.min[projectedDimensionsPair[1]] = min[projectedDimensionsPair[1]];
    boxProjected.max[projectedDimensionsPair[1]] = max[projectedDimensionsPair[1]];

    volumeDataLayer->GetChunksInRegion(boxProjected.min,
                                       boxProjected.max,
                                            &chunksIntersectingPlane);

    for (int iChunkInPlane = 0; iChunkInPlane < int(chunksIntersectingPlane.size()); iChunkInPlane++)
    {
      VolumeDataChunk &chunkInIntersectingPlane = chunksIntersectingPlane[iChunkInPlane];
      auto chunk_it = std::find_if(chunksInRegion.begin(), chunksInRegion.end(), [&chunkInIntersectingPlane] (const VolumeDataChunk &a) { return a.index == chunkInIntersectingPlane.index && a.layer == chunkInIntersectingPlane.layer; });
      if (chunk_it == chunksInRegion.end())
      {
        chunksInRegion.push_back(chunkInIntersectingPlane);
      }
    }

    chunksIntersectingPlane.clear();
  }

  if(chunksInRegion.size() == 0)
  {
    throw std::runtime_error("Requested volume subset does not contain any data");
  }
  return AddJob(chunksInRegion, [boxRequested, buffer, projectedDimensions, voxelPlaneSwapped, format, interpolationMethod, isReplaceNoValue, replacementNoValue](VolumeDataPageImpl* page, VolumeDataChunk dataChunk, Error &error) { return RequestProjectedVolumeSubsetProcessPage(page, dataChunk, boxRequested.min, boxRequested.max, projectedDimensions, voxelPlaneSwapped, format, interpolationMethod, isReplaceNoValue, replacementNoValue, buffer, error);}, format == VolumeDataChannelDescriptor::Format_1Bit);
}

struct VolumeDataSamplePos
{
  NDPos pos;
  int32_t originalSample;
  int64_t chunkIndex;

  bool operator < (VolumeDataSamplePos const &rhs) const
  {
    return chunkIndex < rhs.chunkIndex;
  }
};

template <typename T, InterpolationMethod INTERPMETHOD, bool isUseNoValue>
static void SampleVolume(VolumeDataPageImpl *page, const VolumeDataLayer *volumeDataLayer, const std::vector<VolumeDataSamplePos> &volumeSamplePositions, int32_t iStartSamplePos, int32_t nSamplePos, float noValue, void *destBuffer)
{
  const DataBlock &dataBlock = page->GetDataBlock();
  int64_t chunkIndex = page->GetChunkIndex();

  int32_t chunkDimension0 = volumeDataLayer->GetChunkDimension(0);
  int32_t chunkDimension1 = volumeDataLayer->GetChunkDimension(1);
  int32_t chunkDimension2 = volumeDataLayer->GetChunkDimension(2);

  assert(chunkDimension0 >= 0 && chunkDimension1 >= 0);

  int32_t min[Dimensionality_Max];
  int32_t max[Dimensionality_Max];

  volumeDataLayer->GetChunkMinMax(chunkIndex, min, max, true);

  int32_t LOD = volumeDataLayer->GetLOD();

  float LODScale = 1.0f / (1 << LOD);

  int32_t fullResolutionDimension = volumeDataLayer->GetLayout()->GetFullResolutionDimension();

  VolumeSampler<T, INTERPMETHOD, isUseNoValue> volumeSampler(dataBlock.Size, dataBlock.Pitch, volumeDataLayer->GetValueRange().Min, volumeDataLayer->GetValueRange().Max,
    volumeDataLayer->GetIntegerScale(), volumeDataLayer->GetIntegerOffset(), noValue, noValue);

  const T*buffer = (const T*)(page->GetRawBufferInternal());

  for (int32_t iSamplePos = iStartSamplePos; iSamplePos < nSamplePos; iSamplePos++)
  {
    const VolumeDataSamplePos &volumeDataSamplePos = volumeSamplePositions[iSamplePos];

    if (volumeDataSamplePos.chunkIndex != chunkIndex) break;

    FloatVector3 pos((volumeDataSamplePos.pos.Data[chunkDimension0] - min[chunkDimension0]) * (chunkDimension0 == fullResolutionDimension ? 1 : LODScale),
                     (volumeDataSamplePos.pos.Data[chunkDimension1] - min[chunkDimension1]) * (chunkDimension1 == fullResolutionDimension ? 1 : LODScale),
                      0);

    if (chunkDimension2 >= 0)
    {
      pos[2] = (volumeDataSamplePos.pos.Data[chunkDimension2] - min[chunkDimension2]) * (chunkDimension2 == fullResolutionDimension ? 1 : LODScale);
    }


    typename InterpolatedRealType<T>::type value = volumeSampler.Sample3D(buffer, pos);

    static_cast<float *>(destBuffer)[volumeDataSamplePos.originalSample] = (float)value;
  }
}

template <typename T>
static void SampleVolumeInit(VolumeDataPageImpl *page, const VolumeDataLayer *volumeDataLayer, const std::vector<VolumeDataSamplePos> &volumeDataSamplePositions, InterpolationMethod interpolationMethod, int32_t iStartSamplePos, int32_t nSamplePos, float noValue, void *destBuffer)
{
  if (volumeDataLayer->IsUseNoValue())
  {
    switch (interpolationMethod)
    {
    case InterpolationMethod::Nearest:
      SampleVolume<T, InterpolationMethod::Nearest, true>(page, volumeDataLayer, volumeDataSamplePositions, iStartSamplePos, nSamplePos, noValue, destBuffer);
      break;
    case InterpolationMethod::Linear:
      SampleVolume<T, InterpolationMethod::Linear, true>(page, volumeDataLayer, volumeDataSamplePositions, iStartSamplePos, nSamplePos, noValue, destBuffer);
      break;
    case InterpolationMethod::Cubic:
      SampleVolume<T, InterpolationMethod::Cubic, true>(page, volumeDataLayer, volumeDataSamplePositions, iStartSamplePos, nSamplePos, noValue, destBuffer);
      break;
    case InterpolationMethod::Angular:
      SampleVolume<T, InterpolationMethod::Angular, true>(page, volumeDataLayer, volumeDataSamplePositions, iStartSamplePos, nSamplePos, noValue, destBuffer);
      break;
    case InterpolationMethod::Triangular:
      SampleVolume<T, InterpolationMethod::Triangular, true>(page, volumeDataLayer, volumeDataSamplePositions, iStartSamplePos, nSamplePos, noValue, destBuffer);
      break;
    default:
      throw std::runtime_error("Unknown interpolation method");
    }
  }
  else
  {
    switch (interpolationMethod)
    {
    case InterpolationMethod::Nearest:
      SampleVolume<T, InterpolationMethod::Nearest, false>(page, volumeDataLayer, volumeDataSamplePositions, iStartSamplePos, nSamplePos, noValue, destBuffer);
      break;
    case InterpolationMethod::Linear:
      SampleVolume<T, InterpolationMethod::Linear, false>(page, volumeDataLayer, volumeDataSamplePositions, iStartSamplePos, nSamplePos, noValue, destBuffer);
      break;
    case InterpolationMethod::Cubic:
      SampleVolume<T, InterpolationMethod::Cubic, false>(page, volumeDataLayer, volumeDataSamplePositions, iStartSamplePos, nSamplePos, noValue, destBuffer);
      break;
    case InterpolationMethod::Angular:
      SampleVolume<T, InterpolationMethod::Angular, false>(page, volumeDataLayer, volumeDataSamplePositions, iStartSamplePos, nSamplePos, noValue, destBuffer);
      break;
    case InterpolationMethod::Triangular:
      SampleVolume<T, InterpolationMethod::Triangular, false>(page, volumeDataLayer, volumeDataSamplePositions, iStartSamplePos, nSamplePos, noValue, destBuffer);
      break;
    default:
      throw std::runtime_error("Unknown interpolation method");
    }
  }
}

static bool RequestVolumeSamplesProcessPage(VolumeDataPageImpl *page, VolumeDataChunk &dataChunk, const std::vector<VolumeDataSamplePos> &volumeDataSamplePositions, InterpolationMethod interpolationMethod, bool isUseNoValue, bool isReplaceNoValue, float replacementNoValue, void *buffer, Error &error)
{
  int32_t  samplePosCount = int32_t(volumeDataSamplePositions.size());

  int32_t iStartSamplePos = 0;
  int32_t iEndSamplePos = samplePosCount - 1;

  // Binary search to find samples within chunk
  while (iStartSamplePos < iEndSamplePos)
  {
    int32_t iSamplePos = (iStartSamplePos + iEndSamplePos) / 2;

    int64_t iSampleChunkIndex = volumeDataSamplePositions[iSamplePos].chunkIndex;

    if (iSampleChunkIndex >= dataChunk.index)
    {
      iEndSamplePos = iSamplePos;
    }
    else
    {
      iStartSamplePos = iSamplePos + 1;
    }
  }

  assert(volumeDataSamplePositions[iStartSamplePos].chunkIndex == dataChunk.index &&
    (iStartSamplePos == 0 || volumeDataSamplePositions[size_t(iStartSamplePos) - 1].chunkIndex < dataChunk.index));

  VolumeDataChannelDescriptor::Format format = page->GetDataBlock().Format;

  switch (format)
  {
  case VolumeDataChannelDescriptor::Format_1Bit:
    SampleVolumeInit<bool>(page, dataChunk.layer, volumeDataSamplePositions, interpolationMethod, iStartSamplePos, samplePosCount, replacementNoValue, buffer);
    break;
  case VolumeDataChannelDescriptor::Format_U8:
    SampleVolumeInit<uint8_t>(page, dataChunk.layer, volumeDataSamplePositions, interpolationMethod, iStartSamplePos, samplePosCount, replacementNoValue, buffer);
    break;
  case VolumeDataChannelDescriptor::Format_U16:
    SampleVolumeInit<uint16_t>(page, dataChunk.layer, volumeDataSamplePositions, interpolationMethod, iStartSamplePos, samplePosCount, replacementNoValue, buffer);
    break;
  case VolumeDataChannelDescriptor::Format_U32:
    SampleVolumeInit<uint32_t>(page, dataChunk.layer, volumeDataSamplePositions, interpolationMethod, iStartSamplePos, samplePosCount, replacementNoValue, buffer);
    break;
  case VolumeDataChannelDescriptor::Format_R32:
    SampleVolumeInit<float>(page, dataChunk.layer, volumeDataSamplePositions, interpolationMethod, iStartSamplePos, samplePosCount, replacementNoValue, buffer);
    break;
  case VolumeDataChannelDescriptor::Format_U64:
    SampleVolumeInit<uint64_t>(page, dataChunk.layer, volumeDataSamplePositions, interpolationMethod, iStartSamplePos, samplePosCount, replacementNoValue, buffer);
    break;
  case VolumeDataChannelDescriptor::Format_R64:
    SampleVolumeInit<double>(page, dataChunk.layer, volumeDataSamplePositions, interpolationMethod, iStartSamplePos, samplePosCount, replacementNoValue, buffer);
    break;
  case VolumeDataChannelDescriptor::Format_Any:
    error.code = -1;
    error.string = "Illigal format";
    return false;
  }

  return true;
}

int64_t VolumeDataRequestProcessor::RequestVolumeSamples(void *buffer, VolumeDataLayer const *volumeDataLayer, const float(*samplePositions)[Dimensionality_Max], int32_t samplePosCount, InterpolationMethod interpolationMethod, bool isReplaceNoValue, float replacementNoValue)
{
  std::shared_ptr<std::vector<VolumeDataSamplePos>> volumeDataSamplePositions = std::make_shared<std::vector<VolumeDataSamplePos>>();

  volumeDataSamplePositions->resize(samplePosCount);

  for(int32_t samplePos = 0 ; samplePos < samplePosCount; samplePos++)
  {
    VolumeDataSamplePos &volumeDataSamplePos = volumeDataSamplePositions->at(samplePos);

    std::copy(&samplePositions[samplePos][0], &samplePositions[samplePos][Dimensionality_Max], volumeDataSamplePos.pos.Data);
    volumeDataSamplePos.chunkIndex = volumeDataLayer->GetChunkIndexFromNDPos(volumeDataSamplePos.pos);
    volumeDataSamplePos.originalSample = samplePos;
  }

  std::sort(volumeDataSamplePositions->begin(), volumeDataSamplePositions->end());

  // Force NEAREST interpolation for discrete volume data
  if (volumeDataLayer->IsDiscrete())
  {
    interpolationMethod = InterpolationMethod::Nearest;
  }

  std::vector<VolumeDataChunk> volumeDataChunks;
  int64_t currentChunkIndex = -1;

  for (int32_t samplePos = 0; samplePos < int32_t(volumeDataSamplePositions->size()); samplePos++)
  {
    VolumeDataSamplePos &volumeDataSamplePos = volumeDataSamplePositions->at(samplePos);
    if (volumeDataSamplePos.chunkIndex != currentChunkIndex)
    {
      currentChunkIndex = volumeDataSamplePos.chunkIndex;
      volumeDataChunks.push_back(volumeDataLayer->GetChunkFromIndex(currentChunkIndex));
    }
  }

  return AddJob(volumeDataChunks, [buffer, volumeDataSamplePositions, interpolationMethod, isReplaceNoValue, replacementNoValue](VolumeDataPageImpl* page, VolumeDataChunk dataChunk, Error& error)
    {
      return RequestVolumeSamplesProcessPage(page, dataChunk,  *volumeDataSamplePositions, interpolationMethod, dataChunk.layer->IsUseNoValue(), isReplaceNoValue, isReplaceNoValue ? replacementNoValue : dataChunk.layer->GetNoValue(), buffer, error);
    });
}

template <typename T, InterpolationMethod INTERPMETHOD, bool isUseNoValue>
void TraceVolume(VolumeDataPageImpl *page, const VolumeDataChunk &chunk, const std::vector<VolumeDataSamplePos> &volumeDataSamplePositions, int32_t traceDimension, float noValue, void *targetBuffer)
{
  int32_t traceSize = chunk.layer->GetDimensionNumSamples(traceDimension);

  float *traceBuffer = reinterpret_cast<float *>(targetBuffer);

  const DataBlock & dataBlock = page->GetDataBlock();

  const VolumeDataLayer *volumeDataLayer = chunk.layer;

  int32_t chunkDimension0 = volumeDataLayer->GetChunkDimension(0);
  int32_t chunkDimension1 = volumeDataLayer->GetChunkDimension(1);
  int32_t chunkDimension2 = volumeDataLayer->GetChunkDimension(2);

  assert(chunkDimension0 >= 0 && chunkDimension1 >= 0);

  int32_t  traceDimensionInChunk = -1;

  if (chunkDimension0 == traceDimension)
    traceDimensionInChunk = 0;
  else if (chunkDimension1 == traceDimension)
    traceDimensionInChunk = 1;
  else if (chunkDimension2 == traceDimension)
    traceDimensionInChunk = 2;

  int32_t min[Dimensionality_Max];
  int32_t max[Dimensionality_Max];
  int32_t minExcludingMargin[Dimensionality_Max];
  int32_t maxExcludingMargin[Dimensionality_Max];

  volumeDataLayer->GetChunkMinMax(chunk.index, min, max, true);
  volumeDataLayer->GetChunkMinMax(chunk.index, minExcludingMargin, maxExcludingMargin, false);

  int32_t LOD = volumeDataLayer->GetLOD();

  float LODScale = 1.0f / (1 << LOD);

  int32_t fullResolutionDimension = volumeDataLayer->GetLayout()->GetFullResolutionDimension();

  VolumeSampler<T, INTERPMETHOD, isUseNoValue> volumeSampler(dataBlock.Size, dataBlock.Pitch, volumeDataLayer->GetValueRange().Min, volumeDataLayer->GetValueRange().Max, volumeDataLayer->GetIntegerScale(), volumeDataLayer->GetIntegerOffset(), noValue, noValue);

  const T* pBuffer = (const T*) page->GetRawBufferInternal();

  int32_t traceDimensionLOD = (traceDimension != fullResolutionDimension) ? LOD : 0;
  int32_t overlapCount = GetLODSize(minExcludingMargin[traceDimension], maxExcludingMargin[traceDimension], traceDimensionLOD, maxExcludingMargin[traceDimension] == traceSize);
  int32_t offsetSource = (minExcludingMargin[traceDimension] - min[traceDimension]) >> traceDimensionLOD;
  int32_t offsetTarget = (minExcludingMargin[traceDimension]) >> traceDimensionLOD;

  int32_t traceCount = int32_t(volumeDataSamplePositions.size());

  for (int32_t trace = 0; trace < traceCount; trace++)
  {
    const VolumeDataSamplePos &volumeDataSamplePos = volumeDataSamplePositions[trace];

    bool isInside = true;

    for (int dim = 0; dim < Dimensionality_Max; dim++)
    {
      if (dim != traceDimension &&
        ((int32_t)volumeDataSamplePos.pos.Data[dim] < minExcludingMargin[dim] ||
         (int32_t)volumeDataSamplePos.pos.Data[dim] >= maxExcludingMargin[dim]))
      {
        isInside = false;
        break;
      }
    }

    if (!isInside) continue;

    FloatVector3 pos((volumeDataSamplePos.pos.Data[chunkDimension0] - min[chunkDimension0]) * (chunkDimension0 == fullResolutionDimension ? 1 : LODScale),
      (volumeDataSamplePos.pos.Data[chunkDimension1] - min[chunkDimension1]) * (chunkDimension1 == fullResolutionDimension ? 1 : LODScale),
      0.5f);

    if (chunkDimension2 >= 0)
    {
      pos[2] = (volumeDataSamplePos.pos.Data[chunkDimension2] - min[chunkDimension2]) * (chunkDimension2 == fullResolutionDimension ? 1 : LODScale);
    }

    for (int overlap = 0; overlap < overlapCount; overlap++)
    {
      if (traceDimensionInChunk != -1)
      {
        pos[traceDimensionInChunk] = overlap + offsetSource + 0.5f; // so that we sample the center of the voxel
      }

      typename InterpolatedRealType<T>::type value = volumeSampler.Sample3D(pBuffer, pos);

      traceBuffer[traceSize * volumeDataSamplePos.originalSample + overlap + offsetTarget] = (float)value;
    }
  }
}

template <typename T>
static void TraceVolumeInit(VolumeDataPageImpl *page, const VolumeDataChunk &chunk, const std::vector<VolumeDataSamplePos> &volumeDataSamplePositions, InterpolationMethod interpolationMethod, int32_t traceDimension, float noValue, void *targetBuffer)
{
  if (chunk.layer->IsUseNoValue())
  {
    switch (interpolationMethod)
    {
    case InterpolationMethod::Nearest:
      TraceVolume<T, InterpolationMethod::Nearest, true>(page, chunk, volumeDataSamplePositions, traceDimension, noValue, targetBuffer);
      break;
    case InterpolationMethod::Linear:
      TraceVolume<T, InterpolationMethod::Linear, true>(page, chunk, volumeDataSamplePositions, traceDimension, noValue, targetBuffer);
      break;
    case InterpolationMethod::Cubic:
      TraceVolume<T, InterpolationMethod::Cubic, true>(page, chunk, volumeDataSamplePositions, traceDimension, noValue, targetBuffer);
      break;
    case InterpolationMethod::Angular:
      TraceVolume<T, InterpolationMethod::Angular, true>(page, chunk, volumeDataSamplePositions, traceDimension, noValue, targetBuffer);
      break;
    case InterpolationMethod::Triangular:
      TraceVolume<T, InterpolationMethod::Triangular, true>(page, chunk, volumeDataSamplePositions, traceDimension, noValue, targetBuffer);
      break;
    default:
      throw std::runtime_error("Unknown interpolation method");
    }
  }
  else
  {
    switch (interpolationMethod)
    {
    case InterpolationMethod::Nearest:
      TraceVolume<T, InterpolationMethod::Nearest, false>(page, chunk, volumeDataSamplePositions, traceDimension, noValue, targetBuffer);
      break;
    case InterpolationMethod::Linear:
      TraceVolume<T, InterpolationMethod::Linear, false>(page, chunk, volumeDataSamplePositions, traceDimension, noValue, targetBuffer);
      break;
    case InterpolationMethod::Cubic:
      TraceVolume<T, InterpolationMethod::Cubic, false>(page, chunk, volumeDataSamplePositions, traceDimension, noValue, targetBuffer);
      break;
    case InterpolationMethod::Angular:
      TraceVolume<T, InterpolationMethod::Angular, false>(page, chunk, volumeDataSamplePositions, traceDimension, noValue, targetBuffer);
      break;
    case InterpolationMethod::Triangular:
      TraceVolume<T, InterpolationMethod::Triangular, false>(page, chunk, volumeDataSamplePositions, traceDimension, noValue, targetBuffer);
      break;
    default:
      throw std::runtime_error("Unknown interpolation method");
    }
  }
}

static bool RequestVolumeTracesProcessPage (VolumeDataPageImpl *page, VolumeDataChunk &dataChunk, const std::vector<VolumeDataSamplePos> &volumeDataSamplePositions, InterpolationMethod interpolationMethod, int32_t traceDimension, float noValue, void *buffer, Error &error)
{
  VolumeDataChannelDescriptor::Format format = dataChunk.layer->GetFormat();
  switch (format)
  {
  case VolumeDataChannelDescriptor::Format_1Bit:
    TraceVolumeInit<bool>(page, dataChunk, volumeDataSamplePositions, interpolationMethod, traceDimension, noValue, buffer);
    break;
  case VolumeDataChannelDescriptor::Format_U8:
    TraceVolumeInit<uint8_t>(page, dataChunk, volumeDataSamplePositions, interpolationMethod, traceDimension, noValue, buffer);
    break;
  case VolumeDataChannelDescriptor::Format_U16:
    TraceVolumeInit<uint16_t>(page, dataChunk, volumeDataSamplePositions, interpolationMethod, traceDimension, noValue, buffer);
    break;
  case VolumeDataChannelDescriptor::Format_U32:
    TraceVolumeInit<uint32_t>(page, dataChunk, volumeDataSamplePositions, interpolationMethod, traceDimension, noValue, buffer);
    break;
  case VolumeDataChannelDescriptor::Format_R32:
    TraceVolumeInit<float>(page, dataChunk, volumeDataSamplePositions, interpolationMethod, traceDimension, noValue, buffer);
    break;
  case VolumeDataChannelDescriptor::Format_U64:
    TraceVolumeInit<uint64_t>(page, dataChunk, volumeDataSamplePositions, interpolationMethod, traceDimension, noValue, buffer);
    break;
  case VolumeDataChannelDescriptor::Format_R64:
    TraceVolumeInit<double>(page, dataChunk, volumeDataSamplePositions, interpolationMethod, traceDimension, noValue, buffer);
    break;
  case VolumeDataChannelDescriptor::Format_Any:
    error.code = -1;
    error.string = "Illigal format";
    return false;
  }
  return true;
}

int64_t VolumeDataRequestProcessor::RequestVolumeTraces(void *buffer, VolumeDataLayer const *volumeDataLayer, const float(*tracePositions)[Dimensionality_Max], int32_t tracePositionsCount, int32_t LOD, InterpolationMethod interpolationMethod, int32_t traceDimension, bool isReplaceNoValue, float replacementNoValue)
{
  if (traceDimension < 0 || traceDimension >= Dimensionality_Max)
  {
    throw std::runtime_error("The trace dimension must be a valid dimension.");
  }

  std::shared_ptr<std::vector<VolumeDataSamplePos>> volumeDataSamplePositions = std::make_shared<std::vector<VolumeDataSamplePos>>();
  volumeDataSamplePositions->resize(tracePositionsCount);

  for (int32_t tracePos = 0; tracePos < tracePositionsCount; tracePos++)
  {
    VolumeDataSamplePos &volumeDataSamplePos = volumeDataSamplePositions->at(tracePos);

    for(int dimension = 0; dimension < Dimensionality_Max; dimension++)
    {
      volumeDataSamplePos.pos.Data[dimension] = (dimension != traceDimension) ? tracePositions[tracePos][dimension] : 0;
    }
    volumeDataSamplePos.originalSample = tracePos;
    volumeDataSamplePos.chunkIndex = volumeDataLayer->GetChunkIndexFromNDPos(volumeDataSamplePos.pos);
  }

  std::sort(volumeDataSamplePositions->begin(), volumeDataSamplePositions->end());

  // Force NEAREST interpolation for discrete volume data
  if (volumeDataLayer->IsDiscrete())
  {
    interpolationMethod = InterpolationMethod::Nearest;
  }

  int64_t currentChunkIndex = -1;

  std::vector<VolumeDataChunk> volumeDataChunks;
  int32_t traceMin[Dimensionality_Max];
  memset(traceMin, 0, sizeof(traceMin));
  int32_t traceMax[Dimensionality_Max];
  memset(traceMax, 0, sizeof(traceMax));

  for (int32_t tracePos = 0; tracePos < tracePositionsCount; tracePos++)
  {
    VolumeDataSamplePos &volumeDataSamplePos = volumeDataSamplePositions->at(tracePos);
    if (volumeDataSamplePos.chunkIndex != currentChunkIndex)
    {
      currentChunkIndex = volumeDataSamplePos.chunkIndex;

      VolumeDataChunk volumeDataChunk(volumeDataLayer->GetChunkFromIndex(currentChunkIndex));
      volumeDataChunks.push_back(volumeDataChunk);

      for (int dim = 0; dim < Dimensionality_Max; dim++)
      {
        traceMin[dim] = (int32_t)volumeDataSamplePos.pos.Data[dim];
        traceMax[dim] = (int32_t)volumeDataSamplePos.pos.Data[dim] + 1;
      }

      traceMin[traceDimension] = 0;
      traceMax[traceDimension] = volumeDataLayer->GetDimensionNumSamples(traceDimension);

      volumeDataLayer->GetChunksInRegion(traceMin, traceMax, &volumeDataChunks, true);
    }
  }

  return AddJob(volumeDataChunks, [buffer, volumeDataSamplePositions, interpolationMethod, traceDimension, replacementNoValue](VolumeDataPageImpl* page, VolumeDataChunk dataChunk, Error& error)
    {
      return RequestVolumeTracesProcessPage(page, dataChunk,  *volumeDataSamplePositions, interpolationMethod, traceDimension, replacementNoValue, buffer, error);
    });
}

int64_t VolumeDataRequestProcessor::PrefetchVolumeChunk(VolumeDataLayer const *volumeDataLayer, int64_t chunkIndex)
{
  std::vector<VolumeDataChunk> chunks;
  chunks.push_back(volumeDataLayer->GetChunkFromIndex(chunkIndex));
  return AddJob(chunks, [](VolumeDataPageImpl *page, VolumeDataChunk dataChunk, Error &error) {return true;});
}

int64_t VolumeDataRequestProcessor::StaticGetVolumeSubsetBufferSize(VolumeDataLayout const *volumeDataLayout, const int (&minVoxelCoordinates)[Dimensionality_Max], const int (&maxVoxelCoordinates)[Dimensionality_Max], VolumeDataChannelDescriptor::Format format, int LOD, int channel)
{
  const int dimensionality = volumeDataLayout->GetDimensionality();

  for (int32_t dimension = 0; dimension < dimensionality; dimension++)
  {
    if(minVoxelCoordinates[dimension] < 0 || minVoxelCoordinates[dimension] >= maxVoxelCoordinates[dimension])
    {
      throw std::runtime_error(fmt::format("Illegal volume subset, dimension {} min = {}, max = {}", dimension, minVoxelCoordinates[dimension], maxVoxelCoordinates[dimension]));
    }
  }

  int64_t voxelCount = 1;

  for (int dimension = 0; dimension < dimensionality; dimension++)
  {
    if (static_cast<const VolumeDataLayoutImpl *>(volumeDataLayout)->IsDimensionLODDecimated(dimension))
    {
      voxelCount *= GetLODSize(minVoxelCoordinates[dimension], maxVoxelCoordinates[dimension], LOD);
    }
    else
    {
      voxelCount *= maxVoxelCoordinates[dimension] - minVoxelCoordinates[dimension];
    }
  }

  if(format == VolumeDataChannelDescriptor::Format_1Bit)
  {
    return (voxelCount + 7) / 8;
  }
  else
  {
    return voxelCount * GetElementSize(format, volumeDataLayout->GetChannelComponents(channel));
  }
}

int64_t VolumeDataRequestProcessor::StaticGetProjectedVolumeSubsetBufferSize(VolumeDataLayout const *volumeDataLayout, const int (&minVoxelCoordinates)[Dimensionality_Max], const int (&maxVoxelCoordinates)[Dimensionality_Max], DimensionGroup projectedDimensions, VolumeDataChannelDescriptor::Format format, int LOD, int channel)
{
  const int dimensionality = volumeDataLayout->GetDimensionality();

  for (int32_t dimension = 0; dimension < dimensionality; dimension++)
  {
    if(minVoxelCoordinates[dimension] < 0 || minVoxelCoordinates[dimension] >= maxVoxelCoordinates[dimension])
    {
      throw std::runtime_error(fmt::format("Illegal volume subset, dimension {} min = {}, max = {}", dimension, minVoxelCoordinates[dimension], maxVoxelCoordinates[dimension]));
    }
  }

  if (DimensionGroupUtil::GetDimensionality(projectedDimensions) != 2)
  {
    throw std::runtime_error("The projected dimension group must contain 2 dimensions.");
  }

  const int projectedDimension0 = DimensionGroupUtil::GetDimension(projectedDimensions, 0),
            projectedDimension1 = DimensionGroupUtil::GetDimension(projectedDimensions, 1);

  int64_t voxelCount = GetLODSize(minVoxelCoordinates[projectedDimension0], maxVoxelCoordinates[projectedDimension0], LOD) *
                       GetLODSize(minVoxelCoordinates[projectedDimension1], maxVoxelCoordinates[projectedDimension1], LOD);

  if(format == VolumeDataChannelDescriptor::Format_1Bit)
  {
    return (voxelCount + 7) / 8;
  }
  else
  {
    return voxelCount * GetElementSize(format, volumeDataLayout->GetChannelComponents(channel));
  }
}

int64_t VolumeDataRequestProcessor::StaticGetVolumeSamplesBufferSize(VolumeDataLayout const *volumeDataLayout, int sampleCount, int channel)
{
  const VolumeDataChannelDescriptor::Format format = VolumeDataChannelDescriptor::Format_R32;

  int64_t voxelCount = sampleCount;

  return voxelCount * GetElementSize(format, volumeDataLayout->GetChannelComponents(channel));
}

int64_t VolumeDataRequestProcessor::StaticGetVolumeTracesBufferSize(VolumeDataLayout const *volumeDataLayout, int traceCount, int traceDimension, int LOD, int channel)
{
  const VolumeDataChannelDescriptor::Format format = VolumeDataChannelDescriptor::Format_R32;

  int effectiveLOD = static_cast<const VolumeDataLayoutImpl *>(volumeDataLayout)->IsDimensionLODDecimated(traceDimension) ? LOD : 0;

  int64_t voxelCount = (int64_t)GetLODSize(0, volumeDataLayout->GetDimensionNumSamples(traceDimension), effectiveLOD) * traceCount;

  return voxelCount * GetElementSize(format, volumeDataLayout->GetChannelComponents(channel));
}

OPENVDS_EXPORT int _cleanupthread_timeoutseconds = 30;

static void CleanupThread(PageAccessorNotifier &pageAccessorNotifier,  std::map<PageAccessorKey, VolumeDataPageAccessorImpl *> &pageAccessors)
{
  auto long_block = std::chrono::hours(24 * 32 * 12);
  auto in_progress_block = std::chrono::seconds(_cleanupthread_timeoutseconds);
  while(!pageAccessorNotifier.exit)
  {
    std::unique_lock<std::mutex> lock(pageAccessorNotifier.mutex);
    std::chrono::seconds waitFor = long_block;
    for (auto &it: pageAccessors)
    {
      auto &pageAccessor = it.second;
      int ref = pageAccessor->GetReferenceCount();
      if (ref > 0)
      {
        if (waitFor > in_progress_block)
          waitFor = in_progress_block;
      }
      else
      {
        if (pageAccessor->GetMaxPages() > 0)
        {
          auto duration = pageAccessor->GetLastUsed() - (std::chrono::steady_clock::now() - in_progress_block);
          if (duration < std::chrono::seconds(0))
            pageAccessor->SetMaxPages(0);
          else if (duration < waitFor)
          {
            waitFor = std::chrono::duration_cast<std::chrono::seconds>(duration) + std::chrono::seconds(1);
          }
        }
      }
    }
    pageAccessorNotifier.dirty = false;
    pageAccessorNotifier.jobNotification.wait_for(lock, waitFor, [&pageAccessorNotifier]
      {
        return pageAccessorNotifier.exit || pageAccessorNotifier.dirty;
      }
    );
  }
}

VolumeDataRequestProcessor::VolumeDataRequestProcessor(VolumeDataAccessManagerImpl& manager)
  : m_manager(manager)
  , m_pageAccessorNotifier(m_mutex)
  , m_threadPool(std::thread::hardware_concurrency())
  , m_cleanupThread([this]() { CleanupThread(m_pageAccessorNotifier, m_pageAccessors); } )
{}

VolumeDataRequestProcessor::~VolumeDataRequestProcessor()
{
  for (auto& job : m_jobs)
    job->cancelled = true;
  m_pageAccessorNotifier.setExit();
  m_cleanupThread.join();
}

static int64_t GenJobId()
{
  static std::atomic< std::int64_t > id(0);
  return ++id;
}

struct MarkJobAsDoneOnExit
{
  MarkJobAsDoneOnExit(Job *job, int index)
    : job(job)
    , index(index)
  {}
  ~MarkJobAsDoneOnExit()
  {
    {
      JobPage& jobPage = job->pages[index];
      if (jobPage.page)
        jobPage.page->UnPin();
    }
    if (++job->pagesProcessed == job->pagesCount)
    {
      std::unique_lock<std::mutex> lock(job->pageAccessorNotifier.mutex);
      job->pageAccessor.SetLastUsed(std::chrono::steady_clock::now());
      job->pageAccessor.RemoveReference();
      job->done = true;
      job->pageAccessorNotifier.setDirtyNoLock();
    }
  }
  Job *job;
  int index;
};

static Error ProcessPageInJob(Job *job, int pageIndex, VolumeDataPageAccessorImpl *pageAccessor, std::function<bool(VolumeDataPageImpl *page, const VolumeDataChunk &chunk, Error &error)> processor)
{
  MarkJobAsDoneOnExit jobDone(job, pageIndex);
  JobPage& jobPage = job->pages[pageIndex];

  if (!jobPage.page)
    return Error();

  Error error;
  if (jobPage.page->GetError(error))
  {
    job->cancelled = true;
  }

  if (job->cancelled)
  {
    auto page = jobPage.page;
    jobPage.page = nullptr;
    pageAccessor->CancelPreparedReadPage(page);
  }
  else if (pageAccessor->ReadPreparedPaged(jobPage.page))
  {
    processor(jobPage.page, jobPage.chunk, error);
  }
  else
  {
    jobPage.page->GetError(error);
    job->cancelled = true;
  }

  return error;
}

static void SetErrorForJob(Job* job)
{
  assert(job->cancelled);
  for (auto& future : job->future)
  {
    if (!future.valid())
      continue;
    Error jobError = future.get();

    if (!jobError.code)
      continue;

    job->completedError = jobError;
    break;
  }
}

int64_t VolumeDataRequestProcessor::AddJob(const std::vector<VolumeDataChunk>& chunks, std::function<bool(VolumeDataPageImpl * page, const VolumeDataChunk &volumeDataChunk, Error & error)> processor, bool singleThread)
{
  auto layer = chunks.front().layer;
  DimensionsND dimensions = DimensionGroupUtil::GetDimensionsNDFromDimensionGroup(layer->GetPrimaryChannelLayer().GetChunkDimensionGroup());
  int channel = layer->GetChannelIndex();
  int lod = layer->GetLOD();

  const int maxPages = std::max(8, (int)chunks.size());

  std::unique_lock<std::mutex> lock(m_mutex);
  PageAccessorKey key = { dimensions, lod, channel };
  auto page_accessor_it = m_pageAccessors.find(key);
  if (page_accessor_it == m_pageAccessors.end())
  {
    auto pa = static_cast<VolumeDataPageAccessorImpl *>(m_manager.CreateVolumeDataPageAccessor(dimensions, lod, channel, maxPages, VolumeDataAccessManager::AccessMode_ReadOnly));
    pa->RemoveReference();
    auto insert_result = m_pageAccessors.emplace(key, pa);
    assert(insert_result.second);
    page_accessor_it = insert_result.first;
  }

  VolumeDataPageAccessorImpl *pageAccessor = page_accessor_it->second;
  assert(pageAccessor);

  if(pageAccessor->GetMaxPages() < maxPages)
  {
    pageAccessor->SetMaxPages(maxPages);
  }

  pageAccessor->AddReference();

  // AddJob can become re-entrant when calling PrepareReadPage so we release the lock while preparing the pages
  lock.unlock();
  std::vector<JobPage> pages;
  pages.reserve(chunks.size());
  Error completedError;
  bool cancelled = false;

  for (const auto &c : chunks)
  {
    pages.emplace_back(static_cast<VolumeDataPageImpl *>(pageAccessor->PrepareReadPage(c.index, completedError)), c);
    if (!pages.back().page)
    {
      cancelled = true;
      break;
    }
  }
  lock.lock();

  Job *job = new Job(GenJobId(), m_pageAccessorNotifier, *pageAccessor, int(chunks.size()));
  m_jobs.emplace_back(job);

  job->pagesCount = int(pages.size());
  job->pages = std::move(pages);
  job->completedError = completedError;
  job->cancelled = cancelled;

  if (job->cancelled)
  {
    for (auto &jobPage : job->pages)
    {
      if (jobPage.page)
      {
        pageAccessor->CancelPreparedReadPage(jobPage.page);
        jobPage.page = nullptr;
      }
    }
    job->pagesProcessed = job->pagesCount;
    job->done = true;
    return job->jobId;
  }

  job->future.reserve(chunks.size());
  if (singleThread)
  {
    job->future.push_back(m_threadPool.Enqueue([job, pageAccessor, processor]
    {
      Error error;
      int pages_size = int(job->pages.size());
      for (int i = 0; i < pages_size; i++)
      {
        if (error.code == 0)
        {
          error = ProcessPageInJob(job, i, pageAccessor, processor);
          if (error.code)
          {
            job->cancelled = true;
          }
        }
        else
        {
          if (job->pages[i].page)
          {
            pageAccessor->CancelPreparedReadPage(job->pages[i].page);
            job->pages[i].page = nullptr; 
          }
        }
      }
      return error;
    }));
  } 
  else
  {
    for (int i = 0; i < int(job->pages.size()); i++)
    {
      job->future.push_back(m_threadPool.Enqueue([job, i, pageAccessor, processor]
        {
          return ProcessPageInJob(job, i, pageAccessor, processor);
        }));
    }
  }
  return job->jobId;
}

bool  VolumeDataRequestProcessor::IsActive(int64_t jobID)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  auto job_it = std::find_if(m_jobs.begin(), m_jobs.end(), [jobID](const std::unique_ptr<Job> &job) { return job->jobId == jobID; });
  return job_it != m_jobs.end();
}

bool  VolumeDataRequestProcessor::IsCompleted(int64_t jobID)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_manager.SetCurrentDownloadError(Error());
  auto job_it = std::find_if(m_jobs.begin(), m_jobs.end(), [jobID](const std::unique_ptr<Job> &job) { return job->jobId == jobID; });
  if (job_it == m_jobs.end())
    return false;

  auto job = job_it->get();
  if (job->done && !job->cancelled)
  {
    m_jobs.erase(job_it);
    return true;
  }
  return false;
}

bool VolumeDataRequestProcessor::IsCanceled(int64_t jobID, Error &error)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_manager.SetCurrentDownloadError(Error());
  auto job_it = std::find_if(m_jobs.begin(), m_jobs.end(), [jobID](const std::unique_ptr<Job> &job) { return job->jobId == jobID; });
  if (job_it == m_jobs.end())
    return false;

  auto job = job_it->get();
  if (job->done && job->cancelled)
  {
    static bool should_print = getBooleanEnvironmentVariable("OPENVDS_DEBUG_IS_CANCELLED");
    if (should_print)
    {
      std::string out = "Printing cancelled request results\n";
      for (int i = 0; i < job->pagesCount; i++)
      {
        auto& future = job->future[i];
        Error error = future.get();
        if (!error.code)
          error.string = "OK";
        out += fmt::format("Request channel {} chunk {} result: {}\n", job->pages[i].chunk.layer->GetChannelIndex(), job->pages[i].chunk.index, error.string);
      }
      fmt::print(stderr, "{}", out);
    }
    SetErrorForJob(job);
    m_manager.SetCurrentDownloadError(job->completedError);
    error = job->completedError;
    m_jobs.erase(job_it);
    return true;
  }
  return false;
}
  
bool VolumeDataRequestProcessor::WaitForCompletion(int64_t jobID, int millisecondsBeforeTimeout)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_manager.SetCurrentDownloadError(Error());
  auto job_it = std::find_if(m_jobs.begin(), m_jobs.end(), [jobID](const std::unique_ptr<Job> &job) { return job->jobId == jobID; });
  if (job_it == m_jobs.end())
    return false;

  Job *job = job_it->get();
  if (!job->done)
  {
    if (millisecondsBeforeTimeout > 0)
    {
      std::chrono::milliseconds toWait(millisecondsBeforeTimeout);
      job->pageAccessorNotifier.jobNotification.wait_for(lock, toWait, [job]
        {
          return job->done.load();
        });
    }
    else
    {
      job->pageAccessorNotifier.jobNotification.wait(lock, [job]
        {
          return job->done.load();
        });
    }
  }
  if (job->done && !job->cancelled)
  {
    job_it = std::find_if(m_jobs.begin(), m_jobs.end(), [job](const std::unique_ptr<Job>& jobin) { return job == jobin.get(); });
    m_jobs.erase(job_it);
    return true;
  }
  if (job->cancelled)
  {
    SetErrorForJob(job);
    m_manager.SetCurrentDownloadError(job->completedError);
  }
  return false;
}

void VolumeDataRequestProcessor::Cancel(int64_t jobID)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_manager.SetCurrentDownloadError(Error());
  auto job_it = std::find_if(m_jobs.begin(), m_jobs.end(), [jobID](std::unique_ptr<Job> &job) { return job->jobId == jobID; });
  if (job_it == m_jobs.end())
    return;
  job_it->get()->cancelled = true;
  m_pageAccessorNotifier.setDirtyNoLock();
}

float VolumeDataRequestProcessor::GetCompletionFactor(int64_t jobID)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_manager.SetCurrentDownloadError(Error());
  auto job_it = std::find_if(m_jobs.begin(), m_jobs.end(), [jobID](std::unique_ptr<Job> &job) { return job->jobId == jobID; });
  if (job_it == m_jobs.end())
    return 0.f;
  return float(job_it->get()->pagesProcessed) / float(job_it->get()->pagesCount);
}

int VolumeDataRequestProcessor::CountActivePages()
{
  std::unique_lock<std::mutex> lock(m_mutex);
  m_manager.SetCurrentDownloadError(Error());
  int ret = 0;
  for (auto &pa : m_pageAccessors)
    ret += pa.second->GetMaxPages();
  return ret;
}

}
