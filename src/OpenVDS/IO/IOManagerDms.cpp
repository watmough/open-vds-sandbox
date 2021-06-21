#include "IOManagerDms.h"

#include <future>
#include <condition_variable>

#include <cstdint>

#ifdef _MSC_VER
#pragma warning( disable : 4275 )
#endif
#include <SDException.h>

namespace OpenVDS
{
  GetHeadRequestDms::GetHeadRequestDms(seismicdrive::SDGenericDataset &dataset, const std::string& id, const std::shared_ptr<TransferDownloadHandler>& handler)
    : RequestImpl(id)
    , m_dataset(dataset)
    , m_handler(handler)
  {
  }

  GetHeadRequestDms::~GetHeadRequestDms()
  {
  }


  ReadObjectInfoRequestDms::ReadObjectInfoRequestDms(seismicdrive::SDGenericDataset &dataset, const std::string& id, const std::shared_ptr<TransferDownloadHandler>& handler)
    : GetHeadRequestDms(dataset, id, handler)
  {
  }

  DownloadRequestDms::DownloadRequestDms(seismicdrive::SDGenericDataset &dataset, const std::string& id, const std::shared_ptr<TransferDownloadHandler>& handler)
    : GetHeadRequestDms(dataset, id, handler)
  {
  }

  template<typename T>
  static void run_request(const std::string& requestName, std::weak_ptr <T> weak_request, const IORange &range, std::vector<uint8_t>* data)
  {
  }

  void ReadObjectInfoRequestDms::run(const std::string& requestName, std::weak_ptr<ReadObjectInfoRequestDms> weak_request, ThreadPool &threadPool)
  {
    m_job = threadPool.Enqueue([requestName, weak_request]() {
      auto request = weak_request.lock();
      if (!request)
        return;

      RequestStateHandler requestHandler(*request);

      if (requestHandler.isCancelledRequested())
      {
        return;
      }

      uint64_t size;
      std::string created_date;
      try
      {
        size = request->m_dataset.getBlockSize(requestName);
        created_date = request->m_dataset.getCreatedDate();
      }
      catch (const seismicdrive::SDException& ex)
      {
        request->m_error.code = -1;
        request->m_error.string = ex.what();
      }
      catch (...)
      {
        request->m_error.code = -1;
        request->m_error.string = "Unknown exception in DMS upload";
      }

      if (request->m_error.code == 0)
      {
        request->m_handler->HandleObjectSize(size);
        request->m_handler->HandleObjectLastWriteTime(created_date);
      }
      request->m_handler->Completed(*request, request->m_error);
    });
  }
  void DownloadRequestDms::run(const std::string& requestName, const IORange& range, std::weak_ptr<DownloadRequestDms> weak_request, ThreadPool &threadPool)
  {
    m_job = threadPool.Enqueue([requestName, weak_request, range]() {
      auto request = weak_request.lock();
      if (!request)
        return;

      RequestStateHandler requestHandler(*request);

      if (requestHandler.isCancelledRequested())
      {
        return;
      }

      std::vector<uint8_t> data;
      try
      {
        if (range.end)
        {
          data.resize(range.end - range.start);
          request->m_dataset.readBlock(requestName, (char*)data.data(), range.start, data.size());
        }
        else
        {
          //char* read_data = nullptr;
          //std::size_t read_size = 0;
          //request->m_dataset.readBlock(requestName, &read_data, read_size);
          //data.resize(read_size);
          //memcpy(data.data(), read_data, data.size());
          //delete[] read_data;
          size_t read_size = size_t(request->m_dataset.getBlockSize(requestName));
          if (read_size)
          {
            data.resize(read_size);
            request->m_dataset.readBlock(requestName, (char*)data.data(), read_size);
          }
        }
      }
      catch (const seismicdrive::SDException& ex)
      {
        request->m_error.code = -1;
        request->m_error.string = ex.what();
      }
      catch (...)
      {
        request->m_error.code = -1;
        request->m_error.string = "Unknown exception in DMS upload";
      }

      if (request->m_error.code == 0)
      {
        request->m_handler->HandleObjectSize(data.size());
        request->m_handler->HandleData(std::move(data));
      }
      request->m_handler->Completed(*request, request->m_error);
    });
  }

