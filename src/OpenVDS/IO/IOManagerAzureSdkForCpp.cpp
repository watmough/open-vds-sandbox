/****************************************************************************
** Copyright 2020 The Open Group
** Copyright 2020 Bluware, Inc.
** Copyright 2020 Microsoft Corp.
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

#include "IOManagerAzureSdkForCpp.h"

#include <fmt/format.h>
#include <mutex>
#include <string>
#include <functional>
#include <algorithm>

#include <azure/storage/blobs.hpp>

namespace OpenVDS
{
  class GetHeadRequestAzureSdkForCpp : public RequestImpl
  {
  public:
    GetHeadRequestAzureSdkForCpp(const std::string& id, const std::shared_ptr<TransferDownloadHandler>& handler);
    ~GetHeadRequestAzureSdkForCpp() override;

    void Cancel() override;

    std::shared_ptr<TransferDownloadHandler> m_handler;
  };

  class ReadObjectInfoRequestAzureSdkForCpp : public GetHeadRequestAzureSdkForCpp
  {
  public:
    ReadObjectInfoRequestAzureSdkForCpp(const std::string& id, const std::shared_ptr<TransferDownloadHandler>& handler);

    void run(ThreadPool &threadPool, Azure::Storage::Blobs::BlobServiceClient &serviceClient, const std::string &containerName, const std::string& requestName, std::weak_ptr<ReadObjectInfoRequestAzureSdkForCpp> request);
  };

  class DownloadRequestAzureSdkForCpp : public GetHeadRequestAzureSdkForCpp
  {
  public:
    DownloadRequestAzureSdkForCpp(const std::string& id, const std::shared_ptr<TransferDownloadHandler>& handler);

    void run(ThreadPool &threadPool, Azure::Storage::Blobs::BlobServiceClient &serviceClient, const std::string &containerName, const std::string& requestName, const IORange& range, std::weak_ptr<DownloadRequestAzureSdkForCpp> request);

    IORange m_requestedRange;
  };

  class UploadRequestAzureSdkForCpp : public RequestImpl
  {
  public:
    UploadRequestAzureSdkForCpp(const std::string& id, std::function<void(const Request& request, const Error& error)> completedCallback);
    void run(ThreadPool &threadPool, Azure::Storage::Blobs::BlobServiceClient &serviceClient, const std::string &containerName, const std::string& requestName, const std::string& contentDispositionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::weak_ptr<UploadRequestAzureSdkForCpp> uploadRequest);
    void Cancel() override;

    std::function<void(const Request& request, const Error& error)> m_completedCallback;
    std::shared_ptr<std::vector<uint8_t>> m_data;
  };

  GetHeadRequestAzureSdkForCpp::GetHeadRequestAzureSdkForCpp(const std::string& id, const std::shared_ptr<TransferDownloadHandler>& handler)
    : RequestImpl(id)
    , m_handler(handler)
  {
  }

  GetHeadRequestAzureSdkForCpp::~GetHeadRequestAzureSdkForCpp()
  {
  }

  void GetHeadRequestAzureSdkForCpp::Cancel()
  {
    RequestImpl::Cancel();
  }

  ReadObjectInfoRequestAzureSdkForCpp::ReadObjectInfoRequestAzureSdkForCpp(const std::string& id, const std::shared_ptr<TransferDownloadHandler>& handler)
    : GetHeadRequestAzureSdkForCpp(id, handler)
  {
  }

  void ReadObjectInfoRequestAzureSdkForCpp::run(ThreadPool &threadPool, Azure::Storage::Blobs::BlobServiceClient &serviceClient, const std::string &containerName, const std::string& requestName, std::weak_ptr<ReadObjectInfoRequestAzureSdkForCpp> weak_request)
  {
    threadPool.Enqueue([serviceClient, containerName, requestName, weak_request]
    {
      auto request = weak_request.lock();
      if (!request)
        return;
      RequestStateHandler requestStateHandler(*request);
      if (requestStateHandler.isCancelledRequested())
        return;
      try
      {
        auto blobContainerClient = serviceClient.GetBlobContainerClient(containerName);
        auto blockBlobClient = blobContainerClient.GetBlockBlobClient(requestName);
        auto props = blockBlobClient.GetProperties().Value;
        for (auto metaPair : props.Metadata)
        {
          request->m_handler->HandleMetadata(metaPair.first, metaPair.second);
        }
        request->m_handler->HandleObjectSize(props.BlobSize);
        request->m_handler->HandleObjectLastWriteTime(props.LastModified.ToString());
      }
      catch (const std::exception& ex)
      {
        request->m_error.string = ex.what();
        request->m_error.code = -1;
      }
      request->m_handler->Completed(*request, request->m_error);
    });
  }

  DownloadRequestAzureSdkForCpp::DownloadRequestAzureSdkForCpp(const std::string& id, const std::shared_ptr<TransferDownloadHandler>& handler)
    : GetHeadRequestAzureSdkForCpp(id, handler)
  {
  }

  void DownloadRequestAzureSdkForCpp::run(ThreadPool &threadPool, Azure::Storage::Blobs::BlobServiceClient &serviceClient, const std::string &containerName, const std::string& requestName, const IORange& range, std::weak_ptr<DownloadRequestAzureSdkForCpp> weak_request)
  {
    // set options, we should probably get these through AzureOpenOptions instead of haddong here - default set in the IOMangerAzure
    m_requestedRange = range;
    threadPool.Enqueue([serviceClient, containerName, requestName, weak_request]
    {
      auto request = weak_request.lock();
      if (!request)
        return;
      RequestStateHandler requestStateHandler(*request);
      if (requestStateHandler.isCancelledRequested())
        return;
      try
      {
        auto blobContainerClient = serviceClient.GetBlobContainerClient(containerName);
        auto blockBlobClient = blobContainerClient.GetBlockBlobClient(requestName);
        Azure::Storage::Blobs::DownloadBlobOptions options;
        if (request->m_requestedRange.end)
        {
          options.Range = Azure::Core::Http::HttpRange();
          options.Range.Value().Offset = request->m_requestedRange.start;
          options.Range.Value().Length = request->m_requestedRange.end - request->m_requestedRange.start;
        }
        auto download = blockBlobClient.Download(options);
        for (auto metaPair : download.Value.Details.Metadata)
        {
          request->m_handler->HandleMetadata(metaPair.first, metaPair.second);
        }
        request->m_handler->HandleObjectSize(download.Value.BlobSize);
        request->m_handler->HandleObjectLastWriteTime(download.Value.Details.LastModified.ToString());
        std::vector<uint8_t> data;
        if (request->m_requestedRange.end)
          data.resize(request->m_requestedRange.end - request->m_requestedRange.start);
        else
          data.resize(download.Value.BlobSize);
        download.Value.BodyStream->ReadToCount(data.data(), data.size());
        request->m_handler->HandleData(std::move(data));
      }
      catch (const std::exception& ex)
      {
        request->m_error.string = ex.what();
        request->m_error.code = -1;
      }
      request->m_handler->Completed(*request, request->m_error);
    });
  }

  UploadRequestAzureSdkForCpp::UploadRequestAzureSdkForCpp(const std::string& id, std::function<void(const Request& request, const Error& error)> completedCallback)
    : RequestImpl(id)
    , m_completedCallback(completedCallback)
  {
  }

  void UploadRequestAzureSdkForCpp::run(ThreadPool &threadPool, Azure::Storage::Blobs::BlobServiceClient &serviceClient, const std::string &containerName, const std::string& requestName, const std::string& contentDispositionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::weak_ptr<UploadRequestAzureSdkForCpp> weak_request)
  {
    m_data = data;
    threadPool.Enqueue([serviceClient, containerName, requestName, metadataHeader, contentDispositionFilename, weak_request]
    {
      auto request = weak_request.lock();
      if (!request)
        return;
      RequestStateHandler requestStateHandler(*request);
      if (requestStateHandler.isCancelledRequested())
        return;

      try
      {
        auto blobContainerClient = serviceClient.GetBlobContainerClient(containerName);
        auto blockBlobClient = blobContainerClient.GetBlockBlobClient(requestName);
        Azure::Storage::Blobs::UploadBlockBlobFromOptions options;
        for (auto& pair : metadataHeader)
        {
          options.Metadata.emplace(pair.first, pair.second);
        }
        options.HttpHeaders.ContentDisposition = contentDispositionFilename;
        blockBlobClient.UploadFrom(request->m_data->data(), request->m_data->size(), options);
      }
      catch (const std::exception& ex)
      {
        request->m_error.string = ex.what();
        request->m_error.code = -1;
      }

      if (request->m_completedCallback)
        request->m_completedCallback(*request, request->m_error);
    });
  }

  void UploadRequestAzureSdkForCpp::Cancel()
  {
    RequestImpl::Cancel();
  }

  class BearerCredentials : public Azure::Core::Credentials::TokenCredential
  {

  };

  IOManagerAzureSdkForCpp::IOManagerAzureSdkForCpp(const AzureOpenOptions& openOptions, Error& error)
    : IOManager(OpenOptions::AzureSdkForCpp)
    , m_threadPool(16)
    , m_containerStr(openOptions.container)
    , m_prefix(openOptions.blob)
  {
    if (openOptions.connectionString.empty() && openOptions.bearerToken.empty())
    {
      error.code = -1;
      error.string = "Azure Config error. Must provide a connection string or a bearer token";
      return;
    }
    if (m_containerStr.empty())
    {
      error.code = -1;
      error.string = "Azure Config error. Empty container or blob name";
      return;
    }

    auto this_is_to_work_around_linking_inn_azure_storage_common = Azure::Storage::_internal::UrlEncodeQueryParameter(openOptions.connectionString);
    (void)this_is_to_work_around_linking_inn_azure_storage_common;

    if (openOptions.connectionString.size())
    {
      try
      {
        Azure::Storage::Blobs::BlobServiceClient serviceClient = Azure::Storage::Blobs::BlobServiceClient::CreateFromConnectionString(openOptions.connectionString);
        m_serviceClient.reset(new Azure::Storage::Blobs::BlobServiceClient(serviceClient));
      }
      catch (const std::exception& exception)
      {
        error.string = exception.what();
        error.code = -1;
        return;
      }
    }
    else
    {
      if (openOptions.bearerToken.empty())
      {
        error.code = -1;
        error.string = "Azure Config error. Neither Bearer token or connection string specified";
        return;
      }
      if (openOptions.accountName.empty())
      {
        error.code = -1;
        error.string = "Azure Config error. Account Name is mandatory when specifying bearer token";
        return;
      }
      error.code = -1;
      error.string = "Azure Config error. Bearer token is not supported by AzureSdkForCpp";
      return;
    }
  }

  IOManagerAzureSdkForCpp::~IOManagerAzureSdkForCpp()
  {
  }

  static std::string create_id(const std::string& prefix, const std::string& objectName)
  {
    if (objectName.empty())
    {
      return prefix;
    }
    if (prefix.empty())
    {
      return objectName;
    }
    return prefix + "/" + objectName;
  }

  std::shared_ptr<Request> IOManagerAzureSdkForCpp::ReadObjectInfo(const std::string& objectName, std::shared_ptr<TransferDownloadHandler> handler)
  {
    std::string id = create_id(m_prefix, objectName);
    std::shared_ptr<ReadObjectInfoRequestAzureSdkForCpp> azureRequest = std::make_shared<ReadObjectInfoRequestAzureSdkForCpp>(id, handler);
    azureRequest->run(m_threadPool, *m_serviceClient, m_containerStr, id, azureRequest);
    return azureRequest;
  }

  std::shared_ptr<Request> IOManagerAzureSdkForCpp::ReadObject(const std::string& objectName, std::shared_ptr<TransferDownloadHandler> handler, const IORange& range)
  {
    std::string id = create_id(m_prefix, objectName);
    std::shared_ptr<DownloadRequestAzureSdkForCpp> azureRequest = std::make_shared<DownloadRequestAzureSdkForCpp>(id, handler);
    azureRequest->run(m_threadPool, *m_serviceClient, m_containerStr, id, range, azureRequest);
    return azureRequest;
  }

  std::shared_ptr<Request> IOManagerAzureSdkForCpp::WriteObject(const std::string& objectName, const std::string& contentDispositionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::function<void(const Request& request, const Error& error)> completedCallback)
  {
    std::string id = create_id(m_prefix, objectName);
    std::shared_ptr<UploadRequestAzureSdkForCpp> azureRequest = std::make_shared<UploadRequestAzureSdkForCpp>(id, completedCallback);
    azureRequest->run(m_threadPool, *m_serviceClient, m_containerStr, id, contentDispositionFilename, contentType, metadataHeader, data, azureRequest);
    return azureRequest;
  }
}
