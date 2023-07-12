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

#ifndef VDS_H
#define VDS_H

#include <OpenVDS/VolumeDataLayoutDescriptor.h>
#include <OpenVDS/VolumeDataAxisDescriptor.h>
#include <OpenVDS/VolumeDataChannelDescriptor.h>
#include <OpenVDS/MetadataContainer.h>
#include <OpenVDS/Vector.h>

#include "VDS/VolumeDataLayoutImpl.h"
#include "VDS/VolumeDataLayer.h"
#include "VDS/MetadataManager.h"
#include "VDS/VolumeDataAccessManagerImpl.h"
#include "VDS/VolumeDataRequestProcessor.h"
#include "VDS/VolumeDataStore.h"
#include "VDS/GlobalStateImpl.h"
#include "VDS/Logging.h"

#include <vector>
#include <map>
#include <memory>
#include <unordered_map>
#include <mutex>

namespace OpenVDS
{

class DescriptorStringContainer
{
  std::vector<std::unique_ptr<char[]>> m_descriptorStrings;
public:
  const char *Add(std::string const &descriptorString)
  {
    char *data = new char[descriptorString.size() + 1];
    memcpy(data, descriptorString.data(), descriptorString.size());
    data[descriptorString.size()] = 0;
    m_descriptorStrings.emplace_back(data);
    return data;
  }
};

void ReleaseVolumeDataAccessManager(VolumeDataAccessManagerImpl *);

class VDSMetadataContainer : public MetadataContainer
{
  mutable std::mutex m_mutex;
  bool m_dirty;

public:
  VDSMetadataContainer() : m_dirty(false) {}

  bool IsDirty() const { std::unique_lock<std::mutex> lock(m_mutex); return m_dirty; }
  void ClearDirtyFlag() { std::unique_lock<std::mutex> lock(m_mutex); m_dirty = false; }

