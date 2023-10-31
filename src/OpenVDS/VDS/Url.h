#ifndef OPENVDS_URL_H
#define OPENVDS_URL_H

#include <string>
#include <fmt/format.h>

namespace OpenVDS
{
inline std::string URLDecode(const std::string & url)
{
  std::string result;
  result.reserve(url.size());
  int len = int(url.size());

  for(int i = 0; i < len; i++)
  {
    char c = url[i];

    if(c == '+')
    {
      c = ' ';
    }
    else if(c == '%' && i + 2 < len)
    {
      char temp[3] = { url[i+1], url[i+2] };
      char *end;
      char decoded = char(strtol(temp, &end, 16));
      if(end == temp + 2)
      {
        c = decoded;
        i += 2;
      }
    }

    result.push_back(c);
  }
  return result;
}

inline bool openvds_isalnum(int c) {
  return (c >= 0x30 && c <= 0x39) || (c >= 0x41 && c <= 0x5a) || (c >= 0x61 && c <= 0x7a);
}

inline std::string URLEncode(const std::string& url)
{
  std::string ret;
  ret.reserve(url.size() * 3);
  for (auto c : url)
  {
    if (c == ' ')
    {
      ret.push_back('+');
    }
    else if (openvds_isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
    {
      ret.push_back(c);
    }
    else
    {
      ret += fmt::format("%{0:X}",uint8_t(c));
    }
  }
  return ret;
}

}

#endif
