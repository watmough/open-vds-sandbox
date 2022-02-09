/****************************************************************************
** Copyright 2022 The Open Group
** Copyright 2022 Bluware, Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**   http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
****************************************************************************/

#include "Util.h"

void
EncodeWaveletAdaptiveLevelsMetadata(uint8_t(&levels)[VDSWaveletAdaptiveLevelsChunkMetadata::WAVELET_ADAPTIVE_LEVELS], int totalSize, const int(&adaptiveLevelSizes)[VDSWaveletAdaptiveLevelsChunkMetadata::WAVELET_ADAPTIVE_LEVELS])
{
  int
    remainingSize = totalSize;

  memset(levels, 0, VDSWaveletAdaptiveLevelsChunkMetadata::WAVELET_ADAPTIVE_LEVELS);

  for (int level = 0; remainingSize > 0 && level < VDSWaveletAdaptiveLevelsChunkMetadata::WAVELET_ADAPTIVE_LEVELS; level++)
  {
    assert(adaptiveLevelSizes[level] <= remainingSize);

    levels[level] = (uint8_t)((uint64_t)adaptiveLevelSizes[level] * 255 + (remainingSize - 1)) / remainingSize;

    remainingSize = (int)((uint64_t)remainingSize * levels[level] / 255);
  }
}

void
DecodeWaveletAdaptiveLevelsMetadata(int(&adaptiveLevelSizes)[VDSWaveletAdaptiveLevelsChunkMetadata::WAVELET_ADAPTIVE_LEVELS], int totalSize, const uint8_t(&levels)[VDSWaveletAdaptiveLevelsChunkMetadata::WAVELET_ADAPTIVE_LEVELS])
{
  int
    remainingSize = totalSize;

  for (int level = 0; level < VDSWaveletAdaptiveLevelsChunkMetadata::WAVELET_ADAPTIVE_LEVELS; level++)
  {
    remainingSize = (int)((uint64_t)remainingSize * levels[level] / 255);

    adaptiveLevelSizes[level] = remainingSize;
  }
}

std::string
FormatMetadata(const void *metadata, int metadataLength, int fileType)
{
  if (metadataLength == 0) return "";

  std::stringstream
    out;

  if (fileType == FILETYPE_VDS_LAYER && metadataLength == sizeof(VDSLayerMetadata) || metadataLength == sizeof(VDSLayerMetadataWaveletAdaptive))
  {
    const VDSLayerMetadata *
      layerMetadata = static_cast<const VDSLayerMetadata *>(metadata);

    out << "{ ";
    out << "compressionMethod: ";

    switch (layerMetadata->m_compressionMethod)
    {
    case VDSLayerMetadata::COMPRESSIONMETHOD_NONE:    out << "None";    break;
    case VDSLayerMetadata::COMPRESSIONMETHOD_WAVELET: out << "Wavelet"; break;
    case VDSLayerMetadata::COMPRESSIONMETHOD_RLE:     out << "RLE";     break;
    case VDSLayerMetadata::COMPRESSIONMETHOD_ZIP:     out << "Zip";     break;
    case VDSLayerMetadata::COMPRESSIONMETHOD_WAVELET_NORMALIZE_BLOCK:
                                                      out << "WaveletNormalizeBlockExperimental";     break;
    case VDSLayerMetadata::COMPRESSIONMETHOD_WAVELET_LOSSLESS:
                                                      out << "WaveletLossless"; break;
    case VDSLayerMetadata::COMPRESSIONMETHOD_WAVELET_NORMALIZE_BLOCK_LOSSLESS:
                                                      out << "WaveletNormalizeBlockExperimentalLossless"; break;
    default:
      out << layerMetadata->m_compressionMethod;
    }
    out << ", ";

    out << "compressionTolerance: " << std::showpoint << layerMetadata->m_compressionTolerance << ", ";
    out << "validChunkCount: " << layerMetadata->m_validChunkCount;

    if (metadataLength == sizeof(VDSLayerMetadataWaveletAdaptive))
    {
      out << ", ";

      const VDSLayerMetadataWaveletAdaptive *
        layerMetadataWaveletAdaptive = static_cast<const VDSLayerMetadataWaveletAdaptive *>(metadata);

      out << "uncompressedSize: " << layerMetadataWaveletAdaptive->m_uncompressedSize << ", ";
      out << "adaptiveLevelSizes: ";
      for (int level = 0; level < VDSLayerMetadataWaveletAdaptive::WAVELET_ADAPTIVE_LEVELS; level++)
      {
        if (level != 0)
        {
          out << ", ";
        }

        out << layerMetadataWaveletAdaptive->m_adaptiveLevelSizes[level];
      }
    }

    out << " }";
  }
  else
  {
    const char
      *genericMetadata = static_cast<const char*>(metadata);

    out << "{ metadata: ";
    for (int i = 0; i < metadataLength; i++)
    {
      if (i != 0)
      {
        out << ", ";
      }

      out << "0x" << "0123456789abcdef"[(genericMetadata[i] & 0xf0) >> 4] << "0123456789abcdef"[genericMetadata[i] & 0x0f];
    }
    out << " }";
  }
  return out.str();
}


