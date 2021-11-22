#include "IORefreshToken.h"

#include <fmt/format.h>
#include "json_cpp_include.h"

namespace OpenVDS
{
  TokenRefresher::TokenRefresher(const std::string& authTokenUrl, const std::string &clientId, const std::string &clientSecret, const std::string& scopes, const std::string& refreshToken, CurlHandler& curlHandler, const std::function<void(std::string&& new_token)> &newTokenCallback)
    : m_authTokenUrl(authTokenUrl)
    , m_clientId(clientId)
    , m_clientSecret(clientSecret)
    , m_scopes(scopes.size() > 0 ? scopes : ("openid email"))
    , m_refreshToken(refreshToken)
    , m_curlHandler(curlHandler)
    , m_newTokenCallback(newTokenCallback)
  {
  }

TokenRefresher::TokenRefresher(const std::string& authTokenUrl, const std::string &clientId, const std::string &clientSecret, const std::string& refreshToken, CurlHandler& curlHandler, const std::function<void(std::string&& new_token)> &newTokenCallback)
    : m_authTokenUrl(authTokenUrl)
    , m_clientId(clientId)
    , m_clientSecret(clientSecret)
    , m_scopes("openid email")
    , m_refreshToken(refreshToken)
    , m_curlHandler(curlHandler)
    , m_newTokenCallback(newTokenCallback)
  {
  }

  std::string TokenRefresher::newToken()
  {
    std::vector<std::shared_ptr<std::vector<uint8_t>>> data;
    data.emplace_back(std::make_shared<std::vector<uint8_t>>());
    
    std::string form = m_clientSecret.size() > 0 ? fmt::format("grant_type={}&client_id={}&client_secret={}&refresh_token={}&scope={}", "refresh_token", m_clientId, m_clientSecret, m_refreshToken, m_scopes) : fmt::format("grant_type={}&client_id={}&refresh_token={}&scope={}", "refresh_token", m_clientId,  m_refreshToken, m_scopes);
    data.back()->insert(data.back()->end(), form.begin(), form.end());

    std::shared_ptr<UploadRequestCurl> request = std::make_shared<UploadRequestCurl>("refresh_token", std::function<void(const Request& request, const Error& error)>());

    std::vector<std::string> headers;
    headers.emplace_back("Content-Type: application/x-www-form-urlencoded");
    m_curlHandler.addUploadRequest(request, m_authTokenUrl, headers, true, std::move(data), form.size());
    Error error;
    request->WaitForFinish(error);
    if (error.code || !request->m_uploadHandler){
      return "";
}
    std::string respons_data;
    respons_data.insert(respons_data.end(), request->m_uploadHandler->responsData.begin(), request->m_uploadHandler->responsData.end());
    Json::Value value;

    Json::CharReaderBuilder rbuilder;
    rbuilder["collectComments"] = false;

    try
    {
      std::unique_ptr<Json::CharReader> reader(rbuilder.newCharReader());
      const char* json_begin = reinterpret_cast<const char*>(respons_data.data());
      reader->parse(json_begin, json_begin + respons_data.size(), &value, &error.string);
    }
    catch (Json::Exception& e)
    {
      error.code = -1;
      error.string = e.what() + std::string(" : ") + error.string;
      return "";
    }

    if (value.isMember("refresh_token"))
    {
      m_refreshToken = value["refresh_token"].asString();
    }
    return value["access_token"].asString();
  }
}
