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

#include "WaveletDecompress.h"
#include "WaveletAdaptiveLLDecompress.h"
#include "WaveletOpenMP.h"
#define ENABLE_SSE_TRANSFORM 1
#include "WaveletInverseTransform.h"
#include "WaveletInverseTransformSSE.h"
#include "FSE/fse.h"
#include <assert.h>

namespace Wavelet {

template<typename T>
void WaveletDecompress_ConvertFloatToIntegerType(const WaveletDataBlock &sourceDataBlock, const std::vector<uint8_t> &sourceData, const WaveletDataBlock &targetDataBlock, std::vector<uint8_t> &targetData)
{
  assert(sourceDataBlock.Format == WaveletDataFormat::Format_R32);

  int32_t sourceSizeX = sourceDataBlock.Size[0];
  int32_t sourceSizeY = sourceDataBlock.Size[1];
  int32_t sourceSizeZ = sourceDataBlock.Size[2];
  int32_t sourceAllocatedSizeX = sourceDataBlock.AllocatedSize[0];
  int32_t sourceAllocatedSizeY = sourceDataBlock.AllocatedSize[1];
  uint32_t sourceElementSize = GetElementSize(sourceDataBlock);

  int32_t targetAllocatedSizeX = targetDataBlock.AllocatedSize[0];
  int32_t targetAllocatedSizeY = targetDataBlock.AllocatedSize[1];
  uint32_t targetElementSize = GetElementSize(targetDataBlock);

  for (int32_t iZ = 0; iZ < sourceSizeZ; iZ++)
  {
    for (int32_t iY = 0; iY < sourceSizeY; iY++)
    {
      T* target = reinterpret_cast<T*>(targetData.data() + (iZ * targetAllocatedSizeY * targetAllocatedSizeX + iY * targetAllocatedSizeX) * targetElementSize);
      const float * source = reinterpret_cast<const float *>(sourceData.data() + (iZ * sourceAllocatedSizeY * sourceAllocatedSizeX + iY * sourceAllocatedSizeX) * sourceElementSize);
      for (int32_t iX = 0; iX < sourceSizeX; iX++, target++, source++)
      {
        *target = T(*source);
      }
    }
  }
}

bool Wavelet_Decompress(void *compressedData, int nCompressedAdaptiveDataSize, WaveletDataFormat dataBlockFormat, const FloatRange &valueRange, float integerScale, float integerOffset, bool isUseNoValue, float noValue, bool isNormalize, int nDecompressLevel, bool isLossless, WaveletDataBlock &dataBlock, std::vector<uint8_t> &target, int& errorCode, std::string& errorString)
{
  if (isLossless)
  {
    assert(nDecompressLevel == 0);
  }

  int32_t *intHeaderPtr = (int32_t*)(compressedData);
  
  int32_t  dataVersion = intHeaderPtr[0];

  int32_t createSize[WaveletDataBlock::Dimensionality_Max];
  createSize[0] = intHeaderPtr[2];
  createSize[1] = intHeaderPtr[3];
  createSize[2] = intHeaderPtr[4];
  int32_t dimensions = intHeaderPtr[5] & 0xff;

  uint32_t integerInfo = (intHeaderPtr[5] >> 8) & 0xff;

  if (dataVersion < WAVELET_DATA_VERSION_1_4 ||
    dataVersion > WAVELET_DATA_VERSION_1_6 ||
    dimensions < 1 ||
    dimensions > 3 ||
    createSize[0] < 0 ||
    createSize[0] > 512 ||
    createSize[1] < 0 ||
    createSize[1] > 512 ||
    createSize[2] < 0 ||
    createSize[2] > 512)
  {
    errorCode = 100;
    errorString = "Invalid wavelet header";
    return false;
  }

  if (!dataBlock.Initialize(WaveletDataFormat::Format_R32, (enum WaveletDataBlock::Dimensionality)(dimensions), createSize, errorCode, errorString))
    return false;

  target.resize(GetAllocatedByteSize(dataBlock));
  
  WaveletDecompressor wavelet(integerInfo, compressedData,
    dataBlock.Size[0],
    dataBlock.Size[1],
    dataBlock.Size[2],
    dataBlock.AllocatedSize[0],
    dataBlock.AllocatedSize[1],
    dataBlock.AllocatedSize[2],
    dimensions,
    dataVersion);

  if (integerInfo & WAVELET_INTEGERINFO_ISINTEGER)
  {
    // Check if lossless pass was used (U32)
    if (!(integerInfo & WAVELET_INTEGERINFO_ISCOMPRESSEDWITHDIFFPASS))
    {
      // don't do lossless pass
      isLossless = false;
    }
  }
  

  float threshold;
  float startThreshold;
  float waveletNoValue;

  bool isAnyNoValue;

  if (!wavelet.Decompress(true, -1, -1, -1, &startThreshold, &threshold, dataBlock.Format, valueRange, integerScale, integerOffset, isUseNoValue, noValue, &isAnyNoValue, &waveletNoValue, isNormalize, nDecompressLevel, isLossless, nCompressedAdaptiveDataSize, dataBlock, target, errorCode, errorString))
    return false;

  if (dataBlock.Format != dataBlockFormat)
  {
    WaveletDataBlock finalDataBlock;
    if (!finalDataBlock.Initialize(dataBlockFormat, (enum WaveletDataBlock::Dimensionality)(dimensions), createSize, errorCode, errorString))
      return false;
    std::vector<uint8_t> finalDataTarget;
    finalDataTarget.resize(GetAllocatedByteSize(finalDataBlock));

    if (finalDataBlock.Format == WaveletDataFormat::Format_U8)
    {
      WaveletDecompress_ConvertFloatToIntegerType<uint8_t>(dataBlock, target, finalDataBlock, finalDataTarget);
    }
    else
    {
      WaveletDecompress_ConvertFloatToIntegerType<uint16_t>(dataBlock, target, finalDataBlock, finalDataTarget);
    }
    target = std::move(finalDataTarget);
    dataBlock = finalDataBlock;
  }

  return true;
}

static int32_t WaveletDecompress_FindTransformMethod(IntVector3 (&bandSize)[TRANSFORM_MAX_ITERATIONS + 1], int32_t(&splitMask)[TRANSFORM_MAX_ITERATIONS], int32_t sizeX, int32_t sizeY, int32_t sizeZ, int32_t dataVersion)
{
  int i = 0;

  while (sizeX >= WAVELET_BAND_MIN_SIZE ||
         sizeY >= WAVELET_BAND_MIN_SIZE ||
         sizeZ >= WAVELET_BAND_MIN_SIZE)
  {
    bandSize[i] = {sizeX, sizeY, sizeZ};

    char mask = 0;

    if  (sizeX >= WAVELET_BAND_MIN_SIZE) mask |= 1;
    if  (sizeY >= WAVELET_BAND_MIN_SIZE) mask |= 2;
    if  (sizeZ >= WAVELET_BAND_MIN_SIZE) mask |= 4;
     
    if (mask & 1) sizeX = (sizeX + 1) >> 1;
    if (mask & 2) sizeY = (sizeY + 1) >> 1;
    if (mask & 4) sizeZ = (sizeZ + 1) >> 1;

    splitMask[i] = mask;

    i++;
  }

  bandSize[i] = {sizeX, sizeY, sizeZ};

  if (i==0)
  {
    splitMask[0] = 0;
    bandSize[i+1] = {sizeX, sizeY, sizeZ};
  }

  assert(i <= TRANSFORM_MAX_ITERATIONS);

  return i;
}

static int WaveletDecompress_CalculateBufferSizeNeeded(int maxPixels, int maxChildren)
{
  int size = 0;

  size += WAVELET_MAX_PIXELSETPIXEL_SIZE + ADAPTIVEWAVELET_ALIGNBUFFERSIZE;
  size += WAVELET_MAX_PIXELSETCHILDREN_SIZE + ADAPTIVEWAVELET_ALIGNBUFFERSIZE;

  size += DECODEITERATOR_MAXDECODEBITS * sizeof(int) + ADAPTIVEWAVELET_ALIGNBUFFERSIZE; // valueEncoding
  size += DECODEITERATOR_MAXDECODEBITS * sizeof(int) + ADAPTIVEWAVELET_ALIGNBUFFERSIZE; // valuesAtLevel
  size += DECODEITERATOR_MAXDECODEBITS * sizeof(int) + ADAPTIVEWAVELET_ALIGNBUFFERSIZE; // valueEncodingSingle
  size += DECODEITERATOR_MAXDECODEBITS * sizeof(int) + ADAPTIVEWAVELET_ALIGNBUFFERSIZE; // valuesAtLevelSingle
  size += maxChildren * sizeof(Wavelet_FastDecodeInsig) + ADAPTIVEWAVELET_ALIGNBUFFERSIZE; // Insignificants + align to 256
  size += maxChildren * sizeof(Wavelet_FastDecodeInsig) + ADAPTIVEWAVELET_ALIGNBUFFERSIZE; // Significants + align to 256
  size += (maxPixels + maxChildren) * int32_t(sizeof(int)) + ADAPTIVEWAVELET_ALIGNBUFFERSIZE; // positions + align to 256
  size += 32768 + ADAPTIVEWAVELET_ALIGNBUFFERSIZE; // max sizeof transform hierarchical helper data
  return size;
}

void WaveletDecompress_RleDecode(uint8_t *rleBytes, uint32_t *bitBuffer, int32_t intsToDecode)
{
  int bitsToDecode = intsToDecode * 32;

  int writeBit = 0;

  memset(bitBuffer, 0, intsToDecode * 4);

  while (writeBit < bitsToDecode)
  {
    uint32_t setBits = 0;

    if (WaveletDecompress_RleDecodeOneRun(rleBytes, setBits))
    {
      int endBit = writeBit + setBits;

      while (writeBit < endBit)
      {
        bitBuffer[writeBit / 32] |= 1 << (writeBit & 31);
        writeBit++;
      }
    }
    else
    {
      writeBit += setBits;
    }
  }
}

WaveletDecompressor::WaveletDecompressor(uint32_t integerInfo, void *compressedData, int32_t transformSizeX, int32_t transformSizeY, int32_t transformSizeZ, int32_t allocatedSizeX, int32_t allocatedSizeY, int32_t allocatedSizeZ, int32_t dimensions, int32_t dataVersion)
{
  m_readCompressedData = (uint32_t *)compressedData;
  m_noValueData = nullptr;
  m_integerInfo =  integerInfo;
  m_dataVersion = dataVersion;
  m_dimensions = dimensions;

  m_dataBlockSizeX = transformSizeX;
  m_dataBlockSizeY = transformSizeY;
  m_dataBlockSizeZ = transformSizeZ;

  m_transformSizeX = transformSizeX;
  m_transformSizeY = transformSizeY;
  m_transformSizeZ = transformSizeZ;

  m_allocatedSizeX = allocatedSizeX;
  m_allocatedSizeY = allocatedSizeY;
  m_allocatedSizeZ = allocatedSizeZ;

  m_allocatedSizeXY = allocatedSizeX * allocatedSizeY;

  m_allocatedHalfSizeX = allocatedSizeX;
  m_allocatedHalfSizeY = allocatedSizeY;
  m_allocatedHalfSizeZ = allocatedSizeZ;

  m_transformIterations = WaveletDecompress_FindTransformMethod(m_bandSize, m_transformMask, m_transformSizeX, m_transformSizeY, m_transformSizeZ, m_dataVersion);

  m_allocatedHalfSizeX = m_bandSize[1][0];
  m_allocatedHalfSizeY = m_bandSize[1][1];
  m_allocatedHalfSizeZ = m_bandSize[1][2];

  int allocSize = 256 * 256 * 7;
  m_pixelSetPixelInSignificant.reset(new Wavelet_PixelSetPixel[allocSize]);
  m_pixelSetChildren.reset(new Wavelet_PixelSetChildren[allocSize]);
}

static inline void WaveletDecompress_CreatePixelSetChildren(Wavelet_PixelSetChildren *pixelSet, uint32_t iX, uint32_t iY, uint32_t iZ,  uint32_t uTransformIteration, int32_t iSubBand)
{
  pixelSet->X = iX;
  pixelSet->Y = iY;
  pixelSet->Z = iZ;
  pixelSet->transformIteration = uTransformIteration;
  pixelSet->subBand = iSubBand;
}

static inline void WaveletDecompress_CreatePixelSetPixel(Wavelet_PixelSetPixel *pixelSet, uint32_t iX, uint32_t iY, uint32_t iZ)
{
  pixelSet->X = iX;
  pixelSet->Y = iY;
  pixelSet->Z = iZ;
}

void WaveletDecompressor::Init()
{
  m_pixelSetPixelInSignificantCount = 0;
  m_pixelSetChildrenCount = 0;

  for (int i = 0; i < m_transformIterations; i++)
  {
    char transformMask = m_transformMask[i];

    if (i == (m_transformIterations - 1))
    {
      for (int iSector = 1; iSector < 8; iSector++)
      {
        if ((transformMask & iSector) == iSector)
        {
          int32_t startX = 0;
          int32_t startY = 0;
          int32_t startZ = 0;
          int32_t endX = m_bandSize[i + 1][0];
          int32_t endY = m_bandSize[i + 1][1];
          int32_t endZ = m_bandSize[i + 1][2];

          if (iSector & 1)
          {
            startX = m_bandSize[i + 1][0];
            endX = m_bandSize[i][0];
          };

          if (iSector & 2)
          {
            startY = m_bandSize[i + 1][1];
            endY = m_bandSize[i][1];
          }

          if (iSector & 4)
          {
            startZ = m_bandSize[i + 1][2];
            endZ = m_bandSize[i][2];
          }

          for (int32_t iZ = startZ; iZ < endZ; iZ++)
          {
            for (int32_t iY = startY; iY < endY; iY++)
            {
              for (int32_t iX = startX; iX < endX; iX++)
              {
                if (i > 0)
                {
                  WaveletDecompress_CreatePixelSetChildren(&m_pixelSetChildren[m_pixelSetChildrenCount++], iX, iY, iZ, i, iSector);
                }
              }
            }
          }
        }
      }
    }
  }


  int32_t bandSizeX = m_bandSize[m_transformIterations][0];
  int32_t bandSizeY = m_bandSize[m_transformIterations][1];
  int32_t bandSizeZ = m_bandSize[m_transformIterations][2];

  // take also first "children" into account, the are last level
  if (m_transformIterations == 1)
  {
    bandSizeX = m_bandSize[0][0];
    bandSizeY = m_bandSize[0][1];
    bandSizeZ = m_bandSize[0][2];
  }

  for (int32_t iD2 = 0; iD2 < bandSizeZ; iD2++)
  {
    for (int32_t iD1 = 0; iD1 < bandSizeY; iD1++)
    {
      for (int32_t iD0 = 0; iD0 < bandSizeX; iD0++)
      {
          WaveletDecompress_CreatePixelSetPixel(&m_pixelSetPixelInSignificant[m_pixelSetPixelInSignificantCount++], iD0, iD1, iD2);
      }
    }
  }
}

#define NORMAL_BLOCK_SIZE 8
#define NORMAL_BLOCK_SIZE_FLOAT 8.0f

#define READNORMVAL(X,Y,Z) normalizeField[X + Y * nNormalizeX + Z * nNormalizeX * nNormalizeY]

float Wavelet_GetNormalizedValue(float *normalizeField, int iX, int iY, int iZ, int nNormalizeX, int nNormalizeY,int nNormalizeZ)
{
  float rX = (float)iX;
  float rY = (float)iY;
  float rZ = (float)iZ;

  rX /= NORMAL_BLOCK_SIZE_FLOAT;
  rY /= NORMAL_BLOCK_SIZE_FLOAT;
  rZ /= NORMAL_BLOCK_SIZE_FLOAT;

  rX -= ((NORMAL_BLOCK_SIZE_FLOAT - 1.0f) / 2.0f) / NORMAL_BLOCK_SIZE_FLOAT;
  rY -= ((NORMAL_BLOCK_SIZE_FLOAT - 1.0f) / 2.0f) / NORMAL_BLOCK_SIZE_FLOAT;
  rZ -= ((NORMAL_BLOCK_SIZE_FLOAT - 1.0f) / 2.0f) / NORMAL_BLOCK_SIZE_FLOAT;

  int32_t iNormX = (int32_t)floorf(rX);
  int32_t iNormY = (int32_t)floorf(rY);
  int32_t iNormZ = (int32_t)floorf(rZ);

  // are we at end? if so, move one back so we extrapolate!
  if (iNormX == nNormalizeX - 1) iNormX -= 1;
  if (iNormY == nNormalizeX - 1) iNormY -= 1;
  if (iNormZ == nNormalizeX - 1) iNormZ -= 1;

  int32_t iNormX1 = iNormX+1;
  int32_t iNormY1 = iNormY+1;
  int32_t iNormZ1 = iNormZ+1;

  if (iNormX < 0) iNormX = 0;
  if (iNormY < 0) iNormY = 0;
  if (iNormZ < 0) iNormZ = 0;
  if (iNormX1 < 0) iNormX1 = 0;
  if (iNormY1 < 0) iNormY1 = 0;
  if (iNormZ1 < 0) iNormZ1 = 0;
  if (iNormX1 >= nNormalizeX) iNormX1 = nNormalizeX -1;
  if (iNormY1 >= nNormalizeY) iNormY1 = nNormalizeY -1;
  if (iNormZ1 >= nNormalizeZ) iNormZ1 = nNormalizeZ -1;

  rX -= (float)iNormX;
  rY -= (float)iNormY;
  rZ -= (float)iNormZ;

  if (rX < 0.0f) rX = 0.0f;
  if (rY < 0.0f) rY = 0.0f;
  if (rZ < 0.0f) rZ = 0.0f;

  if (rX > 1.0f) rX = 1.0f;
  if (rY > 1.0f) rY = 1.0f;
  if (rZ > 1.0f) rZ = 1.0f;

  // read and interpolate
  float r0 = READNORMVAL(iNormX, iNormY, iNormZ);
  float r1 = READNORMVAL(iNormX1, iNormY, iNormZ);
  float r2 = READNORMVAL(iNormX, iNormY1, iNormZ);
  float r3 = READNORMVAL(iNormX1, iNormY1, iNormZ);
  float r4 = READNORMVAL(iNormX, iNormY, iNormZ1);
  float r5 = READNORMVAL(iNormX1, iNormY, iNormZ1);
  float r6 = READNORMVAL(iNormX, iNormY1, iNormZ1);
  float r7 = READNORMVAL(iNormX1, iNormY1, iNormZ1);

  float rX0 = (r1 - r0) * rX + r0;
  float rX2 = (r3 - r2) * rX + r2;
  float rX4 = (r5 - r4) * rX + r4;
  float rX6 = (r7 - r6) * rX + r6;

  float rY0 = (rX2 - rX0) * rY + rX0;
  float rY4 = (rX6 - rX4) * rY + rX4;

  float rVal = (rY4 - rY0) * rZ + rY0;

  if (rVal < (1e-22f)) rVal = 1e-22f;

  return rVal;
}

bool WaveletDecompressor::Decompress(bool isTransform, int32_t decompressInfo, float decompressSlice, int32_t decompressFlip, float* startThreshold, float* threshold, WaveletDataFormat dataBlockFormat, const FloatRange& valueRange, float integerScale, float integerOffset, bool isUseNoValue, float noValue, bool* isAnyNoValue, float* waveletNoValue, bool isNormalize, int decompressLevel, bool isLossless, int compressedAdaptiveDataSize, WaveletDataBlock& dataBlock, std::vector<uint8_t>& target, int& errorCode, std::string& errorString)
{
  assert(m_dataVersion >= WAVELET_DATA_VERSION_1_4 && m_dataVersion <= WAVELET_DATA_VERSION_1_6);
  Init();

  int32_t *startOfCompressedData = (int32_t *)m_readCompressedData;

  int32_t compressedSize = startOfCompressedData[1];

  // no data?
  if (compressedSize < WAVELET_MIN_COMPRESSED_HEADER)
  {
    errorString = "Invalid size of compressed wavlet data";
    errorCode = -1;
    *isAnyNoValue = false;
    *waveletNoValue = 0.0f;
    return false;
  }
  else if (compressedSize <= WAVELET_MIN_COMPRESSED_HEADER)
  {
    *isAnyNoValue = false;
    *waveletNoValue = 0.0f;
    return true;
  }

  m_readCompressedData += 6;
  
  int nDecompressZeroSize = 0;
  
  unsigned char *pnDecompressZeroSize = nullptr;
  
  pnDecompressZeroSize = (unsigned char*)m_readCompressedData;
  nDecompressZeroSize = *m_readCompressedData;
  m_readCompressedData += (nDecompressZeroSize + 3) / 4;

  DecompressNoValuesHeader();

  float *normalField = (float *)(m_readCompressedData);
  
  if (isNormalize)
  {
    for (int iZ = 0; iZ < m_dataBlockSizeZ; iZ+=NORMAL_BLOCK_SIZE)
    {
      for (int iY = 0; iY < m_dataBlockSizeY; iY+=NORMAL_BLOCK_SIZE)
      {
        for (int iX = 0; iX < m_dataBlockSizeX; iX+=NORMAL_BLOCK_SIZE)
        {
          m_readCompressedData++;
        }
      }
    }
  }

  float *floatRead = (float *)m_readCompressedData;

  *threshold = *floatRead++,            // Get Threshold
  *startThreshold = *floatRead++;       // Get StartThreshold

  m_readCompressedData = (uint32_t *)floatRead;

  void *pxData = (void*) target.data();                                                             

  int nAdaptiveSize = *m_readCompressedData++;

  float *floatReadWriteData = (float*)pxData;

  // create transform data
  Wavelet_TransformData transformData[TRANSFORM_MAX_ITERATIONS];

  Wavelet_CreateTransformData(transformData, m_bandSize, m_transformMask, m_transformIterations);

  DecompressAdaptiveMode
    decompressAdaptiveMode = DecompressAdaptiveMode::AssumeNoOverwrite;

  if(m_dataVersion < WAVELET_DATA_VERSION_1_6 &&
     WaveletAdaptiveLL_IsWaveletStreamEncodedWithBug(transformData, m_transformIterations, m_transformMask))
  {
    decompressAdaptiveMode = isLossless ? DecompressAdaptiveMode::AllowOverwrite : DecompressAdaptiveMode::PreventOverwrite;
  }

  int cpuTempDecodeSizeNeeded = WaveletDecompress_CalculateBufferSizeNeeded(m_allocatedSizeX * m_allocatedSizeY * m_allocatedSizeZ, m_allocatedHalfSizeX * m_allocatedHalfSizeY * m_allocatedHalfSizeZ);

  std::unique_ptr<uint8_t[]> cpuTempData(new uint8_t[cpuTempDecodeSizeNeeded]);
  uint8_t* cpuTempBuffer = cpuTempData.get();

  bool isInteger = m_integerInfo & WAVELET_INTEGERINFO_ISINTEGER;
  WaveletAdaptiveLL_DecodeIterator decodeIterator = WaveletAdaptiveLLDecompress_CreateDecodeIterator(m_dataVersion, (uint8_t*)m_readCompressedData, floatReadWriteData, m_dimensions, m_allocatedSizeX, m_allocatedSizeY, m_allocatedSizeZ, *threshold, *startThreshold, m_transformMask, transformData, m_transformIterations,
      m_pixelSetChildren.get(), m_pixelSetChildrenCount, m_pixelSetPixelInSignificant.get(), m_pixelSetPixelInSignificantCount, nullptr, nullptr,
      m_allocatedHalfSizeX, m_allocatedHalfSizeX * m_allocatedHalfSizeY, cpuTempBuffer, m_allocatedHalfSizeX * m_allocatedHalfSizeY * m_allocatedHalfSizeZ, m_allocatedSizeX * m_allocatedSizeY * m_allocatedSizeZ, decompressLevel, isInteger);

  int size = WaveletAdaptiveLLDecompress_DecompressAdaptive(decodeIterator, decompressAdaptiveMode);
  (void)size;

  if (isLossless)
  {
    assert(nAdaptiveSize == size);
  }

  cpuTempData.reset();

  InverseTransform(floatReadWriteData);

  // Decompres no values?
  float localWaveletNoValue;

  std::vector<uint32_t> noValueBitBuffer;
  DecompressNoValues(&localWaveletNoValue, noValueBitBuffer);

  if (noValueBitBuffer.size())
  {
    ApplyNoValues(floatReadWriteData, noValueBitBuffer.data(), localWaveletNoValue);
  }

  if (isAnyNoValue) *isAnyNoValue = noValueBitBuffer.size() != 0;
  if (waveletNoValue) *waveletNoValue = localWaveletNoValue;

// DeNormalize?
  if (isNormalize)
  {
    assert(noValueBitBuffer.empty()); // NoValue & Normalization is mutually exclusive (for now).
    int32_t normalizeX = (m_dataBlockSizeX + NORMAL_BLOCK_SIZE - 1) / NORMAL_BLOCK_SIZE;
    int32_t normalizeY = (m_dataBlockSizeY + NORMAL_BLOCK_SIZE - 1) / NORMAL_BLOCK_SIZE;
    int32_t normalizeZ = (m_dataBlockSizeZ + NORMAL_BLOCK_SIZE - 1) / NORMAL_BLOCK_SIZE;

    for (int iZ = 0; iZ < m_dataBlockSizeZ; iZ++)
    {
      for (int iY = 0; iY < m_dataBlockSizeY; iY++)
      {
        for (int iX = 0; iX < m_dataBlockSizeX; iX++)
        {
          floatReadWriteData[iX + iY * m_allocatedSizeX + iZ * m_allocatedSizeXY] *= Wavelet_GetNormalizedValue(normalField, iX, iY, iZ, normalizeX, normalizeY, normalizeZ);
        }
      }
    }
  }

  //  Decompress Zeroes
  if (nDecompressZeroSize > 4) // if equal to 4, then do nothing, no zero runs. 4 is the minimum size stored
  {
    std::vector<uint8_t> buffer;
    buffer.resize(m_transformSizeY * m_transformSizeZ * int32_t(sizeof(unsigned short)));

    WaveletAdaptiveLLDecompress_DecompressZerosAlongX(pnDecompressZeroSize, (void*)floatReadWriteData, 4, 0.0f, m_transformSizeX, m_transformSizeY, m_transformSizeZ, m_allocatedSizeX, m_allocatedSizeY, m_allocatedSizeZ, buffer.data());
  }

  // Do lossless diff
  if (isLossless)
  {
    assert(dataBlockFormat == WaveletDataFormat::Format_R32);

    m_readCompressedData += (nAdaptiveSize + 3) / 4;

    int32_t size = WaveletAdaptiveLLDecompress_DecompressLossless((unsigned char*)m_readCompressedData, floatReadWriteData, m_transformSizeX, m_transformSizeY, m_transformSizeZ, m_allocatedSizeX, m_allocatedSizeXY);

    m_readCompressedData += (size + 3) / 4;
  }

  return true;
}

void WaveletDecompressor::DecompressNoValuesHeader()
{
  int size = *m_readCompressedData;

  if (size == -1)
  {
    m_readCompressedData++;
    m_noValueData = nullptr;
    return; // no NoValues
  }

  m_noValueData = m_readCompressedData;

  m_readCompressedData += 2; // size and no value
  m_readCompressedData += (size + 3) / 4;
}

#ifdef ENABLE_SSE_TRANSFORM

void WaveletDecompressor::InverseTransform(float *source)
{
  int32_t tempBufferSize = ((m_bandSize[0][0] + 3) & ~3) * (m_bandSize[0][1]) * (m_bandSize[0][2]);
  std::vector<float> tempBuffer(tempBufferSize);

  WaveletTransform_InverseTransform(tempBuffer.data(), tempBufferSize, source, m_transformIterations, m_bandSize, m_transformMask, m_allocatedSizeX, m_allocatedSizeXY, m_integerInfo);
}

#else

void WaveletDecompressor::InverseTransform(float *source)
{
  WaveletTransform_InverseTransform(source, m_transformIterations, m_bandSize, m_transformMask, m_allocatedSizeX, m_allocatedSizeXY, m_integerInfo);
}

#endif


void WaveletDecompressor::DecompressNoValues(float *noValue, std::vector<uint32_t> &buffer)
{
  if (!m_noValueData)
  {
    *noValue = 0.0f;
    buffer = std::vector<uint32_t>();
    return;
  }

  m_noValueData++; // Skip byte size.

  // Create a union for type punning that works with strict aliasing
  union { float fValue; int32_t iValue; } convert;

  convert.iValue = *m_noValueData++; *noValue = convert.fValue;

  buffer.resize((((m_transformSizeX + 31) & ~31) / 32) * m_transformSizeY * m_transformSizeZ);

  uint32_t *bitBuffer = buffer.data();

  int bitSizeX = (m_transformSizeX + 31) / 32;

  int intsToDecode = bitSizeX *m_transformSizeY *m_transformSizeZ;

  WaveletDecompress_RleDecode((uint8_t *)m_noValueData, bitBuffer, intsToDecode);
}

void WaveletDecompressor::ApplyNoValues(float *source, uint32_t *bitBuffer, float noValue)
{
  int32_t u32BitRead = 0;

  for (int iDim2 = 0; iDim2 < m_transformSizeZ; iDim2++)
  {
    for (int iDim1 = 0; iDim1 < m_transformSizeY; iDim1++)
    {
      float *write = source + iDim2 * m_allocatedSizeXY + iDim1 * m_allocatedSizeX;

      for (int iDim0 = 0; iDim0 < m_transformSizeX; iDim0+=32)
      {
        uint32_t values = bitBuffer[u32BitRead++];

        int samples = m_transformSizeX - iDim0;

        if (samples > 32) samples = 32;

        for (int iSample = 0; iSample < samples; iSample++)
        {
          if (values & (1 << iSample))
          {
            write[iSample + iDim0] = noValue;
          }
        }
      }
    }
  }
}

bool Wavelet_IsCompressedDataAllNoValue(const void *pxCompressedData, int nCompressedAdaptiveDataSize)
{
  assert(pxCompressedData);
  assert(nCompressedAdaptiveDataSize >= WAVELET_MIN_COMPRESSED_HEADER);

  if(nCompressedAdaptiveDataSize == WAVELET_MIN_COMPRESSED_HEADER)
  {
    return false;
  }

  const uint32_t *puCompressedData = static_cast<const uint32_t *>(pxCompressedData);

  const uint32_t *puReadCompressedData = puCompressedData + WAVELET_MIN_COMPRESSED_HEADER / sizeof(uint32_t);

  int nDecompressZeroSize = puReadCompressedData[0];

  puReadCompressedData += (nDecompressZeroSize + 3) / 4;

  int nNoValuesByteSize = puReadCompressedData[0];

  uint8_t *puRLEByte = (uint8_t *)(puReadCompressedData + 2);

  uint32_t uSetBits = 0;

  bool isSet = WaveletDecompress_RleDecodeOneRun(puRLEByte, uSetBits);

  // If the first run is set and there are no more runs, it means we only have NoValues
  if (isSet && (intptr_t)puRLEByte - (intptr_t)(puReadCompressedData + 2) == nNoValuesByteSize)
  {
    return true;
  }

  return false;
}

}
