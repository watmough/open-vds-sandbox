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

#ifndef DATABLOCK_H
#define DATABLOCK_H

#include <OpenVDS/OpenVDS.h>
#include <OpenVDS/VolumeData.h>
#include "VolumeDataLayer.h"

#include <stdexcept>

namespace OpenVDS
{

struct DataBlock
{
  enum Dimensionality
  {
    Dimensionality_1 = 1,
    Dimensionality_2 = 2,
    Dimensionality_3 = 3,
    Dimensionality_4 = 4,
    Dimensionality_Max = Dimensionality_4 
  };

  VolumeDataFormat Format;
  VolumeDataComponents Components;
  enum Dimensionality Dimensionality;
  int32_t Size[DataBlock::Dimensionality_Max];
  int32_t AllocatedSize[DataBlock::Dimensionality_Max];
  int32_t Pitch[DataBlock::Dimensionality_Max];
};

class DataBlockDescriptor
{
public:
  //This layout is fixed
  int32_t Dimensionality;

  int32_t SizeX;
  int32_t SizeY;
  int32_t SizeZ;

  VolumeDataFormat Format;
  VolumeDataComponents Components;

  bool IsValid(const int32_t (&voxelSize)[DataBlock::Dimensionality_Max]) const
  {
    return Dimensionality >= 1 &&
           Dimensionality <= 3 &&
           (SizeX == voxelSize[0]                        ) &&
           (SizeY == voxelSize[1] || Dimensionality < 2) &&
           (SizeZ == voxelSize[2] || Dimensionality < 3) &&
           (Format == VolumeDataFormat::Format_1Bit ||
            Format == VolumeDataFormat::Format_U8   ||
            Format == VolumeDataFormat::Format_U16  ||
            Format == VolumeDataFormat::Format_U32  ||
            Format == VolumeDataFormat::Format_U64  ||
            Format == VolumeDataFormat::Format_R32  ||
            Format == VolumeDataFormat::Format_R64) &&
           (Components == VolumeDataComponents::Components_1 ||
            Components == VolumeDataComponents::Components_2 ||
            Components == VolumeDataComponents::Components_4);
  }

  bool IsValid() const { int32_t voxelSize[DataBlock::Dimensionality_Max] = {SizeX, SizeY, SizeZ}; return IsValid(voxelSize); }
};

bool InitializeDataBlock(const DataBlockDescriptor &descriptor, DataBlock &dataBlock, Error &error);
bool InitializeDataBlock(VolumeDataFormat format, VolumeDataComponents components, enum DataBlock::Dimensionality dimensionality, const int32_t (&size)[DataBlock::Dimensionality_Max], DataBlock &dataBlock, Error &error);

inline int32_t GetVoxelFormatByteSize(VolumeDataFormat format)
{
  int32_t iRetval = -1;
  switch (format) {
  case VolumeDataFormat::Format_R64:
  case VolumeDataFormat::Format_U64:
    iRetval = 8;
    break;
  case VolumeDataFormat::Format_R32:
  case VolumeDataFormat::Format_U32:
    iRetval = 4;
    break;
  case VolumeDataFormat::Format_U16:
    iRetval = 2;
    break;
  case VolumeDataFormat::Format_U8:
  case VolumeDataFormat::Format_1Bit:
    iRetval =1;
    break;
  default:
    throw std::runtime_error("Unknown voxel format");
  }

  return iRetval;
}

static uint32_t GetElementSize(VolumeDataFormat format, VolumeDataComponents components)
{
  switch(format)
  {
  default:
    throw std::runtime_error("Illegal format");
  case VolumeDataFormat::Format_1Bit:
    return 1;
  case VolumeDataFormat::Format_U8:
    return 1 * components;
  case VolumeDataFormat::Format_U16:
    return 2 * components;
  case VolumeDataFormat::Format_R32:
  case VolumeDataFormat::Format_U32:
    return 4 * components;
  case VolumeDataFormat::Format_U64:
  case VolumeDataFormat::Format_R64:
    return 8 * components;
  }
}

inline uint32_t GetElementSize(const DataBlock &datablock)
{
  return GetElementSize(datablock.Format, datablock.Components);
}

inline uint32_t GetByteSize(const int32_t (&size)[DataBlock::Dimensionality_Max], VolumeDataFormat format, VolumeDataComponents components, bool isBitSize = true)
{
  int byteSize = size[0] * GetElementSize(format, components);

  if(format == VolumeDataFormat::Format_1Bit && isBitSize)
  {
    byteSize = (byteSize + 7) / 8;
  }

  for (int i = 1; i < DataBlock::Dimensionality_Max; i++)
  {
    byteSize *= size[i];
  }

  return byteSize;
}

inline uint32_t GetByteSize(const DataBlock &block)
{
  return GetByteSize(block.Size, block.Format, block.Components);
}

inline uint32_t GetAllocatedByteSize(const DataBlock &block)
{
  return GetByteSize(block.AllocatedSize, block.Format, block.Components, false);
}

inline int32_t GetAllocatedByteSizeForSize(const int32_t size)
{
  return size == 1 ? 1 : (size + 7) & -8;
}

int32_t CombineAndReduceDimensions (int32_t (&sourceSize  )[DataBlock::Dimensionality_Max],
                                    int32_t (&sourceOffset)[DataBlock::Dimensionality_Max],
                                    int32_t (&targetSize  )[DataBlock::Dimensionality_Max],
                                    int32_t (&targetOffset)[DataBlock::Dimensionality_Max],
                                    int32_t (&overlapSize )[DataBlock::Dimensionality_Max],
                                    const int32_t (&origSourceSize  )[Dimensionality_Max],
                                    const int32_t (&origSourceOffset)[Dimensionality_Max],
                                    const int32_t (&origTargetSize  )[Dimensionality_Max],
                                    const int32_t (&origTargetOffset)[Dimensionality_Max],
                                    const int32_t (&origOverlapSize )[Dimensionality_Max]);

void DispatchBlockCopy(VolumeDataFormat format,
                       void       *target, const int32_t (&targetOffset)[DataBlock::Dimensionality_Max], const int32_t (&targetSize)[DataBlock::Dimensionality_Max],
                       void const *source, const int32_t (&sourceOffset)[DataBlock::Dimensionality_Max], const int32_t (&sourceSize)[DataBlock::Dimensionality_Max],
                       const int32_t (&overlapSize) [DataBlock::Dimensionality_Max]);

void FixupBorder(DataBlock const &dataBlock, void *buffer, VolumeDataFormat format, VolumeDataComponents components, BorderMode borderMode, const int (&borderNegativeRadius)[6], const int (&borderPositiveRadius)[6], const int (&layoutMin)[6], const int (&layoutSize)[6], const int (&layoutDimension)[DataBlock::Dimensionality_Max]);

void DownSampleAndCopyRegion(DataBlock const &targetDataBlock,
                             DataBlock const &sourceDataBlock,
                             void *targetBuffer, const void *sourceBuffer,
                             int targetOffsetX, int targetOffsetY, int targetOffsetZ,
                             int targetSizeX,   int targetSizeY,   int targetSizeZ,
                             int sourceOffsetX, int sourceOffsetY, int sourceOffsetZ,
                             float rNoValue, int fullResolutionDimension);

}

#endif //DATABLOCK_H