  UploadRequestDms::UploadRequestDms(seismicdrive::SDGenericDataset &dataset, const std::string& id, std::function<void(const Request& request, const Error& error)> completedCallback)
    : RequestImpl(id)
    , m_dataset(dataset)
    , m_completedCallback(completedCallback)
  {
  }

  void UploadRequestDms::run(const std::string& requestName, const std::string& contentDispositionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::weak_ptr<UploadRequestDms> request, ThreadPool &threadPool)
  {
    m_job = threadPool.Enqueue([requestName, contentDispositionFilename, contentType, metadataHeader, data, request]() {
      auto request_ptr = request.lock();
      if (!request_ptr)
        return;

      RequestStateHandler requestHandler(*request_ptr);
      if (requestHandler.isCancelledRequested())
      {
        return;
      }

      try
      {
        if (data->size())
          request_ptr->m_dataset.writeBlock(requestName, (const char*)data->data(), data->size(), false);
      }
      catch (const seismicdrive::SDException& ex)
      {
        request_ptr->m_error.code = -1;
        request_ptr->m_error.string = ex.what();
      }
      catch (...)
      {
        request_ptr->m_error.code = -1;
        request_ptr->m_error.string = "Unknown exception in DMS upload";
      }

      if (request_ptr->m_completedCallback)
        request_ptr->m_completedCallback(*request_ptr, request_ptr->m_error);
    });
  }

  IOManagerDms::IOManagerDms(const DMSOpenOptions& openOptions, IOManager::AccessPattern accessPatttern, Error& error)
    : IOManager(openOptions.connectionType)
    , m_opened(false)
    , m_threadPool(16)
  {
    if (openOptions.datasetPath.size())
    {
      auto it = openOptions.datasetPath.rfind('/');
      if (it == openOptions.datasetPath.size() - 1)
      {
        it = openOptions.datasetPath.rfind('/', 1);
      }
      if (it != std::string::npos)
      {
        m_filename = openOptions.datasetPath.substr(it+1);
      }
    }
    try {
      m_sdManager.reset(new seismicdrive::SDManager(openOptions.sdAuthorityUrl, openOptions.sdApiKey, openOptions.logLevel));
      m_sdManager->setAuthProviderFromString(openOptions.sdToken);
      m_dataset.reset(new seismicdrive::SDGenericDataset(m_sdManager.get(), openOptions.datasetPath, openOptions.logLevel != 0));

      seismicdrive::SDDatasetDisposition disposition = seismicdrive::SDDatasetDisposition::READ_ONLY;
      switch (accessPatttern)
      {
      case IOManager::ReadOnly:
        disposition = seismicdrive::SDDatasetDisposition::READ_ONLY;
        break;
      case IOManager::ReadWrite:
        disposition = seismicdrive::SDDatasetDisposition::OVERWRITE;
        break;
      }
      m_dataset->open(disposition);
      m_opened = true;
    }
    catch (seismicdrive::SDException &exception)
    {
      error.string = exception.what();
      error.code = -1;
    }
    catch (...)
    {
      error.string = "Unknown DMS exception.";
      error.code = -1;
    }
  }

  IOManagerDms::~IOManagerDms()
  {
    if (m_dataset && m_opened)
    {
      m_dataset->close();
    }
  }

  std::shared_ptr<Request> IOManagerDms::ReadObjectInfo(const std::string& objectName, std::shared_ptr<TransferDownloadHandler> handler)
  {
    std::string toRead = objectName.empty() ? m_filename : objectName;
    auto req = std::make_shared<ReadObjectInfoRequestDms>(*m_dataset, toRead, handler);
    req->run(toRead, req, m_threadPool);
    return req;
  }

  std::shared_ptr<Request> IOManagerDms::ReadObject(const std::string& objectName, std::shared_ptr<TransferDownloadHandler> handler, const IORange& range)
  {
    std::string toRead = objectName.empty() ? m_filename : objectName;
    auto req = std::make_shared<DownloadRequestDms>(*m_dataset, toRead, handler);
    req->run(toRead, range, req, m_threadPool);
    return req;
  }

  std::shared_ptr<Request> IOManagerDms::WriteObject(const std::string& requestName, const std::string& contentDispostionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::function<void(const Request& request, const Error& error)> completedCallback)
  {
    auto req = std::make_shared<UploadRequestDms>(*m_dataset, requestName, completedCallback);
    req->run(requestName, contentDispostionFilename, contentType, metadataHeader, data, req, m_threadPool);
    return req;
  }

}
