#include "IORefreshToken.h"

#include <fmt/format.h>
#include "json_cpp_include.h"

namespace OpenVDS
{
  TokenRefresher::TokenRefresher(const std::string& authTokenUrl, const std::string &clientId, const std::string &clientSecret, const std::string& scopes, const std::string& refreshToken, CurlHandler& curlHandler, const std::function<void(std::string&& new_token)> &newTokenCallback)
    : m_authTokenUrl(authTokenUrl)
    , m_clientId(clientId)
    , m_clientSecret(clientSecret)
    , m_scopes(scopes)
    , m_refreshToken(refreshToken)
    , m_curlHandler(curlHandler)
    , m_newTokenCallback(newTokenCallback)
  {
  }

static std::string extractTokensFromJson(const std::vector<uint8_t>& respons_data, Error& error, std::string& refreshToken);

  std::string TokenRefresher::newToken(Error &error)
  {
    std::vector<std::shared_ptr<std::vector<uint8_t>>> data;
    data.emplace_back(std::make_shared<std::vector<uint8_t>>());
  
    std::string form;
    if (m_refreshToken.empty())
    {
      form = fmt::format("grant_type={}&client_id={}&client_secret={}&scope={}", "client_credentials", m_clientId, m_clientSecret, m_scopes);
    }
    else
    {
      std::string scopes = m_scopes.empty() ? "openid email" : m_scopes;
      form = m_clientSecret.size() > 0 ? fmt::format("grant_type={}&client_id={}&client_secret={}&refresh_token={}&scope={}", "refresh_token", m_clientId, m_clientSecret, m_refreshToken, scopes) : fmt::format("grant_type={}&client_id={}&refresh_token={}&scope={}", "refresh_token", m_clientId, m_refreshToken, m_scopes);
    }

    data.back()->insert(data.back()->end(), form.begin(), form.end());

    std::shared_ptr<UploadRequestCurl> request = std::make_shared<UploadRequestCurl>("refresh_token", std::function<void(const Request& request, const Error& error)>());

    std::vector<std::string> headers;
    headers.emplace_back("Content-Type: application/x-www-form-urlencoded");
    m_curlHandler.addUploadRequest(request, m_authTokenUrl, headers, CurlVerb::POST, std::move(data), form.size());
    request->WaitForFinish(error);
    if (error.code || !request->m_uploadHandler){
      error.string = fmt::format("TokenRefresher: {}", error.string);
      return "";
    }
    return extractTokensFromJson(request->m_uploadHandler->responsData, error, m_refreshToken);
  }

  static std::string extractTokensFromJson(const std::vector<uint8_t>& respons_data, Error& error, std::string& refreshToken)
  {
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
      error.string = fmt::format("TokenRefresher: {} : {}", e.what(), error.string);
      return "";
    }

    if (value.isMember("refresh_token"))
    {
      refreshToken = value["refresh_token"].asString();
    }

    if (!value.isMember("access_token"))
    {
      error.code = -1;
      error.string = "TokenRefresher: access_token not found in JSON response";
      return "";
    }

    return value["access_token"].asString();
  }
}
