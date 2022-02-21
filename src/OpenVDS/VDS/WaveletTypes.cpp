/****************************************************************************
** Copyright 2022 The Open Group
** Copyright 2022 Bluware, Inc.
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

#include "WaveletTypes.h"

namespace Wavelet {

inline static int32_t GetAllocatedByteSizeForSize(const int32_t size)
{
  return size == 1 ? 1 : (size + 7) & -8;
}

bool
WaveletDataBlock::Initialize(WaveletDataFormat format, enum WaveletDataBlock::Dimensionality dimensionality, const int32_t (&size)[WaveletDataBlock::Dimensionality_Max], int& errorCode, std::string& errorString)
{
  Format = format;
  Dimensionality = dimensionality;

  switch(dimensionality)
  {
  case 1:
    Size[0] = size[0];
    Size[1] = 1;
    Size[2] = 1;
    Size[3] = 1;
    break;
  case 2:
    Size[0] = size[0];
    Size[1] = size[1];
    Size[2] = 1;
    Size[3] = 1;
    break;
  case 3:
    Size[0] = size[0];
    Size[1] = size[1];
    Size[2] = size[2];
    Size[3] = 1;
    break;
  default:
    errorString = "Serialized datablock has illegal dimensionality";
    errorCode = -1;
    return false;
  }

  AllocatedSize[0] = (format == WaveletDataFormat::Format_1Bit) ? ((Size[0]) + 7) / 8 : GetAllocatedByteSizeForSize(Size[0]);
  AllocatedSize[1] = GetAllocatedByteSizeForSize(Size[1]);
  AllocatedSize[2] = GetAllocatedByteSizeForSize(Size[2]);
  AllocatedSize[3] = GetAllocatedByteSizeForSize(Size[3]);
  Pitch[0] = 1;

  for (int i = 1; i < WaveletDataBlock::Dimensionality_Max; i++)
  {
    Pitch[i] = Pitch[i - 1] * AllocatedSize[i - 1];
  }

  uint64_t allocatedByteSize = AllocatedSize[0];

  for (int i = 1; i < WaveletDataBlock::Dimensionality_Max; i++)
  {
    allocatedByteSize *= AllocatedSize[i];
  }

  if (allocatedByteSize * GetElementSize(Format) > 0x7FFFFFFF)
  {
    char buffer[4096];
    snprintf(buffer, sizeof(buffer), "Datablock is too big (%d x %d x %d x %d x %d bytes)", AllocatedSize[0], AllocatedSize[1], AllocatedSize[2], AllocatedSize[3], GetElementSize(Format));
    errorString = buffer;
    errorCode = -1;
    return false;
  }
  return true;
}

}