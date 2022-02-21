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

#ifndef WAVELETADAPTIVELLDECOMPRESS_H
#define WAVELETADAPTIVELLDECOMPRESS_H

#include "WaveletTypes.h"
#include <inttypes.h>

namespace Wavelet {

enum class DecompressAdaptiveMode
{
  AssumeNoOverwrite,
  AllowOverwrite,
  PreventOverwrite
};

struct WaveletAdaptiveLL_DecodeIterator
{
  int32_t dataVersion;

  Wavelet_FastEncodeInsig *insig;
  Wavelet_FastEncodeInsig *sig;

  int32_t *pos;
  const uint8_t *compiledTransformData;
  
  float* streamFirstValues;
 
  int32_t *valueEncodingMultiple;
  int32_t *valuesAtLevelMultiple;
  int32_t *valueEncodingSingle;
  int32_t *valuesAtLevelSingle;
  
  float *picture;

  uint8_t *stream;

  const float *maxChildrenBuffer;
  const float *maxChildrenChildrenBuffer;

  int32_t sizeX;
  int32_t sizeY;
  int32_t sizeZ;
  int32_t sizeXY;
  int32_t maxSizeX;
  int32_t maxSizeXY;
  int32_t maxChildren;

  int32_t firstSubBand[8];

  int32_t screenDisplacementAllNormal[8];
  int32_t childDisplacementAllNormal[8];
  int32_t screenDisplacement[8];

  int32_t allNormalAndMask;

  int32_t decodeBits;
  int32_t decompressLevel;
  int32_t dimensions;
  int32_t multiple;

  float threshold;
  float startThreshold;

  bool isAllNormal;
  bool isInteger;

  uint8_t transformMask[12];
  uint8_t children[8];

  Wavelet_PixelSetChildren *pixelSetChildren;
  int32_t pixelSetChildrenCount;

  Wavelet_PixelSetPixel *pixelSetPixelInSignificant;
  int32_t pixelSetPixelInsignificantCount;
};

WaveletAdaptiveLL_DecodeIterator WaveletAdaptiveLLDecompress_CreateDecodeIterator(int dataVersion, uint8_t *stream, float *picture, int dimensions, int sizeX, int sizeY, int sizeZ,
                                                                        const float threshold, const float startThreshold, int *transformMask, Wavelet_TransformData *transformData, int transformDataCount,
                                                                        Wavelet_PixelSetChildren *pixelSetChildren, int pixelSetChildrenCount, Wavelet_PixelSetPixel *pixelSetPixelInSignificant, int pixelSetPixelInsignificantCount,
                                                                        float* maxChildrenBuffer, float* maxChildrenChildrenBuffer, int maxSizeX, int maxSizeXY, uint8_t* &tempBuffer, int maxChildren, int maxPixels, int decompressLevel, bool isInteger);

bool WaveletAdaptiveLL_IsWaveletStreamEncodedWithBug(Wavelet_TransformData* transformData, int transformDataCount, int* transformMask);
int32_t WaveletAdaptiveLLDecompress_DecompressAdaptive(const WaveletAdaptiveLL_DecodeIterator &decodeIterator, DecompressAdaptiveMode decompressAdaptiveMode);
int32_t WaveletAdaptiveLLDecompress_DecompressLossless(uint8_t *in, float *pic, int32_t sizeX, int32_t sizeY, int32_t sizeZ, int32_t allocatedSizeX, int32_t allocatedSizeXY);
void WaveletAdaptiveLLDecompress_DecompressZerosAlongX(const unsigned char* in, void* pic, int elementSize, float replaceValue, int transformSizeX, int transformSizeY, int transformSizeZ, int allocatedSizeX, int allocatedSizeY, int allocatedSizeZ, unsigned char* tempBuffer);
int32_t WaveletAdaptiveLLDecompress_CalculateBufferSizeNeeded(int maxPixels, int maxChildren);

}

#endif
