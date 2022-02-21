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

#ifndef WAVELETTYPES_H
#define WAVELETTYPES_H

#include <math.h>
#include <assert.h>
#include <algorithm>
#include <stdexcept>
#include <string>

#define WAVELET_MIN_COMPRESSION_TOLERANCE 0.01f

#define WAVELET_DATA_VERSION_1_4 (671) // progressive wavelet transform
#define WAVELET_DATA_VERSION_1_5 (672) // U8, U16 and U32 native/lossless compression
#define WAVELET_DATA_VERSION_1_6 (673) // Fixed stream encoding case for rare cases

#define WAVELET_MIN_COMPRESSED_HEADER (6 * 4)

#define WAVELET_BAND_MIN_SIZE  8
#define NORMAL_BLOCK_SIZE 8
#define NORMAL_BLOCK_SIZE_FLOAT 8.0f

#define WAVELET_MAX_PIXELSETPIXEL_SIZE (8 * 8 * 8 * (sizeof(Wavelet::Wavelet_PixelSetPixel)))
#define WAVELET_MAX_PIXELSETCHILDREN_SIZE (7 * 7 * 7 * 7 * (sizeof(Wavelet::Wavelet_PixelSetChildren)))

#define ADAPTIVEWAVELET_ALIGNBUFFERSIZE 256
#define DECODEITERATOR_MAXDECODEBITS    256

#define WAVELET_ADAPTIVE_LEVELS 16

namespace Wavelet {

enum class WaveletDataFormat
{
  Format_Any = -1, ///< Volume data can be in any format
  Format_1Bit,     ///< Volume data is in packed 1-bit format
  Format_U8,       ///< Volume data is in unsigned 8 bit
  Format_U16,      ///< Volume data is in unsigned 16 bit
  Format_R32,      ///< Volume data is in 32 bit float
  Format_U32,      ///< Volume data is in unsigned 32 bit
  Format_R64,      ///< Volume data is in 64 bit double
  Format_U64,      ///< Volume data is in unsigned 64 bit
};

struct WaveletDataBlock
{
  enum Dimensionality
  {
    Dimensionality_1 = 1,
    Dimensionality_2 = 2,
    Dimensionality_3 = 3,
    Dimensionality_4 = 4,
    Dimensionality_Max = Dimensionality_4 
  };

  WaveletDataFormat Format;
  enum Dimensionality Dimensionality;
  int32_t Size[WaveletDataBlock::Dimensionality_Max];
  int32_t AllocatedSize[WaveletDataBlock::Dimensionality_Max];
  int32_t Pitch[WaveletDataBlock::Dimensionality_Max];

  bool Initialize(WaveletDataFormat format, enum WaveletDataBlock::Dimensionality dimensionality, const int32_t(&size)[WaveletDataBlock::Dimensionality_Max], int& errorCode, std::string& errorString);
};

inline uint32_t GetElementSize(WaveletDataFormat format)
{
  switch(format)
  {
  default:
    throw std::runtime_error("Illegal format");
  case WaveletDataFormat::Format_1Bit:
    return 1;
  case WaveletDataFormat::Format_U8:
    return 1;
  case WaveletDataFormat::Format_U16:
    return 2;
  case WaveletDataFormat::Format_R32:
  case WaveletDataFormat::Format_U32:
    return 4;
  case WaveletDataFormat::Format_U64:
  case WaveletDataFormat::Format_R64:
    return 8;
  }
}

inline uint32_t GetElementSize(const WaveletDataBlock &datablock)
{
  return GetElementSize(datablock.Format);
}

inline uint32_t GetByteSize(const int32_t (&size)[WaveletDataBlock::Dimensionality_Max], WaveletDataFormat format, bool isBitSize = true)
{
  int byteSize = size[0] * GetElementSize(format);

  if(format == WaveletDataFormat::Format_1Bit && isBitSize)
  {
    byteSize = (byteSize + 7) / 8;
  }

  for (int i = 1; i < WaveletDataBlock::Dimensionality_Max; i++)
  {
    byteSize *= size[i];
  }

  return byteSize;
}

inline uint32_t GetByteSize(const WaveletDataBlock &block)
{
  return GetByteSize(block.Size, block.Format);
}

inline uint32_t GetAllocatedByteSize(const WaveletDataBlock &block)
{
  return GetByteSize(block.AllocatedSize, block.Format, false);
}

struct FloatRange
{
  float Min;
  float Max;

  FloatRange() = default;
  FloatRange(float min, float max) : Min(min), Max(max) {}
  inline float Size() const { return Max - Min; }
};

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
#endif

struct IntVector3
{
  typedef int32_t element_type;
  enum { element_count = 3 };

  union
  {
    struct
    {
      int32_t X, Y, Z;
    };
    int32_t data[3];
  };

  IntVector3() : X(), Y(), Z() {}
  IntVector3(int32_t X, int32_t Y, int32_t Z) : X(X), Y(Y), Z(Z) {}