  bool        IsMetadataIntAvailable(const char* category, const char* name)        const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::IsMetadataIntAvailable(category, name); }
  bool        IsMetadataIntVector2Available(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::IsMetadataIntVector2Available(category, name); }
  bool        IsMetadataIntVector3Available(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::IsMetadataIntVector3Available(category, name); }
  bool        IsMetadataIntVector4Available(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::IsMetadataIntVector4Available(category, name); }
  bool        IsMetadataFloatAvailable(const char* category, const char* name)        const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::IsMetadataFloatAvailable(category, name); }
  bool        IsMetadataFloatVector2Available(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::IsMetadataFloatVector2Available(category, name); }
  bool        IsMetadataFloatVector3Available(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::IsMetadataFloatVector3Available(category, name); }
  bool        IsMetadataFloatVector4Available(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::IsMetadataFloatVector4Available(category, name); }
  bool        IsMetadataDoubleAvailable(const char* category, const char* name)        const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::IsMetadataDoubleAvailable(category, name); }
  bool        IsMetadataDoubleVector2Available(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::IsMetadataDoubleVector2Available(category, name); }
  bool        IsMetadataDoubleVector3Available(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::IsMetadataDoubleVector3Available(category, name); }
  bool        IsMetadataDoubleVector4Available(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::IsMetadataDoubleVector4Available(category, name); }
  bool        IsMetadataStringAvailable(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::IsMetadataStringAvailable(category, name); }
  bool        IsMetadataBLOBAvailable(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::IsMetadataBLOBAvailable(category, name); }

  int         GetMetadataInt(const char* category, const char* name)        const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::GetMetadataInt(category, name); }
  IntVector2  GetMetadataIntVector2(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::GetMetadataIntVector2(category, name); }
  IntVector3  GetMetadataIntVector3(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::GetMetadataIntVector3(category, name); }
  IntVector4  GetMetadataIntVector4(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::GetMetadataIntVector4(category, name); }

  float        GetMetadataFloat(const char* category, const char* name)        const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::GetMetadataFloat(category, name); }
  FloatVector2 GetMetadataFloatVector2(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::GetMetadataFloatVector2(category, name); }
  FloatVector3 GetMetadataFloatVector3(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::GetMetadataFloatVector3(category, name); }
  FloatVector4 GetMetadataFloatVector4(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::GetMetadataFloatVector4(category, name); }

  double        GetMetadataDouble(const char* category, const char* name)        const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::GetMetadataDouble(category, name); }
  DoubleVector2 GetMetadataDoubleVector2(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::GetMetadataDoubleVector2(category, name); }
  DoubleVector3 GetMetadataDoubleVector3(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::GetMetadataDoubleVector3(category, name); }
  DoubleVector4 GetMetadataDoubleVector4(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::GetMetadataDoubleVector4(category, name); }

  const char* GetMetadataString(const char* category, const char* name) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::GetMetadataString(category, name); }
  void GetMetadataBLOB(const char* category, const char* name, const void **data, size_t *size) const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::GetMetadataBLOB(category, name, data, size); }

  MetadataKeyRange GetMetadataKeys() const override { std::unique_lock<std::mutex> lock(m_mutex); return MetadataContainer::GetMetadataKeys(); }

  void SetMetadataInt(const char* category, const char* name, int value)               override { std::unique_lock<std::mutex> lock(m_mutex); m_dirty = true; return MetadataContainer::SetMetadataInt(category, name, value); }
  void SetMetadataIntVector2(const char* category, const char* name, IntVector2 value) override { std::unique_lock<std::mutex> lock(m_mutex); m_dirty = true; return MetadataContainer::SetMetadataIntVector2(category, name, value); }
  void SetMetadataIntVector3(const char* category, const char* name, IntVector3 value) override { std::unique_lock<std::mutex> lock(m_mutex); m_dirty = true; return MetadataContainer::SetMetadataIntVector3(category, name, value); }
  void SetMetadataIntVector4(const char* category, const char* name, IntVector4 value) override { std::unique_lock<std::mutex> lock(m_mutex); m_dirty = true; return MetadataContainer::SetMetadataIntVector4(category, name, value); }

  void SetMetadataFloat(const char* category, const char* name, float value)               override { std::unique_lock<std::mutex> lock(m_mutex); m_dirty = true; return MetadataContainer::SetMetadataFloat(category, name, value); }
  void SetMetadataFloatVector2(const char* category, const char* name, FloatVector2 value) override { std::unique_lock<std::mutex> lock(m_mutex); m_dirty = true; return MetadataContainer::SetMetadataFloatVector2(category, name, value); }
  void SetMetadataFloatVector3(const char* category, const char* name, FloatVector3 value) override { std::unique_lock<std::mutex> lock(m_mutex); m_dirty = true; return MetadataContainer::SetMetadataFloatVector3(category, name, value); }
  void SetMetadataFloatVector4(const char* category, const char* name, FloatVector4 value) override { std::unique_lock<std::mutex> lock(m_mutex); m_dirty = true; return MetadataContainer::SetMetadataFloatVector4(category, name, value); }

  void SetMetadataDouble(const char* category, const char* name, double value)               override { std::unique_lock<std::mutex> lock(m_mutex); m_dirty = true; return MetadataContainer::SetMetadataDouble(category, name, value); }
  void SetMetadataDoubleVector2(const char* category, const char* name, DoubleVector2 value) override { std::unique_lock<std::mutex> lock(m_mutex); m_dirty = true; return MetadataContainer::SetMetadataDoubleVector2(category, name, value); }
  void SetMetadataDoubleVector3(const char* category, const char* name, DoubleVector3 value) override { std::unique_lock<std::mutex> lock(m_mutex); m_dirty = true; return MetadataContainer::SetMetadataDoubleVector3(category, name, value); }
  void SetMetadataDoubleVector4(const char* category, const char* name, DoubleVector4 value) override { std::unique_lock<std::mutex> lock(m_mutex); m_dirty = true; return MetadataContainer::SetMetadataDoubleVector4(category, name, value); }

  void SetMetadataString(const char* category, const char* name, const char* value) override { std::unique_lock<std::mutex> lock(m_mutex); m_dirty = true; return MetadataContainer::SetMetadataString(category, name, value); }
  void SetMetadataBLOB(const char* category, const char* name, const void *data, size_t size) override { std::unique_lock<std::mutex> lock(m_mutex); m_dirty = true; return MetadataContainer::SetMetadataBLOB(category, name, data, size); }

  void CopyMetadata(const char* category, MetadataReadAccess const *metadataReadAccess) override
  {
    std::unique_lock<std::mutex> lock(m_mutex);

    for (auto &key : metadataReadAccess->GetMetadataKeys())
    {
      if (strcmp(key.GetCategory(), category) == 0)
      {
        switch(key.GetType())
        {
        case MetadataType::Int:
          MetadataContainer::SetMetadataInt(key.GetCategory(), key.GetName(), metadataReadAccess->GetMetadataInt(key.GetCategory(), key.GetName()));
          break;
        case MetadataType::IntVector2:
          MetadataContainer::SetMetadataIntVector2(key.GetCategory(), key.GetName(), metadataReadAccess->GetMetadataIntVector2(key.GetCategory(), key.GetName()));
          break;
        case MetadataType::IntVector3:
          MetadataContainer::SetMetadataIntVector3(key.GetCategory(), key.GetName(), metadataReadAccess->GetMetadataIntVector3(key.GetCategory(), key.GetName()));
          break;
        case MetadataType::IntVector4:
          MetadataContainer::SetMetadataIntVector4(key.GetCategory(), key.GetName(), metadataReadAccess->GetMetadataIntVector4(key.GetCategory(), key.GetName()));
          break;
        case MetadataType::Float:
          MetadataContainer::SetMetadataFloat(key.GetCategory(), key.GetName(), metadataReadAccess->GetMetadataFloat(key.GetCategory(), key.GetName()));
          break;
        case MetadataType::FloatVector2:
          MetadataContainer::SetMetadataFloatVector2(key.GetCategory(), key.GetName(), metadataReadAccess->GetMetadataFloatVector2(key.GetCategory(), key.GetName()));
          break;
        case MetadataType::FloatVector3:
          MetadataContainer::SetMetadataFloatVector3(key.GetCategory(), key.GetName(), metadataReadAccess->GetMetadataFloatVector3(key.GetCategory(), key.GetName()));
          break;
        case MetadataType::FloatVector4:
          MetadataContainer::SetMetadataFloatVector4(key.GetCategory(), key.GetName(), metadataReadAccess->GetMetadataFloatVector4(key.GetCategory(), key.GetName()));
          break;
        case MetadataType::Double:
          MetadataContainer::SetMetadataDouble(key.GetCategory(), key.GetName(), metadataReadAccess->GetMetadataDouble(key.GetCategory(), key.GetName()));
          break;
        case MetadataType::DoubleVector2:
          MetadataContainer::SetMetadataDoubleVector2(key.GetCategory(), key.GetName(), metadataReadAccess->GetMetadataDoubleVector2(key.GetCategory(), key.GetName()));
          break;
        case MetadataType::DoubleVector3:
          MetadataContainer::SetMetadataDoubleVector3(key.GetCategory(), key.GetName(), metadataReadAccess->GetMetadataDoubleVector3(key.GetCategory(), key.GetName()));
          break;
        case MetadataType::DoubleVector4:
          MetadataContainer::SetMetadataDoubleVector4(key.GetCategory(), key.GetName(), metadataReadAccess->GetMetadataDoubleVector4(key.GetCategory(), key.GetName()));
          break;
        case MetadataType::String:
          MetadataContainer::SetMetadataString(key.GetCategory(), key.GetName(), metadataReadAccess->GetMetadataString(key.GetCategory(), key.GetName()));
          break;
        case MetadataType::BLOB:
          std::vector<uint8_t> data;
          metadataReadAccess->GetMetadataBLOB(key.GetCategory(), key.GetName(), data);
          MetadataContainer::SetMetadataBLOB(key.GetCategory(), key.GetName(), data.data(), data.size());
          break;
        }
      }
    }
  }

  void ClearMetadata(const char* category, const char* name) override { std::unique_lock<std::mutex> lock(m_mutex); m_dirty = true; return MetadataContainer::ClearMetadata(category, name); }
  void ClearMetadata(const char* category) override { std::unique_lock<std::mutex> lock(m_mutex); m_dirty = true; return MetadataContainer::ClearMetadata(category); }
};

struct VDS
{
  VDS(int requestThreadCount, LogLevel level)
    : requestThreadCount(requestThreadCount)
    , logger(static_cast<GlobalStateImpl*>(OpenVDS::GetGlobalState())->logInterface, level)
  {}

  VolumeDataLayoutDescriptor
                    layoutDescriptor;

  std::vector<VolumeDataAxisDescriptor>
                    axisDescriptors;

  std::vector<VolumeDataChannelDescriptor>
                    channelDescriptors;

  DescriptorStringContainer
                    descriptorStrings;

  std::vector<VolumeDataLayer::ProduceStatus>
                    produceStatuses;

  VDSMetadataContainer
                    metadataContainer;

  std::unique_ptr<VolumeDataLayoutImpl>
                    volumeDataLayout;
  std::shared_ptr<VolumeDataAccessManagerImpl>
                    accessManager;
  std::unique_ptr<VolumeDataStore>
                    volumeDataStore;
  int               requestThreadCount;
  Logger            logger;
};

void CreateVolumeDataLayout(VDS &handle, CompressionMethod compressionMethod = CompressionMethod::None, float compressionTolerance = 0);

std::string GetLayerName(VolumeDataLayer const &volumeDataLayer);

}

#endif //VDS_H
