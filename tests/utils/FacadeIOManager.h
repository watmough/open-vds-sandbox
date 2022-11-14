/****************************************************************************
** Copyright 2020 The Open Group
** Copyright 2020 Bluware, Inc.
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

#ifndef FACADEIOMANAGER_H
#define FACADEIOMANAGER_H

#include <OpenVDS/OpenVDS.h>

#include <IO/IOManager.h>
#include <IO/IOManagerRequestImpl.h>
#include <ThreadPool/ThreadPool.h>

#include <atomic>
#include <map>

struct Object
{
  std::vector<std::pair<std::string, std::string>> metaHeader;
  std::vector<uint8_t> data;
  OpenVDS::Error error;
};

class IOManagerFacadeLight : public OpenVDS::IOManager
{
public:
  IOManagerFacadeLight(OpenVDS::IOManager *backend)
    : IOManager(backend->connectionType())
    , backend(backend)
  {}

  std::shared_ptr<OpenVDS::Request> ReadObjectInfo(const std::string &objectName, std::shared_ptr<OpenVDS::TransferDownloadHandler> handler) override
  {
    return backend->ReadObjectInfo(objectName, handler);
  }

  std::shared_ptr<OpenVDS::Request> ReadObject(const std::string &objectName, std::shared_ptr<OpenVDS::TransferDownloadHandler> handler, const OpenVDS::IORange& range = OpenVDS::IORange()) override
  {
    return backend->ReadObject(objectName, handler, range);
  }

  std::shared_ptr<OpenVDS::Request> WriteObject(const std::string &objectName, const std::string& contentDispostionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::function<void(const OpenVDS::Request & request, const OpenVDS::Error & error)> completedCallback = nullptr) override
  {
    return backend->WriteObject(objectName, contentDispostionFilename, contentType, metadataHeader, data, completedCallback);
  }

  bool Close(uint64_t serializedSize, uint64_t chunkCount, OpenVDS::Error &error) override
  {
    return backend->Close(serializedSize, chunkCount, error);
  }

  IOManager *backend;
};

class IOManagerFacadeUtil
{
public:
  IOManagerFacadeUtil(OpenVDS::IOManager *backend, std::map<std::string, Object> &data, std::mutex &mutex)
    : backend(backend)
    , threadPool(1)
    , m_data(data)
    , m_mutex(mutex)
  {}

  std::shared_ptr<OpenVDS::Request> ReadObjectInfo(const std::string &objectName, std::shared_ptr<OpenVDS::TransferDownloadHandler> handler)
  {
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      auto object_it = m_data.find(objectName);
      if (object_it != m_data.end())
      {
        auto request = std::make_shared<OpenVDS::RequestImpl>(objectName);
        std::weak_ptr<OpenVDS::RequestImpl> weak_request(request);
        threadPool.Enqueue([handler, objectName, weak_request, this]
        {
          auto objReq = weak_request.lock();
          if (!objReq)
            return;
          OpenVDS::RequestStateHandler requestStateHandler(*objReq);
          if (requestStateHandler.isCancelledRequested())
          {
            return;
          }
          std::unique_lock<std::mutex> lock(m_mutex);
          auto object_it = m_data.find(objectName);
          if (object_it == m_data.end())
          {
            objReq->m_error.code = -1;
            objReq->m_error.string = "Internal state error, object has been removed from IOFacade";
          }
          else
          {
            auto& object = (*object_it).second;
            objReq->m_error = object.error;
            handler->HandleObjectSize(int64_t(object.data.size()));

            for (auto& meta : object.metaHeader)
            {
              handler->HandleMetadata(meta.first, meta.second);
            }
          }
          handler->Completed(*objReq, objReq->m_error);
        });
        return request;
      }
    }
    return backend->ReadObjectInfo(objectName, handler);
  }

  std::shared_ptr<OpenVDS::Request> ReadObject(const std::string& objectName, std::shared_ptr<OpenVDS::TransferDownloadHandler> handler, const OpenVDS::IORange& range = OpenVDS::IORange())
  {
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      auto object_it = m_data.find(objectName);
      if (object_it != m_data.end())
      {
        auto request = std::make_shared<OpenVDS::RequestImpl>(objectName);
        std::weak_ptr<OpenVDS::RequestImpl> weak_request(request);
        threadPool.Enqueue([handler, objectName, weak_request, this]
        {
          auto objReq = weak_request.lock();
          if (!objReq)
            return;
          OpenVDS::RequestStateHandler requestStateHandler(*objReq);
          if (requestStateHandler.isCancelledRequested())
          {
            return;
          }
          std::unique_lock<std::mutex> lock(m_mutex);
          auto object_it = m_data.find(objectName);
          if (object_it == m_data.end())
          {
            objReq->m_error.code = -1;
            objReq->m_error.string = "Internal state error, object has been removed from IOFacade";
          }
          else
          {
            auto& object = (*object_it).second;
            objReq->m_error = object.error;
            handler->HandleObjectSize(int64_t(object.data.size()));

            for (auto& meta : object.metaHeader)
            {
              handler->HandleMetadata(meta.first, meta.second);
            }
            std::vector<uint8_t> data = object.data;
            handler->HandleData(std::move(data));
          }
          handler->Completed(*objReq, objReq->m_error);
        });
        return request;
      }
    }
    return backend->ReadObject(objectName, handler, range);
  }

  std::shared_ptr<OpenVDS::Request> WriteObject(const std::string &objectName, const std::string& contentDispostionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::function<void(const OpenVDS::Request & request, const OpenVDS::Error & error)> completedCallback = nullptr)
  {
    {
      std::unique_lock<std::mutex> lock(m_mutex);
      auto object_it = m_data.find(objectName);
      if (object_it != m_data.end())
      {
        auto request = std::make_shared<OpenVDS::RequestImpl>(objectName);
        std::weak_ptr<OpenVDS::RequestImpl> weak_request(request);
        threadPool.Enqueue([completedCallback, objectName, data, metadataHeader, weak_request, this]
        {
          auto objReq = weak_request.lock();
          if (!objReq)
            return;
          OpenVDS::RequestStateHandler requestStateHandler(*objReq);
          if (requestStateHandler.isCancelledRequested())
          {
            return;
          }
          std::unique_lock<std::mutex> lock(m_mutex);
          auto object_it = m_data.find(objectName);
          if (object_it == m_data.end())
          {
            objReq->m_error.code = -1;
            objReq->m_error.string = "Internal state error, object has been removed from IOFacade";
          }
          else
          {
            auto& object = (*object_it).second;
            object.data = *data;
            object.metaHeader = metadataHeader;
            objReq->m_error = object.error;
          }

          if (completedCallback)
          {
            lock.unlock();
            completedCallback(*objReq, objReq->m_error);
          }
        });
        return request;
      }
    }
    return backend->WriteObject(objectName, contentDispostionFilename, contentType, metadataHeader, data, completedCallback);
  }

  bool Close(uint64_t serializedSize, uint64_t chunkCount, OpenVDS::Error &error) { return backend->Close(serializedSize, chunkCount, error); }

  OpenVDS::IOManager *backend;
  ThreadPool threadPool;
  std::map<std::string, Object> &m_data;
  std::mutex &m_mutex;
};

class IOManagerFacade : public OpenVDS::IOManager
{
public:
  IOManagerFacade(OpenVDS::IOManager *backend)
    : IOManager(backend->connectionType())
    , facade(backend, m_data, m_mutex)
  {}
  std::shared_ptr<OpenVDS::Request> ReadObjectInfo(const std::string& objectName, std::shared_ptr<OpenVDS::TransferDownloadHandler> handler) override { return facade.ReadObjectInfo(objectName, handler); }
  std::shared_ptr<OpenVDS::Request> ReadObject(const std::string& objectName, std::shared_ptr<OpenVDS::TransferDownloadHandler> handler, const OpenVDS::IORange& range = OpenVDS::IORange()) override { return facade.ReadObject(objectName, handler, range); }
  std::shared_ptr<OpenVDS::Request> WriteObject(const std::string& objectName, const std::string& contentDispostionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::function<void(const OpenVDS::Request& request, const OpenVDS::Error& error)> completedCallback = nullptr) override { return facade.WriteObject(objectName, contentDispostionFilename, contentType, metadataHeader, data, completedCallback); }
  bool Close(uint64_t serializedSize, uint64_t chunkCount, OpenVDS::Error &error) override { return facade.Close(serializedSize, chunkCount, error); }

  std::map<std::string, Object> m_data;
  std::mutex m_mutex;
  IOManagerFacadeUtil facade;
};

#endif
