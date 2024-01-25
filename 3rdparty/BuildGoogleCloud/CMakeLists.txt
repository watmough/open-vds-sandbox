cmake_minimum_required(VERSION 3.16)
project(google_cloud_ovds VERSION ${GCP_VERSION})

find_package(absl REQUIRED)
find_package(Crc32c REQUIRED)
find_package(CURL REQUIRED)
if (WIN32)
  find_library(CRYPTO crypto-50 HINTS ${GCP_DEPINSTALL}/lib)
  if (NOT CRYPTO)
    message(FATAL_ERROR "missing crypto lib")
  endif()
else()
  find_package(OpenSSL REQUIRED)
endif()

file(TO_CMAKE_PATH ${GCP_SOURCE_DIR} GCP_SOURCE_DIR)
configure_file(
  ${GCP_SOURCE_DIR}/google/cloud/internal/build_info.cc.in
  ${CMAKE_BINARY_DIR}/google-cloud-cpp/internal/build_info.cc
  )

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN true)

add_library(google_cloud_ovds STATIC
  ${GCP_SOURCE_DIR}/google/cloud/storage/oauth2/anonymous_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/oauth2/authorized_user_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/oauth2/compute_engine_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/oauth2/credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/oauth2/google_application_default_credentials_file.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/oauth2/google_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/oauth2/refreshing_credentials_wrapper.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/oauth2/service_account_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/status.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/openssl_util.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/throw_delegate.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/curl_handle.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/curl_download_request.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/http_response.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/getenv.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/curl_handle_factory.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/curl_request.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/curl_client.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/curl_request_builder.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/compute_engine_util.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/filesystem.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/version.cc
  ${GCP_SOURCE_DIR}/google/cloud/version.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/compiler_info.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/binary_data_as_debug_string.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/curl_wrappers.cc
  ${GCP_SOURCE_DIR}/google/cloud/log.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/strerror.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/well_known_parameters.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/random.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/bucket_acl_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/bucket_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/iam_policy.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/default_object_acl_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/well_known_headers.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/object_acl_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/hmac_key_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/object_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/sha256_hash.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/metadata_parser.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/object_access_control.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/object_metadata.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/format_time_point.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/access_control_common.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/parse_rfc3339.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/hmac_key_metadata.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/notification_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/notification_metadata.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/hashing_options.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/bucket_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/resumable_upload_session.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/service_account_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/sign_blob_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/curl_resumable_upload_session.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/bucket_access_control.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/bucket_metadata.cc
  ${GCP_SOURCE_DIR}/google/cloud/iam_bindings.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/lifecycle_rule.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/client.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/client_options.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/hash_validator.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/object_streambuf.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/logging_client.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/policy_document_request.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/retry_client.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/signed_url_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/object_stream.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/hash_validator_impl.cc
  ${GCP_SOURCE_DIR}/google/cloud/iam_policy.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/empty_response.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/service_account.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/logging_resumable_upload_session.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/idempotency_policy.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/backoff_policy.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/retry_object_read_source.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/retry_resumable_upload_session.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/const_buffer.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/api_client_header.cc
  ${CMAKE_BINARY_DIR}/google-cloud-cpp/internal/build_info.cc
  )

target_link_libraries(google_cloud_ovds PRIVATE
  Crc32c::crc32c
  CURL::libcurl
  absl::memory
  absl::strings
  absl::str_format
  absl::variant
  absl::optional
  absl::time
  )
if (WIN32)
  target_link_libraries(google_cloud_ovds PRIVATE ${CRYPTO})
else()
  target_link_libraries(google_cloud_ovds PRIVATE OpenSSL::Crypto)
endif()

target_include_directories(google_cloud_ovds PRIVATE ${GCP_DEPINSTALL}/include)

if (WIN32)
  target_compile_definitions(google_cloud_ovds PRIVATE CURL_STATICLIB=1)
endif()

if (MSVC AND (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
  target_sources(google_cloud_ovds PRIVATE ${GCP_SOURCE_DIR}/google/cloud/terminate_handler.cc)
endif()

if (NOT (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC"))
  set_source_files_properties(${GCP_SOURCE_DIR}/google/cloud/storage/internal/openssl_util.cc PROPERTIES COMPILE_FLAGS -Wno-pedantic)
  set_source_files_properties(${GCP_SOURCE_DIR}/google/cloud/storage/internal/bucket_requests.cc PROPERTIES COMPILE_FLAGS -Wno-pedantic)
endif()

target_include_directories(google_cloud_ovds PRIVATE
  ${GCP_SOURCE_DIR}
  ${google_nlohmann_SOURCE_DIR}
  )

export(TARGETS google_cloud_ovds NAMESPACE google_cloud_ovds:: FILE google_cloud_ovds-config.cmake)

include(GNUInstallDirs)

file(GLOB cloud_public_headers ${GCP_SOURCE_DIR}/google/cloud/*.h)
set_target_properties(google_cloud_ovds
  PROPERTIES
  PUBLIC_HEADER "${cloud_public_headers}"
)

install(DIRECTORY ${GCP_SOURCE_DIR}/google/cloud/storage DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/google/cloud FILES_MATCHING PATTERN *.h)
install(DIRECTORY ${GCP_SOURCE_DIR}/google/cloud/internal DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/google/cloud FILES_MATCHING PATTERN *.h)
install(TARGETS google_cloud_ovds EXPORT google_cloud_ovds-config
  ARCHIVE
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_BINDIR}
  FRAMEWORK
    DESTINATION framework
  PUBLIC_HEADER
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/google/cloud)

install(
    EXPORT google_cloud_ovds-config
    NAMESPACE ::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/google_cloud_ovds
)
