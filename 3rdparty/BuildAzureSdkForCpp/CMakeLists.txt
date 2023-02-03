# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# setting CMAKE_TOOLCHAIN_FILE must happen before creating the project
# Azure core is compatible with CMake 3.12
cmake_minimum_required (VERSION 3.12)
project(azure-core LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(Threads REQUIRED)

set (AZ_CORE_DIR "${azure-sdk-for-cpp_SOURCE_DIR}/sdk/core/azure-core")
set (AZ_STORAGE_COMMON_DIR "${azure-sdk-for-cpp_SOURCE_DIR}/sdk/storage/azure-storage-common")
set (AZ_STORAGE_BLOBS_DIR "${azure-sdk-for-cpp_SOURCE_DIR}/sdk/storage/azure-storage-blobs")

if(WIN32)
    #set(BUILD_TRANSPORT_WINHTTP ON)
    set(BUILD_TRANSPORT_CURL ON)
else()
    set(BUILD_TRANSPORT_CURL ON)
endif()

if(BUILD_TRANSPORT_CURL)
  SET(CURL_TRANSPORT_ADAPTER_SRC
    ${AZ_CORE_DIR}/src/http/curl/curl.cpp
    ${AZ_CORE_DIR}/src/http/curl/curl_connection_pool_private.hpp
    ${AZ_CORE_DIR}/src/http/curl/curl_connection_private.hpp
    ${AZ_CORE_DIR}/src/http/curl/curl_session_private.hpp
  )
  SET(CURL_TRANSPORT_ADAPTER_INC
    ${AZ_CORE_DIR}/inc/azure/core/http/curl_transport.hpp
  )
endif()
if(BUILD_TRANSPORT_WINHTTP)
  SET(WIN_TRANSPORT_ADAPTER_SRC 
    ${AZ_CORE_DIR}/src/http/winhttp/win_http_transport.cpp
  )
 SET(WIN_TRANSPORT_ADAPTER_INC 
    ${AZ_CORE_DIR}/inc/azure/core/http/win_http_transport.hpp
 )
endif()

set(
  AZURE_CORE_HEADER
    ${CURL_TRANSPORT_ADAPTER_INC}
    ${WIN_TRANSPORT_ADAPTER_INC}
    ${AZ_CORE_DIR}/inc/azure/core.hpp 
    ${AZ_CORE_DIR}/inc/azure/core/azure_assert.hpp
    ${AZ_CORE_DIR}/inc/azure/core/base64.hpp
    ${AZ_CORE_DIR}/inc/azure/core/case_insensitive_containers.hpp
    ${AZ_CORE_DIR}/inc/azure/core/context.hpp
    ${AZ_CORE_DIR}/inc/azure/core/credentials/credentials.hpp
    ${AZ_CORE_DIR}/inc/azure/core/credentials/token_credential_options.hpp
    ${AZ_CORE_DIR}/inc/azure/core/cryptography/hash.hpp
    ${AZ_CORE_DIR}/inc/azure/core/datetime.hpp
    ${AZ_CORE_DIR}/inc/azure/core/diagnostics/logger.hpp
    ${AZ_CORE_DIR}/inc/azure/core/dll_import_export.hpp
    ${AZ_CORE_DIR}/inc/azure/core/etag.hpp
    ${AZ_CORE_DIR}/inc/azure/core/exception.hpp
    ${AZ_CORE_DIR}/inc/azure/core/http/http.hpp
    ${AZ_CORE_DIR}/inc/azure/core/http/http_status_code.hpp
    ${AZ_CORE_DIR}/inc/azure/core/http/policies/policy.hpp
    ${AZ_CORE_DIR}/inc/azure/core/http/raw_response.hpp
    ${AZ_CORE_DIR}/inc/azure/core/http/transport.hpp
    ${AZ_CORE_DIR}/inc/azure/core/internal/client_options.hpp
    ${AZ_CORE_DIR}/inc/azure/core/internal/contract.hpp
    ${AZ_CORE_DIR}/inc/azure/core/internal/cryptography/sha_hash.hpp
    ${AZ_CORE_DIR}/inc/azure/core/internal/diagnostics/log.hpp
    ${AZ_CORE_DIR}/inc/azure/core/internal/environment.hpp
    ${AZ_CORE_DIR}/inc/azure/core/internal/extendable_enumeration.hpp
    ${AZ_CORE_DIR}/inc/azure/core/internal/http/http_sanitizer.hpp
    ${AZ_CORE_DIR}/inc/azure/core/internal/http/pipeline.hpp
    ${AZ_CORE_DIR}/inc/azure/core/internal/http/user_agent.hpp
    ${AZ_CORE_DIR}/inc/azure/core/internal/io/null_body_stream.hpp
    ${AZ_CORE_DIR}/inc/azure/core/internal/json/json.hpp
    ${AZ_CORE_DIR}/inc/azure/core/internal/json/json_optional.hpp
    ${AZ_CORE_DIR}/inc/azure/core/internal/json/json_serializable.hpp
    ${AZ_CORE_DIR}/inc/azure/core/internal/strings.hpp
    ${AZ_CORE_DIR}/inc/azure/core/internal/tracing/service_tracing.hpp
    ${AZ_CORE_DIR}/inc/azure/core/internal/tracing/tracing_impl.hpp
    ${AZ_CORE_DIR}/inc/azure/core/io/body_stream.hpp
    ${AZ_CORE_DIR}/inc/azure/core/match_conditions.hpp
    ${AZ_CORE_DIR}/inc/azure/core/modified_conditions.hpp
    ${AZ_CORE_DIR}/inc/azure/core/nullable.hpp
    ${AZ_CORE_DIR}/inc/azure/core/operation.hpp
    ${AZ_CORE_DIR}/inc/azure/core/operation_status.hpp
    ${AZ_CORE_DIR}/inc/azure/core/paged_response.hpp
    ${AZ_CORE_DIR}/inc/azure/core/platform.hpp
    ${AZ_CORE_DIR}/inc/azure/core/response.hpp
    ${AZ_CORE_DIR}/inc/azure/core/rtti.hpp
    ${AZ_CORE_DIR}/inc/azure/core/tracing/tracing.hpp
    ${AZ_CORE_DIR}/inc/azure/core/url.hpp
    ${AZ_CORE_DIR}/inc/azure/core/uuid.hpp
    )

set(
  AZURE_CORE_SOURCE
    ${CURL_TRANSPORT_ADAPTER_SRC}
    ${WIN_TRANSPORT_ADAPTER_SRC}
    ${AZ_CORE_DIR}/src/azure_assert.cpp
    ${AZ_CORE_DIR}/src/base64.cpp
    ${AZ_CORE_DIR}/src/context.cpp
    ${AZ_CORE_DIR}/src/cryptography/md5.cpp
    ${AZ_CORE_DIR}/src/cryptography/sha_hash.cpp
    ${AZ_CORE_DIR}/src/datetime.cpp
    ${AZ_CORE_DIR}/src/environment.cpp
    ${AZ_CORE_DIR}/src/environment_log_level_listener.cpp
    ${AZ_CORE_DIR}/src/etag.cpp
    ${AZ_CORE_DIR}/src/exception.cpp
    ${AZ_CORE_DIR}/src/http/bearer_token_authentication_policy.cpp
    ${AZ_CORE_DIR}/src/http/http.cpp
    ${AZ_CORE_DIR}/src/http/http_sanitizer.cpp
    ${AZ_CORE_DIR}/src/http/log_policy.cpp
    ${AZ_CORE_DIR}/src/http/policy.cpp
    ${AZ_CORE_DIR}/src/http/raw_response.cpp
    ${AZ_CORE_DIR}/src/http/request.cpp
    ${AZ_CORE_DIR}/src/http/request_activity_policy.cpp
    ${AZ_CORE_DIR}/src/http/retry_policy.cpp
    ${AZ_CORE_DIR}/src/http/telemetry_policy.cpp
    ${AZ_CORE_DIR}/src/http/transport_policy.cpp
    ${AZ_CORE_DIR}/src/http/url.cpp
    ${AZ_CORE_DIR}/src/http/user_agent.cpp
    ${AZ_CORE_DIR}/src/io/body_stream.cpp
    ${AZ_CORE_DIR}/src/io/random_access_file_body_stream.cpp
    ${AZ_CORE_DIR}/src/logger.cpp
    ${AZ_CORE_DIR}/src/operation_status.cpp
    ${AZ_CORE_DIR}/src/private/environment_log_level_listener.hpp
    ${AZ_CORE_DIR}/src/private/package_version.hpp
    #${AZ_CORE_DIR}/src/strings.cpp
    ${AZ_CORE_DIR}/src/tracing/tracing.cpp
    ${AZ_CORE_DIR}/src/uuid.cpp
    )
set(
  AZURE_STORAGE_COMMON_HEADER
    ${AZ_STORAGE_COMMON_DIR}/inc/azure/storage/common/access_conditions.hpp
    ${AZ_STORAGE_COMMON_DIR}/inc/azure/storage/common/account_sas_builder.hpp
    ${AZ_STORAGE_COMMON_DIR}/inc/azure/storage/common/crypt.hpp
    ${AZ_STORAGE_COMMON_DIR}/inc/azure/storage/common/dll_import_export.hpp
    ${AZ_STORAGE_COMMON_DIR}/inc/azure/storage/common/internal/concurrent_transfer.hpp
    ${AZ_STORAGE_COMMON_DIR}/inc/azure/storage/common/internal/constants.hpp
    ${AZ_STORAGE_COMMON_DIR}/inc/azure/storage/common/internal/file_io.hpp
    ${AZ_STORAGE_COMMON_DIR}/inc/azure/storage/common/internal/reliable_stream.hpp
    ${AZ_STORAGE_COMMON_DIR}/inc/azure/storage/common/internal/shared_key_policy.hpp
    ${AZ_STORAGE_COMMON_DIR}/inc/azure/storage/common/internal/storage_per_retry_policy.hpp
    ${AZ_STORAGE_COMMON_DIR}/inc/azure/storage/common/internal/storage_service_version_policy.hpp
    ${AZ_STORAGE_COMMON_DIR}/inc/azure/storage/common/internal/storage_switch_to_secondary_policy.hpp
    ${AZ_STORAGE_COMMON_DIR}/inc/azure/storage/common/internal/xml_wrapper.hpp
    ${AZ_STORAGE_COMMON_DIR}/inc/azure/storage/common/rtti.hpp
    ${AZ_STORAGE_COMMON_DIR}/inc/azure/storage/common/storage_common.hpp
    ${AZ_STORAGE_COMMON_DIR}/inc/azure/storage/common/storage_credential.hpp
    ${AZ_STORAGE_COMMON_DIR}/inc/azure/storage/common/storage_exception.hpp
)

set(
  AZURE_STORAGE_COMMON_SOURCE
    ${AZ_STORAGE_COMMON_DIR}/src/private/package_version.hpp
    ${AZ_STORAGE_COMMON_DIR}/src/account_sas_builder.cpp
    ${AZ_STORAGE_COMMON_DIR}/src/crypt.cpp
    ${AZ_STORAGE_COMMON_DIR}/src/file_io.cpp
    ${AZ_STORAGE_COMMON_DIR}/src/reliable_stream.cpp
    ${AZ_STORAGE_COMMON_DIR}/src/shared_key_policy.cpp
    ${AZ_STORAGE_COMMON_DIR}/src/storage_common.cpp
    ${AZ_STORAGE_COMMON_DIR}/src/storage_credential.cpp
    ${AZ_STORAGE_COMMON_DIR}/src/storage_exception.cpp
    ${AZ_STORAGE_COMMON_DIR}/src/storage_per_retry_policy.cpp
    ${AZ_STORAGE_COMMON_DIR}/src/storage_switch_to_secondary_policy.cpp
    ${AZ_STORAGE_COMMON_DIR}/src/xml_wrapper.cpp
)

set(
  AZURE_STORAGE_BLOBS_HEADER
    ${AZ_STORAGE_BLOBS_DIR}/inc/azure/storage/blobs/append_blob_client.hpp
    ${AZ_STORAGE_BLOBS_DIR}/inc/azure/storage/blobs/blob_batch.hpp
    ${AZ_STORAGE_BLOBS_DIR}/inc/azure/storage/blobs/blob_client.hpp
    ${AZ_STORAGE_BLOBS_DIR}/inc/azure/storage/blobs/blob_container_client.hpp
    ${AZ_STORAGE_BLOBS_DIR}/inc/azure/storage/blobs/blob_lease_client.hpp
    ${AZ_STORAGE_BLOBS_DIR}/inc/azure/storage/blobs/blob_options.hpp
    ${AZ_STORAGE_BLOBS_DIR}/inc/azure/storage/blobs/blob_responses.hpp
    ${AZ_STORAGE_BLOBS_DIR}/inc/azure/storage/blobs/blob_sas_builder.hpp
    ${AZ_STORAGE_BLOBS_DIR}/inc/azure/storage/blobs/blob_service_client.hpp
    ${AZ_STORAGE_BLOBS_DIR}/inc/azure/storage/blobs/block_blob_client.hpp
    ${AZ_STORAGE_BLOBS_DIR}/inc/azure/storage/blobs/deferred_response.hpp
    ${AZ_STORAGE_BLOBS_DIR}/inc/azure/storage/blobs/dll_import_export.hpp
    ${AZ_STORAGE_BLOBS_DIR}/inc/azure/storage/blobs/page_blob_client.hpp
    ${AZ_STORAGE_BLOBS_DIR}/inc/azure/storage/blobs/rest_client.hpp
    ${AZ_STORAGE_BLOBS_DIR}/inc/azure/storage/blobs/rtti.hpp
    ${AZ_STORAGE_BLOBS_DIR}/inc/azure/storage/blobs.hpp
    ${AZ_STORAGE_BLOBS_DIR}/src/private/avro_parser.hpp
    ${AZ_STORAGE_BLOBS_DIR}/src/private/package_version.hpp
)

set(
  AZURE_STORAGE_BLOBS_SOURCE
    ${AZ_STORAGE_BLOBS_DIR}/src/append_blob_client.cpp
    ${AZ_STORAGE_BLOBS_DIR}/src/blob_batch.cpp
    ${AZ_STORAGE_BLOBS_DIR}/src/blob_client.cpp
    ${AZ_STORAGE_BLOBS_DIR}/src/blob_container_client.cpp
    ${AZ_STORAGE_BLOBS_DIR}/src/blob_lease_client.cpp
    ${AZ_STORAGE_BLOBS_DIR}/src/blob_options.cpp
    ${AZ_STORAGE_BLOBS_DIR}/src/blob_responses.cpp
    ${AZ_STORAGE_BLOBS_DIR}/src/blob_sas_builder.cpp
    ${AZ_STORAGE_BLOBS_DIR}/src/blob_service_client.cpp
    ${AZ_STORAGE_BLOBS_DIR}/src/block_blob_client.cpp
    ${AZ_STORAGE_BLOBS_DIR}/src/page_blob_client.cpp
    ${AZ_STORAGE_BLOBS_DIR}/src/private/avro_parser.cpp
    ${AZ_STORAGE_BLOBS_DIR}/src/rest_client.cpp
)

add_library(azure-core STATIC ${AZURE_CORE_HEADER} ${AZURE_CORE_SOURCE} ${AZURE_STORAGE_COMMON_HEADER} ${AZURE_STORAGE_COMMON_SOURCE}  ${AZURE_STORAGE_BLOBS_HEADER} ${AZURE_STORAGE_BLOBS_SOURCE})

target_include_directories(
  azure-core
    PUBLIC
      ${AZ_CORE_DIR}/inc
      ${AZ_STORAGE_COMMON_DIR}/inc
      ${AZ_STORAGE_BLOBS_DIR}/inc
)

# make sure that users can consume the project as a library.
add_library(Azure::azure-core ALIAS azure-core)

target_link_libraries(azure-core INTERFACE Threads::Threads)

get_property(include_3rdparty      GLOBAL PROPERTY OPENVDS_INCLUDE_LIBRARIES)
addSystemHeadersToTarget(azure-core "${include_3rdparty}")


if(BUILD_TRANSPORT_CURL)
  if (USE_LIBRESSL)
    add_dependencies(azure-core libressl)
  else()
    find_package(OpenSSL REQUIRED)
    target_link_libraries(azure-core PRIVATE OpenSSL::SSL OpenSSL::Crypto)
  endif()

  target_link_libraries(azure-core PRIVATE LibXml2)

  if (BUILD_CURL)
    add_dependencies(azure-core curl)
    if (WIN32)
      target_link_libraries(azure-core PRIVATE Ws2_32)
      target_link_libraries(azure-core PRIVATE bcrypt webservices)
    endif()
  else()
    # min version for `CURLSSLOPT_NO_REVOKE`
    # https://curl.haxx.se/libcurl/c/CURLOPT_SSL_OPTIONS.html
    set(CURL_MIN_REQUIRED_VERSION 7.44)
    find_package(CURL ${CURL_MIN_REQUIRED_VERSION} CONFIG QUIET)
    if(NOT CURL_FOUND)
      find_package(CURL ${CURL_MIN_REQUIRED_VERSION} REQUIRED)
    endif()
    message("Libcurl version ${CURL_VERSION_STRING}")
    target_link_libraries(azure-core PUBLIC CURL::libcurl)
  endif() 

  target_compile_definitions(azure-core PRIVATE BUILD_CURL_HTTP_TRANSPORT_ADAPTER)
endif()

if(BUILD_TRANSPORT_WINHTTP)
  target_link_libraries(azure-core PRIVATE winhttp WIL::WIL)
  target_link_libraries(azure-core PRIVATE bcrypt crypt32 webservices)
  target_compile_definitions(azure-core PRIVATE BUILD_TRANSPORT_WINHTTP_ADAPTER)
endif()

