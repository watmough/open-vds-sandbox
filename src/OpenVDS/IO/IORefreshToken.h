#ifndef IOREFRESHTOKEN
#define IOREFRESHTOKEN

#include "IOManagerCurl.h"

#include <string>
#include <functional>

namespace OpenVDS
{
class TokenRefresher
{
public:
  TokenRefresher(const std::string& authTokenUrl, const std::string& clientId, const std::string& clientSecret, const std::string& scopes, const std::string& refreshToken, CurlHandler& curlHandler, const std::function<void(std::string&& new_token)>& newTokenCallback);
  TokenRefresher(const std::string& authTokenUrl, const std::string& clientId, const std::string& clientSecret, const std::string& refreshToken, CurlHandler& curlHandler, const std::function<void(std::string&& new_token)>& newTokenCallback);
  std::string newToken(Error &error);

  std::string m_authTokenUrl;
  std::string m_clientId;
  std::string m_clientSecret;
  std::string m_scopes;
  std::string m_refreshToken;
  CurlHandler& m_curlHandler;
  std::function<void(std::string&& new_token)> m_newTokenCallback;
};
}

#endif
