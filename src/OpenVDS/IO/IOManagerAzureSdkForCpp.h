/****************************************************************************
** Copyright 2019 The Open Group
** Copyright 2019 Bluware, Inc.
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

#ifndef IOMANAGERAZURESDKFORCPP_H
#define IOMANAGERAZURESDKFORCPP_H

#include "IOManager.h"
#include "IOManagerRequestImpl.h"

#include <vector>
#include <string>
#include <memory>

#include <ThreadPool/ThreadPool.h>

#include <azure/core.hpp>
#include <azure/storage/blobs.hpp>
#include "azure/core/http/curl_transport.hpp"

namespace OpenVDS
{
  class IOManagerAzureSdkForCpp : public IOManager
  {
  public:
    IOManagerAzureSdkForCpp(const AzureOpenOptions& openOptions, Error& error);
    ~IOManagerAzureSdkForCpp() override;

    std::shared_ptr<Request> ReadObjectInfo(const std::string& objectName, std::shared_ptr<TransferDownloadHandler> handler) override;
    std::shared_ptr<Request> ReadObject(const std::string& requestName, std::shared_ptr<TransferDownloadHandler> handler, const IORange& range = IORange()) override;
    std::shared_ptr<Request> WriteObject(const std::string& requestName, const std::string& contentDispostionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::function<void(const Request& request, const Error& error)> completedCallback = nullptr) override;


    bool Close(Error& error) override;
  private:
    ThreadPool m_threadPool;
    std::string m_containerStr;
    std::string m_prefix;
    std::unique_ptr<Azure::Storage::Blobs::BlobServiceClient> m_serviceClient;
    std::shared_ptr<Azure::Core::Http::CurlTransport> m_transportAdapter;
  };
}


#endif //IOMANAGERAZURESDKFORCPP_H
