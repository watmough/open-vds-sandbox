/****************************************************************************
** Copyright 2022 The Open Group
** Copyright 2022 Bluware, Inc.
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

#include "IO/SDPath.h"
#include "VDS/Url.h"

#include <gtest/gtest.h>

std::string make_sd_path(const std::string& tenant, const std::string& subproject, const std::string& path, const std::string& dataset)
{
  if (path.empty())
  {
    return std::string("sd://") + tenant + '/' + subproject + '/' + dataset;
  }
  return std::string("sd://") + tenant + '/' + subproject + '/' + path + '/' + dataset;
}

class AssertPathSuccessExpect : public ::testing::TestWithParam<std::tuple<std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string>>
{

};

TEST_P(AssertPathSuccessExpect, DMS_Path)
{
  const std::string& tenant = std::get<0>(GetParam());
  const std::string& tenant_ex = std::get<1>(GetParam());
  const std::string& subproject = std::get<2>(GetParam());
  const std::string& subproject_ex = std::get<3>(GetParam());
  const std::string& path = std::get<4>(GetParam());
  const std::string& path_ex = std::get<5>(GetParam());
  const std::string& dataset = std::get<6>(GetParam());
  const std::string& dataset_ex = std::get<7>(GetParam());

  std::string sd_path = make_sd_path(tenant, subproject, path, dataset);
  std::string target_tenant;
  std::string target_subproject;
  std::string target_path;
  std::string target_dataset;

  OpenVDS::Error error;
  ASSERT_TRUE(OpenVDS::getSdPath(sd_path, target_tenant, target_subproject, target_path, target_dataset, error));
  ASSERT_EQ(tenant_ex, target_tenant);
  ASSERT_EQ(subproject_ex, target_subproject);
  ASSERT_EQ(path_ex, target_path);
  ASSERT_EQ(dataset_ex, target_dataset);
}

INSTANTIATE_TEST_SUITE_P(
  SuccessfulPathsExpect,
  AssertPathSuccessExpect,
  ::testing::Values(
    std::make_tuple("some_tenant", "some_tenant", "subproject", "subproject", "path/path", "path/path", "dataset", "dataset"),
    std::make_tuple("some_tenant", "some_tenant", "subproject", "subproject", "path/path", "path/path", "dataset/", "dataset"),
    std::make_tuple("some_tenant", "some_tenant", "subproject", "subproject", "path/path/", "path/path", "dataset/", "dataset"),
    std::make_tuple("some_tenant/", "some_tenant", "subproject/", "subproject", "path/path/", "path/path", "dataset/", "dataset"),
    std::make_tuple("some_tenant", "some_tenant", "subproject", "subproject", "", "", "dataset", "dataset"),
    std::make_tuple("some_tenant", "some_tenant", "subproject", "subproject", "/", "", "dataset", "dataset"),
    std::make_tuple("some_tenant", "some_tenant", "subproject", "subproject", "/", "", "/dataset", "dataset"),
    std::make_tuple("some_tenant", "some_tenant", "subproject/", "subproject", "/", "", "/dataset", "dataset"),
    std::make_tuple("some_tenant", "some_tenant", "subproject/", "subproject", "/", "", "/dataset/", "dataset"))
);

TEST(ErrorReporting, DMS_PATH)
{
  std::string target_tenant;
  std::string target_subproject;
  std::string target_path;
  std::string target_dataset;
  OpenVDS::Error error;
  ASSERT_FALSE(OpenVDS::getSdPath("sd://", target_tenant, target_subproject, target_path, target_dataset, error));
  target_tenant.clear();
  target_subproject.clear();
  target_path.clear();
  target_dataset.clear();
  error = OpenVDS::Error();
  ASSERT_FALSE(OpenVDS::getSdPath("sd://some_tenant/", target_tenant, target_subproject, target_path, target_dataset, error));
  target_tenant.clear();
  target_subproject.clear();
  target_path.clear();
  target_dataset.clear();
  error = OpenVDS::Error();
  ASSERT_FALSE(OpenVDS::getSdPath("sd://some_tenant///sub_project", target_tenant, target_subproject, target_path, target_dataset, error));
  target_tenant.clear();
  target_subproject.clear();
  target_path.clear();
  target_dataset.clear();
  error = OpenVDS::Error();
  ASSERT_FALSE(OpenVDS::getSdPath("sd://some_tenant///sub_project/", target_tenant, target_subproject, target_path, target_dataset, error));
}

class UrlEncodeDecode: public ::testing::TestWithParam<std::string>
{
};

TEST_P(UrlEncodeDecode, URL)
{
  std::string to_encode = GetParam();
  std::string encoded = OpenVDS::URLEncode(to_encode);
  std::string decoded = OpenVDS::URLDecode(encoded);
  ASSERT_EQ(to_encode, decoded);
}
INSTANTIATE_TEST_SUITE_P(
  UrlEncoding,
  UrlEncodeDecode,
  ::testing::Values("sd://some_tenant/fo bar",
    "hello £$ % ^^&*bar",
    "somestring#';]!\"£$%^&*())"
  )
);
