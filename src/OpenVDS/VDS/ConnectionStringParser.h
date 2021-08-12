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

#ifndef CONNECTIONSTRINGPARSER_H
#define CONNECTIONSTRINGPARSER_H

#include <OpenVDS/OpenVDS.h>
#include <string>
#include <algorithm>
#include <map>
#include <fmt/format.h>

#include <cctype>

namespace OpenVDS
{

inline char asciitolower(char in) {
  if (in <= 'Z' && in >= 'A')
    return in - ('Z' - 'z');
  return in;
}

template<typename IT>
inline std::string trim(IT start, IT end)
{
  while (std::isspace(*start) && start < end)
    start++;
  end--;
  while (end > start && std::isspace(*end))
    end--;
  return std::string(start, end + 1);
}

template<typename It, typename Callback>
inline void TokenizeConnectionString(It begin, It end, Callback callback, Error& error)
{
  auto it = begin;
  while (it < end)
  {
    auto keyValueEnd = std::find(it, end, ';');
    auto equals = std::find(it, keyValueEnd, '=');
    auto name_begin = it;
    auto name_end = equals;
    it = equals + 1;

    std::string name = trim(name_begin, name_end);
    if (name.size() && equals >= keyValueEnd)
    {
      error.code = - 1;
      error.string = fmt::format("Missing required = sign for name {} in connection string. Every key has to have a = sign next to it.", name);
      return;
    }

    if (name.empty() && it < keyValueEnd)
    {
      error.code = - 1;
      error.string = fmt::format("Empty name in connection string. Name must consist of more than empty spaces.");
      return;
    }
   
    if (it > keyValueEnd)
      continue;

    std::transform(name.begin(), name.end(), name.begin(), asciitolower);
    std::string value = it == keyValueEnd ? "" : trim(it, keyValueEnd);
    It newKeyValueEnd = keyValueEnd;
    int endAdjust = 0;
    callback(name_begin, keyValueEnd, std::move(name), std::move(value), &newKeyValueEnd, &endAdjust);

    if (keyValueEnd != newKeyValueEnd)
    {
      keyValueEnd = newKeyValueEnd;
      end += endAdjust;
      it = keyValueEnd;
    }
    else
    {
      it = keyValueEnd < end ? keyValueEnd + 1 : keyValueEnd;
    }
  }
  return;
}

inline std::pair<int, std::string> RemoveKeyValue(const std::vector<std::string> & matchKeys, std::string &connectionString, Error &error)
{
  using It = decltype(connectionString.begin());
  std::pair<int, std::string> ret = std::make_pair(-1, std::string());
  std::vector<std::string> lowercaseMatchKeys = matchKeys;
  for (auto& lowercaseMatchKey : lowercaseMatchKeys)
  {
    std::transform(lowercaseMatchKey.begin(), lowercaseMatchKey.end(), lowercaseMatchKey.begin(), asciitolower);
  }
  TokenizeConnectionString(connectionString.begin(), connectionString.end(),
    [&lowercaseMatchKeys, &connectionString, &ret](It keyValueBegin, It keyValueEnd, std::string&& key, std::string&& value, It *newKeyValueEnd, int *endAdjust)
  {
    auto it = std::find(lowercaseMatchKeys.begin(), lowercaseMatchKeys.end(), key);
    if (it != lowercaseMatchKeys.end())
    {
      It eraseBegin = keyValueBegin;
      It eraseEnd = keyValueEnd;
      if (keyValueBegin != connectionString.begin())
        eraseBegin--;
      else if (keyValueEnd != connectionString.end())
        eraseEnd++;
      connectionString.erase(eraseBegin, eraseEnd);
      ret = std::make_pair(int(it - lowercaseMatchKeys.begin()), std::move(value));
      *endAdjust = int(eraseBegin - eraseEnd);
      *newKeyValueEnd = eraseEnd + *endAdjust;
    }
  }
  , error);
  return ret;
}

inline std::map<std::string, std::string> ParseConnectionString(const char* connectionString, size_t connectionStringSize, Error &error)
{
  std::map<std::string, std::string> ret;
  auto end = connectionString + connectionStringSize;
  using It = const char*;
  TokenizeConnectionString(connectionString, end,
    [&ret](It keyValueBegin, It keyValueEnd, std::string&& key, std::string&& value, It *newKeyValueEnd, int *endAdjust)
  {
    ret.emplace(std::move(key), std::move(value));
  }, error);
  return ret;
}
inline std::map<std::string, std::string> ParseConnectionString(const std::string connectionString, Error& error)
{
  return ParseConnectionString(connectionString.data(), connectionString.size(), error);
}
}

#endif //CONNECTIONSTRINGPARSER_H
