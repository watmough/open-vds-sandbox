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

#include <VDS/ConnectionStringParser.h>
#include <gtest/gtest.h>

std::vector<std::string> mkStringVec(const std::string& str)
{
  std::vector<std::string> ret;
  ret.emplace_back(str);
  return ret;
}
GTEST_TEST(VDS_integration, ParseConnectionString)
{
  OpenVDS::Error error;
  std::string empty;
  auto map = OpenVDS::ParseConnectionString(empty.data(), empty.size(), error);
  EXPECT_TRUE(map.empty());
  EXPECT_EQ(error.code, 0);

  std::string emptyValue = "Foo=hello;bar=";
  map = OpenVDS::ParseConnectionString(emptyValue.data(), emptyValue.size(), error);
  EXPECT_EQ(map.size(), 2);
  EXPECT_EQ(error.code, 0);
  
  std::string emptyValue2 = "Foo=;bar=hello";
  map = OpenVDS::ParseConnectionString(emptyValue2.data(), emptyValue2.size(), error);
  EXPECT_EQ(map.size(), 2);
  EXPECT_EQ(error.code, 0);

  std::string emptyKey = "Foo=bar;=hello";
  map = OpenVDS::ParseConnectionString(emptyKey.data(), emptyKey.size(), error);
  EXPECT_NE(error.code, 0);
 
  error = OpenVDS::Error();
  std::string doubleSemicolon= "Foo=bar;;hello=hello";
  map = OpenVDS::ParseConnectionString(doubleSemicolon.data(), doubleSemicolon.size(), error);
  EXPECT_EQ(error.code, 0);
  EXPECT_EQ(map.size(), 2);
  
  error = OpenVDS::Error();
  std::string manySemicolons= ";Foo=bar;;hello=hello;;";
  map = OpenVDS::ParseConnectionString(manySemicolons.data(), manySemicolons.size(), error);
  EXPECT_EQ(error.code, 0);
  EXPECT_EQ(map.size(), 2);
  
  error = OpenVDS::Error();
  std::string noEquals = "Foo;hello=hello";
  map = OpenVDS::ParseConnectionString(noEquals.data(), noEquals.size(), error);
  EXPECT_NE(error.code, 0);
  
  error = OpenVDS::Error();
  std::string noEquals2 = "Foo=bar;hello";
  map = OpenVDS::ParseConnectionString(noEquals2.data(), noEquals2.size(), error);
  EXPECT_NE(error.code, 0);

  error = OpenVDS::Error();
  std::string duplicates1 = "Foo=bar;Foo=foo";
  map = OpenVDS::ParseConnectionString(duplicates1.data(), duplicates1.size(), error);
  EXPECT_NE(error.code, 0);

  error = OpenVDS::Error();
  std::string duplicates2 = "Foo=bar;Bar=foo;Foo=foo";
  map = OpenVDS::ParseConnectionString(duplicates2.data(), duplicates2.size(), error);
  EXPECT_NE(error.code, 0);
  
  error = OpenVDS::Error();
  std::string duplicates3 = "Bar=foo;Foo=bar;Foo=foo";
  map = OpenVDS::ParseConnectionString(duplicates3.data(), duplicates3.size(), error);
  EXPECT_NE(error.code, 0);
  
  error = OpenVDS::Error();
  std::string duplicates4 = "Foo=bar;Foo=foo;Bar=foo";
  map = OpenVDS::ParseConnectionString(duplicates4.data(), duplicates4.size(), error);
  EXPECT_NE(error.code, 0);

  {
    error = OpenVDS::Error();
    std::string remove = "Hello=World;Another=Element";
    auto removed = OpenVDS::RemoveKeyValue(mkStringVec("Hello"), remove, error);
    EXPECT_EQ(error.code, 0);
    EXPECT_EQ(remove, "Another=Element");
    EXPECT_EQ(removed.first, 0);
    EXPECT_EQ(removed.second, "World");
  }
  {
    error = OpenVDS::Error();
    std::string remove = "Hello=World;Another=Element";
    auto removed = OpenVDS::RemoveKeyValue(mkStringVec("Another"), remove, error);
    EXPECT_EQ(error.code, 0);
    EXPECT_EQ(remove, "Hello=World");
    EXPECT_EQ(removed.second, "Element");
  }
  {
    error = OpenVDS::Error();
    std::string remove = "Hello=World;The=Middle;Another=Element";
    auto removed = OpenVDS::RemoveKeyValue(mkStringVec("The"), remove, error);
    EXPECT_EQ(error.code, 0);
    EXPECT_EQ(remove, "Hello=World;Another=Element");
    EXPECT_EQ(removed.second, "Middle");
  }
  {
    error = OpenVDS::Error();
    std::string remove = "The=Only";
    auto removed = OpenVDS::RemoveKeyValue(mkStringVec("The"), remove, error);
    EXPECT_EQ(error.code, 0);
    EXPECT_EQ(remove, "");
    EXPECT_EQ(removed.second, "Only");
  }
  {
    error = OpenVDS::Error();
    std::string remove = "The=Only;Duplicates=AreNotAllowed;The=Twice";
    auto removed = OpenVDS::RemoveKeyValue(mkStringVec("The"), remove, error);
    EXPECT_EQ(error.code, -1);
  }
  {
    error = OpenVDS::Error();
    std::string remove = "Some=Thing;The=Only;The=Twice;Duplicates=AreNotAllowed;More=Data";
    auto removed = OpenVDS::RemoveKeyValue(mkStringVec("The"), remove, error);
    EXPECT_EQ(error.code, -1);
  }
  {
    error = OpenVDS::Error();
    std::string remove = "The=Only;The=Twice;Duplicates=ShouldBeRemoved;More=Data";
    auto removed = OpenVDS::RemoveKeyValue(mkStringVec("The"), remove, error);
    EXPECT_EQ(error.code, -1);
  }
  {
    error = OpenVDS::Error();
    std::string remove = "Duplicates=ShouldBeRemoved;More=Data;The=Only;The=Twice";
    auto removed = OpenVDS::RemoveKeyValue(mkStringVec("The"), remove, error);
    EXPECT_EQ(error.code, -1);
  }
  {
    error = OpenVDS::Error();
    std::string remove = "Duplicates=ShouldBeRemoved;More=Data;The=Only";
    std::vector<std::string> keys;
    keys.emplace_back("more");
    keys.emplace_back("the");
    auto removed = OpenVDS::RemoveKeyValue(keys, remove, error);
    EXPECT_EQ(error.code, -1);
  }
  {
    error = OpenVDS::Error();
    std::string remove = "Duplicates=;More=Data;The=Only";
    std::vector<std::string> keys;
    keys.emplace_back("more");
    keys.emplace_back("the");
    auto removed = OpenVDS::RemoveKeyValue(keys, remove, error);
    EXPECT_EQ(error.code, -1);
  }
  {
    error = OpenVDS::Error();
    std::string remove = "More=Data;The=Twice;Duplicates=";
    std::vector<std::string> keys;
    keys.emplace_back("more");
    keys.emplace_back("the");
    auto removed = OpenVDS::RemoveKeyValue(keys, remove, error);
    EXPECT_EQ(error.code, -1);
  }
  {
    error = OpenVDS::Error();
    std::string remove = "More=Data;Duplicates=;The=";
    std::vector<std::string> keys;
    keys.emplace_back("more");
    keys.emplace_back("the");
    auto removed = OpenVDS::RemoveKeyValue(keys, remove, error);
    EXPECT_EQ(error.code, -1);
  }

}
