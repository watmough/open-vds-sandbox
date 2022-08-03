#ifndef IOMANAGERDMS_H
#define IOMANAGERDMS_H

#include "IOManager.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#pragma GCC diagnostic ignored "-Wpedantic"
#endif
#include <SDManager.h>
#include <SDGenericDataset.h>
#include <SDException.h>
#ifdef _MSC_VER
#pragma warning(pop)
#else
#pragma GCC diagnostic pop
#endif

#include <atomic>
#include <condition_variable>
#include <memory>
#include <future>

#include <ThreadPool/ThreadPool.h>

#include "IOManagerRequestImpl.h"

namespace OpenVDS
{
  class CurlHandler;
  class TokenRefresher;
  class GetHeadRequestDms : public RequestImpl
  {
  public:
    GetHeadRequestDms(seismicdrive::SDGenericDataset &dataset, const std::string& id, const std::shared_ptr<TransferDownloadHandler>& handler);
    ~GetHeadRequestDms() override;

    seismicdrive::SDGenericDataset & m_dataset;
    std::shared_ptr<TransferDownloadHandler> m_handler;
    std::future<void> m_job;
  };

  class ReadObjectInfoRequestDms : public GetHeadRequestDms
  {
  public:
    ReadObjectInfoRequestDms(seismicdrive::SDGenericDataset &dataset, const std::string& id, const std::shared_ptr<TransferDownloadHandler>& handler);

    void run(const std::string& requestName, std::weak_ptr<ReadObjectInfoRequestDms> request, ThreadPool &threadPool);

  };

  class DownloadRequestDms : public GetHeadRequestDms
  {
  public:
    DownloadRequestDms(seismicdrive::SDGenericDataset &dataset, const std::string& id, const std::shared_ptr<TransferDownloadHandler>& handler);

    void run(const std::string& requestName, const IORange& range, std::weak_ptr<DownloadRequestDms> request, ThreadPool &threadPool);
  };

  class UploadRequestDms : public RequestImpl
  {
  public:
    UploadRequestDms(seismicdrive::SDGenericDataset &dataset, const std::string& id, std::function<void(const Request& request, const Error& error)> completedCallback);
    void run(const std::string& requestName, const std::string& contentDispositionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::weak_ptr<UploadRequestDms> uploadRequest, ThreadPool &threadPool);

    seismicdrive::SDGenericDataset & m_dataset;
    std::function<void(const Request& request, const Error& error)> m_completedCallback;
    std::shared_ptr<std::vector<uint8_t>> m_data;
    std::future<void> m_job;
  };

  class IOManagerDms : public IOManager
  {
  public:
    IOManagerDms(const DMSOpenOptions& openOptions, IOManager::AccessPattern accessPattern, LogHandler logHandler, Error& error);
    ~IOManagerDms() override;

    std::shared_ptr<Request> ReadObjectInfo(const std::string& objectName, std::shared_ptr<TransferDownloadHandler> handler) override;
    std::shared_ptr<Request> ReadObject(const std::string& requestName, std::shared_ptr<TransferDownloadHandler> handler, const IORange& range = IORange()) override;
    std::shared_ptr<Request> WriteObject(const std::string& requestName, const std::string& contentDispostionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::function<void(const Request& request, const Error& error)> completedCallback = nullptr) override;
    bool Close(Error &error) override;

  private:
    static std::string AuthProviderCallback(const void* data);
    std::unique_ptr<seismicdrive::SDManager> m_sdManager;
    std::unique_ptr<seismicdrive::SDGenericDataset> m_dataset;
    std::string m_filename;
    bool m_opened;
    bool m_useFileNameForSingleFileDatasets;
    ThreadPool m_threadPool;
    std::unique_ptr<CurlHandler> m_curlHandler;
    std::unique_ptr<TokenRefresher> m_tokenRefresher;

  };
}

#endif //IOMANAGERDMS_H
