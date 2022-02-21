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

#ifndef WAVELETDECOMPRESS_H
#define WAVELETDECOMPRESS_H

#include "WaveletTypes.h"
#include <memory>
#include <vector>

namespace Wavelet {

class WaveletDecompressor
{
protected:
  uint32_t *m_readCompressedData;

  const uint32_t *m_noValueData;

  IntVector3 m_bandSize[TRANSFORM_MAX_ITERATIONS + 1];
  int32_t m_transformMask[TRANSFORM_MAX_ITERATIONS];
  int32_t m_transformIterations;

  int32_t m_dataVersion;
  int32_t m_dimensions;
  int32_t m_dataBlockSizeX;
  int32_t m_dataBlockSizeY;
  int32_t m_dataBlockSizeZ;
  int32_t m_transformSizeX;
  int32_t m_transformSizeY;
  int32_t m_transformSizeZ;
  int32_t m_allocatedSizeX;
  int32_t m_allocatedSizeY;
  int32_t m_allocatedSizeZ;
  int32_t m_allocatedSizeXY;
  int32_t m_allocatedHalfSizeX;
  int32_t m_allocatedHalfSizeY;
  int32_t m_allocatedHalfSizeZ;
  int32_t m_pixelSetChildrenCount;
  uint32_t m_integerInfo;

  std::unique_ptr<Wavelet_PixelSetPixel[]> m_pixelSetPixelInSignificant;
  std::unique_ptr<Wavelet_PixelSetChildren[]> m_pixelSetChildren;

  int32_t m_pixelSetPixelInSignificantCount;

public:
  WaveletDecompressor(uint32_t integerInfo, void* compressedData, int32_t transformSizeX, int32_t transformSizeY, int32_t transformSizeZ, int32_t allocatedSizeX, int32_t allocatedSizeY, int32_t allocatedSizeZ, int32_t dimensions, int32_t dataVersion);
  void Init();
  
  bool Decompress(bool isTransform, int32_t decompressInfo, float decompressSlice, int32_t decompressFlip, float *startThreshold, float *threshold, WaveletDataFormat dataBlockFormat, const FloatRange &originalValueRange, float integerScale, float integerOffset, bool isUseNoValue, float noValue, bool *isAnyNoValue, float *waveletNoValue, bool isNormalize, int decompressLevel, bool isLossless, int compressedAdaptiveDataSize, WaveletDataBlock &dataBlock, std::vector<uint8_t> &target, int& errorCode, std::string& errorString);
  void DecompressNoValuesHeader();
  void InverseTransform(float *source);
  void DecompressNoValues(float* noValue, std::vector<uint32_t> &buffer);
  void ApplyNoValues(float *source, uint32_t* bitBuffer, float noValue);
};

bool Wavelet_IsCompressedDataAllNoValue(const void* pxCompressedData, int nCompressedAdaptiveDataSize);
bool Wavelet_Decompress(void* compressedData, int nCompressedAdaptiveDataSize, WaveletDataFormat dataBlockFormat, const FloatRange& valueRange, float integerScale, float integerOffset, bool isUseNoValue, float noValue, bool isNormalize, int nDecompressLevel, bool isLossless, WaveletDataBlock& dataBlock, std::vector<uint8_t>& target, int& errorCode, std::string& errorString);

float Wavelet_GetNormalizedValue(float* normalizeField, int iX, int iY, int iZ, int nNormalizeX, int nNormalizeY, int nNormalizeZ);
void WaveletDecompress_RleDecode(uint8_t* rleBytes, uint32_t* bitBuffer, int32_t intsToDecode);

#define RLE_BYTE_MASK 0
#define RLE_2BYTE_MASK 1
#define RLE_3BYTE_MASK 2
#define RLE_4BYTE_MASK 3

inline bool WaveletDecompress_RleDecodeOneRun(uint8_t*& rleByte, uint32_t& setBits)
{
  uint32_t value = *rleByte;
  rleByte++;

  uint8_t mask = (value >> 1) & 0x3;

  setBits = value >> 3;

  if (mask > RLE_BYTE_MASK)
  {
    setBits |= *rleByte << (8 - 3);
    rleByte++;
  }
  if (mask > RLE_2BYTE_MASK)
  {
    setBits |= *rleByte << (16 - 3);
    rleByte++;
  }
  if (mask > RLE_3BYTE_MASK)
  {
    setBits |= *rleByte << (24 - 3);
    rleByte++;
  }

  return value & 1;
}

}

#endif
