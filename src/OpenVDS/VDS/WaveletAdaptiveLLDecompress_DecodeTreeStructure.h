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

#ifndef WAVELETADAPTIVELLDECOMPRESS_DECODETREESTRUCTURE_H
#define WAVELETADAPTIVELLDECOMPRESS_DECODETREESTRUCTURE_H

#include "WaveletTypes.h"
#include "WaveletOpenMP.h"
#include <assert.h>
#include <math.h>
#include <string.h>

namespace Wavelet {

template<bool isAllNormal>
static inline void WaveletAdaptiveLL_InitializeInsignificantsKernel(int iElement, const WaveletAdaptiveLL_DecodeIterator& decodeIterator, const Wavelet_PixelSetChildren* pixelSetChildren)
{
  if (isAllNormal)
  {
    Wavelet_FastEncodeInsigAllNormal inSigChild(pixelSetChildren[iElement].X, pixelSetChildren[iElement].Y, pixelSetChildren[iElement].Z, pixelSetChildren[iElement].transformIteration);

    ((Wavelet_FastEncodeInsigAllNormal*)decodeIterator.insig)[iElement] = inSigChild;
  }
  else
  {
    Wavelet_FastEncodeInsig inSigChild(pixelSetChildren[iElement].X, pixelSetChildren[iElement].Y, pixelSetChildren[iElement].Z, pixelSetChildren[iElement].transformIteration, decodeIterator.firstSubBand[pixelSetChildren[iElement].subBand]);
    decodeIterator.insig[iElement] = inSigChild;
  }
}

template<bool isAllNormal>
static void WaveletAdaptiveLL_InitializeInsignificants(const WaveletAdaptiveLL_DecodeIterator& decodeIterator, const Wavelet_PixelSetChildren* pixelSetChildren, int pixelSetChildrenCount)
{
  for (int iElement = 0; iElement < pixelSetChildrenCount; iElement++)
  {
    WaveletAdaptiveLL_InitializeInsignificantsKernel<isAllNormal>(iElement, decodeIterator, pixelSetChildren);
  }
}

template <bool isInsigPass>
static void WaveletAdaptiveLLDecompress_EvalAndSplitAllNormal(const WaveletAdaptiveLL_DecodeIterator& decodeIterator, uint8_t* puBitStream, int32_t& iCurrentElement, int32_t& nElement, const float rCurrentThreshold, int32_t& nValuesOut, int32_t& nChildrenOut)
{
  assert(iCurrentElement < nElement);

  Wavelet_FastEncodeInsigAllNormal* outputItems = isInsigPass ? (Wavelet_FastEncodeInsigAllNormal*)decodeIterator.sig : (Wavelet_FastEncodeInsigAllNormal*)decodeIterator.insig;

  const Wavelet_FastEncodeInsigAllNormal* inputItems = isInsigPass ? (Wavelet_FastEncodeInsigAllNormal*)decodeIterator.insig : (Wavelet_FastEncodeInsigAllNormal*)decodeIterator.sig;

  const int32_t child = decodeIterator.children[1];

  int32_t* outputPos = decodeIterator.pos;

  int32_t elementsThisRun = nElement - iCurrentElement;

  int32_t writeOut = iCurrentElement;

  // Originally 32. 4 is not ideal for all data sets or all hardware configurations,
  // but it is on average more performant than 32.
#define TEST_THREADS 4

  Wavelet_FastEncodeInsigAllNormal* tempOutputItem[TEST_THREADS];
  Wavelet_FastEncodeInsigAllNormal* tempInputItem[TEST_THREADS];

  int32_t* tempOutputPos[TEST_THREADS];

  int32_t tempOutputItemIndices[TEST_THREADS];
  int32_t tempInputItemIndices[TEST_THREADS];
  int32_t tempOutputPosIndices[TEST_THREADS];

  int32_t threads = elementsThisRun / 1024 + 1;
  int32_t maxThreads = omp_get_max_threads();


  if (threads > TEST_THREADS) threads = TEST_THREADS;
  if (threads > maxThreads) threads = maxThreads;

#pragma omp parallel num_threads(threads)
  {
    int32_t thread = omp_get_thread_num();

    Wavelet_FastEncodeInsigAllNormal* tempOutputItems;
    Wavelet_FastEncodeInsigAllNormal* tempInputItems;

    int32_t* tempCurrentOutputPos;

    int32_t itemStart = (thread * elementsThisRun) / threads;
    int32_t itemEnd = ((thread + 1) * elementsThisRun) / threads;

    tempOutputItems = outputItems + nChildrenOut + (isInsigPass ? itemStart : itemStart * child);
    tempInputItems = ((Wavelet_FastEncodeInsigAllNormal*)inputItems) + iCurrentElement + itemStart;
    tempCurrentOutputPos = outputPos + nValuesOut + itemStart;

    tempOutputPos[thread] = tempCurrentOutputPos;
    tempOutputItem[thread] = tempOutputItems;
    tempInputItem[thread] = tempInputItems;

    for (int iItem = itemStart; iItem < itemEnd; iItem++)
    {
      Wavelet_FastEncodeInsigAllNormal item = inputItems[iItem + iCurrentElement];

      if (puBitStream[iItem >> 3] & (1 << (iItem & 7)))
      {
        // output positions!
        if (isInsigPass)
        {
          // add me to significant list if larger than iteration one (only if insig pass)
          if (item.iterXYZ >= (2 << WAVELET_ADAPTIVELL_ITER_SHIFT)) // same as if (cItem.GetIteration() > 1)
          {
            *tempOutputItems++ = item; // nChildrenOut
          }

          *tempCurrentOutputPos++ = item.iterXYZ; // nValuesOut
        }
        else
        {
          // output insignificants
          // double position and subtract iteration
          item.iterXYZ += item.iterXYZ & decodeIterator.allNormalAndMask;
          item.iterXYZ -= 1 << WAVELET_ADAPTIVELL_ITER_SHIFT;

          for (int iChild = 0; iChild < child; iChild++)
          {
            tempOutputItems[iChild].iterXYZ = item.iterXYZ + decodeIterator.childDisplacementAllNormal[iChild]; //nChildrenOut
          }
          tempOutputItems += child;
        }
      }
      else
      {
        *tempInputItems++ = item;
      }
    }

    tempOutputPosIndices[thread] = (int)(tempCurrentOutputPos - tempOutputPos[thread]);
    tempOutputItemIndices[thread] = (int)(tempOutputItems - tempOutputItem[thread]);
    tempInputItemIndices[thread] = (int)(tempInputItems - tempInputItem[thread]);
  }

  // accumulate
  nChildrenOut += tempOutputItemIndices[0];
  writeOut += tempInputItemIndices[0];
  nValuesOut += tempOutputPosIndices[0];

  // can parallelize this aswell, make accumvalues OPTIMIZEME!
  for (int thread = 1; thread < threads; thread++)
  {
    if (tempOutputItemIndices[thread])
    {
      memmove(outputItems + nChildrenOut, tempOutputItem[thread], tempOutputItemIndices[thread] * sizeof(int));
      nChildrenOut += tempOutputItemIndices[thread];
    }

    if (tempOutputPosIndices[thread])
    {
      memmove(outputPos + nValuesOut, tempOutputPos[thread], tempOutputPosIndices[thread] * sizeof(int));
      nValuesOut += tempOutputPosIndices[thread];
    }

    if (tempInputItemIndices[thread])
    {
      memmove(((Wavelet_FastEncodeInsigAllNormal*)inputItems) + writeOut, tempInputItem[thread], tempInputItemIndices[thread] * sizeof(int));
      writeOut += tempInputItemIndices[thread];
    }
  }

  iCurrentElement = writeOut;
  nElement = writeOut;
}

template <bool isInsigPass, int dimensions>
static void WaveletAdaptiveLLDecompress_EvalAndSplit(const WaveletAdaptiveLL_DecodeIterator& decodeIterator, uint8_t* puBitStream, int32_t& iCurrentElement, int32_t& nElement, const float rCurrentThreshold, int32_t& nValuesOutMultiple, int32_t& nValuesOutSingle, int32_t& nChildrenOut)
{
  assert(iCurrentElement < nElement);

  Wavelet_FastEncodeInsig* outputItems = isInsigPass ? (Wavelet_FastEncodeInsig*)decodeIterator.sig : (Wavelet_FastEncodeInsig*)decodeIterator.insig;

  Wavelet_FastEncodeInsig* inputItems = isInsigPass ? (Wavelet_FastEncodeInsig*)decodeIterator.insig : (Wavelet_FastEncodeInsig*)decodeIterator.sig;

  int32_t* outputPosMultiple = decodeIterator.pos;
  int32_t* outputPosSingle = decodeIterator.pos + decodeIterator.maxChildren;

  int32_t elementsThisRun = nElement - iCurrentElement;

  int32_t writeOut = iCurrentElement;

  int32_t idealChild = 1 << dimensions;

  for (int iItem = 0; iItem < elementsThisRun; iItem++)
  {
    int iActualElement = iItem + iCurrentElement;

    bool isExpand = false;

    {
      if (puBitStream[iItem >> 3] & (1 << (iItem & 7)))
      {
        isExpand = true;
      }
    }

    if (isExpand)
    {
      // add me to significant list if larger than iteration one (only if insig pass)
      if (isInsigPass)
      {
        if (inputItems[iActualElement].iteration > 1)
        {
          outputItems[nChildrenOut++] = inputItems[iActualElement];
        }
      }

      // go through and count children!
      Wavelet_FastEncodeInsig child = inputItems[iActualElement];

      // fast path?
      if (!child.subBandPos || ((Wavelet_Compiled_SubBandInfo*)(decodeIterator.compiledTransformData + child.subBandPos))->isNormal)
      {
        if (idealChild == decodeIterator.children[int(child.iteration)])
        {
          // double position 
          child.xyz += child.xyz;

          if (isInsigPass)
          {
            // write out pos with correct position for first child
            outputPosMultiple[nValuesOutMultiple++] = child.xyz; // nValuesOut
          }
          else
          {
            //subtract iteration
            child.iteration--;

            int32_t childSubBand = 0;

            if (child.subBandPos)
            {
              Wavelet_Compiled_SubBandInfo* subBandInfo = (Wavelet_Compiled_SubBandInfo*)(decodeIterator.compiledTransformData + child.subBandPos);

              childSubBand = subBandInfo->normalChildSubBand;
            }

            for (int iChild = 0; iChild < idealChild; iChild++)
            {
              outputItems[nChildrenOut].xyz = child.xyz + decodeIterator.childDisplacementAllNormal[iChild];
              outputItems[nChildrenOut].iteration = child.iteration;
              outputItems[nChildrenOut].subBandPos = childSubBand;
              nChildrenOut++;
            }
          }
        }
        else
        {
          int32_t nX = child.GetX();
          int32_t nY = child.GetY();
          int32_t nZ = child.GetZ();

          int32_t transformMask = decodeIterator.transformMask[int(child.iteration)];

          if (transformMask & 1) nX <<= 1;
          if (transformMask & 2) nY <<= 1;
          if (transformMask & 4) nZ <<= 1;

          int32_t relativePos = (nX + nY * decodeIterator.sizeX + nZ * decodeIterator.sizeXY);

          int32_t childSubBand = 0;

          if (child.subBandPos)
          {
            Wavelet_Compiled_SubBandInfo* subBandInfo = (Wavelet_Compiled_SubBandInfo*)(decodeIterator.compiledTransformData + child.subBandPos);

            childSubBand = subBandInfo->normalChildSubBand;
          }

          for (int iDecode = 0; iDecode <= transformMask; iDecode++)
          {
            if ((iDecode & transformMask) == iDecode)
            {
              if (isInsigPass)
              {
                int32_t position = relativePos + decodeIterator.screenDisplacement[iDecode];
                outputPosSingle[nValuesOutSingle++] = position;
              }
              else
              {
                int32_t iX = iDecode & 1;
                int32_t iY = (iDecode & 2) >> 1;
                int32_t iZ = (iDecode & 4) >> 2;

                Wavelet_FastEncodeInsig subChild(nX + iX, nY + iY, nZ + iZ, child.iteration - 1, childSubBand);

                outputItems[nChildrenOut++] = subChild;
              }
            }
          }
        }
      }
      else
      {
        int32_t nX = child.GetX();
        int32_t nY = child.GetY();
        int32_t nZ = child.GetZ();

        Wavelet_Compiled_SubBandInfo* subBandInfo = (Wavelet_Compiled_SubBandInfo*)(decodeIterator.compiledTransformData + child.subBandPos);

        int32_t childCountX = subBandInfo->childX;
        int32_t childCountY = subBandInfo->childY;
        int32_t childCountZ = subBandInfo->childZ;

        int32_t relativePosX = (nX - subBandInfo->posX) * childCountX;
        int32_t relativePosY = (nY - subBandInfo->posY) * childCountY;
        int32_t relativePosZ = (nZ - subBandInfo->posZ) * childCountZ;

        Wavelet_Compiled_SubBand* subBand = &subBandInfo->firstSubBand;

        for (int iSubBand = 0; iSubBand < subBandInfo->childSubBand; iSubBand++)
        {
          int32_t childPosX = subBand[iSubBand].childPosX + relativePosX;
          int32_t childPosY = subBand[iSubBand].childPosY + relativePosY;
          int32_t childPosZ = subBand[iSubBand].childPosZ + relativePosZ;

          int32_t legalChildPosX = subBand[iSubBand].legalChildEdgeX;
          int32_t legalChildPosY = subBand[iSubBand].legalChildEdgeY;
          int32_t legalChildPosZ = subBand[iSubBand].legalChildEdgeZ;

          int32_t childSubBand = subBand[iSubBand].childSubBand;

          int32_t actualChildCountX = childCountX;
          int32_t actualChildCountY = childCountY;
          int32_t actualChildCountZ = childCountZ;

          if (nX == subBand[iSubBand].extraChildEdgeX) actualChildCountX += 1;
          if (nY == subBand[iSubBand].extraChildEdgeY) actualChildCountY += 1;
          if (nZ == subBand[iSubBand].extraChildEdgeZ) actualChildCountZ += 1;

          if (isInsigPass)
          {
            int actualChild = actualChildCountX * actualChildCountY * actualChildCountZ;

            // valid child?
            if ((actualChild == idealChild) &&
              ((childPosX + actualChildCountX) <= legalChildPosX) &&
              ((childPosY + actualChildCountY) <= legalChildPosY) &&
              ((childPosZ + actualChildCountZ) <= legalChildPosZ))
            {
              Wavelet_FastEncodeInsigAllNormal fastChild;

              fastChild.SetXYZIter(childPosX, childPosY, childPosZ, 0);
              outputPosMultiple[nValuesOutMultiple++] = fastChild.iterXYZ;
              continue;
            }
          }

          for (int iCountZ = 0; iCountZ < actualChildCountZ; iCountZ++)
            for (int iCountY = 0; iCountY < actualChildCountY; iCountY++)
              for (int iCountX = 0; iCountX < actualChildCountX; iCountX++)
              {
                int32_t posX = iCountX + childPosX;
                int32_t posY = iCountY + childPosY;
                int32_t posZ = iCountZ + childPosZ;

                if (posX < legalChildPosX &&
                  posY < legalChildPosY &&
                  posZ < legalChildPosZ)
                {
                  if (isInsigPass)
                  {
                    int position = posX + posY * decodeIterator.sizeX + posZ * decodeIterator.sizeXY;

                    outputPosSingle[nValuesOutSingle++] = position;
                  }
                  else
                  {
                    Wavelet_FastEncodeInsig subChild(posX, posY, posZ, child.iteration - 1, childSubBand);
                    outputItems[nChildrenOut++] = subChild;
                  }
                }
              }
        }
      }
    }
    else
    {
      inputItems[writeOut++] = inputItems[iActualElement];
    }
  }

  iCurrentElement = writeOut;
  nElement = writeOut;
}

template<bool isAllNormal, int nDimensions>
int WaveletAdaptiveLLDecompress_DecodeTreeStructure(const WaveletAdaptiveLL_DecodeIterator &decodeIterator)
{
  const Wavelet_PixelSetChildren *pixelSetChildren = decodeIterator.pixelSetChildren;
  
  int32_t pixelSetChildrenCount = decodeIterator.pixelSetChildrenCount;
  
  int pixelSetPixelInsignificantCount = decodeIterator.pixelSetPixelInsignificantCount;

  const int startDecodeBits = decodeIterator.decodeBits;
  
  const int maxDecodeLevel = decodeIterator.decompressLevel;

  const float startThreshold = decodeIterator.startThreshold;

  int *valueEncodingMultiple = decodeIterator.valueEncodingMultiple;
  int *valuesAtLevelMultiple = decodeIterator.valuesAtLevelMultiple;

  int *valueEncodingSingle = decodeIterator.valueEncodingSingle;
  int *valuesAtLevelSingle = decodeIterator.valuesAtLevelSingle;

  int valuesMultiple = 0;
  int valuesSingle = 0;

  int streamPos = 0;

  int *numberOfValuesPerLevelMultiple = (int*)decodeIterator.stream;
  int *numberOfValuesPerLevelSingle = NULL;

  // reserve space for number of valuesmultiple output
  streamPos += WAVELET_ADAPTIVE_LEVELS * sizeof(int);

  if (!isAllNormal)
  {
    numberOfValuesPerLevelSingle = (int *)(decodeIterator.stream + streamPos);

    // reserve space for number of valuessingle output
    streamPos += WAVELET_ADAPTIVE_LEVELS * sizeof(int);
  }

  // intital values(lowest bands) that we are going to copy into picture later
  streamPos += (pixelSetChildrenCount + pixelSetPixelInsignificantCount) * sizeof(float);

  // number of sign values encoded/decoded,Insignifacnts and Significants and values in each pass
  int signValueSetMultiple = 0;
  int signValueSetSingle = 0;
  int insig = 0;
  int sig = 0;


  // Create first list of all Inisignificants 
  WaveletAdaptiveLL_InitializeInsignificants<isAllNormal>(decodeIterator, pixelSetChildren, pixelSetChildrenCount);

  // Set number of current insignificants
  insig = pixelSetChildrenCount;

  // for each "level" we record where to encode/decode data

  float decodeBitsThreshold = startThreshold;

  int decodeBits = startDecodeBits;

  // PROFILING/
  /*
  LARGE_INTEGER
    nFrequency,
    nPerformanceCounterStart0,
    nPerformanceCounterEnd0;

  QueryPerformanceFrequency(&nFrequency);
  QueryPerformanceCounter(&nPerformanceCounterStart0);
  */
  int multiple = isAllNormal ? decodeIterator.children[1] : 1 << nDimensions;

  assert(maxDecodeLevel >= 0);

  // for each adaptive level
  while (decodeBits >= maxDecodeLevel)
  {
    int insigCurrent = 0;
    int sigCurrent = 0;

    // Anything more to do in this pass?
    while (insigCurrent < insig ||
      sigCurrent < sig)
    {
      ///////////////////////////////////////////////////////////////////////////////////////////
      // do Insig list
      if (insigCurrent < insig)
      {
        unsigned char *bitField = decodeIterator.stream + streamPos;

        int insigThisPass = insig - insigCurrent;

        streamPos += (insigThisPass + 7) / 8;

        // create count out for how many children each item should make (also mark, lowest bit, which insig should go to significant
        if (isAllNormal)
        {
          WaveletAdaptiveLLDecompress_EvalAndSplitAllNormal<true>(decodeIterator, bitField, insigCurrent, insig, decodeBitsThreshold, valuesMultiple, sig);
        }
        else
        {
          WaveletAdaptiveLLDecompress_EvalAndSplit<true, nDimensions>(decodeIterator, bitField, insigCurrent, insig, decodeBitsThreshold, valuesMultiple, valuesSingle, sig);
        }
      }


      ///////////////////////////////////////////////////////////////////////////////////////////
      // do sig list
      ///////////////////////////////////////////////////////////////////////////////////////////
      if (sigCurrent < sig)
      {
        //printf ("Sig Threads=%d", nSigThisPass);

        unsigned char *bitField = decodeIterator.stream + streamPos;

        int sigThisPass = sig - sigCurrent;

        streamPos += (sigThisPass + 7) / 8;

        // create count out for how many children each item should make (also mark, lowest bit, which insig should go to significant
        int dummy = 0;
        int dummy2 = 0;

        if (isAllNormal)
        {
          WaveletAdaptiveLLDecompress_EvalAndSplitAllNormal<false>(decodeIterator, bitField, sigCurrent, sig, decodeBitsThreshold, dummy, insig);
        }
        else
        {
          WaveletAdaptiveLLDecompress_EvalAndSplit<false, nDimensions>(decodeIterator, bitField, sigCurrent, sig, decodeBitsThreshold, dummy, dummy2, insig);
        }
      }
    }

    // values at each decode step
    valuesAtLevelMultiple[decodeBits] = valuesMultiple * multiple;
    valueEncodingMultiple[decodeBits] = streamPos;

    // increment stream pos for reserving bit space for later
    streamPos += (valuesMultiple * multiple + 7) / 8; // this is where sign bit encoding starts for all new items on this level
    streamPos += ((valuesMultiple - signValueSetMultiple) * multiple + 7) / 8;


    if (!isAllNormal)
    {
      // values at each decode step
      valuesAtLevelSingle[decodeBits] = valuesSingle;
      valueEncodingSingle[decodeBits] = streamPos;

      // increment stream pos for reserving bit space for later
      streamPos += (valuesSingle + 7) / 8; // this is where sign bit encoding starts for all new items on this level
      streamPos += ((valuesSingle - signValueSetSingle) + 7) / 8;
    }

    // write out bytesize fo each adaptive level
    if (decodeBits < WAVELET_ADAPTIVE_LEVELS)
    {
      if (numberOfValuesPerLevelMultiple[decodeBits] != valuesMultiple)
      {
        ;//nop
      }

      assert(numberOfValuesPerLevelMultiple[decodeBits] == valuesMultiple);

      if (!isAllNormal)
      {
        if (numberOfValuesPerLevelSingle[decodeBits] != valuesSingle)
        {
          ;//nop
        }
        assert(numberOfValuesPerLevelSingle[decodeBits] == valuesSingle);
      }
    }

    // Set that we have outputted sign bits all the way up to here!
    signValueSetMultiple = (int)valuesMultiple;
    signValueSetSingle = (int)valuesSingle;

    // next threshold level
    decodeBits--;
    decodeBitsThreshold *= 0.5f;
  }

  /*
  QueryPerformanceCounter(&nPerformanceCounterEnd0);

  double
    rTime = (double)(nPerformanceCounterEnd0.QuadPart - nPerformanceCounterStart0.QuadPart) / (double)nFrequency.QuadPart;

  double
    rMB = cdecodeIterator.m_nMaxPixel * 4.0 / (1024.0 * 1024.0);

  rTotalTime += rTime;
  rTotalMB += rMB;

  //printf ("Decode Values: T: %d at %.2f MB/s  C: %.2f MB/s\n", nValuesOut * nMultiple, rTotalMB / rTotalTime, rMB/rTime);
  */
  return streamPos;
}

}

#endif