  inline       int32_t& operator[] (size_t n) { return data[n]; }
  inline const int32_t& operator[] (size_t n) const { return data[n]; }
  inline void  Assign(int32_t X, int32_t Y, int32_t Z) { data[0] = X; data[1] = Y; data[2] = Z; }
};

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

inline int Wavelet_GetEffectiveWaveletAdaptiveLoadLevel(float desiredTolerance, float compressionTolerance)
{
  assert(desiredTolerance >= WAVELET_MIN_COMPRESSION_TOLERANCE);
  assert(compressionTolerance >= WAVELET_MIN_COMPRESSION_TOLERANCE);

  int waveletAdaptiveLoadLevel = (int)log2(desiredTolerance / compressionTolerance);

  return std::max(0, waveletAdaptiveLoadLevel);
}

inline int Wavelet_GetEffectiveWaveletAdaptiveLoadLevel(float desiredRatio, int64_t const (&adaptiveLevelSizes)[WAVELET_ADAPTIVE_LEVELS], int64_t uncompressedSize)
{
  int level = 0;

  while (level + 1 < WAVELET_ADAPTIVE_LEVELS)
  {
    if (adaptiveLevelSizes[level + 1] == 0 || ((float)uncompressedSize / (float)adaptiveLevelSizes[level + 1]) > desiredRatio)
    {
      break;
    }

    level++;
  }

  return level;
}

inline void Wavelet_AccumulateAdaptiveLevelSizes(int32_t totalSize, int64_t (&adaptiveLevelSizes)[WAVELET_ADAPTIVE_LEVELS], bool subtract, const uint8_t (&targetLevels)[WAVELET_ADAPTIVE_LEVELS])
{
  int32_t remainingSize = totalSize;

  for(int level = 0; level < WAVELET_ADAPTIVE_LEVELS; level++)
  {
    if(targetLevels[level] == 0)
    {
      break;
    }
    assert(remainingSize >= 0);

    remainingSize = (int32_t)((uint64_t)remainingSize * targetLevels[level] / 255);

    if(!subtract)
    {
      adaptiveLevelSizes[level] += remainingSize;
    }
    else
    {
      adaptiveLevelSizes[level] -= remainingSize;
    }
    assert(adaptiveLevelSizes[level] >= 0);
  }
}

inline void Wavelet_EncodeAdaptiveLevelsMetadata(int32_t totalSize, int const (&adaptiveLevels)[WAVELET_ADAPTIVE_LEVELS], uint8_t (&targetLevels)[WAVELET_ADAPTIVE_LEVELS])
{
  int32_t remainingSize = totalSize;

  for(int level = 0; level < WAVELET_ADAPTIVE_LEVELS; level++)
  {
    assert(adaptiveLevels[level] <= remainingSize);

    if(remainingSize == 0)
    {
      targetLevels[level] = 0;
      continue;
    }

    targetLevels[level] = (uint8_t)(((uint64_t)adaptiveLevels[level] * 255 + (remainingSize - 1)) / remainingSize);

    remainingSize = (int)((uint64_t)remainingSize * targetLevels[level] / 255);
  }
}

inline int Wavelet_DecodeAdaptiveLevelsMetadata(uint64_t totalSize, int targetLevel, uint8_t const *levels)
{
  assert(targetLevel >= -1 && targetLevel < WAVELET_ADAPTIVE_LEVELS);

  int remainingSize = (int)totalSize;

  for (int level = 0; level <= targetLevel; level++)
  {
    if (levels[level] == 0)
    {
      break;
    }
    remainingSize = (int)((uint64_t)remainingSize * levels[level] / 255);
  }

  return remainingSize;
}

enum Wavelet_IntegerInfo
{
  WAVELET_INTEGERINFO_ISINTEGER = (1 << 0),
  WAVELET_INTEGERINFO_ISLOSSLESSOPTIMIZED = (1 << 1),
  WAVELET_INTEGERINFO_ISCOMPRESSEDWITHDIFFPASS = (1 << 2),
  WAVELET_INTEGERINFO_16BIT = (1 << 3)
};

struct Wavelet_PixelSetPixel
{
  int32_t X;
  int32_t Y;
  int32_t Z;
};

struct Wavelet_PixelSetChildren
{
  uint32_t transformIteration;

  int32_t X;
  int32_t Y;
  int32_t Z;

  int32_t subBand;
};

struct Wavelet_SubBandInfo
{
  IntVector3 pos;

  int32_t childSubBand;

  IntVector3 childPos[7];

  IntVector3 extraChildEdge[7];

  IntVector3 legalChildEdge[7];

  int32_t childSector[7];
};

struct Wavelet_TransformData
{
  int child;

  IntVector3 childCount;

  int isNormal;

  Wavelet_SubBandInfo subBandInfo[8];
};

#define WAVELET_ADAPTIVELL_X_SHIFT 0
#define WAVELET_ADAPTIVELL_Y_SHIFT 9
#define WAVELET_ADAPTIVELL_Z_SHIFT 18
#define WAVELET_ADAPTIVELL_ITER_SHIFT 27
#define WAVELET_ADAPTIVELL_XYZ_AND_MASK ((1 << 9) - 1)

enum
{
  TRANSFORM_MAX_ITERATIONS = 10
};

struct Wavelet_Compiled_SubBand
{
  uint16_t childPosX;
  uint16_t childPosY;
  uint16_t childPosZ;

