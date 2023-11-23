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

#include "PyGlobal.h"

#include <OpenVDS/Vector.h>
using namespace native;


void 
PyGlobal::initModule(py::module& m)
{
  // These are opaque pointers, so they must not be destructed from pybind11 code
  py::class_<VDS, std::unique_ptr<VDS, py::nodelete>>
    VDS_(m, "VDS");
  VDS_.def("__int__", [](VDS* self) { return (int64_t)self; });
  VDS_.def("__enter__", [](VDS *self) { return self;});
  VDS_.def("__exit__", [](VDS *self, py::args) { OpenVDS::Close(reinterpret_cast<OpenVDS::VDSHandle>(self)); });

  py::class_<IOManager, std::unique_ptr<IOManager, py::nodelete>>(m, "IOManager");

//AUTOGEN-BEGIN
  py::enum_<WaveletAdaptiveMode> 
    WaveletAdaptiveMode_(m,"WaveletAdaptiveMode", OPENVDS_DOCSTRING(WaveletAdaptiveMode));

  WaveletAdaptiveMode_.value("BestQuality"                 , WaveletAdaptiveMode::BestQuality        , OPENVDS_DOCSTRING(WaveletAdaptiveMode_BestQuality));
  WaveletAdaptiveMode_.value("Tolerance"                   , WaveletAdaptiveMode::Tolerance          , OPENVDS_DOCSTRING(WaveletAdaptiveMode_Tolerance));
  WaveletAdaptiveMode_.value("Ratio"                       , WaveletAdaptiveMode::Ratio              , OPENVDS_DOCSTRING(WaveletAdaptiveMode_Ratio));

  // OpenOptions
  py::class_<OpenOptions> 
    OpenOptions_(m,"OpenOptions", OPENVDS_DOCSTRING(OpenOptions));

  OpenOptions_.def_readwrite("connectionType"              , &OpenOptions::connectionType   , OPENVDS_DOCSTRING(OpenOptions_connectionType));
  OpenOptions_.def_readwrite("waveletAdaptiveMode"         , &OpenOptions::waveletAdaptiveMode, OPENVDS_DOCSTRING(OpenOptions_waveletAdaptiveMode));
  OpenOptions_.def_readwrite("waveletAdaptiveTolerance"    , &OpenOptions::waveletAdaptiveTolerance, OPENVDS_DOCSTRING(OpenOptions_waveletAdaptiveTolerance));
  OpenOptions_.def_readwrite("waveletAdaptiveRatio"        , &OpenOptions::waveletAdaptiveRatio, OPENVDS_DOCSTRING(OpenOptions_waveletAdaptiveRatio));
  OpenOptions_.def_readwrite("requestThreadCount"          , &OpenOptions::requestThreadCount, OPENVDS_DOCSTRING(OpenOptions_requestThreadCount));
  OpenOptions_.def_readwrite("logLevel"                    , &OpenOptions::logLevel         , OPENVDS_DOCSTRING(OpenOptions_logLevel));

  py::enum_<OpenOptions::ConnectionType> 
    OpenOptions_ConnectionType_(OpenOptions_,"ConnectionType", OPENVDS_DOCSTRING(OpenOptions_ConnectionType));

  OpenOptions_ConnectionType_.value("AWS"                         , OpenOptions::ConnectionType::AWS        , OPENVDS_DOCSTRING(OpenOptions_ConnectionType_AWS));
  OpenOptions_ConnectionType_.value("Azure"                       , OpenOptions::ConnectionType::Azure      , OPENVDS_DOCSTRING(OpenOptions_ConnectionType_Azure));
  OpenOptions_ConnectionType_.value("AzureSdkForCpp"              , OpenOptions::ConnectionType::AzureSdkForCpp, OPENVDS_DOCSTRING(OpenOptions_ConnectionType_AzureSdkForCpp));
  OpenOptions_ConnectionType_.value("AzurePresigned"              , OpenOptions::ConnectionType::AzurePresigned, OPENVDS_DOCSTRING(OpenOptions_ConnectionType_AzurePresigned));
  OpenOptions_ConnectionType_.value("GoogleStorage"               , OpenOptions::ConnectionType::GoogleStorage, OPENVDS_DOCSTRING(OpenOptions_ConnectionType_GoogleStorage));
  OpenOptions_ConnectionType_.value("DMS"                         , OpenOptions::ConnectionType::DMS        , OPENVDS_DOCSTRING(OpenOptions_ConnectionType_DMS));
  OpenOptions_ConnectionType_.value("Http"                        , OpenOptions::ConnectionType::Http       , OPENVDS_DOCSTRING(OpenOptions_ConnectionType_Http));
  OpenOptions_ConnectionType_.value("VDSFile"                     , OpenOptions::ConnectionType::VDSFile    , OPENVDS_DOCSTRING(OpenOptions_ConnectionType_VDSFile));
  OpenOptions_ConnectionType_.value("InMemory"                    , OpenOptions::ConnectionType::InMemory   , OPENVDS_DOCSTRING(OpenOptions_ConnectionType_InMemory));
  OpenOptions_ConnectionType_.value("Other"                       , OpenOptions::ConnectionType::Other      , OPENVDS_DOCSTRING(OpenOptions_ConnectionType_Other));
  OpenOptions_ConnectionType_.value("ConnectionTypeCount"         , OpenOptions::ConnectionType::ConnectionTypeCount, OPENVDS_DOCSTRING(OpenOptions_ConnectionType_ConnectionTypeCount));

  // AWSOpenOptions
  py::class_<AWSOpenOptions, OpenOptions> 
    AWSOpenOptions_(m,"AWSOpenOptions", OPENVDS_DOCSTRING(AWSOpenOptions));

  AWSOpenOptions_.def(py::init<                              >(), OPENVDS_DOCSTRING(AWSOpenOptions_AWSOpenOptions));
  AWSOpenOptions_.def(py::init<const std::string &, const std::string &, const std::string &, const std::string &, int, int, bool>(), py::arg("bucket").none(false), py::arg("key").none(false), py::arg("region").none(false), py::arg("endpointOverride").none(false), py::arg("connectionTimeoutMs") = 3000, py::arg("requestTimeoutMs") = 6000, py::arg("disableInitApi") = false, OPENVDS_DOCSTRING(AWSOpenOptions_AWSOpenOptions_2));
  AWSOpenOptions_.def_readwrite("bucket"                      , &AWSOpenOptions::bucket        , OPENVDS_DOCSTRING(AWSOpenOptions_bucket));
  AWSOpenOptions_.def_readwrite("key"                         , &AWSOpenOptions::key           , OPENVDS_DOCSTRING(AWSOpenOptions_key));
  AWSOpenOptions_.def_readwrite("region"                      , &AWSOpenOptions::region        , OPENVDS_DOCSTRING(AWSOpenOptions_region));
  AWSOpenOptions_.def_readwrite("endpointOverride"            , &AWSOpenOptions::endpointOverride, OPENVDS_DOCSTRING(AWSOpenOptions_endpointOverride));
  AWSOpenOptions_.def_readwrite("accessKeyId"                 , &AWSOpenOptions::accessKeyId   , OPENVDS_DOCSTRING(AWSOpenOptions_accessKeyId));
  AWSOpenOptions_.def_readwrite("secretKey"                   , &AWSOpenOptions::secretKey     , OPENVDS_DOCSTRING(AWSOpenOptions_secretKey));
  AWSOpenOptions_.def_readwrite("sessionToken"                , &AWSOpenOptions::sessionToken  , OPENVDS_DOCSTRING(AWSOpenOptions_sessionToken));
  AWSOpenOptions_.def_readwrite("expiration"                  , &AWSOpenOptions::expiration    , OPENVDS_DOCSTRING(AWSOpenOptions_expiration));
  AWSOpenOptions_.def_readwrite("connectionTimeoutMs"         , &AWSOpenOptions::connectionTimeoutMs, OPENVDS_DOCSTRING(AWSOpenOptions_connectionTimeoutMs));
  AWSOpenOptions_.def_readwrite("requestTimeoutMs"            , &AWSOpenOptions::requestTimeoutMs, OPENVDS_DOCSTRING(AWSOpenOptions_requestTimeoutMs));
  AWSOpenOptions_.def_readwrite("disableInitApi"              , &AWSOpenOptions::disableInitApi, OPENVDS_DOCSTRING(AWSOpenOptions_disableInitApi));

  // AzureOpenOptions
  py::class_<AzureOpenOptions, OpenOptions> 
    AzureOpenOptions_(m,"AzureOpenOptions", OPENVDS_DOCSTRING(AzureOpenOptions));

  AzureOpenOptions_.def(py::init<                              >(), OPENVDS_DOCSTRING(AzureOpenOptions_AzureOpenOptions));
  AzureOpenOptions_.def(py::init<const std::string &, const std::string &, const std::string &>(), py::arg("connectionString").none(false), py::arg("container").none(false), py::arg("blob").none(false), OPENVDS_DOCSTRING(AzureOpenOptions_AzureOpenOptions_2));
  AzureOpenOptions_.def(py::init<const std::string &, const std::string &, const std::string &, int, int>(), py::arg("connectionString").none(false), py::arg("container").none(false), py::arg("blob").none(false), py::arg("parallelism_factor").none(false), py::arg("max_execution_time").none(false), OPENVDS_DOCSTRING(AzureOpenOptions_AzureOpenOptions_3));
  AzureOpenOptions_.def_static("azureOpenOptionsBearer"      , static_cast<native::AzureOpenOptions(*)(const std::string &, const std::string &, const std::string &, const std::string &)>(&AzureOpenOptions::AzureOpenOptionsBearer), py::arg("accountName").none(false), py::arg("bearerToken").none(false), py::arg("container").none(false), py::arg("blob").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(AzureOpenOptions_AzureOpenOptionsBearer));
  AzureOpenOptions_.def_readwrite("connectionString"            , &AzureOpenOptions::connectionString, OPENVDS_DOCSTRING(AzureOpenOptions_connectionString));
  AzureOpenOptions_.def_readwrite("accountName"                 , &AzureOpenOptions::accountName , OPENVDS_DOCSTRING(AzureOpenOptions_accountName));
  AzureOpenOptions_.def_readwrite("bearerToken"                 , &AzureOpenOptions::bearerToken , OPENVDS_DOCSTRING(AzureOpenOptions_bearerToken));
  AzureOpenOptions_.def_readwrite("container"                   , &AzureOpenOptions::container   , OPENVDS_DOCSTRING(AzureOpenOptions_container));
  AzureOpenOptions_.def_readwrite("blob"                        , &AzureOpenOptions::blob        , OPENVDS_DOCSTRING(AzureOpenOptions_blob));
  AzureOpenOptions_.def_readwrite("parallelism_factor"          , &AzureOpenOptions::parallelism_factor, OPENVDS_DOCSTRING(AzureOpenOptions_parallelism_factor));
  AzureOpenOptions_.def_readwrite("max_execution_time"          , &AzureOpenOptions::max_execution_time, OPENVDS_DOCSTRING(AzureOpenOptions_max_execution_time));

  // AzurePresignedOpenOptions
  py::class_<AzurePresignedOpenOptions, OpenOptions> 
    AzurePresignedOpenOptions_(m,"AzurePresignedOpenOptions", OPENVDS_DOCSTRING(AzurePresignedOpenOptions));

  AzurePresignedOpenOptions_.def(py::init<                              >(), OPENVDS_DOCSTRING(AzurePresignedOpenOptions_AzurePresignedOpenOptions));
  AzurePresignedOpenOptions_.def(py::init<const std::string &, const std::string &>(), py::arg("baseUrl").none(false), py::arg("urlSuffix").none(false), OPENVDS_DOCSTRING(AzurePresignedOpenOptions_AzurePresignedOpenOptions_2));
  AzurePresignedOpenOptions_.def_readwrite("baseUrl"                     , &AzurePresignedOpenOptions::baseUrl, OPENVDS_DOCSTRING(AzurePresignedOpenOptions_baseUrl));
  AzurePresignedOpenOptions_.def_readwrite("urlSuffix"                   , &AzurePresignedOpenOptions::urlSuffix, OPENVDS_DOCSTRING(AzurePresignedOpenOptions_urlSuffix));

  // GoogleCredentialsToken
  py::class_<GoogleCredentialsToken> 
    GoogleCredentialsToken_(m,"GoogleCredentialsToken", OPENVDS_DOCSTRING(GoogleCredentialsToken));

  GoogleCredentialsToken_.def(py::init<const std::string &           >(), py::arg("token").none(false), OPENVDS_DOCSTRING(GoogleCredentialsToken_GoogleCredentialsToken));
  GoogleCredentialsToken_.def(py::init<std::string &&                >(), py::arg("token").none(false), OPENVDS_DOCSTRING(GoogleCredentialsToken_GoogleCredentialsToken_2));

  // GoogleCredentialsPath
  py::class_<GoogleCredentialsPath> 
    GoogleCredentialsPath_(m,"GoogleCredentialsPath", OPENVDS_DOCSTRING(GoogleCredentialsPath));

  GoogleCredentialsPath_.def(py::init<const std::string &           >(), py::arg("path").none(false), OPENVDS_DOCSTRING(GoogleCredentialsPath_GoogleCredentialsPath));
  GoogleCredentialsPath_.def(py::init<std::string &&                >(), py::arg("path").none(false), OPENVDS_DOCSTRING(GoogleCredentialsPath_GoogleCredentialsPath_2));

  // GoogleCredentialsJson
  py::class_<GoogleCredentialsJson> 
    GoogleCredentialsJson_(m,"GoogleCredentialsJson", OPENVDS_DOCSTRING(GoogleCredentialsJson));

  GoogleCredentialsJson_.def(py::init<const std::string &           >(), py::arg("json").none(false), OPENVDS_DOCSTRING(GoogleCredentialsJson_GoogleCredentialsJson));
  GoogleCredentialsJson_.def(py::init<std::string &&                >(), py::arg("json").none(false), OPENVDS_DOCSTRING(GoogleCredentialsJson_GoogleCredentialsJson_2));

  // GoogleCredentialsSignedUrl
  py::class_<GoogleCredentialsSignedUrl> 
    GoogleCredentialsSignedUrl_(m,"GoogleCredentialsSignedUrl", OPENVDS_DOCSTRING(GoogleCredentialsSignedUrl));

  GoogleCredentialsSignedUrl_.def(py::init<const std::string &           >(), py::arg("region").none(false), OPENVDS_DOCSTRING(GoogleCredentialsSignedUrl_GoogleCredentialsSignedUrl));
  GoogleCredentialsSignedUrl_.def(py::init<std::string &&                >(), py::arg("region").none(false), OPENVDS_DOCSTRING(GoogleCredentialsSignedUrl_GoogleCredentialsSignedUrl_2));

  // GoogleCredentialsSignedUrlPath
  py::class_<GoogleCredentialsSignedUrlPath> 
    GoogleCredentialsSignedUrlPath_(m,"GoogleCredentialsSignedUrlPath", OPENVDS_DOCSTRING(GoogleCredentialsSignedUrlPath));

  GoogleCredentialsSignedUrlPath_.def(py::init<const std::string &, const std::string &>(), py::arg("region").none(false), py::arg("path").none(false), OPENVDS_DOCSTRING(GoogleCredentialsSignedUrlPath_GoogleCredentialsSignedUrlPath));
  GoogleCredentialsSignedUrlPath_.def(py::init<std::string &&, const std::string &>(), py::arg("region").none(false), py::arg("path").none(false), OPENVDS_DOCSTRING(GoogleCredentialsSignedUrlPath_GoogleCredentialsSignedUrlPath_2));
  GoogleCredentialsSignedUrlPath_.def(py::init<const std::string &, std::string &&>(), py::arg("region").none(false), py::arg("path").none(false), OPENVDS_DOCSTRING(GoogleCredentialsSignedUrlPath_GoogleCredentialsSignedUrlPath_3));
  GoogleCredentialsSignedUrlPath_.def(py::init<std::string &&, std::string &&>(), py::arg("region").none(false), py::arg("path").none(false), OPENVDS_DOCSTRING(GoogleCredentialsSignedUrlPath_GoogleCredentialsSignedUrlPath_4));

  // GoogleCredentialsSignedUrlJson
  py::class_<GoogleCredentialsSignedUrlJson> 
    GoogleCredentialsSignedUrlJson_(m,"GoogleCredentialsSignedUrlJson", OPENVDS_DOCSTRING(GoogleCredentialsSignedUrlJson));

  GoogleCredentialsSignedUrlJson_.def(py::init<const std::string &, const std::string &>(), py::arg("region").none(false), py::arg("json").none(false), OPENVDS_DOCSTRING(GoogleCredentialsSignedUrlJson_GoogleCredentialsSignedUrlJson));
  GoogleCredentialsSignedUrlJson_.def(py::init<std::string &&, const std::string &>(), py::arg("region").none(false), py::arg("json").none(false), OPENVDS_DOCSTRING(GoogleCredentialsSignedUrlJson_GoogleCredentialsSignedUrlJson_2));
  GoogleCredentialsSignedUrlJson_.def(py::init<const std::string &, std::string &&>(), py::arg("region").none(false), py::arg("json").none(false), OPENVDS_DOCSTRING(GoogleCredentialsSignedUrlJson_GoogleCredentialsSignedUrlJson_3));
  GoogleCredentialsSignedUrlJson_.def(py::init<std::string &&, std::string &&>(), py::arg("region").none(false), py::arg("json").none(false), OPENVDS_DOCSTRING(GoogleCredentialsSignedUrlJson_GoogleCredentialsSignedUrlJson_4));

  // GoogleOpenOptions
  py::class_<GoogleOpenOptions, OpenOptions> 
    GoogleOpenOptions_(m,"GoogleOpenOptions", OPENVDS_DOCSTRING(GoogleOpenOptions));

  GoogleOpenOptions_.def(py::init<                              >(), OPENVDS_DOCSTRING(GoogleOpenOptions_GoogleOpenOptions));
  GoogleOpenOptions_.def(py::init<const std::string &, const std::string &>(), py::arg("bucket").none(false), py::arg("pathPrefix").none(false), OPENVDS_DOCSTRING(GoogleOpenOptions_GoogleOpenOptions_2));
  GoogleOpenOptions_.def(py::init<const std::string &, const std::string &, const native::GoogleCredentialsToken &>(), py::arg("bucket").none(false), py::arg("pathPrefix").none(false), py::arg("credentials").none(false), OPENVDS_DOCSTRING(GoogleOpenOptions_GoogleOpenOptions_3));
  GoogleOpenOptions_.def(py::init<const std::string &, const std::string &, const native::GoogleCredentialsPath &>(), py::arg("bucket").none(false), py::arg("pathPrefix").none(false), py::arg("credentials").none(false), OPENVDS_DOCSTRING(GoogleOpenOptions_GoogleOpenOptions_4));
  GoogleOpenOptions_.def(py::init<const std::string &, const std::string &, const native::GoogleCredentialsJson &>(), py::arg("bucket").none(false), py::arg("pathPrefix").none(false), py::arg("credentials").none(false), OPENVDS_DOCSTRING(GoogleOpenOptions_GoogleOpenOptions_5));
  GoogleOpenOptions_.def(py::init<const std::string &, const std::string &, const native::GoogleCredentialsSignedUrl &>(), py::arg("bucket").none(false), py::arg("pathPrefix").none(false), py::arg("credentials").none(false), OPENVDS_DOCSTRING(GoogleOpenOptions_GoogleOpenOptions_6));
  GoogleOpenOptions_.def(py::init<const std::string &, const std::string &, const native::GoogleCredentialsSignedUrlPath &>(), py::arg("bucket").none(false), py::arg("pathPrefix").none(false), py::arg("credentials").none(false), OPENVDS_DOCSTRING(GoogleOpenOptions_GoogleOpenOptions_7));
  GoogleOpenOptions_.def(py::init<const std::string &, const std::string &, const native::GoogleCredentialsSignedUrlJson &>(), py::arg("bucket").none(false), py::arg("pathPrefix").none(false), py::arg("credentials").none(false), OPENVDS_DOCSTRING(GoogleOpenOptions_GoogleOpenOptions_8));
  GoogleOpenOptions_.def("setSignedUrl"                , static_cast<bool(GoogleOpenOptions::*)()>(&GoogleOpenOptions::SetSignedUrl), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(GoogleOpenOptions_SetSignedUrl));
  GoogleOpenOptions_.def_readwrite("credentialsType"             , &GoogleOpenOptions::credentialsType, OPENVDS_DOCSTRING(GoogleOpenOptions_credentialsType));
  GoogleOpenOptions_.def_readwrite("bucket"                      , &GoogleOpenOptions::bucket     , OPENVDS_DOCSTRING(GoogleOpenOptions_bucket));
  GoogleOpenOptions_.def_readwrite("pathPrefix"                  , &GoogleOpenOptions::pathPrefix , OPENVDS_DOCSTRING(GoogleOpenOptions_pathPrefix));
  GoogleOpenOptions_.def_readwrite("credentials"                 , &GoogleOpenOptions::credentials, OPENVDS_DOCSTRING(GoogleOpenOptions_credentials));
  GoogleOpenOptions_.def_readwrite("storageClass"                , &GoogleOpenOptions::storageClass, OPENVDS_DOCSTRING(GoogleOpenOptions_storageClass));
  GoogleOpenOptions_.def_readwrite("region"                      , &GoogleOpenOptions::region     , OPENVDS_DOCSTRING(GoogleOpenOptions_region));

  py::enum_<GoogleOpenOptions::CredentialsType> 
    GoogleOpenOptions_CredentialsType_(GoogleOpenOptions_,"CredentialsType", OPENVDS_DOCSTRING(GoogleOpenOptions_CredentialsType));

  GoogleOpenOptions_CredentialsType_.value("Default"                     , GoogleOpenOptions::CredentialsType::Default, OPENVDS_DOCSTRING(GoogleOpenOptions_CredentialsType_Default));
  GoogleOpenOptions_CredentialsType_.value("AccessToken"                 , GoogleOpenOptions::CredentialsType::AccessToken, OPENVDS_DOCSTRING(GoogleOpenOptions_CredentialsType_AccessToken));
  GoogleOpenOptions_CredentialsType_.value("Path"                        , GoogleOpenOptions::CredentialsType::Path, OPENVDS_DOCSTRING(GoogleOpenOptions_CredentialsType_Path));
  GoogleOpenOptions_CredentialsType_.value("Json"                        , GoogleOpenOptions::CredentialsType::Json, OPENVDS_DOCSTRING(GoogleOpenOptions_CredentialsType_Json));
  GoogleOpenOptions_CredentialsType_.value("SignedUrl"                   , GoogleOpenOptions::CredentialsType::SignedUrl, OPENVDS_DOCSTRING(GoogleOpenOptions_CredentialsType_SignedUrl));
  GoogleOpenOptions_CredentialsType_.value("SignedUrlPath"               , GoogleOpenOptions::CredentialsType::SignedUrlPath, OPENVDS_DOCSTRING(GoogleOpenOptions_CredentialsType_SignedUrlPath));
  GoogleOpenOptions_CredentialsType_.value("SignedUrlJson"               , GoogleOpenOptions::CredentialsType::SignedUrlJson, OPENVDS_DOCSTRING(GoogleOpenOptions_CredentialsType_SignedUrlJson));

  // DMSOpenOptions
  py::class_<DMSOpenOptions, OpenOptions> 
    DMSOpenOptions_(m,"DMSOpenOptions", OPENVDS_DOCSTRING(DMSOpenOptions));

  DMSOpenOptions_.def(py::init<                              >(), OPENVDS_DOCSTRING(DMSOpenOptions_DMSOpenOptions));
  DMSOpenOptions_.def(py::init<const std::string &, const std::string &, const std::string &, const std::string &, const std::string &, const std::string &, const std::string &, const std::string &, const std::string &, bool, const std::string &, const std::string &>(), py::arg("sdAuthorityUrl").none(false), py::arg("sdApiKey").none(false), py::arg("sdToken").none(false), py::arg("datasetPath").none(false), py::arg("authTokenUrl").none(false), py::arg("refreshToken").none(false), py::arg("clientId").none(false), py::arg("clientSecret").none(false), py::arg("scopes").none(false), py::arg("useFileNameForSingleFileDatasets") = false, py::arg("legalTag").none(false), py::arg("httpProxy").none(false), OPENVDS_DOCSTRING(DMSOpenOptions_DMSOpenOptions_2));
  DMSOpenOptions_.def(py::init<const std::string &, const std::string &, const std::string &, std::string (*)(const void *), const void *, bool, const std::string &, const std::string &>(), py::arg("sdAuthorityUrl").none(false), py::arg("sdApiKey").none(false), py::arg("datasetPath").none(false), py::arg("authProviderCallback").none(false), py::arg("authProviderCallbackData").none(false), py::arg("useFileNameForSingleFileDatasets") = false, py::arg("legalTag").none(false), py::arg("httpProxy").none(false), OPENVDS_DOCSTRING(DMSOpenOptions_DMSOpenOptions_3));
  DMSOpenOptions_.def_readwrite("sdAuthorityUrl"              , &DMSOpenOptions::sdAuthorityUrl, OPENVDS_DOCSTRING(DMSOpenOptions_sdAuthorityUrl));
  DMSOpenOptions_.def_readwrite("sdApiKey"                    , &DMSOpenOptions::sdApiKey      , OPENVDS_DOCSTRING(DMSOpenOptions_sdApiKey));
  DMSOpenOptions_.def_readwrite("sdToken"                     , &DMSOpenOptions::sdToken       , OPENVDS_DOCSTRING(DMSOpenOptions_sdToken));
  DMSOpenOptions_.def_readwrite("datasetPath"                 , &DMSOpenOptions::datasetPath   , OPENVDS_DOCSTRING(DMSOpenOptions_datasetPath));
  DMSOpenOptions_.def_readwrite("authTokenUrl"                , &DMSOpenOptions::authTokenUrl  , OPENVDS_DOCSTRING(DMSOpenOptions_authTokenUrl));
  DMSOpenOptions_.def_readwrite("refreshToken"                , &DMSOpenOptions::refreshToken  , OPENVDS_DOCSTRING(DMSOpenOptions_refreshToken));
  DMSOpenOptions_.def_readwrite("clientId"                    , &DMSOpenOptions::clientId      , OPENVDS_DOCSTRING(DMSOpenOptions_clientId));
  DMSOpenOptions_.def_readwrite("clientSecret"                , &DMSOpenOptions::clientSecret  , OPENVDS_DOCSTRING(DMSOpenOptions_clientSecret));
  DMSOpenOptions_.def_readwrite("scopes"                      , &DMSOpenOptions::scopes        , OPENVDS_DOCSTRING(DMSOpenOptions_scopes));
  DMSOpenOptions_.def_readwrite("useFileNameForSingleFileDatasets", &DMSOpenOptions::useFileNameForSingleFileDatasets, OPENVDS_DOCSTRING(DMSOpenOptions_useFileNameForSingleFileDatasets));
  DMSOpenOptions_.def_readwrite("legalTag"                    , &DMSOpenOptions::legalTag      , OPENVDS_DOCSTRING(DMSOpenOptions_legalTag));
  DMSOpenOptions_.def_readwrite("httpProxy"                   , &DMSOpenOptions::httpProxy     , OPENVDS_DOCSTRING(DMSOpenOptions_httpProxy));

  // HttpOpenOptions
  py::class_<HttpOpenOptions, OpenOptions> 
    HttpOpenOptions_(m,"HttpOpenOptions", OPENVDS_DOCSTRING(HttpOpenOptions));

  HttpOpenOptions_.def(py::init<                              >(), OPENVDS_DOCSTRING(HttpOpenOptions_HttpOpenOptions));
  HttpOpenOptions_.def(py::init<const std::string &           >(), py::arg("url").none(false), OPENVDS_DOCSTRING(HttpOpenOptions_HttpOpenOptions_2));
  HttpOpenOptions_.def_readwrite("url"                         , &HttpOpenOptions::url          , OPENVDS_DOCSTRING(HttpOpenOptions_url));

  // InMemoryOpenOptions
  py::class_<InMemoryOpenOptions, OpenOptions> 
    InMemoryOpenOptions_(m,"InMemoryOpenOptions", OPENVDS_DOCSTRING(InMemoryOpenOptions));

  InMemoryOpenOptions_.def(py::init<                              >(), OPENVDS_DOCSTRING(InMemoryOpenOptions_InMemoryOpenOptions));
  InMemoryOpenOptions_.def(py::init<const char *                  >(), py::arg("name").none(false), OPENVDS_DOCSTRING(InMemoryOpenOptions_InMemoryOpenOptions_2));
  InMemoryOpenOptions_.def(py::init<const std::string &           >(), py::arg("name").none(false), OPENVDS_DOCSTRING(InMemoryOpenOptions_InMemoryOpenOptions_3));
  InMemoryOpenOptions_.def_readwrite("name"                        , &InMemoryOpenOptions::name     , OPENVDS_DOCSTRING(InMemoryOpenOptions_name));

  // VDSFileOpenOptions
  py::class_<VDSFileOpenOptions, OpenOptions> 
    VDSFileOpenOptions_(m,"VDSFileOpenOptions", OPENVDS_DOCSTRING(VDSFileOpenOptions));

  VDSFileOpenOptions_.def(py::init<                              >(), OPENVDS_DOCSTRING(VDSFileOpenOptions_VDSFileOpenOptions));
  VDSFileOpenOptions_.def(py::init<const std::string &           >(), py::arg("fileName").none(false), OPENVDS_DOCSTRING(VDSFileOpenOptions_VDSFileOpenOptions_2));
  VDSFileOpenOptions_.def_readwrite("fileName"                    , &VDSFileOpenOptions::fileName  , OPENVDS_DOCSTRING(VDSFileOpenOptions_fileName));

  m.def("createOpenOptions"           , static_cast<native::OpenOptions *(*)(std::string, std::string, native::Error &)>(&CreateOpenOptions), py::arg("url").none(false), py::arg("connectionString").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(CreateOpenOptions));
  m.def("createOpenOptions"           , [](std::string url, std::string connectionString) { native::Error err; auto ret = CreateOpenOptions(url, connectionString, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("url").none(false), py::arg("connectionString").none(false));
  m.def("isSupportedProtocol"         , static_cast<bool(*)(std::string)>(&IsSupportedProtocol), py::arg("url").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(IsSupportedProtocol));
  m.def("open"                        , static_cast<native::VDSHandle(*)(std::string, std::string, native::Error &)>(&Open), py::arg("url").none(false), py::arg("connectionString").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Open));
  m.def("open"                        , [](std::string url, std::string connectionString) { native::Error err; auto ret = Open(url, connectionString, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("url").none(false), py::arg("connectionString").none(false));
  m.def("openWithAdaptiveCompressionTolerance", static_cast<native::VDSHandle(*)(std::string, std::string, float, native::Error &)>(&OpenWithAdaptiveCompressionTolerance), py::arg("url").none(false), py::arg("connectionString").none(false), py::arg("waveletAdaptiveTolerance").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(OpenWithAdaptiveCompressionTolerance));
  m.def("openWithAdaptiveCompressionTolerance", [](std::string url, std::string connectionString, float waveletAdaptiveTolerance) { native::Error err; auto ret = OpenWithAdaptiveCompressionTolerance(url, connectionString, waveletAdaptiveTolerance, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("url").none(false), py::arg("connectionString").none(false), py::arg("waveletAdaptiveTolerance").none(false));
  m.def("openWithAdaptiveCompressionRatio", static_cast<native::VDSHandle(*)(std::string, std::string, float, native::Error &)>(&OpenWithAdaptiveCompressionRatio), py::arg("url").none(false), py::arg("connectionString").none(false), py::arg("waveletAdaptiveRatio").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(OpenWithAdaptiveCompressionRatio));
  m.def("openWithAdaptiveCompressionRatio", [](std::string url, std::string connectionString, float waveletAdaptiveRatio) { native::Error err; auto ret = OpenWithAdaptiveCompressionRatio(url, connectionString, waveletAdaptiveRatio, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("url").none(false), py::arg("connectionString").none(false), py::arg("waveletAdaptiveRatio").none(false));
  m.def("open"                        , static_cast<native::VDSHandle(*)(std::string, native::Error &)>(&Open), py::arg("url").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Open_2));
  m.def("open"                        , [](std::string url) { native::Error err; auto ret = Open(url, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("url").none(false));
  m.def("open"                        , static_cast<native::VDSHandle(*)(const native::OpenOptions &, native::Error &)>(&Open), py::arg("options").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Open_3));
  m.def("open"                        , [](const OpenVDS::OpenOptions & options) { native::Error err; auto ret = Open(options, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("options").none(false));
  m.def("open"                        , static_cast<native::VDSHandle(*)(native::IOManager *, native::Error &)>(&Open), py::arg("ioManager").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Open_4));
  m.def("open"                        , [](OpenVDS::IOManager * ioManager) { native::Error err; auto ret = Open(ioManager, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("ioManager").none(false));
  m.def("open"                        , static_cast<native::VDSHandle(*)(native::IOManager *, native::LogLevel, native::Error &)>(&Open), py::arg("ioManager").none(false), py::arg("logLevel").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Open_5));
  m.def("open"                        , [](OpenVDS::IOManager * ioManager, OpenVDS::LogLevel logLevel) { native::Error err; auto ret = Open(ioManager, logLevel, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("ioManager").none(false), py::arg("logLevel").none(false));
  m.def("isCompressionMethodSupported", static_cast<bool(*)(native::CompressionMethod)>(&IsCompressionMethodSupported), py::arg("compressionMethod").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(IsCompressionMethodSupported));
  m.def("create"                      , static_cast<native::VDSHandle(*)(std::string, std::string, const native::VolumeDataLayoutDescriptor &, std::vector<VolumeDataAxisDescriptor>, std::vector<VolumeDataChannelDescriptor>, const native::MetadataReadAccess &, native::CompressionMethod, float, native::Error &)>(&Create), py::arg("url").none(false), py::arg("connectionString").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false), py::arg("compressionMethod").none(false), py::arg("compressionTolerance").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Create));
  m.def("create"                      , [](std::string url, std::string connectionString, const OpenVDS::VolumeDataLayoutDescriptor & layoutDescriptor, std::vector<VolumeDataAxisDescriptor> axisDescriptors, std::vector<VolumeDataChannelDescriptor> channelDescriptors, const OpenVDS::MetadataReadAccess & metadata, OpenVDS::CompressionMethod compressionMethod, float compressionTolerance) { native::Error err; auto ret = Create(url, connectionString, layoutDescriptor, axisDescriptors, channelDescriptors, metadata, compressionMethod, compressionTolerance, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("url").none(false), py::arg("connectionString").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false), py::arg("compressionMethod").none(false), py::arg("compressionTolerance").none(false));
  m.def("create"                      , static_cast<native::VDSHandle(*)(std::string, std::string, const native::VolumeDataLayoutDescriptor &, std::vector<VolumeDataAxisDescriptor>, std::vector<VolumeDataChannelDescriptor>, const native::MetadataReadAccess &, native::Error &)>(&Create), py::arg("url").none(false), py::arg("connectionString").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Create_2));
  m.def("create"                      , [](std::string url, std::string connectionString, const OpenVDS::VolumeDataLayoutDescriptor & layoutDescriptor, std::vector<VolumeDataAxisDescriptor> axisDescriptors, std::vector<VolumeDataChannelDescriptor> channelDescriptors, const OpenVDS::MetadataReadAccess & metadata) { native::Error err; auto ret = Create(url, connectionString, layoutDescriptor, axisDescriptors, channelDescriptors, metadata, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("url").none(false), py::arg("connectionString").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false));
  m.def("create"                      , static_cast<native::VDSHandle(*)(std::string, const native::VolumeDataLayoutDescriptor &, std::vector<VolumeDataAxisDescriptor>, std::vector<VolumeDataChannelDescriptor>, const native::MetadataReadAccess &, native::CompressionMethod, float, native::Error &)>(&Create), py::arg("url").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false), py::arg("compressionMethod").none(false), py::arg("compressionTolerance").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Create_3));
  m.def("create"                      , [](std::string url, const OpenVDS::VolumeDataLayoutDescriptor & layoutDescriptor, std::vector<VolumeDataAxisDescriptor> axisDescriptors, std::vector<VolumeDataChannelDescriptor> channelDescriptors, const OpenVDS::MetadataReadAccess & metadata, OpenVDS::CompressionMethod compressionMethod, float compressionTolerance) { native::Error err; auto ret = Create(url, layoutDescriptor, axisDescriptors, channelDescriptors, metadata, compressionMethod, compressionTolerance, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("url").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false), py::arg("compressionMethod").none(false), py::arg("compressionTolerance").none(false));
  m.def("create"                      , static_cast<native::VDSHandle(*)(std::string, const native::VolumeDataLayoutDescriptor &, std::vector<VolumeDataAxisDescriptor>, std::vector<VolumeDataChannelDescriptor>, const native::MetadataReadAccess &, native::Error &)>(&Create), py::arg("url").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Create_4));
  m.def("create"                      , [](std::string url, const OpenVDS::VolumeDataLayoutDescriptor & layoutDescriptor, std::vector<VolumeDataAxisDescriptor> axisDescriptors, std::vector<VolumeDataChannelDescriptor> channelDescriptors, const OpenVDS::MetadataReadAccess & metadata) { native::Error err; auto ret = Create(url, layoutDescriptor, axisDescriptors, channelDescriptors, metadata, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("url").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false));
  m.def("create"                      , static_cast<native::VDSHandle(*)(const native::OpenOptions &, const native::VolumeDataLayoutDescriptor &, std::vector<VolumeDataAxisDescriptor>, std::vector<VolumeDataChannelDescriptor>, const native::MetadataReadAccess &, native::CompressionMethod, float, native::Error &)>(&Create), py::arg("options").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false), py::arg("compressionMethod").none(false), py::arg("compressionTolerance").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Create_5));
  m.def("create"                      , [](const OpenVDS::OpenOptions & options, const OpenVDS::VolumeDataLayoutDescriptor & layoutDescriptor, std::vector<VolumeDataAxisDescriptor> axisDescriptors, std::vector<VolumeDataChannelDescriptor> channelDescriptors, const OpenVDS::MetadataReadAccess & metadata, OpenVDS::CompressionMethod compressionMethod, float compressionTolerance) { native::Error err; auto ret = Create(options, layoutDescriptor, axisDescriptors, channelDescriptors, metadata, compressionMethod, compressionTolerance, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("options").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false), py::arg("compressionMethod").none(false), py::arg("compressionTolerance").none(false));
  m.def("create"                      , static_cast<native::VDSHandle(*)(const native::OpenOptions &, const native::VolumeDataLayoutDescriptor &, std::vector<VolumeDataAxisDescriptor>, std::vector<VolumeDataChannelDescriptor>, const native::MetadataReadAccess &, native::Error &)>(&Create), py::arg("options").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Create_6));
  m.def("create"                      , [](const OpenVDS::OpenOptions & options, const OpenVDS::VolumeDataLayoutDescriptor & layoutDescriptor, std::vector<VolumeDataAxisDescriptor> axisDescriptors, std::vector<VolumeDataChannelDescriptor> channelDescriptors, const OpenVDS::MetadataReadAccess & metadata) { native::Error err; auto ret = Create(options, layoutDescriptor, axisDescriptors, channelDescriptors, metadata, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("options").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false));
  m.def("create"                      , static_cast<native::VDSHandle(*)(native::IOManager *, const native::VolumeDataLayoutDescriptor &, std::vector<VolumeDataAxisDescriptor>, std::vector<VolumeDataChannelDescriptor>, const native::MetadataReadAccess &, native::CompressionMethod, float, native::Error &)>(&Create), py::arg("ioManager").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false), py::arg("compressionMethod").none(false), py::arg("compressionTolerance").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Create_7));
  m.def("create"                      , [](OpenVDS::IOManager * ioManager, const OpenVDS::VolumeDataLayoutDescriptor & layoutDescriptor, std::vector<VolumeDataAxisDescriptor> axisDescriptors, std::vector<VolumeDataChannelDescriptor> channelDescriptors, const OpenVDS::MetadataReadAccess & metadata, OpenVDS::CompressionMethod compressionMethod, float compressionTolerance) { native::Error err; auto ret = Create(ioManager, layoutDescriptor, axisDescriptors, channelDescriptors, metadata, compressionMethod, compressionTolerance, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("ioManager").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false), py::arg("compressionMethod").none(false), py::arg("compressionTolerance").none(false));
  m.def("create"                      , static_cast<native::VDSHandle(*)(native::IOManager *, const native::VolumeDataLayoutDescriptor &, std::vector<VolumeDataAxisDescriptor>, std::vector<VolumeDataChannelDescriptor>, const native::MetadataReadAccess &, native::CompressionMethod, float, native::LogLevel, native::Error &)>(&Create), py::arg("ioManager").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false), py::arg("compressionMethod").none(false), py::arg("compressionTolerance").none(false), py::arg("logLevel").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Create_8));
  m.def("create"                      , [](OpenVDS::IOManager * ioManager, const OpenVDS::VolumeDataLayoutDescriptor & layoutDescriptor, std::vector<VolumeDataAxisDescriptor> axisDescriptors, std::vector<VolumeDataChannelDescriptor> channelDescriptors, const OpenVDS::MetadataReadAccess & metadata, OpenVDS::CompressionMethod compressionMethod, float compressionTolerance, OpenVDS::LogLevel logLevel) { native::Error err; auto ret = Create(ioManager, layoutDescriptor, axisDescriptors, channelDescriptors, metadata, compressionMethod, compressionTolerance, logLevel, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("ioManager").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false), py::arg("compressionMethod").none(false), py::arg("compressionTolerance").none(false), py::arg("logLevel").none(false));
  m.def("create"                      , static_cast<native::VDSHandle(*)(native::IOManager *, const native::VolumeDataLayoutDescriptor &, std::vector<VolumeDataAxisDescriptor>, std::vector<VolumeDataChannelDescriptor>, const native::MetadataReadAccess &, native::Error &)>(&Create), py::arg("ioManager").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Create_9));
  m.def("create"                      , [](OpenVDS::IOManager * ioManager, const OpenVDS::VolumeDataLayoutDescriptor & layoutDescriptor, std::vector<VolumeDataAxisDescriptor> axisDescriptors, std::vector<VolumeDataChannelDescriptor> channelDescriptors, const OpenVDS::MetadataReadAccess & metadata) { native::Error err; auto ret = Create(ioManager, layoutDescriptor, axisDescriptors, channelDescriptors, metadata, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("ioManager").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false));
  m.def("create"                      , static_cast<native::VDSHandle(*)(native::IOManager *, const native::VolumeDataLayoutDescriptor &, std::vector<VolumeDataAxisDescriptor>, std::vector<VolumeDataChannelDescriptor>, const native::MetadataReadAccess &, native::LogLevel, native::Error &)>(&Create), py::arg("ioManager").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false), py::arg("logLevel").none(false), py::arg("error").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Create_10));
  m.def("create"                      , [](OpenVDS::IOManager * ioManager, const OpenVDS::VolumeDataLayoutDescriptor & layoutDescriptor, std::vector<VolumeDataAxisDescriptor> axisDescriptors, std::vector<VolumeDataChannelDescriptor> channelDescriptors, const OpenVDS::MetadataReadAccess & metadata, OpenVDS::LogLevel logLevel) { native::Error err; auto ret = Create(ioManager, layoutDescriptor, axisDescriptors, channelDescriptors, metadata, logLevel, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("ioManager").none(false), py::arg("layoutDescriptor").none(false), py::arg("axisDescriptors").none(false), py::arg("channelDescriptors").none(false), py::arg("metadata").none(false), py::arg("logLevel").none(false));
  m.def("getLayout"                   , static_cast<native::VolumeDataLayout *(*)(native::VDSHandle)>(&GetLayout), py::arg("handle").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(GetLayout));
  m.def("getAccessManagerInterface"   , static_cast<native::IVolumeDataAccessManager *(*)(native::VDSHandle)>(&GetAccessManagerInterface), py::arg("handle").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(GetAccessManagerInterface));
  m.def("getAccessManager"            , static_cast<native::VolumeDataAccessManager(*)(native::VDSHandle)>(&GetAccessManager), py::arg("handle").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(GetAccessManager));
  m.def("getMetadataWriteAccessInterface", static_cast<native::MetadataWriteAccess *(*)(native::VDSHandle)>(&GetMetadataWriteAccessInterface), py::arg("handle").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(GetMetadataWriteAccessInterface));
  m.def("getCompressionMethod"        , static_cast<native::CompressionMethod(*)(native::VDSHandle)>(&GetCompressionMethod), py::arg("handle").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(GetCompressionMethod));
  m.def("getCompressionTolerance"     , static_cast<float(*)(native::VDSHandle)>(&GetCompressionTolerance), py::arg("handle").none(false), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(GetCompressionTolerance));
  m.def("close"                       , static_cast<void(*)(native::VDSHandle, bool)>(&Close), py::arg("handle").none(false), py::arg("flush") = true, py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Close));
  m.def("close"                       , static_cast<void(*)(native::VDSHandle, native::Error &, bool)>(&Close), py::arg("handle").none(false), py::arg("error").none(false), py::arg("flush") = true, py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(Close_2));
  m.def("retryableClose"              , static_cast<void(*)(native::VDSHandle, bool)>(&RetryableClose), py::arg("handle").none(false), py::arg("flush") = true, py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(RetryableClose));
  m.def("retryableClose"              , static_cast<void(*)(native::VDSHandle, native::Error &, bool)>(&RetryableClose), py::arg("handle").none(false), py::arg("error").none(false), py::arg("flush") = true, py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(RetryableClose_2));
  m.def("getGlobalState"              , static_cast<native::GlobalState *(*)()>(&GetGlobalState), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(GetGlobalState));
  m.def("getOpenVDSName"              , static_cast<const char *(*)()>(&GetOpenVDSName), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(GetOpenVDSName));
  m.def("getOpenVDSVersion"           , static_cast<const char *(*)()>(&GetOpenVDSVersion), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(GetOpenVDSVersion));
  m.def("getOpenVDSRevision"          , static_cast<const char *(*)()>(&GetOpenVDSRevision), py::call_guard<py::gil_scoped_release>(), OPENVDS_DOCSTRING(GetOpenVDSRevision));
//AUTOGEN-END
  OpenOptions_.def("__repr__", [](OpenOptions const& self)
    {
      std::string conn = "Unknown";
      switch(self.connectionType)
      {
      case OpenOptions::ConnectionType::AWS            : conn = std::string("AWS"            ); break;
      case OpenOptions::ConnectionType::Azure          : conn = std::string("Azure"          ); break;
      case OpenOptions::ConnectionType::AzureSdkForCpp : conn = std::string("AzureSdkForCpp" ); break;
      case OpenOptions::ConnectionType::AzurePresigned : conn = std::string("AzurePresigned" ); break;
      case OpenOptions::ConnectionType::GoogleStorage  : conn = std::string("GoogleStorage"  ); break;
      case OpenOptions::ConnectionType::Http           : conn = std::string("Http"           ); break;
      case OpenOptions::ConnectionType::DMS            : conn = std::string("Dms"            ); break;
      case OpenOptions::ConnectionType::VDSFile        : conn = std::string("VDSFile"        ); break;
      case OpenOptions::ConnectionType::InMemory       : conn = std::string("InMemory"       ); break;
      case OpenOptions::ConnectionType::Other          : conn = std::string("Other"          ); break;
      case OpenOptions::ConnectionType::ConnectionTypeCount : conn = std::string("ConnectionTypeCount"); break;
      }
      return std::string("OpenOptions(connectionType='" + conn + "')");
    });


// IMPLEMENTED : AWSOpenOptions_.def(py::init<const std::string &, const std::string &, const std::string &, const std::string &>(), py::arg("bucket").none(false), py::arg("key").none(false), py::arg("region").none(false), py::arg("endpointOverride").none(false), OPENVDS_DOCSTRING(AWSOpenOptions_AWSOpenOptions_2));
  AWSOpenOptions_.def(py::init<const std::string &, const std::string &, const std::string &, const std::string &>(), py::arg("bucket").none(false), py::arg("key").none(false), py::arg("region").none(false) = "", py::arg("endpointOverride").none(false) = "", OPENVDS_DOCSTRING(AWSOpenOptions_AWSOpenOptions_2));
}
// IMPLEMENTED : m.def("close"                       , [](OpenVDS::VDSHandle handle) { native::Error err; auto ret = Close(handle, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("handle").none(false));
// IMPLEMENTED : m.def("retryableClose"              , [](OpenVDS::VDSHandle handle) { native::Error err; auto ret = RetryableClose(handle, err); if (err.code) { throw std::runtime_error(err.string); } return ret; }, py::call_guard<py::gil_scoped_release>(), py::arg("handle").none(false));

// IMPLEMENTED : m.def_property_readonly("openVDSName", &GetOpenVDSName, OPENVDS_DOCSTRING(GetOpenVDSName));
// IMPLEMENTED : m.def_property_readonly("openVDSVersion", &GetOpenVDSVersion, OPENVDS_DOCSTRING(GetOpenVDSVersion));
// IMPLEMENTED : m.def_property_readonly("openVDSRevision", &GetOpenVDSRevision, OPENVDS_DOCSTRING(GetOpenVDSRevision));

// IMPLEMENTED : DMSOpenOptions_.def(py::init<const std::string &, const std::string &, const std::string &, std::string (*)(const void *), const void *, bool, const std::string &>(), py::arg("sdAuthorityUrl").none(false), py::arg("sdApiKey").none(false), py::arg("datasetPath").none(false), py::arg("authProviderCallback").none(false), py::arg("authProviderCallbackData").none(false), py::arg("useFileNameForSingleFileDatasets") = false, py::arg("legalTag").none(false), OPENVDS_DOCSTRING(DMSOpenOptions_DMSOpenOptions_3));
// IMPLEMENTED : DMSOpenOptions_.def_readwrite("authProviderCallback"        , &DMSOpenOptions::authProviderCallback, OPENVDS_DOCSTRING(DMSOpenOptions_authProviderCallback));
// IMPLEMENTED : DMSOpenOptions_.def_readwrite("authProviderCallbackData"    , &DMSOpenOptions::authProviderCallbackData, OPENVDS_DOCSTRING(DMSOpenOptions_authProviderCallbackData));

