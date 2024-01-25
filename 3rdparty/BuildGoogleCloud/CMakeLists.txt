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
  ${GCP_SOURCE_DIR}/google/cloud/access_token.cc
  ${GCP_SOURCE_DIR}/google/cloud/credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/kms_key_name.cc
  ${GCP_SOURCE_DIR}/google/cloud/location.cc
  ${GCP_SOURCE_DIR}/google/cloud/log.cc
  ${GCP_SOURCE_DIR}/google/cloud/options.cc
  ${GCP_SOURCE_DIR}/google/cloud/project.cc
  ${GCP_SOURCE_DIR}/google/cloud/status.cc
  ${GCP_SOURCE_DIR}/google/cloud/terminate_handler.cc
  ${GCP_SOURCE_DIR}/google/cloud/tracing_options.cc
  ${GCP_SOURCE_DIR}/google/cloud/version.cc

  ${GCP_SOURCE_DIR}/google/cloud/internal/api_client_header.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/auth_header_error.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/backoff_policy.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/base64_transforms.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/binary_data_as_debug_string.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/compiler_info.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/compute_engine_util.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/credentials_impl.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/curl_handle.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/curl_handle_factory.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/curl_http_payload.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/curl_impl.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/curl_rest_client.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/curl_rest_response.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/curl_wrappers.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/curl_writev.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/debug_future_status.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/debug_string.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/error_context.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/external_account_source_format.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/external_account_token_source_aws.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/external_account_token_source_file.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/external_account_token_source_url.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/filesystem.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/format_time_point.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/future_impl.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/future_then_impl.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/getenv.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/json_parsing.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/log_impl.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/make_jwt_assertion.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/make_status.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/noexcept_action.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/oauth2_access_token_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/oauth2_anonymous_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/oauth2_authorized_user_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/oauth2_cached_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/oauth2_compute_engine_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/oauth2_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/oauth2_decorate_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/oauth2_error_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/oauth2_external_account_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/oauth2_google_application_default_credentials_file.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/oauth2_google_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/oauth2_impersonate_service_account_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/oauth2_logging_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/oauth2_minimal_iam_credentials_rest.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/oauth2_refreshing_credentials_wrapper.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/oauth2_service_account_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/openssl_util.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/opentelemetry.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/opentelemetry_context.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/parse_rfc3339.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/populate_common_options.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/populate_rest_options.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/random.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/rest_carrier.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/rest_context.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/rest_opentelemetry.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/rest_parse_json_error.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/rest_request.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/rest_response.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/retry_loop_helpers.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/retry_policy_impl.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/service_endpoint.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/sha256_hash.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/sha256_hmac.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/status_payload_keys.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/strerror.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/subject_token.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/throw_delegate.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/timer_queue.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/trace_propagator.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/tracing_http_payload.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/tracing_rest_client.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/tracing_rest_response.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/unified_rest_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/url_encode.cc
  ${GCP_SOURCE_DIR}/google/cloud/internal/user_agent_prefix.cc

  ${GCP_SOURCE_DIR}/google/cloud/storage/auto_finalize.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/bucket_access_control.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/bucket_autoclass.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/bucket_cors_entry.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/bucket_custom_placement_config.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/bucket_iam_configuration.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/bucket_logging.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/bucket_metadata.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/bucket_retention_policy.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/client.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/client_options.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/hashing_options.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/hmac_key_metadata.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/iam_policy.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/idempotency_policy.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/lifecycle_rule.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/list_buckets_reader.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/list_hmac_keys_reader.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/list_objects_reader.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/notification_metadata.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/object_access_control.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/object_metadata.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/object_read_stream.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/object_rewriter.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/object_write_stream.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/parallel_upload.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/policy_document.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/service_account.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/version.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/well_known_headers.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/well_known_parameters.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/access_control_common_parser.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/access_token_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/bucket_access_control_parser.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/bucket_acl_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/bucket_metadata_parser.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/bucket_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/compute_engine_util.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/connection_factory.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/connection_impl.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/const_buffer.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/crc32c.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/default_object_acl_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/empty_response.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/error_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/generate_message_boundary.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/generic_stub_adapter.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/generic_stub_factory.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/hash_function.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/hash_function_impl.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/hash_validator.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/hash_validator_impl.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/hash_values.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/hmac_key_metadata_parser.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/hmac_key_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/http_response.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/impersonate_service_account_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/invocation_id_generator.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/lifecycle_rule_parser.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/logging_stub.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/make_jwt_assertion.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/metadata_parser.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/notification_metadata_parser.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/notification_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/object_access_control_parser.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/object_acl_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/object_metadata_parser.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/object_read_streambuf.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/object_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/object_write_streambuf.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/openssl_util.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/patch_builder.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/patch_builder_details.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/policy_document_request.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/request_project_id.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/retry_object_read_source.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/service_account_parser.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/service_account_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/sign_blob_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/signed_url_requests.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/storage_connection.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/tracing_connection.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/unified_rest_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/rest/object_read_source.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/rest/request_builder.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/internal/rest/stub.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/oauth2/anonymous_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/oauth2/authorized_user_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/oauth2/compute_engine_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/oauth2/credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/oauth2/google_application_default_credentials_file.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/oauth2/google_credentials.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/oauth2/refreshing_credentials_wrapper.cc
  ${GCP_SOURCE_DIR}/google/cloud/storage/oauth2/service_account_credentials.cc
  ${CMAKE_BINARY_DIR}/google-cloud-cpp/internal/build_info.cc
  )

target_link_libraries(google_cloud_ovds PRIVATE
  Crc32c::crc32c
  CURL::libcurl
  absl::memory
  absl::strings
  absl::str_format
  absl::string_view
  absl::variant
  absl::optional
  absl::time
  absl::crc32c
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
