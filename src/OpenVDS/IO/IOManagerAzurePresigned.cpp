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

#include "IOManagerAzurePresigned.h"

#include <fmt/format.h>

namespace OpenVDS
{
  IOManagerAzurePresigned::IOManagerAzurePresigned(const std::string& base, const std::string& suffix, std::shared_ptr<CurlHandler> curlHandler, Error &error)
    : IOManager(OpenOptions::AzurePresigned)
    , m_curlHandler(curlHandler)
    , m_base(base)
    , m_suffix(suffix)
  {
    if (m_base.empty())
    {
      error.code = -1;
      error.string = "IOManagerPresigned: empty base url";
      return;
    }

    if (suffix.empty())
    {
      auto it = std::find(base.begin(), base.end(), '?');
      if (it != base.end())
      {
        m_base = std::string(base.begin(), it);
        m_suffix = std::string(it, base.end());
      }
      else
      {
        m_base = base;
      }
    }

    if (m_base.back() == '/')
      m_base.pop_back();

    if (m_suffix.size())
    {
      if (m_suffix[0] != '?')
        m_suffix.insert(m_suffix.begin(), '?');
    }
  }

  IOManager *IOManagerAzurePresigned::CreateIOManagerAzurePresigned(const std::string& base, const std::string& suffix, std::shared_ptr<CurlHandler> curlHandler, Error& error)
  {
    std::unique_ptr<IOManager> ioManager(new IOManagerAzurePresigned(base, suffix, curlHandler, error));
    return (error.code == 0) ? ioManager.release() : nullptr;
  }

  IOManager *IOManagerAzurePresigned::CreateIOManagerAzurePresigned(const std::string& base, const std::string& suffix, const Logger& logger, Error& error)
  {
    auto curlHandler = std::make_shared<CurlHandler>(error, logger);
    return (error.code == 0) ? CreateIOManagerAzurePresigned(base, suffix, curlHandler, error) : nullptr;
  }

  static std::string getUrl(const std::string& base, const std::string& objectName, const std::string &suffix)
  {
    if (objectName.empty())
    {
      return base + suffix;
    }
    return base + "/" + objectName + suffix;
  }

  std::shared_ptr<Request> IOManagerAzurePresigned::ReadObjectInfo(const std::string& objectName, std::shared_ptr<TransferDownloadHandler> handler)
  {
    std::string url = getUrl(m_base, objectName, m_suffix);
    std::shared_ptr<DownloadRequestCurl> request = std::make_shared<DownloadRequestCurl>(objectName, handler);
    std::vector<std::string> headers;
    m_curlHandler->addDownloadRequest(request, url, headers, convertToISO8601, CurlVerb::HEADER);
    return request;
    
  }
  std::shared_ptr<Request> IOManagerAzurePresigned::ReadObject(const std::string& objectName, std::shared_ptr<TransferDownloadHandler> handler, const IORange& range)
  {
    std::string url = getUrl(m_base, objectName, m_suffix);
    std::shared_ptr<DownloadRequestCurl> request = std::make_shared<DownloadRequestCurl>(objectName, handler);
    std::vector<std::string> headers;
    if (range.start != range.end)
    {
      headers.emplace_back(fmt::format("x-ms-range: bytes={}-{}", range.start, range.end - 1));
    }
    m_curlHandler->addDownloadRequest(request, url, headers, convertToISO8601, CurlVerb::GET);
    return request;
  }
  std::shared_ptr<Request> IOManagerAzurePresigned::WriteObject(const std::string& objectName, const std::string& contentDispostionFilename, const std::string& contentType, const std::vector<std::pair<std::string, std::string>>& metadataHeader, std::shared_ptr<std::vector<uint8_t>> data, std::function<void(const Request& request, const Error& error)> completedCallback)
  {
    std::string url = getUrl(m_base, objectName, m_suffix);
    std::shared_ptr<UploadRequestCurl> request = std::make_shared<UploadRequestCurl>(objectName, completedCallback);
    std::vector<std::string> headers;
    headers.emplace_back("x-ms-blob-type: BlockBlob");
    for (auto metaTag : metadataHeader)
    {
      headers.push_back(fmt::format("{}{}: {}", "x-ms-meta-", metaTag.first, metaTag.second));
    }
    m_curlHandler->addUploadRequest(request, url, headers, data);
    return request;
  }
}
