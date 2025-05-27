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

#include "VCL2/vectorclass.h"

#include "WaveletAdaptiveLLDecompress.h"
#include <assert.h>
#include <fmt/format.h>

#define NEW_WAVELETS

#define check_vec3(x,name) Vec8f vec_##name = to_float(x);
#define check_vec2(x,name) check_vec3(x,name)
#define check_vec(x) check_vec2(x, __LINE__)
// #define check_vec(x)

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

    // fmt::print("\nparentValue {:5} ", parentValue);

    for (int32_t child = 0; child < multiple; child++) {

      // objects scoped to this loop
      int32_t value = parentValue * multiple + child;
      int32_t bit = value & 7;
      int32_t bitShifted = 1<<bit;
      int32_t bytePos = value >> 3;
      float rvalue = startValue;
      float currentThreshold = minLevelThreshold;


#ifdef NEW_WAVELETS
      // HEAVIER REFACTORING

      Vec8i vecValue = value;
      Vec8i vecBit = bitShifted;
      Vec8i vecBytePos = value >> 3;

      Vec8f vecRvalueAddends = 0;
      Vec8f vecCurrentThreshold = currentThreshold;

      int valuesNextLevel{0};
      int32_t decodeBitStart{decodeIter.decompressLevel};
      int32_t decodeBit{0};

      // load decodeBit
      Vec8ui vecDecodeBit(0,1,2,3,4,5,6,7);
      Vec8ui vecDecodeBits(decodeIter.decodeBits);
      Vec8f  vecPowersOf2(1,2,4,8,16,32,64,128);

      // calculate current threshold across bits e.g. multiple by 2^0 in stream 0
      vecCurrentThreshold *= vecPowersOf2;

      // multiple needs to be 8
      assert(multiple == 8);

      // decode bit jumps by 8 at a time
      uint32_t iterations = (decodeIter.decodeBits-decodeBitStart+7)/8+1;
      while (iterations--) {

        // calculate indexes into stream - value encoding at each decode bit + byte pos
        // load indexes from valueEncoding table upto decodeBits
        // need to restrict how many values will be loaded from encoding table
        Vec8ib vecValueEncodingInvalidIdx = ((vecDecodeBit+decodeBitStart)>=Vec8i(decodeIter.decodeBits));
check_vec(vecValueEncodingInvalidIdx)
        Vec8i vecValueEncodingIdx = select(vecValueEncodingInvalidIdx, 0,  vecDecodeBit);
check_vec(vecValueEncodingIdx)
        // get the values from valueEncoding that we'll use as a lookup into stream
        Vec8i vecValueEncodingVals = lookup<8>(vecValueEncodingIdx, &valueEncoding[decodeBitStart]);
check_vec(vecValueEncodingVals)

        // read in the stream values
        const unsigned char *streamPos = &stream[bytePos];
        Vec8i vecStreamVals(streamPos[vecValueEncodingVals[0]], streamPos[vecValueEncodingVals[1]], streamPos[vecValueEncodingVals[2]], streamPos[vecValueEncodingVals[3]],
                            streamPos[vecValueEncodingVals[4]], streamPos[vecValueEncodingVals[5]], streamPos[vecValueEncodingVals[6]], streamPos[vecValueEncodingVals[7]]);
check_vec(vecStreamVals)

        // bump rvalue by current threshold dependent on check
        Vec8i vecBumpRvalue;
        vecBumpRvalue = (vecStreamVals & vecBit)>>bit;  // selector using stream val &'d, then set to 1's
check_vec(vecBit)
check_vec(vecBumpRvalue)
        Vec8f vecRvaluePlusCurThreshold = vecCurrentThreshold * to_float(vecBumpRvalue);

        // are we done in this group of 8?
        // return true in a position if that stream will be last one
        // if (horizontal_or(vecValueEncodingInvalidIdx)) {
        //   // assert(false && "Unexpected!");
        //   break;
        // }

        // value >= value in next level
        Vec8i vecValuesAtLevel;
        vecValuesAtLevel.load(&valuesAtLevel[decodeBitStart]);
check_vec(vecValuesAtLevel)
        Vec8ib vecGtNextLevel = (Vec8i(value,value, value, value, value, value, value, value)>=vecValuesAtLevel);
check_vec(vecGtNextLevel)
        if (horizontal_or(vecGtNextLevel) || !iterations) {
          // zero out any addends to not be added
          vecRvalueAddends += select(vecGtNextLevel, 0, vecRvaluePlusCurThreshold);
          // zero out the invalid stream bits
          vecDecodeBit = select(vecGtNextLevel, 0, vecDecodeBit+1);
check_vec(vecDecodeBit)
          int32_t hmax = horizontal_max(vecDecodeBit);
          // decodeBit = decodeBitStart + ((hmax==0 && decodeBitStart>8) ? -1 : hmax);
          decodeBit = decodeBitStart + hmax - 1;
          break;
        }

        // Accumulate previous streams added to rvalue
        vecRvalueAddends += vecRvaluePlusCurThreshold;

        // handle bumping everything up by 8 at end of iteration
        decodeBitStart+=8;
        vecCurrentThreshold*=256;

        // SCALAR VERSION

        // // bump rvalue
        // uint64_t streamIdx = valueEncoding[decodeBit+decodeBitBase]+bytePos;
        // if (stream[streamIdx] & (1<<bit)) {
        //   rvalue += currentThreshold;
        // }
        // // bump threshold
        // currentThreshold *= 2.0f;
        // // is this level last one (check next level)?
        // if (decodeBit+decodeBitBase >= decodeIter.decodeBits)
        //   break;
        // // is value >= value in next level
        // valuesNextLevel = valuesAtLevel[decodeBit + decodeBitBase + 1];
        // if (value >= valuesNextLevel)
        //   break;
      }

      // VECTOR VERSION

      // sum up rvalue from addends
      rvalue = rvalue + horizontal_add(vecRvalueAddends);

      // recover the value of decodeBit + decodeBase
      // handle sign encoding, then break out of loop (can prob move outside loop)
      const uint8_t* signEncoding = stream + valueEncoding[decodeBit] + ((valuesAtLevel[decodeBit] + 7) >> 3);
      int signValue = value - valuesAtLevel[decodeBit + 1];
      if (signEncoding[signValue>>3] & (1<<(signValue&7))) {
        rvalue *= -1.0f;
      }
#endif
      // SCALAR VERSION

      // // handle sign encoding, then break out of loop (can prob move outside loop)
      // const uint8_t* signEncoding = stream + valueEncoding[decodeBit + decodeBitBase] + ((valuesAtLevel[decodeBit + decodeBitBase] + 7) >> 3);
      // int signValue = value - valuesNextLevel;
      // if (signEncoding[signValue>>3] & (1<<(signValue&7))) {
      //   rvalue *= -1.0f;
      // }

      // SLIGHT REFACTOR

      // int valuesNextLevel{0};
      // int32_t decodeBit = decodeIter.decompressLevel;
      // for (; decodeBit <= decodeIter.decodeBits; decodeBit++) {

      //   // bump rvalue
      //   if (stream[valueEncoding[decodeBit]+bytePos] & (1<<bit)) {
      //     rvalue += currentThreshold;
      //   }

      //   // bump threshold
      //   currentThreshold *= 2.0f;

      //   // is this level last one (check next level)?
      //   if (decodeBit >= decodeIter.decodeBits) 
      //     break;
        
      //   // is value >= vale in next level
      //   valuesNextLevel = valuesAtLevel[decodeBit + 1];
      //   if (value >= valuesNextLevel)
      //     break;
      // }

      // // handle sign encoding, then break out of loop (can prob move outside loop)
      // const uint8_t* signEncoding = stream + valueEncoding[decodeBit] + ((valuesAtLevel[decodeBit] + 7) >> 3);
      // int signValue = value - valuesNextLevel;
      // if (signEncoding[signValue>>3] & (1<<(signValue&7))) {
      //   rvalue *= -1.0f;
      // }

      // ORIGINAL(ISH)
#ifdef OLD_WAVELETS
      int valuesNextLevel{0};
      int32_t decodeBit{0};
      for (decodeBit = decodeIter.decompressLevel; decodeBit <= decodeIter.decodeBits; decodeBit++) {

        // bump rvalue
        int32_t streamIdx{valueEncoding[decodeBit]+bytePos};
        const unsigned char streamVal = stream[streamIdx];
        if (streamVal & (1<<bit)) {
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
#endif

      // fmt::print("bit {} rv {:10.3} ", decodeBit, rvalue);

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
    // fmt::print("\n");
}

}

#endif
