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

#include "Rle.h"

#include <assert.h>
#include <string.h>

#include <stdexcept>

#define HUERLE_RUN_LENGTH_FIRST_BYTE_SHIFT    6
#define HUERLE_RUN_LENGTH_FIRST_BYTE_MASK     (0x3f)
#define HUERLE_EXTENDED_RUN_FLAG              (1 << 7)
#define HUERLE_UNIQUE_RUN_FLAG                (1 << 6)
#define HUERLE_MAX_RUN_LENGTH                 (1 << (HUERLE_RUN_LENGTH_FIRST_BYTE_SHIFT + 8))

namespace OpenVDS
{

template<class T>
static uint64_t RlePack(T value)
{
  uint16_t packed16;
  uint32_t packed32;
  uint64_t packed64;

  if(sizeof(T) < 2)
  {
    packed16 = ((uint16_t)value << 8) | (uint16_t)value;
  }
  else if(sizeof(T) == 2)
  {
    packed16 = (uint16_t)value;
  }

  if(sizeof(T) < 4)
  {
    packed32 = ((uint32_t)packed16 << 16) | (uint32_t)packed16;
  }
  else if(sizeof(T) == 4)
  {
    packed32 = (uint32_t)value;
  }

  if(sizeof(T) < 8)
  {
    packed64 = ((uint64_t)packed32 << 32) | (uint64_t)packed32;
  }
  else if(sizeof(T) == 8)
  {
    packed64 = (uint64_t)value;
  }

  return packed64;
}

template <class TYPE>
int32_t RleCompress(uint8_t *target, int32_t targetSizeMax, uint8_t *source, int32_t sourceSize)
{
  uint8_t *originalTarget = target;

  RLEHeader *rleHeader = (RLEHeader *)target;

  int32_t bytesWritten = sizeof(RLEHeader);

  target += bytesWritten;

  int32_t remainingElement = sourceSize / sizeof(TYPE);

  int32_t jafs = sizeof(uint64_t) / sizeof(TYPE);

  assert(int32_t(remainingElement * sizeof(TYPE)) == sourceSize);

  TYPE *typedSource = (TYPE *)source;

  uint8_t *sourceEnd = source + sourceSize;
	(void)sourceEnd;

  while (remainingElement > 0)
  {
    int32_t maxRunLength;

    bool isUnique;

    TYPE *runStart = typedSource;

    TYPE t0 = *typedSource;

    if (remainingElement > HUERLE_MAX_RUN_LENGTH)
    {
      maxRunLength = HUERLE_MAX_RUN_LENGTH;
    }
    else
    {
      maxRunLength = remainingElement;
    }

    if(remainingElement >= 2 && t0 == typedSource[1])
    {
      // Make a run of equal values
      isUnique = false;

      while(1)
      {
        // Check if source is aligned and we might have a long run
        if(((int64_t)typedSource & 7) == 0 && maxRunLength > jafs)
        {
          uint64_t uPacked = RlePack(t0);

          while(maxRunLength > jafs && *(uint64_t *)typedSource == uPacked)
          {
            maxRunLength -= jafs;
            typedSource += jafs;
          }
        }

        if(maxRunLength > 0 && *typedSource == t0)
        {
          maxRunLength--;
          typedSource++;
        }
        else
        {
          break;
        }
      }
    }
    else
    {
      // Make a run of unique values
      isUnique = true;

      while(maxRunLength > 0)
      {
        if(sizeof(TYPE) == 1)
        {
          if(maxRunLength >= 3 && typedSource[0] == typedSource[1] && typedSource[0] == typedSource[2])
          {
            break;
          }
        }
        else
        {
          if(maxRunLength >= 2 && typedSource[0] == typedSource[1])
          {
            break;
          }
        }
        typedSource++;
        maxRunLength--;
      }
    }

    // Write run
    int32_t runLength = (int32_t )(typedSource - runStart);

    assert(runLength > 0);

    // We don't need to encode runs of 0 length, so we store one less than the actual run length
    uint8_t u0 = ((runLength - 1) & HUERLE_RUN_LENGTH_FIRST_BYTE_MASK) | (isUnique ? HUERLE_UNIQUE_RUN_FLAG : 0);
    uint8_t u1 = (runLength - 1) >> HUERLE_RUN_LENGTH_FIRST_BYTE_SHIFT;

    if(u1)
    {
      *target++ = u0 | HUERLE_EXTENDED_RUN_FLAG;
      *target++ = u1;
    }
    else
    {
      *target++ = u0;
    }

    if(isUnique)
    {
      memcpy(target, runStart, (typedSource - runStart) * sizeof(TYPE));
      target += (typedSource - runStart) * sizeof(TYPE);
    }
    else
    {
      *((TYPE *)target) = t0;
      target += sizeof(TYPE);
    }

    remainingElement -= runLength;
  }

  assert(sourceEnd == (uint8_t *)typedSource);

  int32_t written = (int32_t)(target - originalTarget);

  rleHeader->originalSize = sourceSize;
  rleHeader->compressedSize = written;
  rleHeader->rleUnitSize = sizeof(TYPE);

  return rleHeader->compressedSize;
}

int32_t RleCompress(uint8_t *target, int32_t targetSize, uint8_t *source, int32_t sourceSize, int32_t rleUnitSize)
{
  switch (rleUnitSize)
  {
  default:
    throw std::runtime_error(("Unsupported element size for RLE compression"));

  case 1:
    return RleCompress<uint8_t>(target,  targetSize, source, sourceSize);

  case 2:
    return RleCompress<uint16_t>(target, targetSize, source, sourceSize);

  case 4:
    return RleCompress<uint32_t>(target, targetSize, source, sourceSize);

  case 8:
    return RleCompress<uint64_t>(target, targetSize, source, sourceSize);
  }
}

template <typename T>
int32_t RleDecompress(uint8_t *target_parameter, int32_t nTargetSize, uint8_t *source)
{
  RLEHeader * rleHeader = (RLEHeader *)source;

  source += sizeof(RLEHeader);

  T *target = (T *)target_parameter;

  uint8_t *end = target_parameter + rleHeader->originalSize;
     
  int32_t jafs = sizeof(uint64_t) / sizeof(T);

  while ((uint8_t *)target != (uint8_t *)end)
  {
    // Get the run length (one or two bytes) from the source
    uint8_t u0 = *source++;

    int32_t runLength;

    if (u0 & HUERLE_EXTENDED_RUN_FLAG)
    {
      uint8_t u1 = *source++;

      runLength = (u0 & HUERLE_RUN_LENGTH_FIRST_BYTE_MASK) |
                   (u1 << HUERLE_RUN_LENGTH_FIRST_BYTE_SHIFT);
    }
    else
    {
      runLength = (u0 & HUERLE_RUN_LENGTH_FIRST_BYTE_MASK);
    }

    // We don't need to encode runs of 0 length, so we store one less than the actual run length
    runLength++;

    bool isUnique = (u0 & HUERLE_UNIQUE_RUN_FLAG) != 0;

    if(isUnique)
    {
      memcpy(target, source, runLength * sizeof(T));
      target += runLength;
      source += runLength * sizeof(T);
    }
    else
    {
      // Get the value from the source
      T t0 = *((T *)source);

      source += sizeof(T);

      assert(!((int64_t)target & (sizeof(T) - 1)));

      // Check for long runs
      if (runLength >= 2 * jafs)
      {
        // Write data up to an aligned boundary
        while ((int64_t)target & 7)
        {
          *target++ = t0;
          runLength--;
        }

        assert(!((int64_t)target & (sizeof(T) - 1)));

        uint64_t packedValue = RlePack(t0);

        while (runLength >= jafs)
        {
          *(uint64_t *)target = packedValue;
          target += jafs;
          runLength -= jafs;
        }
      }

      // Write tail data (or, in short runs, ALL the data):
      while (runLength > 0)
      {
        assert((uint8_t *)target != (uint8_t *)end);

        *target++ = t0;
        runLength--;
      }
    }
  }

  return rleHeader->originalSize;
}
int32_t RleDecompress(uint8_t *target, int32_t targetSize, uint8_t *source)
{
  RLEHeader *rleHeader = (RLEHeader *)source;

  assert(targetSize == rleHeader->originalSize);

  switch (rleHeader->rleUnitSize)
  {
    case 1:
      return RleDecompress<uint8_t>(target, targetSize, source);

    case 2:
      return RleDecompress<uint16_t>(target, targetSize, source);

    case 4:
      return RleDecompress<uint32_t>(target, targetSize, source);

    case 8:
      return RleDecompress<uint64_t>(target, targetSize, source);

    default:
      assert(("Unsupported unit size"));
  }
  return 0;
}
}
