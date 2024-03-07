#include <OpenVDS/VolumeData.h>
#include <OpenVDS/Vector.h>
#include <OpenVDS/ValueConversion.h>

namespace OpenVDS
{

template <typename T, bool isUseNoValue>
static void CopyTo1Bit(void* __restrict voiddst, const void * __restrict voidsrc, float noValue, int32_t count)
{
  uint8_t* target = (uint8_t*)voiddst;
  T* source = (T*)voidsrc;
  const QuantizingValueConverterWithNoValue<float, T, isUseNoValue> valueConverter;
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
static void CopyFrom1Bit(void * __restrict voiddst, const void* __restrict voidsrc, int32_t count)
{
  T* target = (T*)voiddst;
  uint8_t* source = (uint8_t*)voidsrc;
  uint8_t bits = 0;
  int32_t mask = 0x100;
  for (int i = 0; i < count; i++)
  {
    if (mask == 0x100)
    {
      bits = *source++;
      mask = 1;
    }
    *target++ = (bits & mask) ? T(1) : T(0);
    mask <<= 1;
  }
}

template<class TDST, bool dst1Bit, class TSRC, bool src1Bit,  bool isUseNoValue> void
Convert(void* voiddst, const void * voidsrc, const OpenVDS::FloatVector2& valueRange, float integerScale, float integerOffset, float noValue, float replacementNoValue, int numValues)
{
  TDST* dst = (TDST*)voiddst;
  TSRC* src = (TSRC*)voidsrc;

  if (dst1Bit)
  {
    if (src1Bit)
    {
      //should not reach this path
      assert(false);
    }
    else
    {
      CopyTo1Bit<TSRC, isUseNoValue>(voiddst, voidsrc, noValue, numValues);
    }
  }
  else if (src1Bit)
  {
    CopyFrom1Bit<TDST>(voiddst, voidsrc, numValues);
  }
  else
  {
    QuantizingValueConverterWithNoValue<TDST, TSRC, isUseNoValue> converter(valueRange.X, valueRange.Y, integerScale, integerOffset, noValue, replacementNoValue);
    for (int iValue = 0; iValue < numValues; iValue++)
    {
      dst[iValue] = converter.ConvertValue(src[iValue]);
    }
  }
}

template<class TDST, bool dst1Bit, class TSRC, bool src1Bit> void
ConvertSrc(void* dst, const void* src, const OpenVDS::FloatVector2& valueRange, float integerScale, float integerOffset, bool isUseNoValue, float noValue, float replacementNoValue, int numValues)
{
  if (isUseNoValue)
  {
    Convert<TDST, dst1Bit, TSRC, src1Bit, true>(dst, src, valueRange, integerScale, integerOffset, noValue, replacementNoValue, numValues);
  }
  else
  {
    Convert<TDST, dst1Bit, TSRC, src1Bit, false>(dst, src, valueRange, integerScale, integerOffset, noValue, replacementNoValue, numValues);
  }
}

template<class TDST, bool dst1Bit> void
ConvertDst(void* dst, const void* src, VolumeDataFormat srcFormat, const OpenVDS::FloatVector2& valueRange, float integerScale, float integerOffset, bool isUseNoValue, float noValue, float replacementNoValue, int numValues)
{

  switch (srcFormat)
  {
  case VolumeDataFormat::Format_1Bit:
    ConvertSrc<TDST, dst1Bit, uint8_t, true>(dst, src, valueRange, integerScale, integerOffset, isUseNoValue, noValue, replacementNoValue, numValues);
    break;
  case VolumeDataFormat::Format_U8:
    ConvertSrc<TDST, dst1Bit, uint8_t, false>(dst, src, valueRange, integerScale, integerOffset, isUseNoValue, noValue, replacementNoValue, numValues);
    break;
  case VolumeDataFormat::Format_U16:
    ConvertSrc<TDST, dst1Bit, uint16_t, false>(dst, src, valueRange, integerScale, integerOffset, isUseNoValue, noValue, replacementNoValue, numValues);
    break;
  case VolumeDataFormat::Format_R32:
    ConvertSrc<TDST, dst1Bit, float, false>(dst, src, valueRange, integerScale, integerOffset, isUseNoValue, noValue, replacementNoValue, numValues);
    break;
  case VolumeDataFormat::Format_U32:
    ConvertSrc<TDST, dst1Bit, uint32_t, false>(dst, src, valueRange, integerScale, integerOffset, isUseNoValue, noValue, replacementNoValue, numValues);
    break;
  case VolumeDataFormat::Format_R64:
    ConvertSrc<TDST, dst1Bit, double, false>(dst, src, valueRange, integerScale, integerOffset, isUseNoValue, noValue, replacementNoValue, numValues);
    break;
  case VolumeDataFormat::Format_U64:
    ConvertSrc<TDST, dst1Bit, uint64_t, false>(dst, src, valueRange, integerScale, integerOffset, isUseNoValue, noValue, replacementNoValue, numValues);
    break;
  default:
    assert(!"Illegal format!");
    break;
  }
}

static void
ConvertValues(void* dst, const void* src, VolumeDataFormat dstFormat, VolumeDataFormat srcFormat, const OpenVDS::FloatVector2 & valueRange, float integerScale, float integerOffset, bool isUseNoValue, float noValue, float replacementNoValue, int numValues, int destination_size)
{
  assert(dstFormat == VolumeDataFormat::Format_1Bit ||
    dstFormat == VolumeDataFormat::Format_U8  ||
    dstFormat == VolumeDataFormat::Format_U16 ||
    dstFormat == VolumeDataFormat::Format_R32 ||
    dstFormat == VolumeDataFormat::Format_U32 ||
    dstFormat == VolumeDataFormat::Format_R64 ||
    dstFormat == VolumeDataFormat::Format_U64);
  assert(srcFormat == VolumeDataFormat::Format_1Bit ||
    srcFormat == VolumeDataFormat::Format_U8  ||
    srcFormat == VolumeDataFormat::Format_U16 ||
    srcFormat == VolumeDataFormat::Format_R32 ||
    srcFormat == VolumeDataFormat::Format_U32 ||
    srcFormat == VolumeDataFormat::Format_R64 ||
    srcFormat == VolumeDataFormat::Format_U64);

  // For U8 and U16, NoValue is 255 and 65535 respectively, no matter what replacementNoValue and noValue are.
  if (srcFormat == dstFormat && (!isUseNoValue || noValue == replacementNoValue || dstFormat == VolumeDataFormat::Format_1Bit || dstFormat == VolumeDataFormat::Format_U8 || dstFormat == VolumeDataFormat::Format_U16))
  {
    memcpy(dst, src, destination_size);
    return;
  }

  switch (dstFormat)
  {
  case VolumeDataFormat::Format_1Bit:
    ConvertDst<uint8_t, true>(dst, src, srcFormat, valueRange, integerScale, integerOffset, isUseNoValue, noValue, replacementNoValue, numValues);
    break;
  case VolumeDataFormat::Format_U8:
    ConvertDst<uint8_t, false>(dst, src, srcFormat, valueRange, integerScale, integerOffset, isUseNoValue, noValue, replacementNoValue, numValues);
    break;
  case VolumeDataFormat::Format_U16:
    ConvertDst<uint16_t, false>(dst, src, srcFormat, valueRange, integerScale, integerOffset, isUseNoValue, noValue, replacementNoValue, numValues);
    break;
  case VolumeDataFormat::Format_R32:
    ConvertDst<float, false>(dst, src, srcFormat, valueRange, integerScale, integerOffset, isUseNoValue, noValue, replacementNoValue, numValues);
    break;
  case VolumeDataFormat::Format_U32:
    ConvertDst<uint32_t, false>(dst, src, srcFormat, valueRange, integerScale, integerOffset, isUseNoValue, noValue, replacementNoValue, numValues);
    break;
  case VolumeDataFormat::Format_R64:
    ConvertDst<double, false>(dst, src, srcFormat, valueRange, integerScale, integerOffset, isUseNoValue, noValue, replacementNoValue, numValues);
    break;
  case VolumeDataFormat::Format_U64:
    ConvertDst<uint64_t, false>(dst, src, srcFormat, valueRange, integerScale, integerOffset, isUseNoValue, noValue, replacementNoValue, numValues);
    break;
  default:
    assert(!"Illegal format!");
    break;
  }
}

}

