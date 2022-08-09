/****************************************************************************
** Copyright 2019 The Open Group
** Copyright 2019 Bluware, Inc.
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

#ifndef __EMSCRIPTEN__
#include "VolumeDataStore.h"

#include "VolumeDataLayoutImpl.h"

#include "VolumeDataHash.h"
#include "ParsedMetadata.h"
#include <OpenVDS/ValueConversion.h>
#include <VDS/VDS.h>
#include <VDS/GlobalStateImpl.h>

#include <fmt/format.h>
#endif

#include "WaveletDecompress.h"
#include "WaveletTypes.h"
#include "Rle.h"
#include "DataBlock.h"

#include <stdlib.h>
#include <assert.h>

#include <zlib.h>

namespace OpenVDS
{

static uint32_t GetByteSize(const DataBlockDescriptor &descriptor)
{
  int32_t size[DataBlock::Dimensionality_Max];
  size[0] = descriptor.SizeX;
  size[1] = descriptor.SizeY;
  size[2] = descriptor.SizeZ;
  size[3] = 1;
  return GetByteSize(size, descriptor.Format, descriptor.Components);
}

static void CopyLinearBufferIntoDataBlock(const void *sourceBuffer, const DataBlock &dataBlock, std::vector<uint8_t> &targetBuffer)
{

  int32_t sizeX = dataBlock.Size[0];
  int32_t sizeY = dataBlock.Size[1];
  int32_t sizeZ = dataBlock.Size[2];

  int32_t allocatedSizeX = dataBlock.AllocatedSize[0];
  int32_t allocatedSizeY = dataBlock.AllocatedSize[1];

  if(dataBlock.Format == VolumeDataChannelDescriptor::Format_1Bit)
  {
    sizeX = ((sizeX * dataBlock.Components) + 7) / 8;
  }

  uint32_t elementSize = GetElementSize(dataBlock);

  for(int32_t iZ = 0; iZ < sizeZ; iZ++)
  {
    for(int32_t iY = 0; iY < sizeY; iY++)
    {
      uint8_t *target = targetBuffer.data()                             +  (iZ * allocatedSizeY + iY) * allocatedSizeX * elementSize;
      const uint8_t *source = static_cast<const uint8_t*>(sourceBuffer) +  (iZ * sizeY          + iY) * sizeX          * elementSize;
      memcpy(target, source, size_t(sizeX) * elementSize);
    }
  }
}

inline static DataBlock ToDataBlock(const Wavelet::WaveletDataBlock &waveletDataBlock)
{
  DataBlock dataBlock;
  dataBlock.Format = (VolumeDataFormat)waveletDataBlock.Format;
  dataBlock.Components = VolumeDataChannelDescriptor::Components_1;
  dataBlock.Dimensionality = (enum DataBlock::Dimensionality)waveletDataBlock.Dimensionality;
  memcpy(dataBlock.Size, waveletDataBlock.Size, sizeof(waveletDataBlock.Size));
  memcpy(dataBlock.AllocatedSize, waveletDataBlock.AllocatedSize, sizeof(waveletDataBlock.Size));
  memcpy(dataBlock.Pitch, waveletDataBlock.Pitch, sizeof(waveletDataBlock.Size));
  return dataBlock;
}

bool DeserializeVolumeData(const std::vector<uint8_t> &serializedData, VolumeDataChannelDescriptor::Format format, CompressionMethod compressionMethod, const FloatRange &valueRange, float integerScale, float integerOffset, bool isUseNoValue, float noValue, int32_t adaptiveLevel, DataBlock &dataBlock, std::vector<uint8_t> &destination, Error &error)
{
  if(CompressionMethod_IsWavelet(compressionMethod))
  {
    const void *data = serializedData.data();

    int32_t dataVersion = ((int32_t *)data)[0];
    (void)dataVersion;
    assert(dataVersion >= WAVELET_DATA_VERSION_1_4 && dataVersion <= WAVELET_DATA_VERSION_1_6);

    bool isNormalize = false;
    bool isLossless = false;

    if((compressionMethod == CompressionMethod::WaveletNormalizeBlock) ||
       (compressionMethod == CompressionMethod::WaveletNormalizeBlockLossless))
    {
      isNormalize = true;
    }

    if((compressionMethod == CompressionMethod::WaveletLossless) ||
       (compressionMethod == CompressionMethod::WaveletNormalizeBlockLossless))
    {
      isLossless = true;
    }

    // if adaptive level is 0 or more, don't use lossless data
    if (adaptiveLevel >= 0)
    {
      isLossless = false;
    }
    else
    {
      assert(adaptiveLevel == -1);
      // Now we can use lossless if there is lossless data - set iAdaptivelevel to max adaptive.
      adaptiveLevel = 0;
    }

    Wavelet::WaveletDataFormat waveletDataFormat = (Wavelet::WaveletDataFormat)format;
    Wavelet::FloatRange waveletValueRange(valueRange.Min, valueRange.Max);
    Wavelet::WaveletDataBlock waveletDataBlock;
    
    if (!Wavelet_Decompress(const_cast<void*>(data), int32_t(serializedData.size()), waveletDataFormat, waveletValueRange, integerScale, integerOffset, isUseNoValue, noValue, isNormalize, adaptiveLevel, isLossless, waveletDataBlock, destination, error.code, error.string))
      return false;

    // Copy updated WaveletDataBlock to in/out DataBlock parameter
    dataBlock = ToDataBlock(waveletDataBlock);
  }
  else if(compressionMethod == CompressionMethod::RLE)
  {
    DataBlockDescriptor *dataBlockDescriptor = (DataBlockDescriptor *)serializedData.data();

    if(!dataBlockDescriptor->IsValid())
    {
      error.code = -1;
      error.string = "Failed to decode DataBlockDescriptor";
      return false;
    }

    if (!InitializeDataBlock(*dataBlockDescriptor, dataBlock, error))
      return false;

    void * source = dataBlockDescriptor + 1;

    int32_t byteSize = GetByteSize(*dataBlockDescriptor);
    std::unique_ptr<uint8_t[]>buffer(new uint8_t[byteSize]);

    int32_t decompressedSize = RleDecompress((uint8_t *)buffer.get(), byteSize, (uint8_t *)source);
    (void)decompressedSize;
    assert(decompressedSize == byteSize);

    int allocatedSize = GetAllocatedByteSize(dataBlock);
    destination.resize(allocatedSize);
    CopyLinearBufferIntoDataBlock(buffer.get(), dataBlock, destination);
  }
  else if(compressionMethod == CompressionMethod::Zip)
  {
    DataBlockDescriptor *dataBlockDescriptor = (DataBlockDescriptor *)serializedData.data();

    if(!dataBlockDescriptor->IsValid())
    {
      error.code = -1;
      error.string = "Failed to decode DataBlockDescriptor";
      return false;
    }
    if (!InitializeDataBlock(*dataBlockDescriptor, dataBlock, error))
      return false;

    void * source = dataBlockDescriptor + 1;

    int32_t byteSize = GetByteSize(*dataBlockDescriptor);
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[byteSize]);

    unsigned long destLen = byteSize;

    int status = uncompress(buffer.get(), &destLen, (uint8_t *)source, uint32_t(serializedData.size() - sizeof(DataBlockDescriptor)));

    if (status != Z_OK)
    {
      error.code = -1;
      error.string = fmt::format("zlib uncompress failed (status {})", status);
      return false;
    }

    int allocatedSize = GetAllocatedByteSize(dataBlock);
    destination.resize(allocatedSize);
    CopyLinearBufferIntoDataBlock(buffer.get(), dataBlock, destination);
  }
  else if(compressionMethod == CompressionMethod::None)
  {
    DataBlockDescriptor *dataBlockDescriptor = (DataBlockDescriptor *)serializedData.data();

    if(!dataBlockDescriptor->IsValid())
    {
      error.code = -1;
      error.string = "Failed to decode DataBlockDescriptor";
      return false;
    }
    if (!InitializeDataBlock(*dataBlockDescriptor, dataBlock, error))
      return false;

    void * source = dataBlockDescriptor + 1;

    size_t sourceDataBlockSize = serializedData.size() - sizeof(*dataBlockDescriptor);
    size_t requiredDataBlockSize = size_t(GetByteSize(*dataBlockDescriptor));

    if (sourceDataBlockSize != requiredDataBlockSize)
    {
      error.string = "Required size of uncompressed chunk is not present. Possible data corruptions.";
      error.code = -1;
      return false;
    }

    int32_t byteSize = GetAllocatedByteSize(dataBlock);
    destination.resize(byteSize);
    CopyLinearBufferIntoDataBlock(source, dataBlock, destination);
  }

  if(dataBlock.Format != format)
  {
    error.string = "Formats doesn't match in deserialization\n";
    error.code = -1;
    return false;
  }
  return true;
}

#ifndef __EMSCRIPTEN__
VolumeDataStore::VolumeDataStore(OpenOptions::ConnectionType connectionType)
  : m_globalStateVds(static_cast<GlobalStateImpl *>(GetGlobalState())->downloaded[connectionType],
                     static_cast<GlobalStateImpl *>(GetGlobalState())->downloadedChunks[connectionType],
                     static_cast<GlobalStateImpl *>(GetGlobalState())->decompressed[connectionType],
                     static_cast<GlobalStateImpl *>(GetGlobalState())->decompressedChunks[connectionType])
{
}

VolumeDataStore::~VolumeDataStore()
{
  if (m_requests.size())
    fputs(fmt::format("VolumeDataStore request cache is not empty {}\n", m_requests.size()).c_str(), stderr);
}

bool
VolumeDataStore::PrepareReadChunk(const VolumeDataChunk& volumeDataChunk, int adaptiveLevel, Error& error)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  auto it = std::find_if(m_requests.begin(), m_requests.end(), [volumeDataChunk, adaptiveLevel](const std::unique_ptr<StorageRequest>& request)
    {
      return request->chunk == volumeDataChunk && request->adaptiveLevel <= adaptiveLevel;
    });
  if (it == m_requests.end())
  {
    m_requests.emplace_back(new StorageRequest());
    auto& r = m_requests.back();
    r->chunk = volumeDataChunk;
    r->adaptiveLevel = adaptiveLevel;
    auto ret = PrepareReadChunkImpl(volumeDataChunk, adaptiveLevel, error);
    r->data = std::make_shared<std::vector<uint8_t>>();
    r->error = error;
    r->hasData = false;
    r->settingData = false;
    r->refCount++;
    return ret;
  }
  auto r = it->get();
  r->refCount++;
  error = r->error;
  return r->error.code == 0;
}
          
bool
VolumeDataStore::ReadChunk(const VolumeDataChunk& chunk, int adaptiveLevel, std::vector<uint8_t>& serializedData, std::vector<uint8_t>& metadata, CompressionInfo& compressionInfo, Error& error)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  auto it = std::find_if(m_requests.begin(), m_requests.end(), [chunk, adaptiveLevel](const std::unique_ptr<StorageRequest>& request)
    {
      return request->chunk == chunk && request->adaptiveLevel <= adaptiveLevel;
    });
  if (it == m_requests.end())
  {
    error.string = fmt::format("Unexpected error in ReadChunk. No prepare request was found for chunk {}", chunk.index);
    error.code = 1;
    return false;
  }
  auto r = it->get();
  if (!r->hasData)
  {
    if (!r->settingData)
    {
      r->settingData = true;
      lock.unlock();
      ReadChunkImpl(chunk, r->adaptiveLevel, *r->data, r->metadata, r->compressionInfo, r->error);
      lock.lock();
      r->hasData = true;
    }
    else
    {
      m_wait.wait(lock, [r] { return r->hasData; });
    }
  }
  r->refCount--;
  if (r->refCount == 0)
  {
    serializedData = std::move(*r->data);
    metadata = std::move(r->metadata);
    compressionInfo = std::move(r->compressionInfo);
    error = std::move(r->error);
    auto it = std::find_if(m_requests.begin(), m_requests.end(), [chunk, adaptiveLevel](const std::unique_ptr<StorageRequest>& request)
      {
        return request->chunk == chunk && request->adaptiveLevel <= adaptiveLevel;
      });
    m_requests.erase(it);
  }
  else
  {
    m_wait.notify_all();
    serializedData = *r->data;
    metadata = r->metadata;
    compressionInfo = r->compressionInfo;
    error = r->error;
  }
  return error.code == 0;
}
          
bool
VolumeDataStore::CancelReadChunk(const VolumeDataChunk& chunk, Error& error)
{
  //there is a leak in the api that adaptive level is not specified.
  std::unique_lock<std::mutex> lock(m_mutex);
  auto it = std::find_if(m_requests.begin(), m_requests.end(), [chunk](const std::unique_ptr<StorageRequest>& request)
    {
      return request->chunk == chunk;
    });
  if (it == m_requests.end())
  {
    error.string = fmt::format("Unexpected error in CancelReadChunk. No prepare request was found for chunk {}", chunk.index);
    error.code = 1;
    return false;
  }
  auto r = it->get();
  r->refCount--;
  if (r->refCount == 0)
  {
    if (!r->hasData && !r->settingData)
    {
      CancelReadChunkImpl(chunk, error);
    }
    m_requests.erase(it);
    return error.code == 0;
  }
  else
  {
    m_wait.notify_all();
  }
  return true;
}

bool
VolumeDataStore::WriteChunk(const VolumeDataChunk& chunk, const std::vector<uint8_t>& serializedData, const std::vector<uint8_t>& metadata)
{
  std::unique_lock<std::mutex> lock(m_mutex);
  auto it = std::find_if(m_requests.begin(), m_requests.end(), [chunk](const std::unique_ptr<StorageRequest>& request)
    {
      return request->chunk == chunk;
    });
  StorageRequest* r = nullptr;;
  if (it == m_requests.end())
  {
    m_requests.emplace_back(new StorageRequest());
    r = m_requests.back().get();
    r->refCount = 1;
  }
  else
  {
    r = it->get();
    r->refCount++;
    m_wait.wait(lock, [r] {return r->refCount == 1; });
  }

  r->chunk = chunk;
  r->adaptiveLevel = -1;
  r->error = Error();
  r->hasData = true;
  r->settingData = false;
  r->data = std::make_shared<std::vector<uint8_t>>(serializedData);
  r->metadata = metadata;
  r->compressionInfo = CompressionInfo(CompressionMethod(chunk.layer->GetEffectiveCompressionMethod()), chunk.layer->GetEffectiveCompressionTolerance(), 0);

  lock.unlock();
  return WriteChunkImpl(chunk, r->data, r->metadata, [this, chunk](const Error& error)
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      auto it = std::find_if(m_requests.begin(), m_requests.end(), [chunk](const std::unique_ptr<StorageRequest>& request)
        {
          return request->chunk == chunk;
        });
      if (it == m_requests.end())
        throw std::runtime_error("Missing chunk in write completed callback");

      auto r = it->get();
      r->refCount--;
      if (r->refCount == 0)
      {
        m_requests.erase(it);
      }
      else
      {
        m_wait.notify_all();
      }
    });
}

bool VolumeDataStore::Verify(const VolumeDataChunk &volumeDataChunk, const std::vector<uint8_t> &serializedData, CompressionMethod compressionMethod, bool isFullyRead)
{
  bool isValid = false;

  int32_t voxelSize[DataBlock::Dimensionality_Max];

  volumeDataChunk.layer->GetChunkVoxelSize(volumeDataChunk.index, voxelSize);

  if(serializedData.empty())
  {
    isValid = true;
  }
  else if(CompressionMethod_IsWavelet(compressionMethod))
  {
    if(serializedData.size() >= sizeof(int32_t) * 6)
    {
      const int32_t *waveletHeader = (const int32_t *)serializedData.data();

      int32_t dataVersion    = waveletHeader[0];
      int32_t compressedSize = waveletHeader[1];
      int32_t createSizeX    = waveletHeader[2];
      int32_t createSizeY    = waveletHeader[3];
      int32_t createSizeZ    = waveletHeader[4];
      int32_t dimensions     = waveletHeader[5] & 0xff;

      isValid = dataVersion >= WAVELET_DATA_VERSION_1_4 &&
                dataVersion <= WAVELET_DATA_VERSION_1_6 &&
                (compressedSize <= int32_t(serializedData.size()) || !isFullyRead) &&
                (createSizeX == voxelSize[0]                  ) &&
                (createSizeY == voxelSize[1] || dimensions < 2) &&
                (createSizeZ == voxelSize[2] || dimensions < 3) &&
                dimensions >= 1 &&
                dimensions <= 3;
    }
  }
  else if(compressionMethod == CompressionMethod::None ||
          compressionMethod == CompressionMethod::RLE ||
          compressionMethod == CompressionMethod::Zip)
  {
    if(serializedData.size() > sizeof(DataBlockDescriptor))
    {
      DataBlockDescriptor *serializedDescriptor = (DataBlockDescriptor*)serializedData.data();
      isValid = serializedDescriptor->IsValid(voxelSize);
    }
  }

  return isValid;
}

static bool CopyDataBlockIntoLinearBuffer(const DataBlock &dataBlock, const void *sourceBuffer, void *targetBuffer, int32_t bufferSize)
{
  int32_t size[DataBlock::Dimensionality_Max];
  memcpy(size, dataBlock.Size, sizeof(size));
  int32_t allocatedSize[DataBlock::Dimensionality_Max];
  memcpy(allocatedSize, dataBlock.AllocatedSize, sizeof(allocatedSize));

  int32_t elementSize = int32_t(GetElementSize(dataBlock));

  if(dataBlock.Format == VolumeDataChannelDescriptor::Format_1Bit)
  {
    size[0] = ((size[0] * dataBlock.Components) + 7) / 8;
  }

  // Check if first row is constant
  bool isConstant = true;
  switch(dataBlock.Format)
  {
  default:
    assert(0 && "Illegal format");
    return true;
  case VolumeDataChannelDescriptor::Format_1Bit:
    isConstant = (reinterpret_cast<const uint8_t*>(sourceBuffer)[0] == 0x00 || reinterpret_cast<const uint8_t*>(sourceBuffer)[0] == 0xff);
    // Fall through
  case VolumeDataChannelDescriptor::Format_U8:
    for(int32_t iX = 1; isConstant && iX < size[0]; iX++)
    {
      isConstant = reinterpret_cast<const uint8_t *>(sourceBuffer)[0] == reinterpret_cast<const uint8_t *>(sourceBuffer)[iX];
    }
    break;
  case VolumeDataChannelDescriptor::Format_U16:
    for(int32_t iX = 1; isConstant && iX < size[0]; iX++)
    {
      isConstant = reinterpret_cast<const uint16_t *>(sourceBuffer)[0] == reinterpret_cast<const uint16_t *>(sourceBuffer)[iX];
    }
    break;

  case VolumeDataChannelDescriptor::Format_R32:
  case VolumeDataChannelDescriptor::Format_U32:
    for(int32_t iX = 1; isConstant && iX < size[0]; iX++)
    {
      isConstant = reinterpret_cast<const uint32_t *>(sourceBuffer)[0] == reinterpret_cast<const uint32_t *>(sourceBuffer)[iX];
    }
    break;

  case VolumeDataChannelDescriptor::Format_U64:
  case VolumeDataChannelDescriptor::Format_R64:
    for(int32_t iX = 1; isConstant && iX < size[0]; iX++)
    {
      isConstant = reinterpret_cast<const uint64_t *>(sourceBuffer)[0] == reinterpret_cast<const uint64_t *>(sourceBuffer)[iX];
    }
    break;
  }

  assert(bufferSize >= size[0] * size[1] * size[2] * elementSize);

  for(int32_t iZ = 0; iZ < size[2]; iZ++)
  {
    for(int32_t iY = 0; iY < size[1]; iY++)
    {
      uint8_t *puSource = (uint8_t *)sourceBuffer;
      uint8_t *puTarget = (uint8_t *)targetBuffer;

      puSource += (iZ * allocatedSize[1] + iY) * allocatedSize[0] * elementSize;
      puTarget += (iZ * size[1]          + iY) * size[0]          * elementSize;

      if(isConstant)
      {
        isConstant = (memcmp(sourceBuffer, puSource, size[0] * elementSize) == 0);
      }

      memcpy(puTarget, puSource, size[0] * elementSize);
    }
  }

  return isConstant;
}

template<typename T>
static uint64_t GetConstantValueVolumeDataHash(T value, const Range<float> &valueRange, float integerScale, float integerOffset, bool isUseNoValue, float noValue)
{
  if (isUseNoValue)
  {
    if(value == ConvertNoValue<T>(noValue))
    {
      return VolumeDataHash::NOVALUE;
    }
    else
    {
      QuantizingValueConverterWithNoValue<float, T, true> converter(valueRange.Min, valueRange.Max, integerScale, integerOffset, noValue, noValue, false);
      return VolumeDataHash(converter.ConvertValue(value)).CalculateHash();
    }
  }
  else
  {
    QuantizingValueConverterWithNoValue<float, T, false> converter(valueRange.Min, valueRange.Max, integerScale, integerOffset, noValue, noValue, false);
    return VolumeDataHash(converter.ConvertValue(value)).CalculateHash();
  }
}

static uint64_t GetConstantValueVolumeDataHash(const DataBlock  &dataBlock, const uint8_t *sourceBuffer, const Range<float> &valueRange, float integerScale, float integerOffset, bool isUseNoValue, float noValue)
{
  switch(dataBlock.Format)
  {
  case VolumeDataChannelDescriptor::Format_1Bit:
    assert(reinterpret_cast<const uint8_t *>(sourceBuffer)[0] == 0x00 || reinterpret_cast<const uint8_t *>(sourceBuffer)[0] == 0xff);
    return VolumeDataHash((float)(reinterpret_cast<const uint8_t *>(sourceBuffer)[0] != 0)).CalculateHash();
  case VolumeDataChannelDescriptor::Format_U8:  return GetConstantValueVolumeDataHash(reinterpret_cast<const uint8_t  *>(sourceBuffer)[0], valueRange, integerScale, integerOffset, isUseNoValue, noValue);
  case VolumeDataChannelDescriptor::Format_U16: return GetConstantValueVolumeDataHash(reinterpret_cast<const uint16_t *>(sourceBuffer)[0], valueRange, integerScale, integerOffset, isUseNoValue, noValue);
  case VolumeDataChannelDescriptor::Format_R32: return GetConstantValueVolumeDataHash(reinterpret_cast<const float *>(sourceBuffer)[0], valueRange, integerScale, integerOffset, isUseNoValue, noValue);
  case VolumeDataChannelDescriptor::Format_U32: return GetConstantValueVolumeDataHash(reinterpret_cast<const uint32_t *>(sourceBuffer)[0], valueRange, integerScale, integerOffset, isUseNoValue, noValue);
  case VolumeDataChannelDescriptor::Format_R64: return GetConstantValueVolumeDataHash(reinterpret_cast<const double *>(sourceBuffer)[0], valueRange, integerScale, integerOffset, isUseNoValue, noValue);
  case VolumeDataChannelDescriptor::Format_U64: return GetConstantValueVolumeDataHash(reinterpret_cast<const uint64_t *>(sourceBuffer)[0], valueRange, integerScale, integerOffset, isUseNoValue, noValue);

  default:
    assert(0 && "Unknown format");
    return VolumeDataHash::UNKNOWN;
  }
}

static int64_t GetSerializationTargetBufferSize(int64_t sourceSize, CompressionMethod compressionMethod)
{
  if(CompressionMethod_IsWavelet(compressionMethod))
  {
    return std::max(int64_t(128*128*128*2), sourceSize) * 2 * 2;
  }
  else if(compressionMethod == CompressionMethod::RLE)
  {
    return sizeof(DataBlockDescriptor) + sizeof(RLEHeader) + sourceSize * 2;
  }
  else if(compressionMethod == CompressionMethod::Zip)
  {
    return sizeof(DataBlockDescriptor) + compressBound((uLong)sourceSize);
  }
  else
  {
    assert(compressionMethod == CompressionMethod::None);
    return sizeof(DataBlockDescriptor) + sourceSize;
  }
}

static float GetConvertedConstantValue(VolumeDataChannelDescriptor const &volumeDataChannelDescriptor, VolumeDataChannelDescriptor::Format format, float noValue, VolumeDataHash const &constantValueVolumeDataHash)
{
  if(format == VolumeDataChannelDescriptor::Format_1Bit)
  {
    return float(constantValueVolumeDataHash.GetConstantValue(0) != 0);
  }
  else if(format == VolumeDataChannelDescriptor::Format_U8 || format == VolumeDataChannelDescriptor::Format_U16)
  {
    if(constantValueVolumeDataHash.IsNoValue())
    {
      if(format == VolumeDataChannelDescriptor::Format_U8)
      {
        return ConvertNoValue<uint8_t>(noValue);
      }
      else
      {
        assert(format == VolumeDataChannelDescriptor::Format_U16);
        return ConvertNoValue<uint16_t>(noValue);
      }
    }

    int32_t buckets = (format == VolumeDataChannelDescriptor::Format_U8 ? 256 : 65536) - volumeDataChannelDescriptor.IsUseNoValue();

    float reciprocalScale;
    float offset;

    if(volumeDataChannelDescriptor.GetFormat() != VolumeDataChannelDescriptor::Format_U8 && volumeDataChannelDescriptor.GetFormat() != VolumeDataChannelDescriptor::Format_U16)
    {
      offset = volumeDataChannelDescriptor.GetValueRange().Min;
      reciprocalScale = float(buckets - 1) / rangeSize(volumeDataChannelDescriptor.GetValueRange());
    }
    else
    {
      offset = volumeDataChannelDescriptor.GetIntegerOffset();
      reciprocalScale = 1.0f / volumeDataChannelDescriptor.GetIntegerScale();
    }

    return (float)QuantizeValueWithReciprocalScale(constantValueVolumeDataHash.GetConstantValue(noValue), offset, reciprocalScale, buckets);
  }
  else
  {
    assert(format == VolumeDataChannelDescriptor::Format_R32 || format == VolumeDataChannelDescriptor::Format_U32 || format == VolumeDataChannelDescriptor::Format_R64 || format == VolumeDataChannelDescriptor::Format_U64);
    return constantValueVolumeDataHash.GetConstantValue(noValue);
  }
}

template <typename T>
static void FillConstantValueBuffer(uint8_t *buffer, int32_t allocatedElements, float value)
{
  T v = ConvertValue<T>(value);
  T *b = reinterpret_cast<T *>(buffer);
  for(int32_t element = 0; element < allocatedElements; element++)
  {
    b[element] = v;
  }
}

static bool FillConstantValueBuffer(uint8_t* buffer, int32_t allocatedElements, VolumeDataChannelDescriptor::Format format, float value, Error &error)
{
  VolumeDataChannelDescriptor::Format effectiveFormat = format;

  // Use U8 format fill methods for 1-bit
  if(format == VolumeDataChannelDescriptor::Format_1Bit)
  {
    effectiveFormat = VolumeDataChannelDescriptor::Format_U8;
    value = ConvertValue<bool>(value) ? 255.0f : 0.0f;
  }

  switch (effectiveFormat)
  {
  default:
    error.code = -1;
    error.string = "Invalid format in createConstantValuedataBlock";
    return false;
  case VolumeDataChannelDescriptor::Format_U8:  FillConstantValueBuffer<uint8_t>(buffer, allocatedElements, value); break;
  case VolumeDataChannelDescriptor::Format_U16: FillConstantValueBuffer<uint16_t>(buffer, allocatedElements, value); break;
  case VolumeDataChannelDescriptor::Format_R32: FillConstantValueBuffer<float>(buffer, allocatedElements, value); break;
  case VolumeDataChannelDescriptor::Format_U32: FillConstantValueBuffer<uint32_t>(buffer, allocatedElements, value); break;
  case VolumeDataChannelDescriptor::Format_R64: FillConstantValueBuffer<double>(buffer, allocatedElements, value); break;
  case VolumeDataChannelDescriptor::Format_U64: FillConstantValueBuffer<uint64_t>(buffer, allocatedElements, value); break;
  }
  return true;
}

bool VolumeDataStore::CreateConstantValueDataBlock(VolumeDataChunk const &volumeDataChunk, VolumeDataChannelDescriptor::Format format, float noValue, VolumeDataChannelDescriptor::Components components, VolumeDataHash const &constantValueVolumeDataHash, DataBlock &dataBlock, std::vector<uint8_t> &buffer, Error &error)
{
  int32_t size[4];
  volumeDataChunk.layer->GetChunkVoxelSize(volumeDataChunk.index, size);
  int32_t dimensionality = volumeDataChunk.layer->GetChunkDimensionality();
  if (!InitializeDataBlock(format, components, (enum DataBlock::Dimensionality)(dimensionality), size, dataBlock, error))
    return false;

 
  int32_t allocatedSize = GetAllocatedByteSize(dataBlock);
  buffer.resize(allocatedSize);

  int32_t allocatedElements = dataBlock.AllocatedSize[0] * dataBlock.AllocatedSize[1] * dataBlock.AllocatedSize[2] * dataBlock.AllocatedSize[3] * dataBlock.Components;

  float convertedConstantValue = GetConvertedConstantValue(volumeDataChunk.layer->GetVolumeDataChannelDescriptor(), format, noValue, constantValueVolumeDataHash);

  assert(dataBlock.Format == format);

  return FillConstantValueBuffer(buffer.data(), allocatedElements, format, convertedConstantValue, error);
}

bool VolumeDataStore::DeserializeVolumeData(const VolumeDataChunk& volumeDataChunk, const std::vector<uint8_t>& serializedData, const std::vector<uint8_t>& metadata, CompressionMethod compressionMethod, int32_t adaptiveLevel, VolumeDataChannelDescriptor::Format loadFormat, DataBlock& dataBlock, std::vector<uint8_t>& target, uint64_t& targetHash, Error& error)
{
  targetHash = VolumeDataHash::UNKNOWN;

  bool waveletAdaptive = CompressionMethod_IsWavelet(compressionMethod) && metadata.size() == sizeof(uint64_t) + sizeof(uint8_t[WAVELET_ADAPTIVE_LEVELS]);
  if (!waveletAdaptive && metadata.size() != sizeof(uint64_t))
  {
    error.code = -1;
    error.string = fmt::format("Invalid metadata of size {} for chunk: {}/{}", metadata.size(), GetLayerName(*volumeDataChunk.layer), volumeDataChunk.index);
    return false;
  }

  memcpy(&targetHash, metadata.data(), sizeof(uint64_t));

  VolumeDataHash volumeDataHash(targetHash);

  VolumeDataLayer const *volumeDataLayer = volumeDataChunk.layer;

  if (volumeDataHash.IsConstant())
  {
    return CreateConstantValueDataBlock(volumeDataChunk, volumeDataLayer->GetFormat(), volumeDataLayer->GetNoValue(), volumeDataLayer->GetComponents(), volumeDataHash, dataBlock, target, error);
  }
  else if(serializedData.empty())
  {
    error.code = -1;
    error.string = fmt::format("Missing data for chunk: {}/{}", GetLayerName(*volumeDataChunk.layer), volumeDataChunk.index);
    return false;
  }

  if (!Verify(volumeDataChunk, serializedData, compressionMethod, !waveletAdaptive))
  {
    error.code = -1;
    error.string = fmt::format("Invalid header (e.g. unsupported Wavelet compression version) for chunk: {}/{}", GetLayerName(*volumeDataChunk.layer), volumeDataChunk.index);
    return false;
  }

  targetHash = uint64_t(volumeDataHash) ^ (uint64_t(adaptiveLevel) + 1) * 0x4068934683409867ULL; // It is intentional that an iAdaptiveLevel of -1 produces the original hash

  //create a value range from scale and offset so that conversion to 8 or 16 bit is done correctly inside deserialization
  FloatRange deserializeValueRange = volumeDataLayer->GetValueRange();

  if (volumeDataLayer->GetFormat() == VolumeDataChannelDescriptor::Format_U16 || volumeDataLayer->GetFormat() == VolumeDataChannelDescriptor::Format_U8)
  {
    if (loadFormat == VolumeDataChannelDescriptor::Format_U16)
    {
      deserializeValueRange.Min = volumeDataLayer->GetIntegerOffset();
      deserializeValueRange.Max = volumeDataLayer->GetIntegerScale() * (volumeDataLayer->IsUseNoValue() ? 65534.0f : 65535.0f) + volumeDataLayer->GetIntegerOffset();
    }
    else if (loadFormat == VolumeDataChannelDescriptor::Format_U8 && volumeDataLayer->GetFormat() != VolumeDataChannelDescriptor::Format_U16)
    {
      deserializeValueRange.Min = volumeDataLayer->GetIntegerOffset();
      deserializeValueRange.Max = volumeDataLayer->GetIntegerScale() * (volumeDataLayer->IsUseNoValue() ? 254.0f : 255.0f) + volumeDataLayer->GetIntegerOffset();
    }
  }

  bool ret = OpenVDS::DeserializeVolumeData(serializedData, loadFormat, compressionMethod, deserializeValueRange, volumeDataLayer->GetIntegerScale(), volumeDataLayer->GetIntegerOffset(), volumeDataLayer->IsUseNoValue(), volumeDataLayer->GetNoValue(), adaptiveLevel, dataBlock, target, error);
  m_globalStateVds.addDecompressed(target.size());
  return ret;
}

static void SerializeRLEFromBuffer(const DataBlock &dataBlock, const DataBlockDescriptor &dataBlockHeader, void *targetBuffer, uint64_t &targetBufferSize, int32_t sourceSize, void *sourceBuffer) 
{
  // Placement new constructs the descriptor in the buffer
  int32_t nElementSize = GetElementSize(dataBlock);

  // Check for unsupported element sizes for RLE encoding
  if(nElementSize != 1 && nElementSize != 2 && nElementSize != 4 && nElementSize != 8)
  {
    nElementSize /= dataBlock.Components;
    assert(nElementSize == 1 || nElementSize == 2 || nElementSize == 4 || nElementSize == 8);
  }

  int32_t nCompressedSize = RleCompress((uint8_t*)targetBuffer, (int32_t)targetBufferSize, (uint8_t *)sourceBuffer, sourceSize, nElementSize);

  targetBufferSize = (int64_t)(sizeof(DataBlockDescriptor) + nCompressedSize);
}

struct ShrinkToSizeOnExit
{
  ShrinkToSizeOnExit(std::vector<uint8_t>& to_shrink)
    : to_shrink(to_shrink)
  {}
  ~ShrinkToSizeOnExit()
  {
    to_shrink.shrink_to_fit();
  }
  std::vector<uint8_t> to_shrink;
};

uint64_t
VolumeDataStore::SerializeVolumeData(const VolumeDataChunk& chunk, const DataBlock& dataBlock, const std::vector<uint8_t>& chunkData, CompressionMethod compressionMethod, float, std::vector<uint8_t>& destinationBuffer)
{
  DataBlockDescriptor dataBlockHeader;
  dataBlockHeader.Components = dataBlock.Components;
  dataBlockHeader.Dimensionality = dataBlock.Dimensionality;
  dataBlockHeader.Format = dataBlock.Format;
  dataBlockHeader.SizeX = dataBlock.Size[0];
  dataBlockHeader.SizeY = dataBlock.Size[1];
  dataBlockHeader.SizeZ = dataBlock.Size[2];

  if (compressionMethod == CompressionMethod::None)
  {
    destinationBuffer.resize(GetByteSize(dataBlock) + sizeof(DataBlockDescriptor));
  }
  else
  {
    destinationBuffer.resize(size_t(GetSerializationTargetBufferSize(int64_t(GetAllocatedByteSize(dataBlock)), compressionMethod)));
  }

  ShrinkToSizeOnExit autoShrinker(destinationBuffer);

  switch (compressionMethod)
  {
  case CompressionMethod::None:
  {
    void *targetBuffer = destinationBuffer.data();
    memcpy(targetBuffer, &dataBlockHeader, sizeof(dataBlockHeader));
    targetBuffer = ((uint8_t *)targetBuffer) + sizeof(dataBlockHeader);

    bool isConstant = CopyDataBlockIntoLinearBuffer(dataBlock, chunkData.data(), targetBuffer, int32_t(destinationBuffer.size() - sizeof(dataBlockHeader)));

    if(isConstant)
    {
      destinationBuffer.resize(0);
      auto& layer = *chunk.layer;
      return GetConstantValueVolumeDataHash(dataBlock, (const uint8_t *) targetBuffer, layer.GetValueRange(), layer.GetIntegerScale(), layer.GetIntegerOffset(), layer.IsUseNoValue(), layer.GetNoValue());
    }
    break;
  }
  case CompressionMethod::Zip:
  case CompressionMethod::RLE:
  {
    uint32_t tmpbuffersize = GetByteSize(dataBlock);
    std::unique_ptr<uint8_t[]> tmpdata(new uint8_t[tmpbuffersize]);
    bool isConstant = CopyDataBlockIntoLinearBuffer(dataBlock, chunkData.data(), tmpdata.get(), tmpbuffersize);

    if (isConstant)
    {
      destinationBuffer.resize(0);
      auto& layer = *chunk.layer;
      return GetConstantValueVolumeDataHash(dataBlock, (const uint8_t *) tmpdata.get(), layer.GetValueRange(), layer.GetIntegerScale(), layer.GetIntegerOffset(), layer.IsUseNoValue(), layer.GetNoValue());
    }
    void *targetBuffer = destinationBuffer.data();
    memcpy(targetBuffer, &dataBlockHeader, sizeof(dataBlockHeader));
    targetBuffer = ((uint8_t *)targetBuffer) + sizeof(dataBlockHeader);

    if (compressionMethod == CompressionMethod::Zip)
    {
      unsigned long compressedSize = (unsigned long)destinationBuffer.size();
      int status = compress((uint8_t*)targetBuffer, &compressedSize, tmpdata.get(), tmpbuffersize);
      destinationBuffer.resize(compressedSize + sizeof(dataBlockHeader));

      if (status != Z_OK)
      {
        throw std::runtime_error("zlib compression failed");
      }
    }
    else
    {
      uint64_t compressedSize = (uint64_t) destinationBuffer.size();
      SerializeRLEFromBuffer(dataBlock, dataBlockHeader, targetBuffer, compressedSize, tmpbuffersize, tmpdata.get());
      destinationBuffer.resize(compressedSize);
    }

    break;
  }
  default:
    throw std::runtime_error("Invalid compression method specified when serializing a VolumeDataChunk");
  }
  return VolumeDataHash::UNKNOWN;
}

bool
VolumeDataStore::IsCompressionMethodSupported(CompressionMethod compressionMethod)
{
  return !CompressionMethod_IsWavelet(compressionMethod);
}
#endif

}

#ifdef __EMSCRIPTEN__

#include <array>

inline std::array<int32_t, 4> DataBlock_getArrayProperty(const int32_t (&values)[4])
{
  return std::array<int32_t, 4>{values[0], values[1], values[2], values[3]};
}

inline void DataBlock_setArrayProperty(int32_t (&destination)[4], const std::array<int32_t, 4> &source)
{
  std::copy(source.begin(), source.end(), destination);
}

std::array<int32_t, 4> DataBlock_getSize(const OpenVDS::DataBlock &dataBlock) { return DataBlock_getArrayProperty(dataBlock.Size); }
void DataBlock_setSize(OpenVDS::DataBlock &dataBlock, const std::array<int32_t, 4> &size) { DataBlock_setArrayProperty(dataBlock.Size, size); }

std::array<int32_t, 4> DataBlock_getAllocatedSize(const OpenVDS::DataBlock &dataBlock) { return DataBlock_getArrayProperty(dataBlock.AllocatedSize); }
void DataBlock_setAllocatedSize(OpenVDS::DataBlock &dataBlock, const std::array<int32_t, 4> &size) { DataBlock_setArrayProperty(dataBlock.AllocatedSize, size); }

std::array<int32_t, 4> DataBlock_getPitch(const OpenVDS::DataBlock &dataBlock) { return DataBlock_getArrayProperty(dataBlock.Pitch); }
void DataBlock_setPitch(OpenVDS::DataBlock &dataBlock, const std::array<int32_t, 4> &pitch) { DataBlock_setArrayProperty(dataBlock.Pitch, pitch); }

size_t vector_u8_data(std::vector<uint8_t> &vec)
{
  return (size_t)vec.data();
}

#include <emscripten/bind.h>
EMSCRIPTEN_BINDINGS(module)
{
  emscripten::enum_<OpenVDS::VolumeDataChannelDescriptor::Format>("Format")
    .value("Any",   OpenVDS::VolumeDataChannelDescriptor::Format_Any)
    .value("_1Bit", OpenVDS::VolumeDataChannelDescriptor::Format_1Bit)
    .value("U8",    OpenVDS::VolumeDataChannelDescriptor::Format_U8)
    .value("U16",   OpenVDS::VolumeDataChannelDescriptor::Format_U16)
    .value("R32",   OpenVDS::VolumeDataChannelDescriptor::Format_R32)
    .value("U32",   OpenVDS::VolumeDataChannelDescriptor::Format_U32)
    .value("R64",   OpenVDS::VolumeDataChannelDescriptor::Format_R64)
    .value("U64",   OpenVDS::VolumeDataChannelDescriptor::Format_U64);

  emscripten::enum_<OpenVDS::VolumeDataChannelDescriptor::Components>("Components")
    .value("_1",    OpenVDS::VolumeDataChannelDescriptor::Components_1)
    .value("_2",    OpenVDS::VolumeDataChannelDescriptor::Components_2)
    .value("_4",    OpenVDS::VolumeDataChannelDescriptor::Components_4);

  emscripten::enum_<enum OpenVDS::DataBlock::Dimensionality>("Dimensionality")
    .value("_1",    OpenVDS::DataBlock::Dimensionality_1)
    .value("_2",    OpenVDS::DataBlock::Dimensionality_2)
    .value("_3",    OpenVDS::DataBlock::Dimensionality_3)
    .value("_4",    OpenVDS::DataBlock::Dimensionality_4);

  emscripten::enum_<OpenVDS::CompressionMethod>("CompressionMethod")
    .value("None",                          OpenVDS::CompressionMethod::None)
    .value("Wavelet",                       OpenVDS::CompressionMethod::Wavelet)
    .value("RLE",                           OpenVDS::CompressionMethod::RLE)
    .value("Zip",                           OpenVDS::CompressionMethod::Zip)
    .value("WaveletNormalizeBlock",         OpenVDS::CompressionMethod::WaveletNormalizeBlock)
    .value("WaveletLossless",               OpenVDS::CompressionMethod::WaveletLossless)
    .value("WaveletNormalizeBlockLossless", OpenVDS::CompressionMethod::WaveletNormalizeBlockLossless);

  emscripten::class_<OpenVDS::FloatRange>("FloatRange")
    .constructor<>()
    .constructor<float, float>()
    .property("Min", &OpenVDS::FloatRange::Min)
    .property("Max", &OpenVDS::FloatRange::Max);

  emscripten::value_array<std::array<int32_t, 4>>("i32x4")
    .element(emscripten::index<0>())
    .element(emscripten::index<1>())
    .element(emscripten::index<2>())
    .element(emscripten::index<3>());

  emscripten::class_<OpenVDS::DataBlock>("DataBlock")
    .constructor<>()
    .property("Format",          &OpenVDS::DataBlock::Format)
    .property("Components",      &OpenVDS::DataBlock::Components)
    .property("Dimensionality",  &OpenVDS::DataBlock::Dimensionality)
    .property("Size",            &DataBlock_getSize,          &DataBlock_setSize)
    .property("AllocatedSize",   &DataBlock_getAllocatedSize, &DataBlock_setAllocatedSize)
    .property("Pitch",           &DataBlock_getPitch,         &DataBlock_setPitch);

  emscripten::class_<OpenVDS::Error>("Error")
    .constructor<>()
    .property("code",   &OpenVDS::Error::code)
    .property("string", &OpenVDS::Error::string);

  emscripten::function("DeserializeVolumeData", &OpenVDS::DeserializeVolumeData);

  emscripten::register_vector<uint8_t>("vector_u8")
    .function("data", &vector_u8_data);
}
#endif
