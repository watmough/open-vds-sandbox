/****************************************************************************
** Copyright 2019 The Open Group
** Copyright 2019 Bluware, Inc.
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

#include <OpenVDS/OpenVDS.h>

#include "VDS/VDS.h"

#include "VDS/ParseVDSJson.h"

#include <memory>
#include <map>
#include <set>
#include <limits>
#include <functional>

#include <OpenVDS/VolumeDataAccess.h>

#include "VDS/VolumeDataLayoutImpl.h"
#include "VDS/VolumeDataPageAccessorImpl.h"
#include "VDS/VolumeDataAccessManagerImpl.h"
#include "VDS/VolumeDataRequestProcessor.h"
#include "VDS/ConnectionStringParser.h"
#include "VDS/VolumeDataStoreIOManager.h"
#include "VDS/VolumeDataStoreVDSFile.h"
#include "VDS/GlobalStateImpl.h"
#include "VDS/WaveletTypes.h"
#include "VDS/StringToDouble.h"

#include "IO/IOManager.h"
#include "IO/IOManagerTransformer.h"

#include <fmt/format.h>

namespace OpenVDS
{

static std::function<IOManager* (IOManager*)> iomanagerTransformer;

static bool isProtocol(const std::string &str, const std::string &literal)
{
  if (str.size() < literal.size())
    return false;
  std::string protocol(str.data(), str.data() + literal.size());
  std::transform(protocol.begin(), protocol.end(), protocol.begin(), asciitolower);
  return memcmp(protocol.data(), literal.data(), literal.size()) == 0;
}

static std::string removeProtocol(const std::string &str, const std::string &literal)
{
  return std::string(str.begin() + literal.size(), str.end());
}

static std::string urlDecode(const std::string & url)
{
  const char *input = url.data();
  std::vector<char> output;
  output.reserve(url.size());

  for(int i = 0; i < (int)url.size(); i++)
  {
    if(input[i] == '+')
    {
      output.push_back(' ');
    }
    else if(input[i] == '%')
    {
      char temp[5] = "0x";
      if(i + 1 < (int)url.size()) temp[2] = input[++i];
      if(i + 1 < (int)url.size()) temp[3] = input[++i];
      output.push_back(atoi(temp));
    }
    else
    {
      output.push_back(input[i]);
    }
  }
  return std::string(output.data(), output.data() + output.size());
}

static bool isTrue(const std::string& str)
{
  auto value = str;
  std::transform(value.begin(), value.end(), value.begin(), asciitolower);
  static const std::string trueValues[] = { "1", "on", "true", "yes" };
  for (auto& trueValue : trueValues)
  {
    if (value == trueValue)
    {
      return true;
    }
  }

  return false;
}

static std::unique_ptr<OpenOptions> createS3OpenOptions(const std::string &url, const std::string &connectionString, Error &error)
{
  std::unique_ptr<AWSOpenOptions> openOptions(new AWSOpenOptions());

  auto connectionStringMap = ParseConnectionString(connectionString, error);
  if (error.code)
  {
    return nullptr;
  }

  if (url.size() < 1)
  {
    error.code = -1;
    error.string = "S3 url is missing bucket";
  }
  auto end = url.data() + url.size();
  auto bucket_end = std::find(url.data(), end, '/');
  openOptions->bucket = std::string(url.data(), bucket_end);

  auto urlKeyBegin = bucket_end + 1;
  if (urlKeyBegin < end)
  {
    openOptions->key = std::string(urlKeyBegin, end);
  }

  for (auto& connectionPair : connectionStringMap)
  {
    if (connectionPair.first == "region")
    {
      openOptions->region = connectionPair.second;
    }
    else if (connectionPair.first == "endpointoverride" || connectionPair.first == "endpoint_override")
    {
      openOptions->endpointOverride = connectionPair.second;
    }
    else if (connectionPair.first == "accesskeyid" || connectionPair.first == "access_key_id")
    {
      openOptions->accessKeyId = connectionPair.second;
    }
    else if (connectionPair.first == "secretkey" || connectionPair.first == "secretaccesskey"
      || connectionPair.first == "secret_key" || connectionPair.first == "secret_access_key")
    {
      openOptions->secretKey = connectionPair.second;
    }
    else if (connectionPair.first == "sessiontoken" || connectionPair.first == "session_token")
    {
      openOptions->sessionToken = connectionPair.second;
    }
    else if (connectionPair.first == "expiration")
    {
      openOptions->expiration = connectionPair.second;
    }
    else if (connectionPair.first == "logfilenameprefix" || connectionPair.first == "log_filename_prefix")
    {
      openOptions->logFilenamePrefix = connectionPair.second;
    }
    else if (connectionPair.first == "loglevel" || connectionPair.first == "log_level")
    {
      openOptions->loglevel = connectionPair.second;
    }
    else if (connectionPair.first == "connectiontimeoutms" || connectionPair.first == "connection_timeout_ms")
    {
      openOptions->connectionTimeoutMs = strtol(&connectionPair.second[0], nullptr, 10);
      if (openOptions->connectionTimeoutMs == 0)
      {
        error.string = "Invalid connectionTimeoutMs connection string parameter";
        error.code = -1;
        return nullptr;
      }
    }
    else if (connectionPair.first == "requesttimeoutms" || connectionPair.first == "request_timeout_ms")
    {
      openOptions->requestTimeoutMs = strtol(&connectionPair.second[0], nullptr, 10);
      if (openOptions->requestTimeoutMs == 0)
      {
        error.string = "Invalid requestTimeoutMs connection string parameter";
        error.code = -1;
        return nullptr;
      }
    }
    else if (connectionPair.first == "disableinitapi" || connectionPair.first == "disable_init_api")
    {
      openOptions->disableInitApi = isTrue(connectionPair.second);
    }
    else
    {
      error.code = -1;
      error.string = fmt::format("Invalid key \"{}\" in S3 connection string.", connectionPair.first);
      return openOptions;
    }
  }
  return openOptions;
}

static std::unique_ptr<OpenOptions> createAzureOpenOptions(const std::string &url, const std::string &connectionString, Error &error)
{
  std::unique_ptr<AzureOpenOptions> openOptions(new AzureOpenOptions());
  if (url.size() < 1)
  {
    error.code = -1;
    error.string = "Azure url is missing container";
  }
  auto end = url.data() + url.size();
  auto container_end = std::find(url.data(), end, '/');
  openOptions->container = std::string(url.data(), container_end);

  auto urlBlobBegin = container_end + 1;
  if (urlBlobBegin < end)
  {
    openOptions->blob = std::string(urlBlobBegin, end);
  }
  
  auto connectionStringMap = ParseConnectionString(connectionString, error);
  auto bearer_it = connectionStringMap.find("bearertoken");
  if (bearer_it != connectionStringMap.end())
  {
    for (auto& connectionPair : connectionStringMap)
    {
      if (connectionPair.first == "accountname")
        openOptions->accountName = connectionPair.second;
      else if (connectionPair.first == "bearertoken")
      {
        openOptions->bearerToken = connectionPair.second;
      }
      else
      {
        error.code = -1;
        error.string = fmt::format("Invalid key \"{}\" in Azure connection string.", connectionPair.first);
        return nullptr;
      }
    }
    openOptions->bearerToken = bearer_it->second;
    openOptions->accountName = connectionStringMap["accountname"];
  }
  else
  {
    openOptions->connectionString = connectionString;
  }
  return openOptions;
}

static std::unique_ptr<OpenOptions> createAzureSASOpenOptions(const std::string &url, const std::string &connectionString, Error& error)
{
  std::unique_ptr<AzurePresignedOpenOptions> openOptions(new AzurePresignedOpenOptions());
  const char http[] = "https://";
  openOptions->baseUrl.reserve(sizeof(http) - 1 + url.size());
  openOptions->baseUrl.insert(0, http, sizeof(http) - 1);
  openOptions->baseUrl.append(url.data(), url.data() + url.size());

  auto connectionStringMap = ParseConnectionString(connectionString, error);
  for (auto& connectionPair : connectionStringMap)
  {
    if (connectionPair.first == "suffix")
    {
      openOptions->urlSuffix = connectionPair.second;
    }
    else
    {
      error.code = -1;
      error.string = fmt::format("Invalid key \"{}\" in AzureSAS connection string.", connectionPair.first);
      return openOptions;
    }
  }
  return openOptions;
}

const std::set<std::string> TrueWords = { "true", "yes", "on" };
const std::set<std::string> FalseWords = { "false", "no", "off" };

static std::unique_ptr<OpenOptions> createGoogleOpenOptions(const std::string & url, const std::string & connectionString, Error& error)
{
  std::unique_ptr<GoogleOpenOptions> openOptions(new GoogleOpenOptions());
  auto connectionStringMap = ParseConnectionString(connectionString, error);
  if (error.code)
  {
    return nullptr;
  }

  if (url.size() < 1)
  {
    error.code = -1;
    error.string = "GS url is missing bucket";
  }
  auto end = url.data() + url.size();
  auto bucket_end = std::find(url.data(), end, '/');
  openOptions->bucket = std::string(url.data(), bucket_end);

  auto urlKeyBegin = bucket_end + 1;
  if (urlKeyBegin < end)
  {
    openOptions->pathPrefix = std::string(urlKeyBegin, end);
  }

  bool useSignedUrl = false;

  for (auto& connectionPair : connectionStringMap)
  {
    if (connectionPair.first == "token")
    {
      openOptions->credentialsType = GoogleOpenOptions::CredentialsType::AccessToken;
      openOptions->credentials = connectionPair.second;
    }
    else if (connectionPair.first == "credentialsfile")
    {
        openOptions->credentialsType = GoogleOpenOptions::CredentialsType::Path;
        openOptions->credentials = connectionPair.second;
    }
    else if (connectionPair.first == "jsoncredentials")
    {
        openOptions->credentialsType = GoogleOpenOptions::CredentialsType::Json;
        openOptions->credentials = connectionPair.second;
    }
    else if (connectionPair.first == "signedurl")
    {
        std::string value = connectionPair.second;
        std::transform(value.begin(), value.end(), value.begin(), asciitolower);
        if (TrueWords.find(value) != TrueWords.end())
        {
            useSignedUrl = true;
        }
        else if (FalseWords.find(value) == FalseWords.end())
        {
            error.code = -3;
            error.string = fmt::format("Invalid value \"{}\" for SignedUrl in GS connection string.", connectionPair.second);
            return openOptions;
        }
    }
    else
    {
      error.code = -1;
      error.string = fmt::format("Invalid key \"{}\" in GS connection string.", connectionPair.first);
      return openOptions;
    }
  }
  
  if (useSignedUrl)
  {
      if (!openOptions->SetSignedUrl())
      {
          error.code = -2;
          error.string = "SignedUrl option cannot be applied in GS connection string.";
      }
  }

  return openOptions;
}

static std::unique_ptr<OpenOptions> createDMSOpenOptions(const std::string & url, const std::string & connectionString, Error& error)
{
  std::unique_ptr<DMSOpenOptions> openOptions(new DMSOpenOptions());
  openOptions->logLevel = 0;
  auto connectionStringMap = ParseConnectionString(connectionString, error);
  if (error.code)
  {
    return nullptr;
  }

  if (url.size() < 1)
  {
    error.code = -1;
    error.string = "SD url is missing bucket";
  }

  openOptions->datasetPath = url;

  for (auto& connectionPair : connectionStringMap)
  {
    if (connectionPair.first == "sdauthorityurl" || connectionPair.first == "sd_authority_url")
      openOptions->sdAuthorityUrl = connectionPair.second;
    if (connectionPair.first == "sdapikey" || connectionPair.first == "sd_api_key")
      openOptions->sdApiKey = connectionPair.second;
    if (connectionPair.first == "sdtoken" || connectionPair.first == "sd_token")
      openOptions->sdToken = connectionPair.second;
    if (connectionPair.first == "loglevel" || connectionPair.first == "log_level")
      openOptions->logLevel = atoi(connectionPair.second.c_str());
    if (connectionPair.first == "authtokenurl" || connectionPair.first == "auth_token_url")
      openOptions->authTokenUrl = connectionPair.second;
    if (connectionPair.first == "refreshtoken" || connectionPair.first == "refresh_token")
      openOptions->refreshToken = connectionPair.second;
    if (connectionPair.first == "clientid" || connectionPair.first == "client_id")
      openOptions->clientId = connectionPair.second;
    if (connectionPair.first == "clientsecret" || connectionPair.first == "client_secret")
      openOptions->clientSecret = connectionPair.second;
    if (connectionPair.first == "scopes")
      openOptions->scopes = connectionPair.second;
    if (connectionPair.first == "usefilenameforsinglefiledatasets" || connectionPair.first == "use_file_name_for_single_file_datasets"
      || connectionPair.first == "use_filename_for_single_file_datasets")
      openOptions->useFileNameForSingleFileDatasets = isTrue(connectionPair.second);
  }

  return openOptions;
}

static std::unique_ptr<OpenOptions> createHttpOpenOptions(const std::string & url, const std::string & connectionString, Error& error)
{
  std::unique_ptr<HttpOpenOptions> openOptions(new HttpOpenOptions(url));
  return openOptions;
}

static std::unique_ptr<OpenOptions> createVDSFileOpenOptions(const std::string & url, const std::string & connectionString, Error& error)
{
  std::unique_ptr<VDSFileOpenOptions> openOptions(new VDSFileOpenOptions());
  openOptions->fileName = urlDecode(url);
  return openOptions;
}

static std::unique_ptr<OpenOptions> createInMemoryOpenOptions(const std::string & url, const std::string & connectionString, Error& error)
{
  std::unique_ptr<InMemoryOpenOptions> openOptions(new InMemoryOpenOptions());
  openOptions->name =  urlDecode(url);
  return openOptions;
}

typedef std::string (*UrlTransformer)(const std::string &url, const std::string &transformer);
typedef std::unique_ptr<OpenOptions>(*OpenOptionsCreator)(const std::string &url, const std::string &connectionString, Error& error);
struct UrlToOpenOptions
{
  std::string protocol;
  UrlTransformer transformer;
  OpenOptionsCreator creator;
};

static const std::vector<UrlToOpenOptions> urlToOpenOptions = {
  {std::string("s3://"), &removeProtocol, &createS3OpenOptions },
  {std::string("az://"), &removeProtocol, &createAzureOpenOptions },
  {std::string("azure://"), &removeProtocol, &createAzureOpenOptions },
  {std::string("azuresas://"), &removeProtocol, &createAzureSASOpenOptions },
  {std::string("gs://"), &removeProtocol, &createGoogleOpenOptions },
  {std::string("sd://"), nullptr, &createDMSOpenOptions },
  {std::string("http://"), nullptr, &createHttpOpenOptions },
  {std::string("https://"), nullptr, &createHttpOpenOptions },
  {std::string("file://"), &removeProtocol, &createVDSFileOpenOptions },
  {std::string("inmemory://"), &removeProtocol, &createInMemoryOpenOptions}
};

static bool GetWaveletAdaptiveInfo(std::string& connectionString, WaveletAdaptiveMode &mode, float &tolerance, float &ratio, Error &error)
{
  std::vector<std::string> waveletAdaptiveKeys;
  waveletAdaptiveKeys.emplace_back("WaveletAdaptiveTolerance");
  waveletAdaptiveKeys.emplace_back("WaveletAdaptiveRatio");
  auto waveletAdaptivePair = RemoveKeyValue(waveletAdaptiveKeys, connectionString, error);
  if (error.code || waveletAdaptivePair.first < 0)
    return false;
  assert(waveletAdaptivePair.first < int(waveletAdaptiveKeys.size()));
  if (waveletAdaptivePair.first == 0)
  {
    tolerance = float(StringToDouble(waveletAdaptivePair.second, error));
    if (error.code)
      error.string = fmt::format("Connection string parameter WaveletAdaptiveRatio: {}", error.string);
    mode = WaveletAdaptiveMode::Tolerance;
  }
  else if (waveletAdaptivePair.first == 0)
  {
    ratio = float(StringToDouble(waveletAdaptivePair.second, error));
    if (error.code)
      error.string = fmt::format("Connection string parameter WaveletAdaptiveRatio: {}", error.string);
    mode = WaveletAdaptiveMode::Ratio;
  }
  return true;
}

OpenOptions* CreateOpenOptions(StringWrapper urlWrapper, StringWrapper connectionStringWrapper, Error& error)
{
  error = Error();
  std::unique_ptr<OpenOptions> openOptions;
  
  std::string url(urlWrapper.data, urlWrapper.size);
  std::string connectionString(connectionStringWrapper.data, connectionStringWrapper.size);

  WaveletAdaptiveMode adaptiveMode;
  float adaptiveTolerance;
  float adaptiveRatio;
  bool adaptiveDataSet = GetWaveletAdaptiveInfo(connectionString, adaptiveMode, adaptiveTolerance, adaptiveRatio, error);
  if (error.code)
    return nullptr;

  for (auto& urlToOpenOption : urlToOpenOptions)
  {
    if (!isProtocol(url, urlToOpenOption.protocol))
      continue;
    auto transformed_url = urlToOpenOption.transformer ? urlToOpenOption.transformer(url, urlToOpenOption.protocol) : url;
    openOptions = urlToOpenOption.creator(transformed_url, connectionString, error);
    break;
  }

  if (error.code)
  {
    return nullptr;
  }

  if (!openOptions)
  {
    openOptions.reset(new VDSFileOpenOptions(url));
  }

  if (openOptions && adaptiveDataSet)
  {
    openOptions->waveletAdaptiveMode = adaptiveMode;
    if (adaptiveMode == WaveletAdaptiveMode::Ratio)
      openOptions->waveletAdaptiveRatio = adaptiveRatio;
    if (adaptiveMode == WaveletAdaptiveMode::Tolerance)
      openOptions->waveletAdaptiveTolerance = adaptiveTolerance;
  }

  return openOptions.release();
}

bool IsSupportedProtocol(StringWrapper url)
{
  std::string u(url.data, url.size);
  for (auto& urlToOpenOpiton : urlToOpenOptions)
  {
    if (isProtocol(u, urlToOpenOpiton.protocol))
      return true;
  }
  return false;
}

VDSHandle Open(StringWrapper url, StringWrapper connectionString, Error& error)
{
  std::unique_ptr<OpenOptions> openOptions(CreateOpenOptions(url, connectionString, error));
  if (error.code || !openOptions)
    return nullptr;

  return Open(*(openOptions.get()), error);
}

VDSHandle OpenWithAdaptiveCompressionTolerance(StringWrapper url, StringWrapper connectionString, float waveletAdaptiveTolerance, Error& error)
{
  std::unique_ptr<OpenOptions> openOptions(CreateOpenOptions(url, connectionString, error));
  if (error.code || !openOptions)
    return nullptr;

  openOptions->waveletAdaptiveMode = WaveletAdaptiveMode::Tolerance;
  openOptions->waveletAdaptiveTolerance = waveletAdaptiveTolerance;

  return Open(*(openOptions.get()), error);
}

VDSHandle OpenWithAdaptiveCompressionRatio(StringWrapper url, StringWrapper connectionString, float waveletAdaptiveRatio, Error& error)
{
  std::unique_ptr<OpenOptions> openOptions(CreateOpenOptions(url, connectionString, error));
  if (error.code || !openOptions)
    return nullptr;

  openOptions->waveletAdaptiveMode = WaveletAdaptiveMode::Ratio;
  openOptions->waveletAdaptiveRatio = waveletAdaptiveRatio;

  return Open(*(openOptions.get()), error);
}

static bool Init(VDS *vds, VolumeDataStore *volumeDataStore, Error& error)
{
  vds->produceStatuses.clear();
  vds->produceStatuses.resize(int(Dimensions_45) + 1, VolumeDataLayer::ProduceStatus_Unavailable);

  vds->volumeDataStore.reset(volumeDataStore);

  std::vector<uint8_t> serializedVolumeDataLayout;
  if(!vds->volumeDataStore->ReadSerializedVolumeDataLayout(serializedVolumeDataLayout, error))
  {
    return false;
  }
  if(!ParseVolumeDataLayout(serializedVolumeDataLayout, vds->layoutDescriptor, vds->axisDescriptors, vds->channelDescriptors, vds->descriptorStrings, vds->metadataContainer, error))
  {
    return false;
  }
  CreateVolumeDataLayout(*vds);

  vds->accessManager = std::shared_ptr<VolumeDataAccessManagerImpl>(VolumeDataAccessManagerImpl::Create(*vds), [](VolumeDataAccessManagerImpl *accessManager) { accessManager->Release(); });
  return true;
}

void InitWaveletAdaptiveLoadLevel(VDS &vds, OpenOptions const &options)
{
  assert(vds.volumeDataLayout.get());

  DimensionGroup
    dimensionGroup = (vds.volumeDataLayout->GetDimensionality() == 2) ? DimensionGroup::DimensionGroup_01 : DimensionGroup::DimensionGroup_012;

  VolumeDataLayer *volumeDataLayer = vds.volumeDataLayout->GetBaseLayer(dimensionGroup, 0);

  if(!volumeDataLayer || volumeDataLayer->GetProduceStatus() != VolumeDataLayer::ProduceStatus_Normal)
  {
    for (int i = DimensionGroup::DimensionGroup_01; i < DimensionGroup::DimensionGroup_3D_Max; i++)
    {
      dimensionGroup = DimensionGroup(i);
      volumeDataLayer = vds.volumeDataLayout->GetBaseLayer(dimensionGroup, 0);
      if(volumeDataLayer && volumeDataLayer->GetProduceStatus() == VolumeDataLayer::ProduceStatus_Normal)
      {
        break;
      }
    }
  }

  if(volumeDataLayer && volumeDataLayer->GetProduceStatus() == VolumeDataLayer::ProduceStatus_Normal)
  {
    CompressionInfo
      compressionInfo = vds.volumeDataStore->GetEffectiveAdaptiveLevel(volumeDataLayer, options.waveletAdaptiveMode, options.waveletAdaptiveTolerance, options.waveletAdaptiveRatio);

    vds.volumeDataLayout->SetCompressionMethod(compressionInfo.GetCompressionMethod());
    vds.volumeDataLayout->SetCompressionTolerance(compressionInfo.GetTolerance());
    vds.volumeDataLayout->SetWaveletAdaptiveLoadLevel(compressionInfo.GetAdaptiveLevel());
  }
}

VDSHandle Open(IOManager *ioManager, Error& error)
{
  std::unique_ptr<VDS> ret(new VDS());
  error = Error();

  if(Init(ret.get(), new VolumeDataStoreIOManager(*ret, ioManager), error))
  {
    return ret.release();
  }
  else
  {
    return nullptr;
  }
}

VDS *Open(const OpenOptions &options, Error &error)
{
  std::unique_ptr<VDS> ret(new VDS());
  std::unique_ptr<VolumeDataStore> volumeDataStore;
  error = Error();

  if(options.connectionType != OpenOptions::VDSFile)
  {
    std::unique_ptr<IOManager> ioManager(IOManager::CreateIOManager(options, IOManager::AccessPattern::ReadOnly, error));
    if (error.code)
      return nullptr;

    if (iomanagerTransformer)
      ioManager.reset(iomanagerTransformer(ioManager.release()));

    volumeDataStore.reset(new VolumeDataStoreIOManager(*ret, ioManager.release()));
  }
  else
  {
    const VDSFileOpenOptions &fileOptions = static_cast<const VDSFileOpenOptions &>(options);
    volumeDataStore.reset(new VolumeDataStoreVDSFile(*ret, fileOptions.fileName, VolumeDataStoreVDSFile::ReadOnly, error));
    if (error.code)
      return nullptr;
  }

  if(Init(ret.get(), volumeDataStore.release(), error))
  {
    assert(ret.get());
    InitWaveletAdaptiveLoadLevel(*ret.get(), options);
    return ret.release();
  }
  else
  {
    return nullptr;
  }
}

VolumeDataLayout *GetLayout(VDS *vds)
{
  if (!vds)
    return nullptr;
  return vds->volumeDataLayout.get();
}

IVolumeDataAccessManager *GetAccessManagerInterface(VDS *vds)
{
  if (!vds)
    return nullptr;
  return vds->accessManager.get();
}

static int32_t GetInternalCubeSizeLOD0(const VolumeDataLayoutDescriptor &desc)
{
  int32_t size = int32_t(1) << desc.GetBrickSize();

  size -= desc.GetNegativeMargin();
  size -= desc.GetPositiveMargin();

  assert(size > 0);

  return size;
}

static int32_t GetLODCount(const VolumeDataLayoutDescriptor &desc)
{
  return desc.GetLODLevels() + 1;
}

std::string GetLayerName(VolumeDataLayer const &volumeDataLayer)
{
  if(volumeDataLayer.GetChannelIndex() == 0)
  {
    return fmt::format("{}LOD{}", DimensionGroupUtil::GetDimensionGroupName(volumeDataLayer.GetChunkDimensionGroup()), volumeDataLayer.GetLOD());
  }
  else
  {
    assert(std::string(volumeDataLayer.GetVolumeDataChannelDescriptor().GetName()) != "");
    return fmt::format("{}{}LOD{}", volumeDataLayer.GetVolumeDataChannelDescriptor().GetName(), DimensionGroupUtil::GetDimensionGroupName(volumeDataLayer.GetPrimaryChannelLayer().GetChunkDimensionGroup()), volumeDataLayer.GetLOD());
  }
}

void CreateVolumeDataLayout(VDS &vds, CompressionMethod compressionMethod, float compressionTolerance)
{
  int32_t dimensionality = int32_t(vds.axisDescriptors.size());

  // Check if input layouts are valid so we can create a new layout
  if (dimensionality < 2)
  {
    vds.volumeDataLayout.reset();
    return;
  }

  // Update compression settings if they are outside effectively valid ranges,
  if (compressionTolerance < WAVELET_MIN_COMPRESSION_TOLERANCE)
  {
    compressionTolerance = WAVELET_MIN_COMPRESSION_TOLERANCE;
  }

  if (compressionMethod == CompressionMethod::WaveletLossless || compressionMethod == CompressionMethod::WaveletNormalizeBlockLossless)
  {
    compressionTolerance = WAVELET_MIN_COMPRESSION_TOLERANCE;
  }

  const int actualValueRangeChannel = -1;
  const FloatRange actualValueRange = FloatRange(1, 0);
  const int waveletAdaptiveLoadLevel = -1;

  vds.volumeDataLayout.reset(
    new VolumeDataLayoutImpl(
      vds,
      vds.layoutDescriptor,
      vds.axisDescriptors,
      vds.channelDescriptors,
      actualValueRangeChannel,
      actualValueRange,
      VolumeDataHash::GetUniqueHash(),
      compressionMethod,
      compressionTolerance,
      false,
      waveletAdaptiveLoadLevel));

  for(int32_t dimensionGroupIndex = 0; dimensionGroupIndex < DimensionGroup_3D_Max; dimensionGroupIndex++)
  {
    DimensionGroup dimensionGroup = (DimensionGroup)dimensionGroupIndex;

    int32_t nChunkDimensionality = DimensionGroupUtil::GetDimensionality(dimensionGroup);

        // Check if highest dimension in chunk is higher than the highest dimension in the dataset or 1D
    if(DimensionGroupUtil::GetDimension(dimensionGroup, nChunkDimensionality - 1) >= dimensionality ||
       nChunkDimensionality == 1)
    {
      continue;
    }

    assert(nChunkDimensionality == 2 || nChunkDimensionality == 3);

    int32_t physicalLODLevels = (nChunkDimensionality == 3 || vds.layoutDescriptor.IsCreate2DLODs()) ? GetLODCount(vds.layoutDescriptor) : 1;
    int32_t brickSize = GetInternalCubeSizeLOD0(vds.layoutDescriptor) * (nChunkDimensionality == 2 ? vds.layoutDescriptor.GetBrickSizeMultiplier2D() : 1);

    vds.volumeDataLayout->CreateLayers(dimensionGroup, brickSize, physicalLODLevels, vds.produceStatuses[DimensionGroupUtil::GetDimensionsNDFromDimensionGroup(dimensionGroup)]);
  }
}

static void copyMetadataToContainer(MetadataContainer &container, const MetadataReadAccess &readAccess)
{
  std::unordered_set<std::string> categories;
  for (auto &key : readAccess.GetMetadataKeys())
  {
    categories.insert(key.GetCategory());
  }
  for (auto &category : categories)
  {
    container.CopyMetadata(category.c_str(), &readAccess);
  }
}

static bool Init(VDS *vds, VolumeDataStore* volumeDataStore, VolumeDataLayoutDescriptor const &layoutDescriptor, VectorWrapper<VolumeDataAxisDescriptor> axisDescriptors, VectorWrapper<VolumeDataChannelDescriptor> channelDescriptors, MetadataReadAccess const &metadata, CompressionMethod compressionMethod, float compressionTolerance, Error &error)
{
  if(!VolumeDataStore::IsCompressionMethodSupported(compressionMethod))
  {
    error.code = -1;
    error.string = "Unsupported compression method";
    return false;
  }

  vds->produceStatuses.clear();
  vds->produceStatuses.resize(int(Dimensions_45) + 1, VolumeDataLayer::ProduceStatus_Unavailable);

  vds->volumeDataStore.reset(volumeDataStore);
  vds->layoutDescriptor = layoutDescriptor;

  DescriptorStringContainer &
    descriptorStrings = vds->descriptorStrings;

  for(size_t i = 0; i < axisDescriptors.size; i++)
  {
    auto &axisDescriptor = axisDescriptors.data[i];
    vds->axisDescriptors.push_back(VolumeDataAxisDescriptor(axisDescriptor.GetNumSamples(), descriptorStrings.Add(axisDescriptor.GetName()), descriptorStrings.Add(axisDescriptor.GetUnit()), axisDescriptor.GetCoordinateMin(), axisDescriptor.GetCoordinateMax()));
  }

  for(size_t i = 0; i < channelDescriptors.size; i++)
  {
    auto &channelDescriptor = channelDescriptors.data[i];
    VolumeDataChannelDescriptor::Flags flags = VolumeDataChannelDescriptor::Default;

    if(channelDescriptor.IsDiscrete())                     flags = flags | VolumeDataChannelDescriptor::DiscreteData;
    if(!channelDescriptor.IsAllowLossyCompression())       flags = flags | VolumeDataChannelDescriptor::NoLossyCompression;
    if(channelDescriptor.IsUseZipForLosslessCompression()) flags = flags | VolumeDataChannelDescriptor::NoLossyCompressionUseZip;
    if(!channelDescriptor.IsRenderable())                  flags = flags | VolumeDataChannelDescriptor::NotRenderable;

    if(channelDescriptor.IsUseNoValue())
    {
      vds->channelDescriptors.push_back(VolumeDataChannelDescriptor(channelDescriptor.GetFormat(), channelDescriptor.GetComponents(), descriptorStrings.Add(channelDescriptor.GetName()), descriptorStrings.Add(channelDescriptor.GetUnit()), channelDescriptor.GetValueRangeMin(), channelDescriptor.GetValueRangeMax(), channelDescriptor.GetMapping(), channelDescriptor.GetMappedValueCount(), flags, channelDescriptor.GetNoValue(), channelDescriptor.GetIntegerScale(), channelDescriptor.GetIntegerOffset()));
    }
    else
    {
      vds->channelDescriptors.push_back(VolumeDataChannelDescriptor(channelDescriptor.GetFormat(), channelDescriptor.GetComponents(), descriptorStrings.Add(channelDescriptor.GetName()), descriptorStrings.Add(channelDescriptor.GetUnit()), channelDescriptor.GetValueRangeMin(), channelDescriptor.GetValueRangeMax(), channelDescriptor.GetMapping(), channelDescriptor.GetMappedValueCount(), flags, channelDescriptor.GetIntegerScale(), channelDescriptor.GetIntegerOffset()));
    }
  }

  copyMetadataToContainer(vds->metadataContainer, metadata);

  CreateVolumeDataLayout(*vds, compressionMethod, compressionTolerance);

  vds->produceStatuses.clear();
  vds->produceStatuses.resize(int(Dimensions_45) + 1, VolumeDataLayer::ProduceStatus_Unavailable);

  if (!vds->volumeDataStore->WriteSerializedVolumeDataLayout(SerializeVolumeDataLayout(*vds), error))
    return false;

  vds->accessManager = std::shared_ptr<VolumeDataAccessManagerImpl>(VolumeDataAccessManagerImpl::Create(*vds), [](VolumeDataAccessManagerImpl *accessManager) { accessManager->Release(); });
  return true;
}

bool IsCompressionMethodSupported(CompressionMethod compressionMethod)
{
  return VolumeDataStore::IsCompressionMethodSupported(compressionMethod);
}

VDSHandle Create(IOManager *ioManager, VolumeDataLayoutDescriptor const& layoutDescriptor, VectorWrapper<VolumeDataAxisDescriptor> axisDescriptors, VectorWrapper<VolumeDataChannelDescriptor> channelDescriptors, MetadataReadAccess const& metadata, CompressionMethod compressionMethod, float compressionTolerance, Error& error)
{
  std::unique_ptr<VDS> ret(new VDS());
  error = Error();

  if(Init(ret.get(), new VolumeDataStoreIOManager(*ret, ioManager), layoutDescriptor, axisDescriptors, channelDescriptors, metadata, compressionMethod, compressionTolerance, error))
  {
    return ret.release();
  }
  else
  {
    return nullptr;
  }
}

VDSHandle Create(StringWrapper url, StringWrapper connectionString, VolumeDataLayoutDescriptor const& layoutDescriptor, VectorWrapper<VolumeDataAxisDescriptor> axisDescriptors, VectorWrapper<VolumeDataChannelDescriptor> channelDescriptors, MetadataReadAccess const& metadata, CompressionMethod compressionMethod, float compressionTolerance, Error& error)
{
  std::unique_ptr<OpenOptions> openOptions(CreateOpenOptions(url, connectionString, error));
  if (error.code || !openOptions)
    return nullptr;

  return Create(*openOptions, layoutDescriptor, axisDescriptors, channelDescriptors, metadata, compressionMethod, compressionTolerance, error);
}

VDSHandle Create(const OpenOptions& options, VolumeDataLayoutDescriptor const& layoutDescriptor, VectorWrapper<VolumeDataAxisDescriptor> axisDescriptors, VectorWrapper<VolumeDataChannelDescriptor> channelDescriptors, MetadataReadAccess const& metadata, CompressionMethod compressionMethod, float compressionTolerance, Error& error)
{
  std::unique_ptr<VDS> ret(new VDS());
  std::unique_ptr<VolumeDataStore> volumeDataStore;
  error = Error();

  if(options.connectionType != OpenOptions::VDSFile)
  {
    std::unique_ptr<IOManager> ioManager(IOManager::CreateIOManager(options, IOManager::AccessPattern::ReadWrite, error));
    if (error.code)
      return nullptr;

    volumeDataStore.reset(new VolumeDataStoreIOManager(*ret, ioManager.release()));
  }
  else
  {
    const VDSFileOpenOptions &fileOptions = static_cast<const VDSFileOpenOptions &>(options);
    volumeDataStore.reset(new VolumeDataStoreVDSFile(*ret, fileOptions.fileName, VolumeDataStoreVDSFile::Create, error));
    if (error.code)
      return nullptr;
  }

  if(Init(ret.get(), volumeDataStore.release(), layoutDescriptor, axisDescriptors, channelDescriptors, metadata, compressionMethod, compressionTolerance, error))
  {
    return ret.release();
  }
  else
  {
    return nullptr;
  }
}

CompressionMethod GetCompressionMethod(VDSHandle handle)
{
  return handle->volumeDataLayout->GetCompressionMethod();
}

float GetCompressionTolerance(VDSHandle handle)
{
  return handle->volumeDataLayout->GetCompressionTolerance();
}

void Close(VDS *vds)
{
  if (!vds)
    return;
  vds->accessManager->Invalidate();
  delete vds;
}

GlobalState *GetGlobalState()
{
  static GlobalStateImpl globalState;
  return &globalState;
}


void SetIoManagerTransformer(std::function<IOManager* (IOManager*)> transformer)
{
  iomanagerTransformer = transformer;
}

const char *GetOpenVDSName()
{
  return PROJECT_NAME;
}

const char *GetOpenVDSVersion()
{
  return PROJECT_VERSION;
}
}
