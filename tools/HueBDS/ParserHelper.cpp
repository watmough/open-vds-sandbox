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


#include "ParserHelper.h"


static bool
MatchToken(const char **string, const char *token)
{
  size_t tokenLength = strlen(token);

  if (strncmp(*string, token, tokenLength) == 0)
  {
    *string += tokenLength;
    *string += strspn(*string, " \t\r\n");
    return true;
  }

  return false;
}

static bool
MatchInteger(const char **string, int *result, int base)
{
  char *end;

  *result = strtol(*string, &end, base);

  if (end != *string)
  {
    *string = end;
    *string += strspn(*string, " \t\r\n");
    return true;
  }

  return false;
}

static bool
MatchInteger64(const char **string, int64_t *result, int base)
{
  char *end;

  *result = strtoll(*string, &end, base);

  if (end != *string)
  {
    *string = end;
    *string += strspn(*string, " \t\r\n");
    return true;
  }

  return false;
}

static bool
MatchFloat(const char **string, float *result)
{
  char *end;

#if __cplusplus >= 201103L || _MSC_VER >= 1800
  * result = strtof_l(*string, &end, C_locale);
#else
  *result = (float)strtod_l(*string, &end, C_locale);
#endif

  if (end != *string)
  {
    *string = end;
    *string += strspn(*string, " \t\r\n");
    return true;
  }

  return false;
}

bool
ParseMetadata(void *metadata, int *metadataLength, const char *metadataBuffer, int fileType)
{
  const char *
    currentToken = metadataBuffer;

  // Match start of metadata
  if (!MatchToken(&currentToken, "{"))
  {
    return false;
  }

  if (fileType == FILETYPE_VDS_LAYER)
  {
    bool
      isWaveletAdaptive = false;

    VDSLayerMetadataWaveletAdaptive
      layerMetadata;

    memset(&layerMetadata, 0, sizeof(VDSLayerMetadataWaveletAdaptive));

    while (*currentToken && *currentToken != '}')
    {
      if (MatchToken(&currentToken, "compressionMethod:"))
      {
        if (MatchToken(&currentToken, "None"))
        {
          layerMetadata.m_compressionMethod = VDSLayerMetadata::COMPRESSIONMETHOD_NONE;
        }
        else if (MatchToken(&currentToken, "Wavelet"))
        {
          layerMetadata.m_compressionMethod = VDSLayerMetadata::COMPRESSIONMETHOD_WAVELET;
        }
        else if (MatchToken(&currentToken, "RLE"))
        {
          layerMetadata.m_compressionMethod = VDSLayerMetadata::COMPRESSIONMETHOD_RLE;
        }
        else if (MatchToken(&currentToken, "Zip"))
        {
          layerMetadata.m_compressionMethod = VDSLayerMetadata::COMPRESSIONMETHOD_ZIP;
        }
        else if (MatchToken(&currentToken, "WaveletNormalizeBlockExperimental"))
        {
          layerMetadata.m_compressionMethod = VDSLayerMetadata::COMPRESSIONMETHOD_WAVELET_NORMALIZE_BLOCK;
        }
        else if (MatchToken(&currentToken, "WaveletLossless"))
        {
          layerMetadata.m_compressionMethod = VDSLayerMetadata::COMPRESSIONMETHOD_WAVELET_LOSSLESS;
        }
        else if (MatchToken(&currentToken, "WaveletNormalizeBlockExperimentalLossless"))
        {
          layerMetadata.m_compressionMethod = VDSLayerMetadata::COMPRESSIONMETHOD_WAVELET_NORMALIZE_BLOCK_LOSSLESS;
        }
        else if (!MatchInteger(&currentToken, &layerMetadata.m_compressionMethod))
        {
          return false;
        }
      }
      else if (MatchToken(&currentToken, "compressionTolerance:"))
      {
        if (!MatchFloat(&currentToken, &layerMetadata.m_compressionTolerance))
        {
          return false;
        }
      }
      else if (MatchToken(&currentToken, "validChunkCount:"))
      {
        if (!MatchInteger(&currentToken, &layerMetadata.m_validChunkCount))
        {
          return false;
        }
      }
      else if (MatchToken(&currentToken, "uncompressedSize:"))
      {
        isWaveletAdaptive = true;

        if (!MatchInteger64(&currentToken, &layerMetadata.m_uncompressedSize))
        {
          return false;
        }
      }
      else if (MatchToken(&currentToken, "adaptiveLevelSizes:"))
      {
        for (int level = 0; level < VDSLayerMetadataWaveletAdaptive::WAVELET_ADAPTIVE_LEVELS; level++)
        {
          if (!MatchInteger64(&currentToken, &layerMetadata.m_adaptiveLevelSizes[level]))
          {
            return false;
          }

          if (!MatchToken(&currentToken, ","))
          {
            break;
          }
        }
      }
      else
      {
        // Unknown property
        return false;
      }

      if (!MatchToken(&currentToken, ","))
      {
        break;
      }
    }

    // Match end of metadata
    if (!MatchToken(&currentToken, "}"))
    {
      return false;
    }

    int
      vdsLayerMetadataLength = isWaveletAdaptive ? (int)sizeof(VDSLayerMetadataWaveletAdaptive) : (int)sizeof(VDSLayerMetadata);

    if (metadata)
    {
      if (*metadataLength != vdsLayerMetadataLength)
      {
        return false;
      }

      memcpy(metadata, &layerMetadata, vdsLayerMetadataLength);
    }
    else
    {
      *metadataLength = vdsLayerMetadataLength;
    }
  }
  else
  {
    std::vector<unsigned char>
      genericMetadata;

    if (MatchToken(&currentToken, "metadata:"))
    {
      while (true)
      {
        int value;

        if (!MatchInteger(&currentToken, &value, 16))
        {
          return false;
        }

        if (value < 0 || value > 255)
        {
          return false;
        }

        genericMetadata.push_back(value);

        if (!MatchToken(&currentToken, ","))
        {
          break;
        }
      }
    }
    else
    {
      // Unknown property
      return false;
    }

    // Match end of metadata
    if (!MatchToken(&currentToken, "}"))
    {
      return false;
    }

    if (metadata)
    {
      if (*metadataLength != genericMetadata.size())
      {
        return false;
      }

      memcpy(metadata, &genericMetadata[0], genericMetadata.size());
    }
    else
    {
      *metadataLength = (int)genericMetadata.size();
    }
  }

  return true;
}