  uint16_t extraChildEdgeX;
  uint16_t extraChildEdgeY;
  uint16_t extraChildEdgeZ;

  uint16_t legalChildEdgeX;
  uint16_t legalChildEdgeY;
  uint16_t legalChildEdgeZ;

  uint16_t childSubBand;
};

struct Wavelet_Compiled_SubBandInfo
{
  uint8_t isNormal;
  uint8_t dummyAlign;

  uint16_t normalChildSubBand;

  uint8_t childX;
  uint8_t childY;
  uint8_t childZ;

  uint16_t posX;
  uint16_t posY;
  uint16_t posZ;

  uint8_t childSubBand;
  uint8_t dummyAlign2;

  Wavelet_Compiled_SubBand firstSubBand; // can me more than one based on nChildSubBand
};

struct Wavelet_FastDecodeInsig
{
  int32_t xyz;

  uint16_t subBandPos;

  uint8_t iteration;
  uint8_t padding;
   
  int     GetX() const {return (xyz >> WAVELET_ADAPTIVELL_X_SHIFT) & WAVELET_ADAPTIVELL_XYZ_AND_MASK; }
  int     GetY() const {return (xyz >> WAVELET_ADAPTIVELL_Y_SHIFT) & WAVELET_ADAPTIVELL_XYZ_AND_MASK; }
  int     GetZ() const {return (xyz >> WAVELET_ADAPTIVELL_Z_SHIFT) & WAVELET_ADAPTIVELL_XYZ_AND_MASK; }

  void    SetXYZ(int uX, int uY, int uZ) {xyz = uX | (uY << WAVELET_ADAPTIVELL_Y_SHIFT) | (uZ << WAVELET_ADAPTIVELL_Z_SHIFT); }
 
  Wavelet_FastDecodeInsig() : xyz(), subBandPos(), iteration(), padding() {}
  Wavelet_FastDecodeInsig(int nX, int nY, int nZ, int nIteration, unsigned short iSubBandPos)
  {
    SetXYZ(nX, nY, nZ);
    iteration = nIteration;
    subBandPos = iSubBandPos;
    padding = 0;
  }

};

struct Wavelet_FastEncodeInsig
{
  int xyz;

  unsigned short subBandPos;

  char iteration;
  char isDeleteMe;
   
  int GetX() const {return (xyz >> WAVELET_ADAPTIVELL_X_SHIFT) & WAVELET_ADAPTIVELL_XYZ_AND_MASK; }
  int GetY() const {return (xyz >> WAVELET_ADAPTIVELL_Y_SHIFT) & WAVELET_ADAPTIVELL_XYZ_AND_MASK; }
  int GetZ() const {return (xyz >> WAVELET_ADAPTIVELL_Z_SHIFT) & WAVELET_ADAPTIVELL_XYZ_AND_MASK; }

  void SetXYZ(int uX, int uY, int uZ) {xyz = uX | (uY << WAVELET_ADAPTIVELL_Y_SHIFT) | (uZ << WAVELET_ADAPTIVELL_Z_SHIFT); }
 
  Wavelet_FastEncodeInsig() : xyz(), subBandPos(), iteration(), isDeleteMe() {}
  Wavelet_FastEncodeInsig(int nX, int nY, int nZ, int nIteration, unsigned short iSubBandPos)
  {
    SetXYZ(nX, nY, nZ);
    iteration = nIteration;
    subBandPos = iSubBandPos;
    isDeleteMe = 0;
  }
};

struct Wavelet_FastEncodeInsigAllNormal
{
public:
  unsigned int iterXYZ;

  int GetX() const { return (iterXYZ >> WAVELET_ADAPTIVELL_X_SHIFT) & WAVELET_ADAPTIVELL_XYZ_AND_MASK; }
  int GetY() const { return (iterXYZ >> WAVELET_ADAPTIVELL_Y_SHIFT) & WAVELET_ADAPTIVELL_XYZ_AND_MASK; }
  int GetZ() const { return (iterXYZ >> WAVELET_ADAPTIVELL_Z_SHIFT) & WAVELET_ADAPTIVELL_XYZ_AND_MASK; }
  int GetIteration() const { return iterXYZ >> WAVELET_ADAPTIVELL_ITER_SHIFT; }

  void SetXYZIter(int uX, int uY, int uZ, int uIter) { iterXYZ = uX | (uY << WAVELET_ADAPTIVELL_Y_SHIFT) | (uZ << WAVELET_ADAPTIVELL_Z_SHIFT) | (uIter << WAVELET_ADAPTIVELL_ITER_SHIFT); }

  Wavelet_FastEncodeInsigAllNormal() {}

  Wavelet_FastEncodeInsigAllNormal(int nX, int nY, int nZ, int nIteration)
  {
    SetXYZIter(nX, nY, nZ, nIteration);
  }

};

}

#endif
