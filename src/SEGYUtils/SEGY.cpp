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

#include <SEGYUtils/SEGY.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <algorithm>
#include <string>

namespace SEGY
{

/////////////////////////////////////////////////////////////////////////////
// ibm2ieee

/* ibm2ieee - Converts a number from IBM 370 single precision floating
point format to IEEE 754 single precision format. For normalized
numbers, the IBM format has greater range but less precision than the
IEEE format. Numbers within the overlapping range are converted
exactly. Numbers which are too large are converted to IEEE Infinity
with the correct sign. Numbers which are too small are converted to
IEEE denormalized numbers with a potential loss of precision (including
complete loss of precision which results in zero with the correct
sign). When precision is lost, rounding is toward zero (because it's
fast and easy -- if someone really wants round to nearest it shouldn't
be TOO difficult). */

void
Ibm2ieee(void *to, const void *from, size_t len)
{
  for (; len-- > 0; to = (char *)to + 4, from = (const char *)from + 4)
  {
    unsigned int
      fr = *(const uint32_t *)from;

    if (fr == 0)
    { 
      /* short-circuit for everything is zero. Then (also endian converted) matissa also is */

      *(unsigned *)to = 0x00000000;

      continue;
    }

#ifdef WIN32
    fr =_byteswap_ulong(fr);
#else
    fr = __builtin_bswap32(fr);
#endif // WIN32

    unsigned int
      sgn = fr & 0x80000000; /* save sign */

    unsigned int
      expIBM4 = (fr & 0x7f000000) >> 22;   // i.e. 4 * expIBM, shift (24 down and then 2 up in one go).

    fr &= 0x00ffffff;

    if (fr == 0)
    { 
      /* short-circuit for zero matissa*/
      *(unsigned *)to = sgn;  // Matissa all zeroes, set exponent all zero, maintain sign.
     
      continue;
    }

    // The involved magic numbers will fall out when studying the IEEE and IBM representations.
  
    const bool
      isNormalNumberEnsured = (expIBM4 > 154) && (expIBM4 < 385);

    // Inside this range, none of the special considerations needs be taken. 

    if (isNormalNumberEnsured)
    {
      // This approach should be SSE friendly, for further optimization.

      // First let the CPU instruction convert the IBM matissa conversion into an IEEE float.
      // This will handle the possible leading zeroes on the IBM matissa without any loop,
      // or use of count leading zero instuction, which is not part of SSE (to the version
      // we can currently assume in common use.)

      float
        rValue = float (fr);

      uint32_t
        uValue;
      memcpy(&uValue, &rValue, sizeof(uValue));

      // Then mod the exponent on the IEEE number, and we are done.

      uint32_t
        uExp = (expIBM4 - uint32_t(280)) << 23;

      uValue = uValue + uExp; 

      *(unsigned int*)to = (uValue | sgn);

    }
    else
    {
      // Note: This else clause is able to handle all cases (i.e. also the normal range above).

       /*
      adjust exponent from base 16 offset 64 radix point before first digit
          to base 2 offset 127 radix point after first digit
      (exp - 64) * 4 + 127 - 1 == exp * 4 - 256 + 126 == (exp << 2) - 130
      */
      int
        expModified = expIBM4 - 130;

      int
        nNormalizeShift = 0;

      /* normalize */
      while (fr < 0x00800000)
      {
        fr <<= 1;
        nNormalizeShift++;
      }

      expModified = expModified - nNormalizeShift;

      if (expModified <= 0)
      { 
        /* underflow */
        if (expModified < -23)
        {
          /* complete underflow - return properly signed zero */
          fr = 0;
        }
        else
        {
          /* partial underflow - return denormalized number */
          fr >>= 1-expModified;
        }
        expModified = 0;
      }
      else if (expModified >= 255)
      { 
        /* overflow - return infinity */
        fr = 0;
        expModified = 255;
      }
      else
      { 
        /* Standard number - assumed high bit masked away below */
      }

      *(unsigned *)to = (fr & 0x007fffff) | (expModified << 23) | sgn;  
    }
  }
}

/////////////////////////////////////////////////////////////////////////////
// ieee2ibm

/* ieee2ibm - Converts a number from IEEE 754 single precision floating
point format to IBM 370 single precision format. For normalized
numbers, the IBM format has greater range but less precision than the
IEEE format. IEEE Infinity is mapped to the largest representable
IBM 370 number. When precision is lost, rounding is toward zero
(because it's fast and easy -- if someone really wants round to nearest
it shouldn't be TOO difficult). */

void
Ieee2ibm(void *to, const void *from, size_t len)
{
  unsigned fr; /* fraction */
  int exp; /* exponent */
  int sgn; /* sign */

  for (; len-- > 0; to = (char *)to + 4, from = (const char *)from + 4)
  {
        /* split into sign, exponent, and fraction */
    fr = *(const unsigned *)from; /* pick up value */
    sgn = fr >> 31; /* save sign */
    fr <<= 1; /* shift sign out */
    exp = fr >> 24; /* save exponent */
    fr <<= 8; /* shift exponent out */

    if (exp == 255) { /* infinity (or NAN) - map to largest */
      fr = 0xffffff00;
      exp = 0x7f;
      goto done;
    }
    else if (exp > 0) /* add assumed digit */
      fr = (fr >> 1) | 0x80000000;
    else if (fr == 0) /* short-circuit for zero */
      goto done;

        /* adjust exponent from base 2 offset 127 radix point after first digit
           to base 16 offset 64 radix point before first digit */
    exp += 130;
    fr >>= -exp & 3;
    exp = (exp + 3) >> 2;

        /* (re)normalize */
    while (fr < 0x10000000)
    { /* never executed for normalized input */
      --exp;
      fr <<= 4;
    }

    done:
/* put the pieces back together and return it */
    fr = (fr >> 8) | (exp << 24) | (sgn << 31);

#ifdef WIN32
    fr =_byteswap_ulong(fr);
#else
    fr = __builtin_bswap32(fr);
#endif // WIN32

    *(unsigned *)to = fr;
  }
}

int
ReadFieldFromHeader(const void *header, HeaderField const &headerField, Endianness endianness)
{
  if(!headerField.Defined())
  {
    return 0;
  }

  // NOTE: SEG-Y byte locations start at 1
  int index = headerField.byteLocation - 1;

  auto signed_header   = reinterpret_cast<const signed   char *>(header);
  auto unsigned_header = reinterpret_cast<const unsigned char *>(header);

  if(headerField.fieldWidth == FieldWidth::FourByte)
  {
    if(endianness == Endianness::BigEndian)
    {
      return (int32_t)(signed_header[index + 0] << 24 | unsigned_header[index + 1] << 16 | unsigned_header[index + 2] << 8 | unsigned_header[index + 3]);
    }
    else
    {
      assert(endianness == Endianness::LittleEndian);
      return (int32_t)(signed_header[index + 3] << 24 | unsigned_header[index + 2] << 16 | unsigned_header[index + 1] << 8 | unsigned_header[index + 0]);
    }
  }
  else
  {
    assert(headerField.fieldWidth == FieldWidth::TwoByte);
    if(endianness == Endianness::BigEndian)
    {
      return (int16_t)(signed_header[index + 0] << 8 | unsigned_header[index + 1]);
    }
    else
    {
      assert(endianness == Endianness::LittleEndian);
      return (int16_t)(signed_header[index + 1] << 8 | unsigned_header[index + 0]);
    }
  }
}

int FormatSize(BinaryHeader::DataSampleFormatCode dataSampleFormatCode)
{
  switch(dataSampleFormatCode)
  {
    case BinaryHeader::DataSampleFormatCode::Int8:
    case BinaryHeader::DataSampleFormatCode::UInt8:
      return 1;
    case BinaryHeader::DataSampleFormatCode::Int16:
    case BinaryHeader::DataSampleFormatCode::UInt16:
      return 2;
    case BinaryHeader::DataSampleFormatCode::Int24:
    case BinaryHeader::DataSampleFormatCode::UInt24:
      return 3;
    case BinaryHeader::DataSampleFormatCode::IBMFloat:
    case BinaryHeader::DataSampleFormatCode::Int32:
    case BinaryHeader::DataSampleFormatCode::FixedPoint:
    case BinaryHeader::DataSampleFormatCode::IEEEFloat:
    case BinaryHeader::DataSampleFormatCode::UInt32:
      return 4;
    case BinaryHeader::DataSampleFormatCode::IEEEDouble:
    case BinaryHeader::DataSampleFormatCode::Int64:
    case BinaryHeader::DataSampleFormatCode::UInt64:
      return 8;
    case BinaryHeader::DataSampleFormatCode::Unknown:
      return 0;
  }
  return 0;
}

bool
IsSEGYTypeUnbinned(SEGYType segyType)
{
  return segyType == SEGYType::CDPGathers || segyType == SEGYType::ReceiverGathers || segyType == SEGYType::ShotGathers;
}

bool
IsSEGYTypeWithGatherOffset(const SEGYType segyType)
{
  // TODO what other SEGY types have offsets?
  return segyType == SEGY::SEGYType::Prestack || IsSEGYTypeUnbinned(segyType);
}

bool autoDetectSEGYTextHeaderIsEBCDIC(const void* buffer, size_t bufferSize)
{
  int toScan = int(std::min(bufferSize, size_t(3200)));
  int ASCIISpace = 0;
  int EBCDICSpace = 0;

  const char* textHeader = static_cast<const char*>(buffer);

  for(int i = 0; i < toScan; i++)
  {
    if(textHeader[i] == 0x20) ASCIISpace++;
    if(textHeader[i] == 0x40) EBCDICSpace++;
  }

  return EBCDICSpace > ASCIISpace;
}

size_t convertSEGYEBCDICHeaderToASCII(const void* inputBuffer, size_t inputBufferSize, char* outputBuffer, size_t outputBufferSize, int columnWidth)
{
  static const char ebcdicToAscii[256] =
  {
    /*   0*/ 0, 0, 0, 0, 0, 0, 0, 0,
    /*   8*/ 0, 0, 0, 0, 0, '\r', 0, 0,
    /*  16*/ 0, 0, 0, 0, 0, 0, 0, 0,
    /*  24*/ 0, 0, 0, 0, 0, 0, 0, 0,
    /*  32*/ 0, 0, 0, 0, 0, '\n', 0, 0,
    /*  40*/ 0, 0, 0, 0, 0, 0, 0, 0,
    /*  48*/ 0, 0, 0, 0, 0, 0, 0, 0,
    /*  56*/ 0, 0, 0, 0, 0, 0, 0, 0,
    /*  64*/ ' ', 0, 0, 0, 0, 0, 0, 0,
    /*  72*/ 0, 0, 0, '.', '<', '(', '+', 0,
    /*  80*/ '&', 0, 0, 0, 0, 0, 0, 0,
    /*  88*/ 0, 0, '!', '$', '*', ')', ';', 0,
    /*  96*/ '-', '/', 0, 0, 0, 0, 0, 0,
    /* 104*/ 0, 0, '|', ',', '%', '_', '>', '?',
    /* 112*/ 0, 0, 0, 0, 0, 0, 0, 0,
    /* 120*/ 0, 0, ':', '#', '@', '\'', '=', '"',
    /* 128*/ 0, 'a', 'b', 'c', 'd', 'e', 'f', 'g',
    /* 136*/ 'h', 'i', 0, 0, 0, 0, 0, 0,
    /* 144*/ 0, 'j', 'k', 'l', 'm', 'n', 'o', 'p',
    /* 152*/ 'q', 'r', 0, 0, 0, 0, 0, 0,
    /* 160*/ 0, '~', 's', 't', 'u', 'v', 'w', 'x',
    /* 168*/ 'y', 'z', 0, 0, 0, 0, 0, 0,
    /* 176*/ 0, 0, 0, 0, 0, 0, 0, 0,
    /* 184*/ 0, '`', 0, 0, 0, 0, 0, 0,
    /* 192*/ '{', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
    /* 200*/ 'H', 'I', 0, 0, 0, 0, 0, 0,
    /* 208*/ '}', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    /* 216*/ 'Q', 'R', 0, 0, 0, 0, 0, 0,
    /* 224*/ '\\', 0, 'S', 'T', 'U', 'V', 'W', 'X',
    /* 232*/ 'Y', 'Z', 0, 0, 0, 0, 0, 0,
    /* 240*/ '0', '1', '2', '3', '4', '5', '6', '7',
    /* 248*/ '8', '9', 0, 0, 0, 0, 0, 0,
  };
  if (outputBuffer >= inputBuffer && outputBuffer < ((const char*)inputBuffer) + inputBufferSize)
  {
    fprintf(stderr, "Overlapping input and output buffer.");
    abort();
  }

  size_t toCopy = std::min(inputBufferSize, std::min(outputBufferSize, size_t(3200)));
  size_t copied = 0;
  size_t filled = 0;
  if (columnWidth > 0)
  {
    while (copied < toCopy && filled < outputBufferSize)
    {
      size_t copyLineSize = std::min(size_t(columnWidth), toCopy - copied);
      memcpy(outputBuffer + filled, static_cast<const char*>(inputBuffer) + copied, copyLineSize);
      copied += copyLineSize;
      filled += copyLineSize;
      if (filled < outputBufferSize)
      {
        outputBuffer[filled] = 0x25;
        filled++;
      }
    }
  }
  else
  {
    memcpy(outputBuffer, inputBuffer, toCopy);
    copied = toCopy;
    filled = toCopy;
  }
  
  std::transform(outputBuffer, outputBuffer + filled, outputBuffer, [](const uint8_t &d) { return ebcdicToAscii[d]; });

  return filled;
}

} // end namespace SEGY
