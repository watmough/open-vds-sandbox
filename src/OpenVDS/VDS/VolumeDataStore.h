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

#ifndef VOLUMEDATASTORE_H
#define VOLUMEDATASTORE_H

#include <OpenVDS/VolumeDataChannelDescriptor.h>
#include <OpenVDS/VolumeData.h>
#include <OpenVDS/OpenVDS.h>
#include <OpenVDS/VolumeDataAccess.h>

#include "MetadataManager.h"
#include "DataBlock.h"
#include "VolumeDataHash.h"
#include "ParsedMetadata.h"
#include "GlobalStateImpl.h"
#include "LayerMetadataContainer.h"
#include "Logging.h"

#include <vector>
#include <map>
#include <condition_variable>
#include <functional>

namespace OpenVDS
{

class CompressionInfo
{
  CompressionMethod   compressionMethod;
  float               tolerance;
  int                 adaptiveLevel;
public:
  CompressionInfo() : compressionMethod(CompressionMethod::None), tolerance(0.0f), adaptiveLevel(0) {}
  CompressionInfo(CompressionMethod compressionMethod, float tolerance, int adaptiveLevel) : compressionMethod(compressionMethod), tolerance(tolerance), adaptiveLevel(adaptiveLevel) {}

  CompressionMethod   GetCompressionMethod() const { return compressionMethod; }
  float               GetTolerance()         const { return tolerance; }
  int                 GetAdaptiveLevel()     const { return adaptiveLevel; }
};

class VolumeDataStore
{
public:
           VolumeDataStore(OpenOptions::ConnectionType connectionType, Logger &logger);
  virtual ~VolumeDataStore();

  virtual CompressionInfo
                        GetEffectiveAdaptiveLevel(VolumeDataLayer* volumeDataLayer, WaveletAdaptiveMode waveletAdaptiveMode, float tolerance, float ratio) = 0;
          bool          PrepareReadChunk(const VolumeDataChunk &volumeDataChunk, int adaptiveLevel, Error &error);
  virtual bool          PrepareReadChunkImpl(const VolumeDataChunk &volumeDataChunk, int adaptiveLevel, Error &error) = 0;
          bool          ReadChunk(const VolumeDataChunk& chunk, int adaptiveLevel, std::vector<uint8_t>& serializedData, std::vector<uint8_t>& metadata, CompressionInfo& compressionInfo, Error& error);
  virtual bool          ReadChunkImpl(const VolumeDataChunk& chunk, int adaptiveLevel, std::vector<uint8_t>& serializedData, std::vector<uint8_t>& metadata, CompressionInfo& compressionInfo, Error& error) = 0;
          bool          CancelReadChunk(const VolumeDataChunk& chunk, Error& error);
  virtual bool          CancelReadChunkImpl(const VolumeDataChunk& chunk, Error& error) = 0;
  virtual bool          ReadChunkDataHash(const VolumeDataChunk& chunk, uint64_t &chunkDataHash, Error& error) = 0;
          bool          WriteChunk(const VolumeDataChunk& chunk, const std::vector<uint8_t>& serializedData, const std::vector<uint8_t>& metadata);
  virtual bool          WriteChunkImpl(const VolumeDataChunk& chunk, std::shared_ptr<std::vector<uint8_t>> &serializedData, const std::vector<uint8_t>& metadata, std::function<void(const Error &error)> completed) = 0;
  virtual void          Flush(Error &error) = 0;
  virtual bool          ReadSerializedVolumeDataLayout(std::vector<uint8_t>& serializedVolumeDataLayout, Error &error) = 0;
  virtual bool          WriteSerializedVolumeDataLayout(const std::vector<uint8_t>& serializedVolumeDataLayout, Error &error) = 0;
  virtual bool          SerializeAndWriteLayerStatus(Error& error) = 0;
  virtual bool          AddLayer(VolumeDataLayer* volumeDataLayer, int chunkMetadataPageSize, bool overwriteExisting) = 0;
  virtual bool          RemoveLayer(VolumeDataLayer* volumeDataLayer) = 0;
  virtual bool          Close(Error &error) = 0;
  virtual bool          EnableWriting(Error& error) = 0;

  virtual std::function<bool(std::string const& channelName, bool isPrimary)>
                        IsChannelZipped() const = 0;

  bool DeserializeVolumeData(const VolumeDataChunk &volumeDataChunk, const std::vector<uint8_t>& serializedData, const std::vector<uint8_t>& metadata, CompressionMethod compressionMethod, int32_t adaptiveLevel, VolumeDataChannelDescriptor::Format loadFormat, DataBlock &dataBlock, std::vector<uint8_t>& target, uint64_t& targetHash, Error& error);

  static bool Verify(const VolumeDataChunk& volumeDataChunk, const std::vector<uint8_t>& serializedData, CompressionMethod compressionMethod, bool isFullyRead);
  static bool CreateConstantValueDataBlock(VolumeDataChunk const &volumeDataChunk, VolumeDataChannelDescriptor::Format format, float noValue, VolumeDataChannelDescriptor::Components components, VolumeDataHash const &constantValueVolumeDataHash, DataBlock &dataBlock, std::vector<uint8_t> &buffer, Error &error);
  static uint64_t
              SerializeVolumeData(const VolumeDataChunk& chunk, const DataBlock &dataBlock, const std::vector<uint8_t>& chunkData, CompressionMethod compressionMethod, float compressionTolerance, std::vector<uint8_t>& destinationBuffer);
  static bool IsCompressionMethodSupported(CompressionMethod compressionMethod);

protected:
  GlobalStateVds        m_globalStateVds;
  struct StorageRequest
  {
    VolumeDataChunk chunk;
    int adaptiveLevel;
    int refCount;
    bool hasData;
    bool settingData;
    std::shared_ptr<std::vector<uint8_t>> data;
    std::vector<uint8_t> metadata;
    CompressionInfo compressionInfo;
    Error error;
  };
  std::mutex m_mutex;
  std::condition_variable m_wait;
  std::vector<std::unique_ptr<StorageRequest>> m_requests;
  Logger m_logger;
};

}

#endif //VOLUMEDATASTORE_H
