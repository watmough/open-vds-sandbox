function(BuildOpenSSL)
  get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if (${_isMultiConfig})
    set(TOOLSET_VERSION ${MSVC_TOOLSET_VERSION_LOCAL})
  endif()
  if (WIN32)
  elseif(APPLE)
    set(CRYPTO_LIB "lib/libcrypto.a")
    set(SSL_LIB    "lib/libssl.a")
  else()
    set(CRYPTO_LIB "lib64/libcrypto.a")
    set(SSL_LIB    "lib64/libssl.a")
  endif()
  list(APPEND OPENSSL_DLLS_LIST "${CRYPTO_LIB}")
  list(APPEND OPENSSL_DLLS_LIST "${SSL_LIB}")

  GetRootInstallDir(INSTALL_INT openssl ${openssl_VERSION}) 
  set(INSTALL_INT_CONFIG "${INSTALL_INT}/$<CONFIG>")
 
  set_property(GLOBAL APPEND PROPERTY OPENVDS_INCLUDE_LIBRARIES "${INSTALL_INT_CONFIG}/include")
  set_property(GLOBAL APPEND PROPERTY OPENVDS_DEPENDENCY_TARGETS openssl)
  get_filename_component(ABS_PATH_INSTALL_INT_CONFIG "${INSTALL_INT_CONFIG}" ABSOLUTE)
  foreach (LIB IN LISTS OPENSSL_DLLS_LIST)
    set_property(GLOBAL APPEND PROPERTY OPENVDS_LINK_LIBRARIES "${ABS_PATH_INSTALL_INT_CONFIG}/${LIB}")
    set_property(GLOBAL APPEND PROPERTY OPENVDS_RUNTIME_LIBS "${ABS_PATH_INSTALL_INT_CONFIG}/${LIB}")
    list(APPEND BUILDBYPRODUCTS "${ABS_PATH_INSTALL_INT_CONFIG}/${LIB}")
  endforeach()

 include(ExternalProject)
 ExternalProject_Add(
  openssl
  PREFIX ${PROJECT_BINARY_DIR}/openssl_${openssl_VERSION}
  SOURCE_DIR ${openssl_SOURCE_DIR}
  CONFIGURE_COMMAND
    ${openssl_SOURCE_DIR}/config
    --prefix=${INSTALL_INT_CONFIG}
    no-module
    no-shared
    no-zlib
  BUILD_COMMAND make build_sw
  TEST_COMMAND ""
  INSTALL_COMMAND make install_sw
  INSTALL_DIR ${INSTALL_INT}
  BUILD_BYPRODUCTS "${BUILDBYPRODUCTS}"
)
set(OPENSSL_ROOT_DIR ${INSTALL_INT_CONFIG} PARENT_SCOPE)
set(OPENSSL_CRYPTO_LIBRARY ${ABS_PATH_INSTALL_INT_CONFIG}/${CRYPTO_LIB} PARENT_SCOPE)
set(OPENSSL_SSL_LIBRARY ${ABS_PATH_INSTALL_INT_CONFIG}/${SSL_LIB} PARENT_SCOPE)

endfunction()
