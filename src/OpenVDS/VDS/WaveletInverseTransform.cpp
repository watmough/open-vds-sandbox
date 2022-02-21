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

#define ENABLE_SSE_TRANSFORM 1
#include "WaveletInverseTransform.h"
#include "WaveletInverseTransformSSE.h"
#include "WaveletOpenMP.h"
#include <assert.h>

#define WAVELET_MAX_DIMENSION_SIZE 4096

namespace Wavelet {

void Wavelet_CreateTransformData(Wavelet_TransformData *transformData, IntVector3 *bandSize, int *transformMask, int transformIterations)
{
  for (int i=0; i < transformIterations; i++)
  {
    int currentTransformMask = transformMask[i];

    transformData[i].childCount = { 0,0,0 };
    transformData[i].isNormal = false;

    for (int iSector = 0; iSector < 8; iSector++)
    {
      transformData[i].subBandInfo[iSector].childPos[0] = { -100000,-100000,-100000 };
      transformData[i].subBandInfo[iSector].extraChildEdge[0] = { -100000,-100000,-100000 };
      transformData[i].subBandInfo[iSector].legalChildEdge[0] = {-100000,-100000,-100000 };
      transformData[i].subBandInfo[iSector].childSector[0] = -1;
      transformData[i].subBandInfo[iSector].childSubBand = 666;
    }

    if (i>0)
    {
      transformData[i].childCount = {1,1,1};

      if (currentTransformMask & 1) transformData[i].childCount.X = 2;
      if (currentTransformMask & 2) transformData[i].childCount.Y = 2;
      if (currentTransformMask & 4) transformData[i].childCount.Z = 2;

      transformData[i].child = transformData[i].childCount.X * transformData[i].childCount.Y * transformData[i].childCount.Z; 

      int nextTransformMask = transformMask[i - 1];

      int otherTransformMask = currentTransformMask ^ nextTransformMask;

       bool isAddExtras = true;

      bool isNormal = true;

      for (int iSector = 1; iSector < 8; iSector++)
      {
        int childSubBand = 0;

        if ((iSector & currentTransformMask) == iSector)
        {
          // Find subband pos
          IntVector3 subBandPos(0,0,0);

          if (iSector & 1) subBandPos.X = bandSize[i+1].X;
          if (iSector & 2) subBandPos.Y = bandSize[i+1].Y;
          if (iSector & 4) subBandPos.Z = bandSize[i+1].Z;

          IntVector3 childPos(0,0,0);

          if (iSector & 1) childPos.X = bandSize[i].X;
          if (iSector & 2) childPos.Y = bandSize[i].Y;
          if (iSector & 4) childPos.Z = bandSize[i].Z;

          // Add default subband
          transformData[i].subBandInfo[iSector].pos = subBandPos;
          transformData[i].subBandInfo[iSector].childPos[childSubBand] = childPos;
          transformData[i].subBandInfo[iSector].childSector[childSubBand] = iSector;
          childSubBand++;

          if (otherTransformMask)
          {
            isNormal = false;
            // Go through the inverted mask
            for (int iOtherSector = 1; iOtherSector < 8; iOtherSector++)
            {
              if ((otherTransformMask & iOtherSector) == iOtherSector)
              {
                // Add this one aswell
                IntVector3 currentSubPos = childPos;

                if (iOtherSector & 1) currentSubPos.X = bandSize[i].X;
                if (iOtherSector & 2) currentSubPos.Y = bandSize[i].Y;
                if (iOtherSector & 4) currentSubPos.Z = bandSize[i].Z;

                transformData[i].subBandInfo[iSector].childPos[childSubBand] = currentSubPos;
                transformData[i].subBandInfo[iSector].childSector[childSubBand] = iSector + iOtherSector;
                childSubBand++;
              }
            }

            // Add extras
            if (isAddExtras)
            {
              isAddExtras = false;
            
              for (int iOtherSector = 1; iOtherSector < 8; iOtherSector++)
              {
                if ((otherTransformMask & iOtherSector) == iOtherSector)
                {
                  // Add this one aswell
                  IntVector3 currentSubPos = subBandPos;

                  if (iOtherSector & 1) currentSubPos.X = bandSize[i].X;
                  if (iOtherSector & 2) currentSubPos.Y = bandSize[i].Y;
                  if (iOtherSector & 4) currentSubPos.Z = bandSize[i].Z;

                  if (iSector & 1) currentSubPos.X = 0;
                  if (iSector & 2) currentSubPos.Y = 0;
                  if (iSector & 4) currentSubPos.Z = 0;

                  transformData[i].subBandInfo[iSector].childPos[childSubBand] = currentSubPos;
                  transformData[i].subBandInfo[iSector].childSector[childSubBand] = iOtherSector;
                  childSubBand++;
                }
              }
            }
          }
        }
        
        transformData[i].subBandInfo[iSector].childSubBand = childSubBand;

        IntVector3 startWrite(0, 0, 0);
        IntVector3 endWrite = bandSize[i];

        if (iSector & 1) startWrite.X = bandSize[i + 1].X;
        else             endWrite.X = bandSize[i + 1].X;

        if (iSector & 2) startWrite.Y = bandSize[i + 1].Y;
        else             endWrite.Y = bandSize[i + 1].Y;

        if (iSector & 4) startWrite.Z = bandSize[i + 1].Z;
        else             endWrite.Z = bandSize[i + 1].Z;


        IntVector3 delta = endWrite;

        delta.X -= startWrite.X;
        delta.Y -= startWrite.Y;
        delta.Z -= startWrite.Z;

        if (currentTransformMask & 1) delta.X *= 2;
        if (currentTransformMask & 2) delta.Y *= 2;
        if (currentTransformMask & 4) delta.Z *= 2;

        int childX = 1;
        int childY = 1;
        int childZ = 1; 

        if (currentTransformMask & 1) childX = 2;
        if (currentTransformMask & 2) childY = 2;
        if (currentTransformMask & 4) childZ = 2;

        for (int iSubBand = 0; iSubBand < transformData[i].subBandInfo[iSector].childSubBand; iSubBand++)
        {
          int iChildSector = transformData[i].subBandInfo[iSector].childSector[iSubBand];

          IntVector3 startRead(transformData[i].subBandInfo[iSector].childPos[iSubBand]);
          IntVector3 endRead = bandSize[i];

          if (iChildSector & 1) endRead.X = bandSize[i-1].X;
          if (iChildSector & 2) endRead.Y = bandSize[i-1].Y;
          if (iChildSector & 4) endRead.Z = bandSize[i-1].Z;

          transformData[i].subBandInfo[iSector].legalChildEdge[iSubBand] = endRead;

          // Can we easy double to get to the position?
          if ((startWrite.X * childX) != startRead.X ||
              (startWrite.Y * childY) != startRead.Y ||
              (startWrite.Z * childZ) != startRead.Z)
          {
            isNormal = false;
          }

          // Can a child be created outside legal band?
          if ((startRead.X + delta.X) > endRead.X ||
              (startRead.Y + delta.Y) > endRead.Y ||
              (startRead.Z + delta.Z) > endRead.Z)
          {
            isNormal = false;
          }

          // Are there some places we need to create 3 children along one axis?

          endRead.X -= startRead.X;
          endRead.Y -= startRead.Y;
          endRead.Z -= startRead.Z;

          transformData[i].subBandInfo[iSector].extraChildEdge[iSubBand] = {-1,-1,-1};
          
          if (endRead.X > delta.X)
          {
            isNormal = false;
            transformData[i].subBandInfo[iSector].extraChildEdge[iSubBand].X = endWrite.X - 1;
          }
          
          if (endRead.Y > delta.Y)
          {
            isNormal = false;
            transformData[i].subBandInfo[iSector].extraChildEdge[iSubBand].Y = endWrite.Y - 1;
          }

          if (endRead.Z > delta.Z) 
          {
            isNormal = false;
            transformData[i].subBandInfo[iSector].extraChildEdge[iSubBand].Z = endWrite.Z - 1;
          }
        }
      }

      transformData[i].isNormal = isNormal;
    }
  }
}

#ifdef ENABLE_SSE_TRANSFORM

void WaveletTransform_InverseTransform(float* tempBuffer, int32_t tempBufferSize, float* source, int32_t transformIterations, const IntVector3(&bandSize)[TRANSFORM_MAX_ITERATIONS + 1], const int32_t(&transformMask)[TRANSFORM_MAX_ITERATIONS], int32_t allocatedSizeX, int32_t allocatedSizeXY, uint32_t integerInfo)
{
  assert(tempBufferSize >= ((bandSize[0][0] + 3) & ~3) * (bandSize[0][1]) * (bandSize[0][2]));

  for (int i = transformIterations - 1; i >= 0; i--)
  {
    int32_t bandSizeCurr[3];

    char transformMaskCurr = transformMask[i];

    int32_t bandSizeX = bandSize[i][0];
    int32_t bandSizeY = bandSize[i][1];
    int32_t bandSizeZ = bandSize[i][2];

    bandSizeCurr[0] = bandSizeX;
    bandSizeCurr[1] = bandSizeY;
    bandSizeCurr[2] = bandSizeZ;

    assert(bandSizeX <= bandSize[0][0]);
    assert(bandSizeY <= bandSize[0][1]);
    assert(bandSizeZ <= bandSize[0][2]);

    int32_t bufferPitchXY = (bandSizeX + 3) & ~3;
    int32_t bufferPitchX = bufferPitchXY * bandSizeZ; // Swapping pitches is faster.

    int32_t readPitchX = allocatedSizeX;
    int32_t readPitchXY = allocatedSizeXY;
    int32_t writePitchX = bufferPitchX;
    int32_t writePitchXY = bufferPitchXY;

    float *read = source;
    float *write = tempBuffer;

    const int threadCount = Wavelet_GetEffectiveOpenMPThreadCount(WAVELET_OPENMP_SSE_THREAD_COUNT);

    if (transformMaskCurr == 7)
    {
      // changed so it doez Z first, then Y, then X.
      #pragma omp parallel for num_threads(threadCount) schedule(guided)
      for (int32_t iD1 = 0; iD1 < bandSizeCurr[1]; ++iD1)
      {
        Wavelet_InverseTransformSliceInterleave(tempBuffer + iD1 * bufferPitchX, bufferPitchXY, source + iD1 * allocatedSizeX, allocatedSizeXY, bandSizeX, bandSizeZ, integerInfo);
      }

      #pragma omp parallel for num_threads(threadCount) schedule(guided)
      for (int32_t iD2 = 0; iD2 < bandSizeCurr[2]; ++iD2)
      {
        Wavelet_InverseTransformSlice(tempBuffer + iD2 * bufferPitchXY, bufferPitchX, tempBuffer + iD2 * bufferPitchXY, bufferPitchX, bandSizeX, bandSizeY, integerInfo);

        for (int32_t iD1 = 0; iD1 < bandSizeCurr[1]; ++iD1)
        {
          int32_t iD1Interleaved = (iD1 & 1 ? (bandSizeCurr[1] + 1) >> 1 : 0) + (iD1 >> 1);

          // Wavelet transform x
          float *readLine = tempBuffer + (iD2 * bufferPitchXY + iD1Interleaved * bufferPitchX);

          Wavelet_InverseTransformLine(readLine, bandSizeX, integerInfo);
          Wavelet_InterleaveLine(source + (iD1 * allocatedSizeX + iD2 * allocatedSizeXY), readLine, readLine + ((bandSizeX + 1) >> 1), bandSizeX);
        }
      }
    }
    else
    {
      if (transformMaskCurr & 4)
      {
        #pragma omp parallel for num_threads(threadCount) schedule(guided)
        for (int32_t iD1 = 0; iD1 < bandSizeCurr[1]; ++iD1)
        {
          Wavelet_InverseTransformSliceInterleave(write + iD1 * writePitchX, writePitchXY, read + iD1 * readPitchX, readPitchXY, bandSizeX, bandSizeZ, integerInfo);
        }

        read = write;
        readPitchX = writePitchX;
        readPitchXY = writePitchXY;

        write = source;
        writePitchX = allocatedSizeX;
        writePitchXY = allocatedSizeXY;
      }

      if (transformMaskCurr & 2)
      {
        #pragma omp parallel for num_threads(threadCount) schedule(guided)
        for (int32_t iD2 = 0; iD2 < bandSizeCurr[2]; ++iD2)
        {
          Wavelet_InverseTransformSliceInterleave(write + iD2 * writePitchXY, writePitchX, read + iD2 * readPitchXY, readPitchX, bandSizeX, bandSizeY, integerInfo);
        }

        read = write;
        readPitchX = writePitchX;
        readPitchXY = writePitchXY;

        write = source;
        writePitchX = allocatedSizeX;
        writePitchXY = allocatedSizeXY;
      }

      if (transformMaskCurr & 1)
      {
        #pragma omp parallel for num_threads(threadCount) schedule(guided)
        for (int32_t iD2 = 0; iD2 < bandSizeCurr[2]; ++iD2)
        {
          for (int32_t iD1 = 0; iD1 < bandSizeCurr[1]; ++iD1)
          {
            // Wavelet transform x
            float *readDisplaced = read + (iD1 * readPitchX + iD2 * readPitchXY);

            Wavelet_InverseTransformLine(readDisplaced, bandSizeX, integerInfo);
            Wavelet_InterleaveLine(write + (iD1 * writePitchX + iD2 * writePitchXY), readDisplaced, readDisplaced + ((bandSizeX + 1) >> 1), bandSizeX);
          }
        }

        read = write;
        readPitchX = writePitchX;
        readPitchXY = writePitchXY;
      }

      if (read != source)
      {
        #pragma omp parallel for num_threads(threadCount) schedule(static)
        for (int32_t iD2 = 0; iD2 < bandSizeZ; ++iD2)
        {
          Wavelet_CopySlice(source + iD2 * allocatedSizeXY, allocatedSizeX, read + iD2 * readPitchXY, readPitchX, bandSizeX, bandSizeY);
        }
      }
    }
  }
}

#else

static void GetLine(float *write, float *read, int32_t length, int32_t modulo)
{
  for (int32_t i = 0; i < length; i++)
  {
    write[i] = read[i * modulo];
  }
}

static inline void TransformRotateFloatLeft(float *buffer)
{
  buffer[3] = buffer[2];
  buffer[2] = buffer[1];
  buffer[1] = buffer[0];
}

template<bool isInteger>
static void TransformUpdateCoarse(float *write, float *read, int32_t length, float sign, bool isOdd, uint32_t integerInfo)
{
  float temp[4];

  int32_t extra;

  // Clip 21
  temp[3] = read[1];
  temp[2] = read[0];
  temp[1] = read[0];
  read++;

  if (isOdd)
  {
    // Clip 22
    extra = 2;
  }
  else
  {
    // Clip 21
    extra = 1;
  }

  int32_t dstLengthMiddle = length - extra;

  // Go through two times with different directions
  for (int32_t iPos = 0; iPos < dstLengthMiddle; iPos++)
  {
    // Read in new pixel
    temp[0] = *read++;

    if (isInteger)
    {
      float currentInt = *write < 0.0f ? floorf(*write) : ceilf(*write);

      *write = currentInt + rintf(sign * (9.0f * (temp[1] + temp[2]) - (temp[0] + temp[3])));
    }
    else
    {
      *write += sign * (9.0f * (temp[1] + temp[2]) - (temp[0] + temp[3]));
    }

    write++;

    TransformRotateFloatLeft(temp);
  }

  if (!isOdd)
  {
    read--;
  }

  for (int32_t pos = 0; pos < extra; pos++)
  {
    // Read in new pixel
    temp[0] = *--read;

    if (isInteger)
    {
      float currentInt = *write < 0.0f ? floorf(*write) : ceilf(*write);
      *write = currentInt + rintf(sign * (9.0f * (temp[1] + temp[2]) - (temp[0] + temp[3])));
    }
    else
    {
      *write += sign * (9.0f * (temp[1] + temp[2]) - (temp[0] + temp[3]));
    }

    write++;

    TransformRotateFloatLeft(temp);
  }
}

template<bool isInteger>
static void TransformPredictDetail(float *write, float *read, int32_t length, float sign, bool isOdd, uint32_t integerInfo)
{
  float buffer[4];

  buffer[3] = read[0]; // New format 1
  buffer[2] = read[0];
  buffer[1] = read[1];
  read += 2;

  int nExtra;

  if (isOdd)
  {
    nExtra = 1;
  }
  else
  {
    nExtra = 2;
  }

  int32_t dstLengthMiddle = (length - nExtra);

  // Go through two times with different directions
  for (int32_t iPos = 0; iPos < dstLengthMiddle; iPos++)
  {
    // Read in new pixel
    buffer[0] = *read++;

    if (isInteger)
    {
      *write -= rintf(sign * (9.0f * (buffer[1] + buffer[2]) - (buffer[0] + buffer[3])));
    }
    else
    {
      *write -= sign * (9.0f * (buffer[1] + buffer[2]) - (buffer[0] + buffer[3]));
    }
    write++;

    TransformRotateFloatLeft(buffer);
  }

  if (isOdd) read--;

  for (int32_t iPos = 0; iPos < nExtra; iPos++)
  {
    // Read in new pixel
    buffer[0] = *--read;

    if (isInteger)
    {
      *write -= rintf(sign * (9.0f * (buffer[1] + buffer[2]) - (buffer[0] + buffer[3])));
    }
    else
    {
      *write -= sign * (9.0f * (buffer[1] + buffer[2]) - (buffer[0] + buffer[3]));
    }
    write++;

    TransformRotateFloatLeft(buffer);
  }
}

template<bool isInteger>
static void InverseTransformLine_2(float *readWrite, int32_t length, uint32_t integerInfo)
{
  int32_t lengthLow = (length + 1) >> 1;
  int32_t lengthHigh = length >> 1;

  float val = integerInfo & WAVELET_INTEGERINFO_ISLOSSLESSOPTIMIZED ? 1.0f : (float)REAL_INVSQRT2;

  for (int32_t i = 0; i < lengthLow; i++)
  {
    readWrite[i] *= val;
  }

  TransformUpdateCoarse<isInteger>(readWrite, readWrite + lengthLow, lengthLow, -1.0 / 32.0f, length & 1, integerInfo);
  TransformPredictDetail<isInteger>(readWrite + lengthLow, readWrite, lengthHigh, -1.0 / 16.0f, length & 1, integerInfo);
}

static void InverseTransformLine(float *readWrite, int32_t length, uint32_t integerInfo)
{
  if (integerInfo & WAVELET_INTEGERINFO_ISINTEGER)
    InverseTransformLine_2<true>(readWrite, length, integerInfo); 
  else
    InverseTransformLine_2<false>(readWrite, length, integerInfo); 
}


static void WriteLineFromLowHighBand(float *write, float *read, int32_t length, int32_t modulo)
{
    int32_t lengthLowBand = (length + 1) >> 1;
    int32_t lengthHighBand = length >> 1;

  float *lowBand = read;
  float *highBand = read + lengthLowBand;

  int32_t i=0;

  for (i = 0; i < lengthHighBand; i++)
  {
    write[i * 2 * modulo] = lowBand[i];
    write[i * 2 * modulo + modulo] = highBand[i];
  }

  if (lengthLowBand != lengthHighBand)
  {
    write[i * 2 * modulo] = lowBand[i];
  }
}

void WaveletTransform_InverseTransform(float* source, int32_t transformIterations, const IntVector3(&bandSize)[TRANSFORM_MAX_ITERATIONS + 1], const int32_t(&transformMask)[TRANSFORM_MAX_ITERATIONS], int32_t allocatedSizeX, int32_t allocatedSizeXY, uint32_t integerInfo)
{
  float line[WAVELET_MAX_DIMENSION_SIZE];

  for (int i = transformIterations - 1; i >= 0; i--)
  {
    int32_t bandSizeCurr[3];

    char transformMaskCurr = transformMask[i];

    int32_t   bandSizeX = bandSize[i][0];
    int32_t   bandSizeY = bandSize[i][1];
    int32_t   bandSizeZ = bandSize[i][2];

    bandSizeCurr[0] = bandSizeX;
    bandSizeCurr[1] = bandSizeY;
    bandSizeCurr[2] = bandSizeZ;

    if (transformMaskCurr & 4)
    {
      for (int32_t iD1 = 0; iD1 < bandSizeCurr[1]; iD1++)
        for (int32_t iD0 = 0; iD0 < bandSizeCurr[0]; iD0++)
        {
          int32_t displacement = (iD0 + iD1 * allocatedSizeX);

          // Wavelet transform z
          GetLine(line, source + displacement, bandSizeZ, allocatedSizeXY);
          InverseTransformLine(line, bandSizeZ, integerInfo);
          WriteLineFromLowHighBand(source + displacement, line, bandSizeZ, allocatedSizeXY);
        }
    }

    if (transformMaskCurr & 2)
    {
      for (int32_t iD2 = 0; iD2 < bandSizeCurr[2]; iD2++)
        for (int32_t iD0 = 0; iD0 < bandSizeCurr[0]; iD0++)
        {
          int32_t displacement = (iD0 + iD2 * allocatedSizeXY);

          // Wavelet transform y
          GetLine(line, source + displacement, bandSizeY, allocatedSizeX);
          InverseTransformLine(line, bandSizeY, integerInfo);
          WriteLineFromLowHighBand(source + displacement, line, bandSizeY, allocatedSizeX);
        }
    }

    if (transformMaskCurr & 1)
    {
      for (int32_t iD2 = 0; iD2 < bandSizeCurr[2]; iD2++)
        for (int32_t iD1 = 0; iD1 < bandSizeCurr[1]; iD1++)
        {
          int32_t displacement = (iD1 * allocatedSizeX + iD2 * allocatedSizeXY);

          // Wavelet transform x
          GetLine(line, source + displacement, bandSizeX, 1);
          InverseTransformLine(line, bandSizeX, integerInfo);
          WriteLineFromLowHighBand(source + displacement, line, bandSizeX, 1);
        }
    }
  }
}

#endif

}
