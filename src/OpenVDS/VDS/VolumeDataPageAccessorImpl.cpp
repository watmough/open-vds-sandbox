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

#include "VolumeDataPageAccessorImpl.h"
#include "VolumeDataLayoutImpl.h"
#include "VolumeDataChannelMapping.h"
#include "VolumeDataAccessManagerImpl.h"
#include "VolumeDataLayer.h"
#include "VolumeDataPageImpl.h"
#include "VolumeDataStore.h"
#include "MetadataManager.h"
#include "Logging.h"

#include <IO/IOManager.h>

#include "DimensionGroup.h"

#include <chrono>
#include <algorithm>

#include <fmt/printf.h>

namespace OpenVDS
{

VolumeDataPageAccessorImpl::VolumeDataPageAccessorImpl(VolumeDataAccessManagerImpl* accessManager, VolumeDataPageAccessorImpl *parentVolumeDataPageAccessor, VolumeDataLayer const* layer, int maxPages, AccessMode accessMode, Logger &logger)
  : m_accessManager(accessManager)
  , m_parentVolumeDataPageAccessor(parentVolumeDataPageAccessor)
  , m_layer(layer)
  , m_pagesFound(0)
  , m_pagesRead(0)
  , m_pagesWritten(0)
  , m_maxPages(maxPages)
  , m_references(1)
  , m_accessMode(accessMode)
  , m_isCommitInProgress(false)
  , m_isLayerWriteLocked(false)
  , m_lastUsed(std::chrono::steady_clock::now())
  , m_logger(logger)
{
}

VolumeDataPageAccessorImpl::~VolumeDataPageAccessorImpl()
{
  for (auto& page : m_pages)
  {
    delete page;
  }

  if(m_parentVolumeDataPageAccessor)
  {
    if(m_parentVolumeDataPageAccessor->RemoveReference() == 0)
    {
      delete m_parentVolumeDataPageAccessor;
    }
  }
}

void
VolumeDataPageAccessorImpl::Invalidate()
{
  std::unique_lock<std::mutex> pageListMutexLock(m_pagesMutex);

  m_maxPages = 0;
  m_layer = nullptr;
  LimitPageListSize(0, pageListMutexLock);
}

VolumeDataLayout const* VolumeDataPageAccessorImpl::GetLayout() const
{
  return m_layer->GetLayout();
}

int VolumeDataPageAccessorImpl::GetLOD() const
{
  return m_layer->GetLOD();
}

int VolumeDataPageAccessorImpl::GetChannelIndex() const
{
  return m_layer->GetChannelIndex();
}

VolumeDataChannelDescriptor VolumeDataPageAccessorImpl::GetChannelDescriptor() const
{
  return m_layer->GetVolumeDataChannelDescriptor();
}

void  VolumeDataPageAccessorImpl::GetNumSamples(int(&numSamples)[Dimensionality_Max]) const
{
  for (int i = 0; i < Dimensionality_Max; i++)
  {
    numSamples[i] = m_layer->GetDimensionNumSamples(i);
  }
}

int64_t VolumeDataPageAccessorImpl::GetChunkCount() const
{
  return m_layer->GetTotalChunkCount();
}

void  VolumeDataPageAccessorImpl::GetChunkMinMax(int64_t chunk, int(&min)[Dimensionality_Max], int(&max)[Dimensionality_Max]) const
{
  m_layer->GetChunkMinMax(chunk, min, max, true);
}

void  VolumeDataPageAccessorImpl::GetChunkMinMaxExcludingMargin(int64_t chunk, int(&min)[Dimensionality_Max], int(&max)[Dimensionality_Max]) const
{
  m_layer->GetChunkMinMax(chunk, min, max, false);
}

uint64_t VolumeDataPageAccessorImpl::GetChunkVolumeDataHash(int64_t chunkIndex) const
{
  if (chunkIndex < 0 || chunkIndex > m_layer->GetTotalChunkCount())
  {
    throw InvalidArgument("The requested chunk doesn't exist", "chunkIndex");
  }

  uint64_t
    chunkDataHash = VolumeDataHash::UNKNOWN;

  std::unique_lock<std::mutex> pageListMutexLock(m_pagesMutex);

  auto page_it = std::find_if(m_pages.begin(), m_pages.end(), [chunkIndex](VolumeDataPageImpl const *page) { return page->GetChunkIndex() == chunkIndex; });
  if(page_it != m_pages.end())
  {
    return (*page_it)->GetVolumeDataHash();
  }
  pageListMutexLock.unlock();

  if(m_layer->GetProduceStatus() == VolumeDataLayer::ProduceStatus_Normal)
  {
    Error error;
    m_accessManager->GetVolumeDataStore()->ReadChunkDataHash(m_layer->GetChunkFromIndex(chunkIndex), chunkDataHash, error);
  }

  return chunkDataHash;
}

int64_t VolumeDataPageAccessorImpl::GetChunkIndex(const int(&position)[Dimensionality_Max]) const
{
  int32_t index_array[Dimensionality_Max];
  for (int i = 0; i < Dimensionality_Max; i++)
  {
    index_array[i] = m_layer->VoxelToIndex(position[i], i);
  }
  return m_layer->IndexArrayToChunkIndex(index_array);
}

int64_t VolumeDataPageAccessorImpl::GetMappedChunkIndex(int64_t primaryChannelChunkIndex) const
{
  auto volumeDataChannelMapping = m_layer->GetVolumeDataChannelMapping();
  if(volumeDataChannelMapping)
  {
    return volumeDataChannelMapping->GetMappedChunkIndex(m_layer->GetPrimaryChannelLayer(), primaryChannelChunkIndex);
  }
  else
  {
    return primaryChannelChunkIndex;
  }
}

int64_t VolumeDataPageAccessorImpl::GetPrimaryChannelChunkIndex(int64_t chunkIndex) const
{
  auto volumeDataChannelMapping = m_layer->GetVolumeDataChannelMapping();

  if(volumeDataChannelMapping)
  {
    return volumeDataChannelMapping->GetPrimaryChunkIndex(m_layer->GetPrimaryChannelLayer(), chunkIndex);
  }
  else
  {
    return chunkIndex;
  }
}

int VolumeDataPageAccessorImpl::AddReference()
{
  return ++m_references;
}

int VolumeDataPageAccessorImpl::RemoveReference()
{
  return --m_references;
}

int VolumeDataPageAccessorImpl::GetMaxPages()
{
  std::unique_lock<std::mutex> pageListMutexLock(m_pagesMutex);
  return m_maxPages;
}

void VolumeDataPageAccessorImpl::SetMaxPages(int maxPages)
{
  std::unique_lock<std::mutex> pageListMutexLock(m_pagesMutex);
  m_maxPages = maxPages;
  LimitPageListSize(m_maxPages, pageListMutexLock);
}

VolumeDataPage* VolumeDataPageAccessorImpl::CreatePage(int64_t chunk)
{
  std::unique_lock<std::mutex> pageListMutexLock(m_pagesMutex);

  if(!m_layer)
  {
    return nullptr;
  }

  // Wait for commit to finish before inserting a new page
  while(m_isCommitInProgress)
  {
    m_commitFinishedCondition.wait_for(pageListMutexLock, std::chrono::milliseconds(1000));
    if(!m_layer)
    {
      return nullptr;
    }
  }

  // This will throw if we can't acquire the write lock
  AcquireLayerWriteLock();

  auto page_it = std::find_if(m_pages.begin(), m_pages.end(), [chunk](VolumeDataPageImpl *page)->bool { return page->GetChunkIndex() == chunk; });
  if(page_it != m_pages.end())
  {
    throw InvalidOperation("Cannot create a page that already exists");
  }

  VolumeDataPageImpl *parentPage = nullptr;
  if(m_parentVolumeDataPageAccessor)
  {
    int64_t parentChunk = m_layer->GetParentIndex(chunk, *m_parentVolumeDataPageAccessor->m_layer);
    parentPage = static_cast<VolumeDataPageImpl *>(m_parentVolumeDataPageAccessor->ReadPage(parentChunk));
  }

  // Create a new page
  VolumeDataPageImpl *page = new VolumeDataPageImpl(this, chunk, parentPage);

  m_pages.push_front(page);

  assert(page->IsPinned());

  if(parentPage && parentPage->GetErrorInternal().code != 0)
  {
    page->SetError(parentPage->GetErrorInternal());
    return page;
  }

  Error error;
  VolumeDataChunk volumeDataChunk = m_layer->GetChunkFromIndex(chunk);

  pageListMutexLock.unlock();

  std::vector<uint8_t> page_data;
  DataBlock dataBlock;
  VolumeDataHash volumeDataHash = m_layer->IsUseNoValue() ? VolumeDataHash::NOVALUE : VolumeDataHash(0.0f);
  if (!VolumeDataStore::CreateConstantValueDataBlock(volumeDataChunk, m_layer->GetFormat(), m_layer->GetNoValue(), m_layer->GetComponents(), volumeDataHash, dataBlock, page_data, error))
  {
    pageListMutexLock.lock();
    page->UnPin();
    m_logger.LogError(fmt::format("Failed when creating chunk: {}", error.string.c_str()));
    return nullptr;
  }

  pageListMutexLock.lock();
  page->SetBufferData(dataBlock, m_layer->GetChunkDimensionGroup(), std::move(page_data), uint64_t(volumeDataHash));
  page->MakeDirty();

  m_pageReadCondition.notify_all();

  if(!m_layer)
  {
    page->UnPin();
    page = nullptr;
  }

  LimitPageListSize(m_maxPages, pageListMutexLock);

  return page;
}

VolumeDataPage* VolumeDataPageAccessorImpl::PrepareReadPage(int64_t chunk, Error &error)
{
  std::unique_lock<std::mutex> pageListMutexLock(m_pagesMutex);

  if(!m_layer)
  {
    error.code = -1;
    error.string = "PrepareReadPage missing layer";
    return nullptr;
  }

  if (m_layer->GetProduceStatus() == VolumeDataLayer::ProduceStatus_Unavailable)
  {
    error.code = -1;
    error.string = "The accessed dimension group or channel is unavailable (check produce status on VDS before accessing data)";
    return nullptr;
  }

  for(auto page_it = m_pages.begin(); page_it != m_pages.end(); ++page_it)
  {
    if((*page_it)->GetChunkIndex() == chunk)
    {
      if (page_it != m_pages.begin())
      {
        m_pages.splice(m_pages.begin(), m_pages, page_it, std::next(page_it));
      }
      (*page_it)->Pin();

      m_pagesFound++;
      return *page_it;
    }
  }

  // Wait for commit to finish before inserting a new page
  while(m_isCommitInProgress)
  {
    m_commitFinishedCondition.wait_for(pageListMutexLock, std::chrono::milliseconds(1000));
    if(!m_layer)
    {
      error.code = -1;
      error.string = "PrepareReadPage loosing layer while waiting for commit";
      return nullptr;
    }
  }

  VolumeDataPageImpl *parentPage = nullptr;
  if(m_parentVolumeDataPageAccessor)
  {
    int64_t parentChunk = m_layer->GetParentIndex(chunk, *m_parentVolumeDataPageAccessor->m_layer);
    parentPage = static_cast<VolumeDataPageImpl *>(m_parentVolumeDataPageAccessor->ReadPage(parentChunk));
  }

  // Not found, we need to create a new page
  VolumeDataPageImpl *page = new VolumeDataPageImpl(this, chunk, parentPage);

  m_pages.push_front(page);

  assert(page->IsPinned());

  if(parentPage && parentPage->GetErrorInternal().code != 0)
  {
    page->SetError(parentPage->GetErrorInternal());
    return page;
  }

  VolumeDataChunk volumeDataChunk = m_layer->GetChunkFromIndex(chunk);

  VolumeDataLayer const *remapFromLayer = m_layer->GetLayerToRemapFrom();

  if(remapFromLayer != m_layer)
  {
    std::vector<VolumeDataChunk> volumeDataChunkRead;
    remapFromLayer->GetChunksOverlappingChunk(volumeDataChunk, &volumeDataChunkRead);
    page->SetJobID(m_accessManager->AddRemapJob(*page, volumeDataChunkRead));
  }
  else
  {
    if (!m_accessManager->GetVolumeDataStore()->PrepareReadChunk(volumeDataChunk, m_layer->GetEffectiveWaveletAdaptiveLoadLevel(), error))
    {
      page->SetError(error);
    }
  }

  return page;
}

bool VolumeDataPageAccessorImpl::ReadPreparedPaged(VolumeDataPage* page)
{
  std::unique_lock<std::mutex> pageListMutexLock(m_pagesMutex, std::defer_lock);

  VolumeDataPageImpl *pageImpl = static_cast<VolumeDataPageImpl *>(page);

  if (!pageImpl->RequestPrepared())
    return pageImpl->GetErrorInternal().code == 0;

  if (pageImpl->EnterSettingData())
  {
    if (!pageImpl->RequestPrepared())
    {
      pageImpl->LeaveSettingData();
      return true;
    }

    Error error;
    bool success = false;

    int64_t jobID = pageImpl->JobID();
    if(jobID != -1)
    {
      success = m_accessManager->WaitForCompletion(jobID);

      if(!success)
      {
        ReadErrorException readError("", 0);
        bool canceled = m_accessManager->IsCanceled(jobID, &readError);
        (void)canceled;
        assert(canceled);
        error.code = readError.GetErrorCode();
        error.string = readError.GetErrorMessage();

        pageListMutexLock.lock();
        pageImpl->SetError(error);
        pageImpl->SetRequestPrepared(false);
        pageImpl->LeaveSettingData();
        m_pageReadCondition.notify_all();
        m_logger.LogError(fmt::format("Failed when waiting for chunk: {}", error.string.c_str()));
        return false;
      }
    }
    else
    {
      VolumeDataChunk volumeDataChunk = m_layer->GetChunkFromIndex(pageImpl->GetChunkIndex());
      std::vector<uint8_t> serialized_data;
      std::vector<uint8_t> metadata;
      CompressionInfo compressionInfo;

      if (!m_accessManager->GetVolumeDataStore()->ReadChunk(volumeDataChunk, m_layer->GetEffectiveWaveletAdaptiveLoadLevel(), serialized_data, metadata, compressionInfo, error))
      {
        pageListMutexLock.lock();
        pageImpl->SetError(error);
        pageImpl->SetRequestPrepared(false);
        pageImpl->LeaveSettingData();
        m_pageReadCondition.notify_all();
        m_logger.LogError(fmt::format("Failed when waiting for chunk: {}", error.string.c_str()));
        return false;
      }

      std::vector<uint8_t> page_data;
      uint64_t page_hash = VolumeDataHash::UNKNOWN;
      DataBlock dataBlock;
      bool sparse = false;

      if(metadata.size() >= sizeof(uint64_t))
      {
        uint64_t hash = VolumeDataHash::UNKNOWN;
        memcpy(&hash, metadata.data(), sizeof(uint64_t));

        // Check for sparse data
        if(hash == VolumeDataHash::UNKNOWN)
        {
          VolumeDataHash constantValueVolumeDataHash = m_layer->IsUseNoValue() ? VolumeDataHash(VolumeDataHash::NOVALUE) : VolumeDataHash(0.0f);
          m_accessManager->GetVolumeDataStore()->CreateConstantValueDataBlock(volumeDataChunk, m_layer->GetFormat(), m_layer->GetNoValue(), m_layer->GetComponents(), constantValueVolumeDataHash, dataBlock, page_data, error);
          sparse = true;
        }
      }

      if (!sparse)
      {
        bool success = m_accessManager->GetVolumeDataStore()->DeserializeVolumeData(volumeDataChunk, serialized_data, metadata, compressionInfo.GetCompressionMethod(), compressionInfo.GetAdaptiveLevel(), m_layer->GetFormat(), dataBlock, page_data, page_hash, error);
        if(!success)
        {
          pageListMutexLock.lock();
          pageImpl->SetError(error);
          pageImpl->SetRequestPrepared(false);
          pageImpl->LeaveSettingData();
          m_pageReadCondition.notify_all();
          m_logger.LogError(fmt::format("Failed when deserializing chunk: {}", error.string.c_str()));
          return false;
        }
      }

      pageListMutexLock.lock();
      pageImpl->SetBufferData(dataBlock, m_layer->GetChunkDimensionGroup(), std::move(page_data), page_hash);
    }

    m_pagesRead++;
    pageImpl->SetRequestPrepared(false);
    pageImpl->LeaveSettingData();
    m_pageReadCondition.notify_all();
  }
  else
  {
    pageImpl->LeaveSettingData();
    pageListMutexLock.lock();
    m_pageReadCondition.wait(pageListMutexLock, [pageImpl]{return !pageImpl->SettingData();});
  }

  if (pageImpl->GetErrorInternal().code)
    return false;

  if(!m_layer)
  {
    page = nullptr;
  }

  LimitPageListSize(m_maxPages, pageListMutexLock);
  return m_layer != nullptr;
}

void VolumeDataPageAccessorImpl::CancelPreparedReadPage(VolumeDataPage* page)
{
  std::unique_lock<std::mutex> pageListMutexLock(m_pagesMutex);

  VolumeDataPageImpl *pageImpl = static_cast<VolumeDataPageImpl *>(page);

  pageImpl->UnPin();
  if (pageImpl->IsPinned())
    return;

  auto page_it = std::find(m_pages.begin(), m_pages.end(), page);
  if (page_it != m_pages.end())
    m_pages.erase(page_it);
  pageListMutexLock.unlock();

  if (pageImpl->RequestPrepared())
  {
    if(pageImpl->JobID() != -1)
    {
      m_accessManager->CancelAndWaitForCompletion(pageImpl->JobID());
    }
    else
    {
      VolumeDataChunk volumeDataChunk = m_layer->GetChunkFromIndex(pageImpl->GetChunkIndex());
      Error error;
      m_accessManager->GetVolumeDataStore()->CancelReadChunk(volumeDataChunk, error);
      if (error.code)
      {
        pageImpl->SetError(error);
      }
    }
    pageImpl->SetRequestPrepared(false);
  }

  pageImpl->LeaveSettingData();
  delete pageImpl;
  m_pageReadCondition.notify_all();
}
  
void VolumeDataPageAccessorImpl::CopyPage(int64_t chunkIndex, VolumeDataPageAccessor const &source)
{
  if (!m_layer)
  {
    return;
  }

  auto const &sourceVolumeDataPageAccessor = static_cast<VolumeDataPageAccessorImpl const &>(source);

  if(!sourceVolumeDataPageAccessor.m_layer || (sourceVolumeDataPageAccessor.m_layer->GetProduceStatus() == VolumeDataLayer::ProduceStatus_Unavailable))
  {
    throw InvalidOperation("The source volume data page accessor is not valid");
  }

  if(!(static_cast<VolumeDataPartition const &>(*sourceVolumeDataPageAccessor.m_layer) == static_cast<VolumeDataPartition const &>(*m_layer)))
  {
    throw InvalidOperation("The source volume data page accessor layout is not compatible with the layout of this volume data page accessor");
  }

  if(!IsReadWrite())
  {
    throw InvalidOperation("Trying to copy a page to a read-only volume data page accessor");
  }

  VolumeDataChunk chunk = m_layer->GetChunkFromIndex(chunkIndex);
  m_accessManager->AddCopyPageJob(chunk, *this, const_cast<VolumeDataPageAccessorImpl &>(sourceVolumeDataPageAccessor));
  m_pagesWritten++;
}

VolumeDataPage* VolumeDataPageAccessorImpl::ReadPage(int64_t chunk)
{
  if(!m_layer)
  {
    return nullptr;
  }

  if(!m_isLayerWriteLocked && m_layer->IsWriteLocked())
  {
    throw InvalidOperation("Invalid read from write locked layer, please make sure all previously created VolumeDataPageAccessors have committed their writes.");
  }

  Error error;

  VolumeDataPage *page = PrepareReadPage(chunk, error);
  if (page)
  {
    (void)ReadPreparedPaged(page);
  }
  return page;
}

VolumeDataPage* VolumeDataPageAccessorImpl::ReadPageAtPosition(const int (&position)[Dimensionality_Max])
{
  for (int i = 0; i < Dimensionality_Max; i++)
  {
    if(position[i] < 0 || position[i] >= m_layer->GetDimensionNumSamples(i))
    {
      return nullptr;
    }
  }
  return ReadPage(GetChunkIndex(position));
}

void VolumeDataPageAccessorImpl::LimitPageListSize(int maxPages, std::unique_lock<std::mutex>& pageListMutexLock)
{
  while(int(m_pages.size()) > m_maxPages)
  {
    // Wait for commit to finish before deleting a page
    while(m_isCommitInProgress)
    {
      m_commitFinishedCondition.wait_for(pageListMutexLock, std::chrono::milliseconds(1000));
    }

    // Find a page to evict
    auto page_it = std::find_if(m_pages.rbegin(), m_pages.rend(), [](VolumeDataPageImpl *page)->bool { return !page->IsPinned(); });

    if(page_it == m_pages.rend())
    {
      return;
    }

    VolumeDataPageImpl *page = *page_it;

    if(page->IsWritten())
    {
      // Finish reading all pages currently being read
      while(1)
      {
        bool isReadInProgress = false;

        for(VolumeDataPageImpl *targetPage : m_pages)
        {
          if(page->IsCopyMarginNeeded(targetPage))
          {
            if(targetPage->IsEmpty())
            {
              isReadInProgress = true;
              break;
            }
          }
        }

        if(!isReadInProgress)
        {
          break;
        }

        page->Pin(); // Make sure this page isn't deleted by LimitPageListSize!

        // Wait for the page getting read
        m_pageReadCondition.wait_for(pageListMutexLock, std::chrono::milliseconds(1000));

        page->UnPin();

        // Check if the page was pinned while we released the mutex, we have to abort evicting this page
        if(page->IsPinned())
        {
          break;
        }
      }

      // Check if the page was pinned while we released the mutex, we have to abort evicting this page
      if(page->IsPinned())
      {
        continue;
      }

      // Copy margins
      if(page->IsWritten())
      {
        for(VolumeDataPageImpl *targetPage : m_pages)
        {
          if(page->IsCopyMarginNeeded(targetPage))
          {
            page->CopyMargin(targetPage);
          }
        }
      }
    }

    m_pages.erase(std::prev(page_it.base()));

    if(page->IsDirty() && m_layer)
    {
      page->WriteBack(m_layer, pageListMutexLock);
      m_pagesWritten++;
    }

    delete page;
  }
}

int64_t VolumeDataPageAccessorImpl::RequestWritePage(int64_t chunk, const DataBlock& dataBlock, const std::vector<uint8_t>& data, uint64_t hash)
{
  std::vector<uint8_t> serializedData;
  uint64_t serializedHash = VolumeDataStore::SerializeVolumeData({ m_layer, chunk }, dataBlock, data, m_layer->GetEffectiveCompressionMethod(), m_layer->GetEffectiveCompressionTolerance(), serializedData);

  if(serializedHash != VolumeDataHash::UNKNOWN)
  {
    hash = serializedHash;
  }

  assert(hash != VolumeDataHash::UNKNOWN);

  std::vector<uint8_t> metadata(sizeof(hash));
  memcpy(metadata.data(), &hash, sizeof(hash));

  return m_accessManager->GetVolumeDataStore()->WriteChunk({ m_layer, chunk }, serializedData, metadata);
}

void VolumeDataPageAccessorImpl::AcquireLayerWriteLock()
{
  if(!m_isLayerWriteLocked)
  {
    if(!m_layer->AcquireWriteLock())
    {
      throw InvalidOperation("Couldn't acquire write lock, please make sure all previously created VolumeDataPageAccessors have committed their writes.");
    }
    m_isLayerWriteLocked = true;
  }
}

void VolumeDataPageAccessorImpl::CommitInternal(std::unique_lock<std::mutex>& pageListMutexLock)
{
  // Finish reading all pages currently being read
  for(VolumeDataPageImpl *page : m_pages)
  {
    while(page->IsEmpty() && !page->IsCanceled())
    {
      // Wait for the page getting read
      m_pageReadCondition.wait_for(pageListMutexLock, std::chrono::milliseconds(1000));
    }
  }

  // Copy all margins
  for(VolumeDataPageImpl *page : m_pages)
  {
    if(page->IsWritten())
    {
      for(VolumeDataPageImpl *targetPage : m_pages)
      {
        if(page->IsCopyMarginNeeded(targetPage))
        {
          page->CopyMargin(targetPage);
        }
      }
    }
  }

  for(VolumeDataPageImpl *page : m_pages)
  {
    if(page->IsDirty() && m_layer)
    {
      page->WriteBack(m_layer, pageListMutexLock);
      m_pagesWritten++;
    }
  }

  if(m_isLayerWriteLocked)
  {
    if(m_layer)
    {
      m_layer->ReleaseWriteLock();
    }
    m_isLayerWriteLocked = false;
  }

  if(m_parentVolumeDataPageAccessor)
  {
    m_parentVolumeDataPageAccessor->CommitInternal(pageListMutexLock);
  }
}

void VolumeDataPageAccessorImpl::Commit()
{
  std::unique_lock<std::mutex> pageListMutexLock(m_pagesMutex);

  if(m_isCommitInProgress)
  {
    return;
  }

  // Make sure we don't start reading any new pages while we're finishing up the current waiting reads
  m_isCommitInProgress = true;
  CommitInternal(pageListMutexLock);
  m_isCommitInProgress = false;
  m_commitFinishedCondition.notify_all();
}
}
