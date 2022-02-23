/****************************************************************************
** Copyright 2019 The Open Group
** Copyright 2019 Bluware, Inc.
** Copyright 2020 Microsoft Corp.
** Copyright 2020 Google, Inc.
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

#include "IOManager.h"

#include "IOManagerInMemory.h"

#ifndef OPENVDS_NO_AWS_IOMANAGER
#include "IOManagerAWS.h"
#include "IOManagerAWSCurl.h"
#endif
#ifndef OPENVDS_NO_AZURE_IOMANAGER
#include "IOManagerAzure.h"
#endif
#ifndef OPENVDS_NO_AZURE_SDK_FOR_CPP_IOMANAGER
#include "IOManagerAzureSdkForCpp.h"
#endif
#ifndef OPENVDS_NO_GCP_IOMANAGER
#include "IOManagerGoogle.h"
#endif
#ifndef OPENVDS_NO_AZURE_PRESIGNED_IOMANAGER
#include "IOManagerAzurePresigned.h"
#endif
#ifndef OPENVDS_NO_HTTP_IOMANAGER
#include "IOManagerHttp.h"
#endif

#include "IOManagerDmsFactory.h"

#include "VDS/Env.h"

namespace OpenVDS
{

IOManager* IOManager::CreateIOManager(const OpenOptions& options, IOManager::AccessPattern accessPattern, Error &error)
{
  switch(options.connectionType)
  {
  case OpenOptions::AWS:
#ifndef OPENVDS_NO_AWS_IOMANAGER
  {
    bool useAwsSdk = getBooleanEnvironmentVariable("OPENVDS_AWSSDK");
    if (useAwsSdk)
      return new IOManagerAWS(static_cast<const AWSOpenOptions&>(options), error);
    return new IOManagerAWSCurl(static_cast<const AWSOpenOptions&>(options), error);
  }
#endif
  break;
  case OpenOptions::Azure:
  {
#ifndef OPENVDS_NO_AZURE_SDK_FOR_CPP_IOMANAGER
    bool useAzureSdkForCpp = getBooleanEnvironmentVariable("OPENVDS_AZURESDKFORCPP");
    if (useAzureSdkForCpp)
      return new IOManagerAzureSdkForCpp(static_cast<const AzureOpenOptions&>(options), error);
#endif
#ifndef OPENVDS_NO_AZURE_IOMANAGER
    if (options.connectionType == OpenOptions::Azure)
      return new IOManagerAzure(static_cast<const AzureOpenOptions&>(options), error);
#endif
    error.code = -1;
    error.string = "Unknown type for OpenOptions";
    return nullptr;
  }
#ifndef OPENVDS_NO_AZURE_PRESIGNED_IOMANAGER
  case OpenOptions::AzurePresigned:
    return new IOManagerAzurePresigned(static_cast<const AzurePresignedOpenOptions&>(options).baseUrl, static_cast<const AzurePresignedOpenOptions&>(options).urlSuffix, error);
#endif
#ifndef OPENVDS_NO_GCP_IOMANAGER
  case OpenOptions::GoogleStorage:
    return new IOManagerGoogle(static_cast<const GoogleOpenOptions &>(options), error);
#endif
#ifndef OPENVDS_NO_HTTP_IOMANAGER
  case OpenOptions::Http:
    return new IOManagerHttp(static_cast<const HttpOpenOptions &>(options), error);
#endif
  case OpenOptions::DMS:
    return CreateDMSIOManager(static_cast<const DMSOpenOptions&>(options), accessPattern, error);
  case OpenOptions::InMemory:
    return IOManagerInMemory::CreateIOManagerInMemory(static_cast<const InMemoryOpenOptions &>(options), error);
  default:
    error.code = -1;
    error.string = "Unknown type for OpenOptions";
    return nullptr;
  }
  return nullptr;
}

IOManager *IOManager::CreateIOManager(const StringWrapper &url, const StringWrapper &connectionString, IOManager::AccessPattern accessPattern, Error& error)
{
  std::unique_ptr<OpenOptions> openOptions(CreateOpenOptions(url, connectionString, error));
  if (error.code || !openOptions)
    return nullptr;
  return CreateIOManager(*(openOptions.get()), accessPattern, error);
}

}
