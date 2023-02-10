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

#include "WaveletAdaptiveLLDecompress.h"
#include "WaveletAdaptiveLLDecompress_DecodeAllBits.h"
#include "WaveletAdaptiveLLDecompress_DecodeTreeStructure.h"
#include "WaveletOpenMP.h"
#include "FSE/fse.h"
#include <assert.h>
#include <algorithm>
#include <math.h>
#include <string.h>

#define DECODEITERATOR_MAXDECODEBITS       256

namespace Wavelet {

static uint8_t* AssignPtrAndIncrementOffset(int nSize, uint8_t*& workBuffer)
{
  // round off buffer to 256 bytes offset
  uintptr_t round = (ADAPTIVEWAVELET_ALIGNBUFFERSIZE - (uintptr_t)workBuffer) % ADAPTIVEWAVELET_ALIGNBUFFERSIZE;

  workBuffer += round;

  uint8_t* returnBuffer = workBuffer;

  workBuffer += nSize;

  return returnBuffer;
}

static int32_t AddSubBand(bool isStreamFix, uint8_t* out, Wavelet_TransformData* transformData, int32_t transformIndex, int32_t iSector, int32_t subBandOffset[][8], bool* isAllNormal)
{
  Wavelet_Compiled_SubBandInfo* band = (Wavelet_Compiled_SubBandInfo*)out;

  bool isNormal = false;

  if (!transformData[transformIndex].isNormal)
  {
    *isAllNormal = false;
  }
  else
  {
    isNormal = true;
  }

  // check if this band is actually normal!
  if (!isNormal && transformData[transformIndex].subBandInfo[iSector].childSubBand == 1)
  {
    // any extra children?
    if (transformData[transformIndex].subBandInfo[iSector].extraChildEdge[0][0] == -1 &&
      transformData[transformIndex].subBandInfo[iSector].extraChildEdge[0][1] == -1 &&
      transformData[transformIndex].subBandInfo[iSector].extraChildEdge[0][2] == -1)
    {
      if (!isStreamFix)
      {
        // all divisible by two?
        if (!(transformData[transformIndex].subBandInfo[iSector].legalChildEdge[0][0] & 1) &&
            !(transformData[transformIndex].subBandInfo[iSector].legalChildEdge[0][1] & 1) &&
           (!(transformData[transformIndex].subBandInfo[iSector].legalChildEdge[0][2] & 1) || transformData[transformIndex].child == 4))
        {
          isNormal = true;
        }
      }
      else
      {
        // all divisible by two or no children?
        if ( (!(transformData[transformIndex].subBandInfo[iSector].legalChildEdge[0][0] & 1) || (transformData[transformIndex].childCount[0] == 1)) &&
             (!(transformData[transformIndex].subBandInfo[iSector].legalChildEdge[0][1] & 1) || (transformData[transformIndex].childCount[1] == 1)) &&
             (!(transformData[transformIndex].subBandInfo[iSector].legalChildEdge[0][2] & 1) || (transformData[transformIndex].childCount[2] == 1)))
        {
          isNormal = true;
        }
      }
    }
  }

  band->isNormal = isNormal;

  if (isNormal)
  {
    // normal all the way down?
    if (transformIndex <= 1 ||
      !subBandOffset[transformIndex - 1][iSector])
    {
      return 0;
    }

    band->normalChildSubBand = subBandOffset[transformIndex - 1][iSector];
    return 4;
  }

  band->normalChildSubBand = -1;
  band->childX = transformData[transformIndex].childCount[0];
  band->childY = transformData[transformIndex].childCount[1];
  band->childZ = transformData[transformIndex].childCount[2];

  band->childSubBand = transformData[transformIndex].subBandInfo[iSector].childSubBand;
  band->posX = transformData[transformIndex].subBandInfo[iSector].pos[0];
  band->posY = transformData[transformIndex].subBandInfo[iSector].pos[1];
  band->posZ = transformData[transformIndex].subBandInfo[iSector].pos[2];

  int size = sizeof(Wavelet_Compiled_SubBandInfo) + sizeof(Wavelet_Compiled_SubBand) * (band->childSubBand - 1);

  Wavelet_Compiled_SubBand* subBand = &band->firstSubBand;

  for (int i = 0; i < band->childSubBand; i++)
  {
    subBand[i].childPosX = transformData[transformIndex].subBandInfo[iSector].childPos[i][0];
    subBand[i].childPosY = transformData[transformIndex].subBandInfo[iSector].childPos[i][1];
    subBand[i].childPosZ = transformData[transformIndex].subBandInfo[iSector].childPos[i][2];
    subBand[i].extraChildEdgeX = transformData[transformIndex].subBandInfo[iSector].extraChildEdge[i][0];
    subBand[i].extraChildEdgeY = transformData[transformIndex].subBandInfo[iSector].extraChildEdge[i][1];
    subBand[i].extraChildEdgeZ = transformData[transformIndex].subBandInfo[iSector].extraChildEdge[i][2];
    subBand[i].legalChildEdgeX = transformData[transformIndex].subBandInfo[iSector].legalChildEdge[i][0];
    subBand[i].legalChildEdgeY = transformData[transformIndex].subBandInfo[iSector].legalChildEdge[i][1];
    subBand[i].legalChildEdgeZ = transformData[transformIndex].subBandInfo[iSector].legalChildEdge[i][2];

    int iChildSector = transformData[transformIndex].subBandInfo[iSector].childSector[i];

    if (transformIndex > 1)
    {
      subBand[i].childSubBand = subBandOffset[transformIndex - 1][iChildSector];
    }
    else
    {
      subBand[i].childSubBand = 0;
    }
  }

  return size;
}

static int32_t CompileTransformData(bool isStreamFix, uint8_t* compiledTransformData, int32_t* firstSubBand, const Wavelet_PixelSetChildren* pixelSetChildren, int32_t pixelSetChildrenCount, Wavelet_TransformData* transformData, int32_t transformDataCount, int32_t* transformMask, bool* isAllNormal)
{
  int subBand[32][8];

  // must offset by ! 0 so that 0 means no band!
  int writePos = 4;

  for (int iTransform = 1; iTransform < transformDataCount; iTransform++)
  {
    // go through sectors
    for (int iSector = 1; iSector < 8; iSector++)
    {
      if ((iSector & transformMask[iTransform]) == iSector)
      {
        int size = AddSubBand(isStreamFix, compiledTransformData + writePos, transformData, iTransform, iSector, subBand, isAllNormal);

        if (size == 0)
        {
          subBand[iTransform][iSector] = 0;
        }
        else
        {
          subBand[iTransform][iSector] = writePos;
          writePos += size;
        }
      }
    }
  }

  int iLastSector = 0;

  for (int i = 0; i < pixelSetChildrenCount; i++)
  {
    if (pixelSetChildren[i].subBand != iLastSector)
    {
      iLastSector = pixelSetChildren[i].subBand;

      int iteration = pixelSetChildren[i].transformIteration;

      assert(iteration == transformDataCount - 1);

      firstSubBand[iLastSector] = subBand[iteration][iLastSector];
    }
  }


  // all normal?
  if (writePos == 4)
  {
    assert(*isAllNormal);
    writePos = 0;
  }

  return writePos;
}

WaveletAdaptiveLL_DecodeIterator WaveletAdaptiveLLDecompress_CreateDecodeIterator(int dataVersion, uint8_t* stream, float* picture, int dimensions, int sizeX, int sizeY, int sizeZ,
  const float threshold, const float startThreshold, int* transformMask, Wavelet_TransformData* transformData, int transformDataCount,
  Wavelet_PixelSetChildren* pixelSetChildren, int pixelSetChildrenCount, Wavelet_PixelSetPixel* pixelSetPixelInSignificant, int pixelSetPixelInsignificantCount,
  float *maxChildrenBuffer, float *maxChildrenChildrenBuffer, int maxSizeX, int maxSizeXY, uint8_t* &tempBuffer, int maxChildren, int maxPixels, int decompressLevel, bool isInteger)
{
  int sizeXY = sizeX * sizeY;

  WaveletAdaptiveLL_DecodeIterator decodeIterator;

  decodeIterator.dataVersion = dataVersion;
  decodeIterator.isInteger = isInteger;
  decodeIterator.sizeX = sizeX;
  decodeIterator.sizeY = sizeY;
  decodeIterator.sizeZ = sizeZ;
  decodeIterator.sizeXY = sizeXY;
  decodeIterator.maxSizeX = maxSizeX;
  decodeIterator.maxSizeXY = maxSizeXY;
  decodeIterator.maxChildren = maxChildren;

  // Allocate buffers
  decodeIterator.pixelSetPixelInSignificant = (Wavelet_PixelSetPixel*)AssignPtrAndIncrementOffset(WAVELET_MAX_PIXELSETPIXEL_SIZE, tempBuffer);
  decodeIterator.pixelSetChildren = (Wavelet_PixelSetChildren*)AssignPtrAndIncrementOffset(WAVELET_MAX_PIXELSETCHILDREN_SIZE, tempBuffer);

  decodeIterator.valueEncodingMultiple = (int32_t*)AssignPtrAndIncrementOffset(DECODEITERATOR_MAXDECODEBITS * sizeof(int32_t), tempBuffer);
  decodeIterator.valuesAtLevelMultiple = (int32_t*)AssignPtrAndIncrementOffset(DECODEITERATOR_MAXDECODEBITS * sizeof(int32_t), tempBuffer);
  decodeIterator.valueEncodingSingle = (int32_t*)AssignPtrAndIncrementOffset(DECODEITERATOR_MAXDECODEBITS * sizeof(int32_t), tempBuffer);
  decodeIterator.valuesAtLevelSingle = (int32_t*)AssignPtrAndIncrementOffset(DECODEITERATOR_MAXDECODEBITS * sizeof(int32_t), tempBuffer);

  decodeIterator.insig = (Wavelet_FastEncodeInsig*)AssignPtrAndIncrementOffset(maxChildren * sizeof(Wavelet_FastEncodeInsig), tempBuffer);
  decodeIterator.sig = (Wavelet_FastEncodeInsig*)AssignPtrAndIncrementOffset(maxChildren * sizeof(Wavelet_FastEncodeInsig), tempBuffer);
  decodeIterator.pos = (int32_t*)AssignPtrAndIncrementOffset((maxPixels + maxChildren) * sizeof(int32_t), tempBuffer);
  decodeIterator.compiledTransformData = (uint8_t*)AssignPtrAndIncrementOffset(32768, tempBuffer);

  decodeIterator.stream = stream;
  
  decodeIterator.maxChildrenBuffer = maxChildrenBuffer;
  decodeIterator.maxChildrenChildrenBuffer = maxChildrenChildrenBuffer;
  
  decodeIterator.picture = picture;
  // Init precalc data for tree traversal
  for (int iTransform = 0; iTransform < transformDataCount; iTransform++)
  {
    decodeIterator.transformMask[iTransform] = (uint8_t)transformMask[iTransform];
  }

  for (int iTransform = 0; iTransform < transformDataCount; iTransform++)
  {
    int child = 1;

    if (transformMask[iTransform] & 1) child *= 2;
    if (transformMask[iTransform] & 2) child *= 2;
    if (transformMask[iTransform] & 4) child *= 2;

    decodeIterator.children[iTransform] = child;
  }

  int secondTransformMask = (transformDataCount > 1) ? decodeIterator.transformMask[1] : 0;

  int displacement = 0;


  int andMask = 0;

  if (secondTransformMask & 1) andMask |= WAVELET_ADAPTIVELL_XYZ_AND_MASK << WAVELET_ADAPTIVELL_X_SHIFT;
  if (secondTransformMask & 2) andMask |= WAVELET_ADAPTIVELL_XYZ_AND_MASK << WAVELET_ADAPTIVELL_Y_SHIFT;
  if (secondTransformMask & 4) andMask |= WAVELET_ADAPTIVELL_XYZ_AND_MASK << WAVELET_ADAPTIVELL_Z_SHIFT;

  decodeIterator.allNormalAndMask = andMask;

  for (int iZ = 0; iZ < 2; iZ++)
  {
    for (int iY = 0; iY < 2; iY++)
    {
      for (int iX = 0; iX < 2; iX++)
      {
        int count = iX + iY * 2 + iZ * 4;

        decodeIterator.screenDisplacement[count] = iX + iY * decodeIterator.sizeX + iZ * decodeIterator.sizeXY;

        if ((count & secondTransformMask) == count)
        {
          decodeIterator.screenDisplacementAllNormal[displacement] = iX + iY * decodeIterator.sizeX + iZ * decodeIterator.sizeXY;
          decodeIterator.childDisplacementAllNormal[displacement] = (iX << WAVELET_ADAPTIVELL_X_SHIFT) +
            (iY << WAVELET_ADAPTIVELL_Y_SHIFT) +
            (iZ << WAVELET_ADAPTIVELL_Z_SHIFT);

          displacement++;
        }
      }
    }
  }

  // copy data into potentitally host allocated buffers, so can be used in stream later
  memcpy(decodeIterator.pixelSetChildren, pixelSetChildren, sizeof(Wavelet_PixelSetChildren) * pixelSetChildrenCount);
  memcpy(decodeIterator.pixelSetPixelInSignificant, pixelSetPixelInSignificant, sizeof(Wavelet_PixelSetPixel) * pixelSetPixelInsignificantCount);
  decodeIterator.pixelSetChildrenCount = pixelSetChildrenCount;
  decodeIterator.pixelSetPixelInsignificantCount = pixelSetPixelInsignificantCount;

  // Find if traversal off bit tree is "AllNormal" -> no special rules, just doubling of positions,
  // And compile tree traversal data based on wavelet tranfsform for this item
  bool isAllNormal = true;

  bool
    isStreamFix = decodeIterator.dataVersion >= WAVELET_DATA_VERSION_1_6 ? true : false;

  int compiledTransformData = CompileTransformData(isStreamFix, (uint8_t*)decodeIterator.compiledTransformData, decodeIterator.firstSubBand, pixelSetChildren, pixelSetChildrenCount, transformData, transformDataCount, transformMask, &isAllNormal);
  (void) compiledTransformData;


  // Write where we find initial uncompressed values (Lowest Band)
  decodeIterator.streamFirstValues = (float*)(decodeIterator.stream + WAVELET_ADAPTIVE_LEVELS * sizeof(float));

  // if !isAllNormal, we have more ADAPTIVE Level results we need to store, so where we find first uncompressed values is displaced
  if (!isAllNormal)
  {
    decodeIterator.streamFirstValues += WAVELET_ADAPTIVE_LEVELS;
  }

  // find number of bits we are to decode
  int decodeBits = 0;

  float rT = startThreshold;

  while (rT >= threshold)
  {
    rT *= 0.5f;
    decodeBits++;
  }

  // make 0 based
  decodeBits--;

  assert(decodeBits >= 0);
  assert(decodeBits < DECODEITERATOR_MAXDECODEBITS);

  assert(decompressLevel < WAVELET_ADAPTIVE_LEVELS);

  // Write these results into decode iterator
  decodeIterator.decodeBits = decodeBits;
  decodeIterator.decompressLevel = decompressLevel;
  decodeIterator.dimensions = dimensions;
  decodeIterator.isAllNormal = isAllNormal;
  decodeIterator.threshold = threshold;
  decodeIterator.startThreshold = startThreshold;
  decodeIterator.multiple = isAllNormal ? decodeIterator.children[1] : 1 << dimensions;

  return decodeIterator;
}

static inline void ReadStartValuesKernel(const int value, float* picture, const int sizeX, const int sizeXY, const Wavelet_PixelSetPixel* pixelSetPixelInSignificant, int pixelSetPixelInsignificantCount, const Wavelet_PixelSetChildren* pixelSetChildren, int pixelSetChildrenCount, float* rw)
{
  int pos;

  if (value >= pixelSetPixelInsignificantCount)
  {
    int read = value - pixelSetPixelInsignificantCount;
    pos = pixelSetChildren[read].X + pixelSetChildren[read].Y * sizeX + pixelSetChildren[read].Z * sizeXY;
  }
  else
  {
    pos = pixelSetPixelInSignificant[value].X + pixelSetPixelInSignificant[value].Y * sizeX + pixelSetPixelInSignificant[value].Z * sizeXY;
  }

  picture[pos] = rw[value];
}

static inline void ReadStartValues(const WaveletAdaptiveLL_DecodeIterator& decodeIterator, const Wavelet_PixelSetPixel* pixelSetPixelInSignificant, int pixelSetPixelInsignificantCount, const Wavelet_PixelSetChildren* pixelSetChildren, int pixelSetChildrenCount)
{
  int totalValue = pixelSetPixelInsignificantCount + pixelSetChildrenCount;

  for (int value = 0; value < totalValue; value++)
  {
    ReadStartValuesKernel(value, decodeIterator.picture, decodeIterator.sizeX, decodeIterator.sizeXY, pixelSetPixelInSignificant, pixelSetPixelInsignificantCount, pixelSetChildren, pixelSetChildrenCount, decodeIterator.streamFirstValues);
  }
}

int32_t WaveletAdaptiveLLDecompress_DecompressAdaptive(const WaveletAdaptiveLL_DecodeIterator &decodeIterator, DecompressAdaptiveMode decompressAdaptiveMode)
{
  if (decodeIterator.dataVersion >= WAVELET_DATA_VERSION_1_6)
  {
    assert(decompressAdaptiveMode == DecompressAdaptiveMode::AssumeNoOverwrite);
  }

  int streamSize = 0;

  if (decodeIterator.isAllNormal)
  {
    if (decodeIterator.dimensions == 1)
    {
      streamSize = WaveletAdaptiveLLDecompress_DecodeTreeStructure<true, 1>(decodeIterator);
    }
    else if (decodeIterator.dimensions == 2)
    {
      streamSize = WaveletAdaptiveLLDecompress_DecodeTreeStructure<true, 2>(decodeIterator);
    }
    else //if (decodeIterator.m_dimensions == 3)
    {
      streamSize = WaveletAdaptiveLLDecompress_DecodeTreeStructure<true, 3>(decodeIterator);
    }
  }
  else
  {
    if (decodeIterator.dimensions == 1)
    {
      streamSize = WaveletAdaptiveLLDecompress_DecodeTreeStructure<false, 1>(decodeIterator);
    }
    else if (decodeIterator.dimensions == 2)
    {
      streamSize = WaveletAdaptiveLLDecompress_DecodeTreeStructure<false, 2>(decodeIterator);
    }
    else //if (decodeIterator.m_dimensions == 3)
    {
      streamSize = WaveletAdaptiveLLDecompress_DecodeTreeStructure<false, 3>(decodeIterator);
    }
  }

  memset(decodeIterator.picture, 0, size_t(decodeIterator.sizeX) * size_t(decodeIterator.sizeY) * size_t(decodeIterator.sizeZ) * sizeof(float));

  ReadStartValues(decodeIterator, decodeIterator.pixelSetPixelInSignificant, decodeIterator.pixelSetPixelInsignificantCount, decodeIterator.pixelSetChildren, decodeIterator.pixelSetChildrenCount);

  // The limit value 8 seems to provide good performance across
  // different hardware configurations. This OMP block originally
  // had no thread limit, which resulted in poor performance on
  // some hardware.
  // Alternatively, we want to use fewer than 8 threads if OMP
  // max threads is less than that.
  int threads = (decompressAdaptiveMode == DecompressAdaptiveMode::AssumeNoOverwrite) ? std::min(8, omp_get_max_threads()) : 1;

  if (decodeIterator.decompressLevel <= decodeIterator.decodeBits)
  {
    int multiple = decodeIterator.isAllNormal ? decodeIterator.children[1] : 1 << decodeIterator.dimensions;

    int valuesMultiple = ((int*)decodeIterator.stream)[decodeIterator.decompressLevel];

    if (valuesMultiple)
    {
      if (decodeIterator.isAllNormal)
      {
        if(decompressAdaptiveMode != DecompressAdaptiveMode::PreventOverwrite)
        {
          if (multiple == 1)      WaveletAdaptiveLLDecompress_DecodeAllBits<true, true, 1, false>(decodeIterator, decodeIterator.threshold, decodeIterator.valueEncodingMultiple, decodeIterator.valuesAtLevelMultiple, valuesMultiple, decodeIterator.decodeBits, decodeIterator.decompressLevel, threads);
          else if (multiple == 2) WaveletAdaptiveLLDecompress_DecodeAllBits<true, true, 2, false>(decodeIterator, decodeIterator.threshold, decodeIterator.valueEncodingMultiple, decodeIterator.valuesAtLevelMultiple, valuesMultiple, decodeIterator.decodeBits, decodeIterator.decompressLevel, threads);
          else if (multiple == 4) WaveletAdaptiveLLDecompress_DecodeAllBits<true, true, 4, false>(decodeIterator, decodeIterator.threshold, decodeIterator.valueEncodingMultiple, decodeIterator.valuesAtLevelMultiple, valuesMultiple, decodeIterator.decodeBits, decodeIterator.decompressLevel, threads);
          else if (multiple == 8) WaveletAdaptiveLLDecompress_DecodeAllBits<true, true, 8, false>(decodeIterator, decodeIterator.threshold, decodeIterator.valueEncodingMultiple, decodeIterator.valuesAtLevelMultiple, valuesMultiple, decodeIterator.decodeBits, decodeIterator.decompressLevel, threads);
        }
        else
        {
          if (multiple == 1)      WaveletAdaptiveLLDecompress_DecodeAllBits<true, true, 1, true>(decodeIterator, decodeIterator.threshold, decodeIterator.valueEncodingMultiple, decodeIterator.valuesAtLevelMultiple, valuesMultiple, decodeIterator.decodeBits, decodeIterator.decompressLevel, threads);
          else if (multiple == 2) WaveletAdaptiveLLDecompress_DecodeAllBits<true, true, 2, true>(decodeIterator, decodeIterator.threshold, decodeIterator.valueEncodingMultiple, decodeIterator.valuesAtLevelMultiple, valuesMultiple, decodeIterator.decodeBits, decodeIterator.decompressLevel, threads);
          else if (multiple == 4) WaveletAdaptiveLLDecompress_DecodeAllBits<true, true, 4, true>(decodeIterator, decodeIterator.threshold, decodeIterator.valueEncodingMultiple, decodeIterator.valuesAtLevelMultiple, valuesMultiple, decodeIterator.decodeBits, decodeIterator.decompressLevel, threads);
          else if (multiple == 8) WaveletAdaptiveLLDecompress_DecodeAllBits<true, true, 8, true>(decodeIterator, decodeIterator.threshold, decodeIterator.valueEncodingMultiple, decodeIterator.valuesAtLevelMultiple, valuesMultiple, decodeIterator.decodeBits, decodeIterator.decompressLevel, threads);
        }
      }
      else
      {
        if(decompressAdaptiveMode != DecompressAdaptiveMode::PreventOverwrite)
        {
          // multiple can't be one if using 1 << nDimensions
          if (multiple == 2)      WaveletAdaptiveLLDecompress_DecodeAllBits<true, false, 2, false>(decodeIterator, decodeIterator.threshold, decodeIterator.valueEncodingMultiple, decodeIterator.valuesAtLevelMultiple, valuesMultiple, decodeIterator.decodeBits, decodeIterator.decompressLevel, threads);
          else if (multiple == 4) WaveletAdaptiveLLDecompress_DecodeAllBits<true, false, 4, false>(decodeIterator, decodeIterator.threshold, decodeIterator.valueEncodingMultiple, decodeIterator.valuesAtLevelMultiple, valuesMultiple, decodeIterator.decodeBits, decodeIterator.decompressLevel, threads);
          else if (multiple == 8) WaveletAdaptiveLLDecompress_DecodeAllBits<true, false, 8, false>(decodeIterator, decodeIterator.threshold, decodeIterator.valueEncodingMultiple, decodeIterator.valuesAtLevelMultiple, valuesMultiple, decodeIterator.decodeBits, decodeIterator.decompressLevel, threads);
        }
        else
        {
          // multiple can't be one if using 1 << nDimensions
          if (multiple == 2)      WaveletAdaptiveLLDecompress_DecodeAllBits<true, false, 2, true>(decodeIterator, decodeIterator.threshold, decodeIterator.valueEncodingMultiple, decodeIterator.valuesAtLevelMultiple, valuesMultiple, decodeIterator.decodeBits, decodeIterator.decompressLevel, threads);
          else if (multiple == 4) WaveletAdaptiveLLDecompress_DecodeAllBits<true, false, 4, true>(decodeIterator, decodeIterator.threshold, decodeIterator.valueEncodingMultiple, decodeIterator.valuesAtLevelMultiple, valuesMultiple, decodeIterator.decodeBits, decodeIterator.decompressLevel, threads);
          else if (multiple == 8) WaveletAdaptiveLLDecompress_DecodeAllBits<true, false, 8, true>(decodeIterator, decodeIterator.threshold, decodeIterator.valueEncodingMultiple, decodeIterator.valuesAtLevelMultiple, valuesMultiple, decodeIterator.decodeBits, decodeIterator.decompressLevel, threads);
        }
      }
    }

    if (!decodeIterator.isAllNormal)
    {
      int valuesSingle = ((int32_t*)decodeIterator.stream)[decodeIterator.decompressLevel + WAVELET_ADAPTIVE_LEVELS];

      if (valuesSingle)
      {
        if(decompressAdaptiveMode != DecompressAdaptiveMode::PreventOverwrite)
        {
          WaveletAdaptiveLLDecompress_DecodeAllBits<false, false, 1, false>(decodeIterator, decodeIterator.threshold, decodeIterator.valueEncodingSingle, decodeIterator.valuesAtLevelSingle, valuesSingle, decodeIterator.decodeBits, decodeIterator.decompressLevel, threads);
        }
        else
        {
          WaveletAdaptiveLLDecompress_DecodeAllBits<false, false, 1, true>(decodeIterator, decodeIterator.threshold, decodeIterator.valueEncodingSingle, decodeIterator.valuesAtLevelSingle, valuesSingle, decodeIterator.decodeBits, decodeIterator.decompressLevel, threads);
        }
      }
    }
  }
  return streamSize;
}

#define ADAPTIVEWAVELET_LOSSLESS_CHANNEL_ZERO            0
#define ADAPTIVEWAVELET_LOSSLESS_CHANNEL_CONSTANT        1
#define ADAPTIVEWAVELET_LOSSLESS_CHANNEL_UNCOMPRESSED    -1
int32_t WaveletAdaptiveLLDecompress_DecompressLossless(uint8_t *in, float *pic, int32_t sizeX, int32_t sizeY, int32_t sizeZ, int32_t allocatedSizeX, int32_t allocatedSizeXY)
{
  unsigned char *start = in;

  unsigned char *count[4];

  int
    nPixels = sizeX * sizeY * sizeZ;

  count[0] = (unsigned char *)malloc(size_t(sizeX) * (sizeY) * (sizeZ) * sizeof(unsigned char));
  count[1] = (unsigned char *)malloc(size_t(sizeX) * (sizeY) * (sizeZ) * sizeof(unsigned char));
  count[2] = (unsigned char *)malloc(size_t(sizeX) * (sizeY) * (sizeZ) * sizeof(unsigned char));
  count[3] = (unsigned char *)malloc(size_t(sizeX) * (sizeY) * (sizeZ) * sizeof(unsigned char));

  uint8_t *compressedData[4];


  in += sizeof(int);

  int *readSize = (int *)in;

  in += sizeof(int) * 4;

  int totalSize = 0;

  for (int i = 0; i < 4; i++)
  {
    compressedData[i] = in + totalSize;
    int size = readSize[i];

    if (size == ADAPTIVEWAVELET_LOSSLESS_CHANNEL_UNCOMPRESSED)
    {
      size = nPixels;
    }

    totalSize += size;
  }

  const int threadCount = Wavelet_GetEffectiveOpenMPThreadCount(WAVELET_OPENMP_SSE_THREAD_COUNT);
  (void) threadCount;
#if defined(_OPENMP)
#pragma omp parallel for num_threads(threadCount) schedule(static)
#endif
  for (int i = 0; i < 4; i++)
  {
    switch (readSize[i])
    {
    case ADAPTIVEWAVELET_LOSSLESS_CHANNEL_ZERO:
      memset(count[i], 0, nPixels);
      break;
    case ADAPTIVEWAVELET_LOSSLESS_CHANNEL_CONSTANT:
      memset(count[i], compressedData[i][0], nPixels);
      break;
    case ADAPTIVEWAVELET_LOSSLESS_CHANNEL_UNCOMPRESSED:
      memcpy(count[i], compressedData[i], nPixels);
      break;
    default:
      assert(readSize[i] > 1);
      FSE_decompress(count[i], nPixels, compressedData[i], readSize[i]);
      break;
    }
  }
  
  int *intPic = (int *)pic;

#if defined(_OPENMP)
#pragma omp parallel for if(sizeZ > 1) num_threads(threadCount) schedule(static)
#endif
  for (int iZ = 0; iZ < sizeZ; iZ++)
  {
#if defined(_OPENMP)
#pragma omp parallel for if(sizeZ == 1) num_threads(threadCount) schedule(static)
#endif
    for (int iY = 0; iY < sizeY; iY++)
    {
      int
        iWrite = iY * sizeX + iZ * sizeX * sizeY;

      for (int iX = 0; iX < sizeX; iX++)
      {
        int
          iPixel = iX + iY * allocatedSizeX + iZ * allocatedSizeXY;

        int
          uValue = count[0][iWrite] |
                  (count[1][iWrite] << 8) |
                  (count[2][iWrite] << 16) |
                  (count[3][iWrite] << 24);

        iWrite++;

        intPic[iPixel] ^= uValue;
      }
    }
  }
  
  free(count[0]);
  free(count[1]);
  free(count[2]);
  free(count[3]);

  return (int)(in + totalSize - start);
}

static bool CheckIfWaveletSectorHasStreamBug(Wavelet_TransformData * transformData, int transformIndex, int iSector)
{
  if (transformData[transformIndex].isNormal)
  {
    return false;
  }

  // check if this band is actually normal!
  if (transformData[transformIndex].subBandInfo[iSector].childSubBand == 1)
  {
    // any extra children?
    if (transformData[transformIndex].subBandInfo[iSector].extraChildEdge[0][0] == -1 &&
      transformData[transformIndex].subBandInfo[iSector].extraChildEdge[0][1] == -1 &&
      transformData[transformIndex].subBandInfo[iSector].extraChildEdge[0][2] == -1)
    {
      // all divisible by two?
      if (!(transformData[transformIndex].subBandInfo[iSector].legalChildEdge[0][0] & 1) &&
        !(transformData[transformIndex].subBandInfo[iSector].legalChildEdge[0][1] & 1) &&
        (!(transformData[transformIndex].subBandInfo[iSector].legalChildEdge[0][2] & 1) || transformData[transformIndex].child == 4))
      {
        // Are we in the case of 4 children
        if (transformData[transformIndex].child == 4)
        {
          // Are we odd numbered
          if (transformData[transformIndex].subBandInfo[iSector].legalChildEdge[0][2] & 1)
          {
            // And do we have children
            if (transformData[transformIndex].childCount[2] > 1)
            {
              return true;
            }
          }
        }
      }
    }
  }

  return false;
}

bool WaveletAdaptiveLL_IsWaveletStreamEncodedWithBug(Wavelet_TransformData *transformData, int transformDataCount, int *transformMask)
{
  for (int transformIndex = 1; transformIndex < transformDataCount; transformIndex++)
  {
    // go through sectors
    for (int iSector = 1; iSector < 8; iSector++)
    {
      if ((iSector & transformMask[transformIndex]) == iSector)
      {
        if (CheckIfWaveletSectorHasStreamBug(transformData, transformIndex, iSector))
        {
          return true;
        }
      }
    }
  }

  return false;
}

template <class T, bool isHigh>
static void WaveletAdaptiveLL_ReplaceZeroFromZeroCount(T * pxPic, int nTransformSizeY, int nTransformSizeZ, int nAllocatedSizeX, int nAllocatedSizeY, const unsigned char *puCountLow, const unsigned char *puCountHigh, T replaceValue)
{
  const int threadCount = Wavelet_GetEffectiveOpenMPThreadCount(WAVELET_OPENMP_SSE_THREAD_COUNT);
  (void) threadCount;
#if defined(_OPENMP)
#pragma omp parallel for if(nTransformSizeZ > 1) num_threads(threadCount) schedule(static)
#endif
  for (int iZ=0; iZ<nTransformSizeZ;iZ++)
  {
#if defined(_OPENMP)
#pragma omp parallel for if(nTransformSizeZ == 1) num_threads(threadCount) schedule(static)
#endif
    for (int iY=0; iY<nTransformSizeY;iY++)
    {
      T *ptRead = pxPic + iY * nAllocatedSizeX + iZ * nAllocatedSizeX * nAllocatedSizeY;

      uint16_t uCount = puCountLow[iY + iZ * nTransformSizeY];
      
      if (isHigh)
      {
        uCount |= (puCountHigh[iY + iZ * nTransformSizeY] << 8);
      }

      for (int i=0;i<uCount;i++)
      {
        ptRead[i] = replaceValue;
      }
    }
  }
}

// This function decompresses how many zeros along X from X0 - using FSE to compress counts (
void WaveletAdaptiveLLDecompress_DecompressZerosAlongX(const unsigned char *in, void *pic, int elementSize, float replaceValue, int transformSizeX, int transformSizeY, int transformSizeZ, int allocatedSizeX, int allocatedSizeY, int allocatedSizeZ, unsigned char* tempBuffer)
{
  int totalSize = *((int *)in);
  
  in +=4;

  assert(totalSize >= 4);

  // No zero runs in data, return immediately and do nothing!!
  if (totalSize == 4)
  {
    return;
  }
  
  bool isHigh = transformSizeX >= 256;

  int transformSizeYZ = transformSizeY * transformSizeZ;

  uint8_t* countLow = tempBuffer;
  uint8_t *countHigh = tempBuffer + transformSizeYZ;

  int *readSize = (int *)in;

  in +=4;
 
  int size = *readSize++;
  
  if (size == 0)
  {
    int value = *readSize++;
    memset(countLow, value, transformSizeYZ);
    in += 4;
  }
  else if (size < 0)
  {
    memcpy(countLow, in, transformSizeYZ);
    in += transformSizeYZ;
    assert(-size == transformSizeYZ);
  }
  else
  {
    FSE_decompress(countLow, transformSizeYZ, in, size);
    in+=size;
  }

  if (isHigh)
  {
    readSize = (int *)in;
    in +=4;

    size = *readSize++;
  
    if (size == 0)
    {
      int value = *readSize++;
      memset(countHigh, value, transformSizeYZ);
      in += 4;
    }
    else if (size < 0)
    {
      memcpy(countHigh, in, transformSizeYZ);
      in += transformSizeYZ;
      assert(-size == transformSizeYZ);
    }
    else
    {
      FSE_decompress(countHigh, transformSizeYZ, in, size);
      in+=size;
    }
  }

  if (elementSize == 1)
  {
    if (isHigh) WaveletAdaptiveLL_ReplaceZeroFromZeroCount<uint8_t, true>((uint8_t*)pic, transformSizeY, transformSizeZ, allocatedSizeX, allocatedSizeY, countLow, countHigh, (uint8_t)replaceValue);
    else        WaveletAdaptiveLL_ReplaceZeroFromZeroCount<uint8_t, false>((uint8_t*)pic, transformSizeY, transformSizeZ, allocatedSizeX, allocatedSizeY, countLow, countHigh, (uint8_t)replaceValue);
  }
  else if (elementSize == 2)
  {
    if (isHigh) WaveletAdaptiveLL_ReplaceZeroFromZeroCount<uint16_t, true>((uint16_t*)pic, transformSizeY, transformSizeZ, allocatedSizeX, allocatedSizeY, countLow, countHigh, (uint16_t)replaceValue);
    else        WaveletAdaptiveLL_ReplaceZeroFromZeroCount<uint16_t, false>((uint16_t*)pic, transformSizeY, transformSizeZ, allocatedSizeX, allocatedSizeY, countLow, countHigh, (uint16_t)replaceValue);
  }
  else
  {
    assert(elementSize == 4);
    if (isHigh) WaveletAdaptiveLL_ReplaceZeroFromZeroCount<float, true>((float*)pic, transformSizeY, transformSizeZ, allocatedSizeX, allocatedSizeY, countLow, countHigh, replaceValue);
    else        WaveletAdaptiveLL_ReplaceZeroFromZeroCount<float, false>((float*)pic, transformSizeY, transformSizeZ, allocatedSizeX, allocatedSizeY, countLow, countHigh, replaceValue);
  }
}

int32_t  WaveletAdaptiveLLDecompress_CalculateBufferSizeNeeded(int maxPixels, int maxChildren)
{
  int
    nSize = 0;

  nSize += WAVELET_MAX_PIXELSETPIXEL_SIZE + ADAPTIVEWAVELET_ALIGNBUFFERSIZE;
  nSize += WAVELET_MAX_PIXELSETCHILDREN_SIZE + ADAPTIVEWAVELET_ALIGNBUFFERSIZE;

  nSize += DECODEITERATOR_MAXDECODEBITS * sizeof(int) + ADAPTIVEWAVELET_ALIGNBUFFERSIZE; // aiValueEncoding
  nSize += DECODEITERATOR_MAXDECODEBITS * sizeof(int) + ADAPTIVEWAVELET_ALIGNBUFFERSIZE; // anValuesAtLevel
  nSize += DECODEITERATOR_MAXDECODEBITS * sizeof(int) + ADAPTIVEWAVELET_ALIGNBUFFERSIZE; // aiValueEncodingSingle
  nSize += DECODEITERATOR_MAXDECODEBITS * sizeof(int) + ADAPTIVEWAVELET_ALIGNBUFFERSIZE; // anValuesAtLevelSingle
  nSize += maxChildren * sizeof(Wavelet_FastEncodeInsig) + ADAPTIVEWAVELET_ALIGNBUFFERSIZE; // Insignificants + align to 256
  nSize += maxChildren * sizeof(Wavelet_FastEncodeInsig) + ADAPTIVEWAVELET_ALIGNBUFFERSIZE; // Significants + align to 256
  nSize += (maxPixels + maxChildren) * sizeof(int) + ADAPTIVEWAVELET_ALIGNBUFFERSIZE; // positions + align to 256
  nSize += 32768 + ADAPTIVEWAVELET_ALIGNBUFFERSIZE; // max sizeof transform hierarchical helper data

  return nSize;
}

}
