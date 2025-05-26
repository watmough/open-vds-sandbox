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

#ifndef WAVELETADAPTIVELLDECOMPRESS_DECODEALLBITS_H
#define WAVELETADAPTIVELLDECOMPRESS_DECODEALLBITS_H

#include "WaveletAdaptiveLLDecompress.h"
#include "WaveletOpenMP.h"
#include <assert.h>

namespace Wavelet {

// caller example: (before)
// WaveletAdaptiveLLDecompress_DecodeAllBits<true, true, 1, false>(decodeIterator, decodeIterator.threshold, decodeIterator.valueEncodingMultiple, decodeIterator.valuesAtLevelMultiple, valuesMultiple, decodeIterator.decodeBits, decodeIterator.decompressLevel, threads);

// caller example: (after)
// WaveletAdaptiveLLDecompress_DecodeAllBits<true, true, 1, false>(decodeIterator, threads);

// original function header
// void WaveletAdaptiveLLDecompress_DecodeAllBits(const WaveletAdaptiveLL_DecodeIterator& decodeIterator, float threshold, const int* valueEncoding, const int* valuesAtLevel, const int values, const int startDecodeBits, const int maxDecodeLevel, int threads)

template<bool isMultiple, bool isAllNormal, int multiple, bool isPreventOverwrite>
void WaveletAdaptiveLLDecompress_DecodeAllBits(const WaveletAdaptiveLL_DecodeIterator& decodeIter, int threads)
{
  const float minLevelThreshold = decodeIter.threshold * powf(2.0f, (float)decodeIter.decompressLevel);

  // what would have been passed through from caller
  // int valuesMultiple = ((int*)decodeIter.stream)[decodeIter.decompressLevel];
        int  values;
  const int *valuesAtLevel{0};
  const int *valueEncoding{0};
  if constexpr (isMultiple) {
    // when isMultiple
    values = ((int*)decodeIter.stream)[decodeIter.decompressLevel];
    valuesAtLevel = decodeIter.valuesAtLevelMultiple;
    valueEncoding = decodeIter.valueEncodingMultiple;
  } else {
    // !isMultiple
    values/*Single*/ = ((int32_t*)decodeIter.stream)[decodeIter.decompressLevel + WAVELET_ADAPTIVE_LEVELS];
    assert(values != 0);
    valuesAtLevel = decodeIter.valuesAtLevelSingle;
    valueEncoding = decodeIter.valueEncodingSingle;
  }
  // info from passed decode iterator
  const unsigned char* stream = decodeIter.stream;
  const int*        positions = !isMultiple ? decodeIter.pos+decodeIter.maxChildren  : decodeIter.pos;
  const float      startValue = !decodeIter.isInteger ? decodeIter.threshold * 0.5f : 0.f;

  // iterate over 1000's sets of values
  for (int32_t parentValue = 0; parentValue < values; parentValue++)
  {
    for (int32_t child = 0; child < multiple; child++) {

      // objects scoped to this loop
      int32_t value = parentValue * multiple + child;
      int32_t bit = value & 7;
      int32_t bytePos = value >> 3;
      float rvalue = startValue;
      float currentThreshold = minLevelThreshold;

      int valuesNextLevel = 0;
      for (int32_t decodeBit = decodeIter.decompressLevel; decodeBit <= decodeIter.decodeBits; decodeBit++) {

        // bump rvalue
        if (stream[valueEncoding[decodeBit]+bytePos] & (1<<bit)) {
          rvalue += currentThreshold;
        }

        // bump threshold
        currentThreshold *= 2.0f;

        // is this level last one (check next level)?
        valuesNextLevel = (decodeBit < decodeIter.decodeBits) ? valuesAtLevel[decodeBit + 1] : 0;
        if (value >= valuesNextLevel) {

          // handle sign encoding, then break out of loop (can prob move outside loop)
          const uint8_t* signEncoding = stream + valueEncoding[decodeBit] + ((valuesAtLevel[decodeBit] + 7) >> 3);
          int signValue = value - valuesNextLevel;
          if (signEncoding[signValue>>3] & (1<<(signValue&7))) {
            rvalue *= -1.0f;
          }
          break;
        }
      }

      if (isMultiple) {

        // initialize item, and calculate position
        Wavelet_FastEncodeInsigAllNormal item;
        item.iterXYZ = positions[parentValue];
        if (isAllNormal) {
          item.iterXYZ += item.iterXYZ & decodeIter.allNormalAndMask;
        }
        int currentPos = item.GetX() + item.GetY() * decodeIter.sizeX + item.GetZ() * decodeIter.sizeXY;

        if (isAllNormal) {
          if(!isPreventOverwrite || decodeIter.picture[currentPos + decodeIter.screenDisplacementAllNormal[child]] == 0.0f) {
            decodeIter.picture[currentPos + decodeIter.screenDisplacementAllNormal[child]] = rvalue;
          }
        } else {
          // !isAllNormal
          if(!isPreventOverwrite || decodeIter.picture[currentPos + decodeIter.screenDisplacement[child]] == 0.0f) {
            decodeIter.picture[currentPos + decodeIter.screenDisplacement[child]] = rvalue;
          }
        }
      } else {
        // !isMultiple
        if(!isPreventOverwrite || decodeIter.picture[positions[parentValue]] == 0.0f) {
          decodeIter.picture[positions[parentValue]] = rvalue;
        }
      }
    }
  }
}

}

#endif
