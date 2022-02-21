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

template<bool isMultiple, bool isAllNormal, int multiple, bool isPreventOverwrite>
void WaveletAdaptiveLLDecompress_DecodeAllBits(const WaveletAdaptiveLL_DecodeIterator& decodeIterator, float threshold, const int* valueEncoding, const int* valuesAtLevel, const int values, const int startDecodeBits, const int maxDecodeLevel, int threads)
{
  const float minLevelThreshold = threshold * powf(2.0f, (float)maxDecodeLevel);

  const unsigned char* stream = decodeIterator.stream;

  const int* positions = decodeIterator.pos;

  if (!isMultiple)
  {
    positions += decodeIterator.maxChildren;
  }

  float startValue = 0.0f;

  if (!decodeIterator.isInteger)
  {
    startValue = threshold * 0.5f;
  }

#pragma omp parallel for schedule(dynamic, 256) num_threads(threads)
  for (int32_t parentValue = 0; parentValue < values; parentValue++)
  {
    for (int32_t child = 0; child < multiple; child++)
    {
      int32_t value = parentValue * multiple + child;

      int32_t bit = value & 7;
      int32_t bytePos = value >> 3;

      float rvalue = startValue;

      float currentThreshold = minLevelThreshold;

      for (int32_t decodeBit = maxDecodeLevel; decodeBit <= startDecodeBits; decodeBit++)
      {
        int32_t decodeLevelOffset = valueEncoding[decodeBit];

        // we can read out bit!
        if (stream[decodeLevelOffset + bytePos] & (1 << bit))
        {
          rvalue += currentThreshold;
        }

        currentThreshold *= 2.0f;

        // is this level last one (check next level)?
        int valuesNextLevel = 0;

        if (decodeBit < startDecodeBits)
        {
          valuesNextLevel = valuesAtLevel[decodeBit + 1];
        }

        if (value >= valuesNextLevel)
        {
          int signValue = value - valuesNextLevel;

          bit = signValue & 7;
          bytePos = signValue >> 3;

          const uint8_t* signEncoding = stream + decodeLevelOffset + ((valuesAtLevel[decodeBit] + 7) >> 3);

          if (signEncoding[bytePos] & (1 << bit))
          {
            rvalue *= -1.0f;
          }
          break;
        }
      }

      if (isMultiple)
      {
        Wavelet_FastEncodeInsigAllNormal item;

        item.iterXYZ = positions[parentValue];

        if (isAllNormal)
        {
          item.iterXYZ += item.iterXYZ & decodeIterator.allNormalAndMask;
        }

        int currentPos = item.GetX() + item.GetY() * decodeIterator.sizeX + item.GetZ() * decodeIterator.sizeXY;

        if (isAllNormal)
        {
          if(!isPreventOverwrite || decodeIterator.picture[currentPos + decodeIterator.screenDisplacementAllNormal[child]] == 0.0f)
          {
            decodeIterator.picture[currentPos + decodeIterator.screenDisplacementAllNormal[child]] = rvalue;
          }
        }
        else
        {
          if(!isPreventOverwrite || decodeIterator.picture[currentPos + decodeIterator.screenDisplacement[child]] == 0.0f)
          {
            decodeIterator.picture[currentPos + decodeIterator.screenDisplacement[child]] = rvalue;
          }
        }
      }
      else
      {
        if(!isPreventOverwrite || decodeIterator.picture[positions[parentValue]] == 0.0f)
        {
          decodeIterator.picture[positions[parentValue]] = rvalue;
        }
      }
    }
  }
}

}

#endif
