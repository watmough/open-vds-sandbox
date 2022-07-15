#This file depends on being in open-vds/CMake directory
set(Fetch3rdPartyDirInternal "${CMAKE_CURRENT_LIST_DIR}")

macro(Fetch3rdParty_File name dir_prefix version ext url url_hash)
  if (OPENVDS_3RD_PARTY_DIR)
    set(Fetch3rdPartyDir "${OPENVDS_3RD_PARTY_DIR}")
  else()
    set(Fetch3rdPartyDir "${Fetch3rdPartyDirInternal}/../3rdparty")
  endif()
  get_filename_component(thirdParty "${Fetch3rdPartyDir}/${dir_prefix}" ABSOLUTE)
  file(MAKE_DIRECTORY ${thirdParty})
  set(SRC_FILE ${thirdParty}/${name}-${version}.${ext})
  set(${name}_SOURCE_FILE ${SRC_FILE} PARENT_SCOPE)
  set(${name}_VERSION ${version} PARENT_SCOPE)
  if (NOT (EXISTS ${SRC_FILE}))
    file(DOWNLOAD ${url}
      ${SRC_FILE}
      SHOW_PROGRESS
      EXPECTED_HASH ${url_hash}
      )
  endif()
endmacro()

macro(Fetch3rdParty_FileTarget name dir_prefix dest version url url_hash)
  if (OPENVDS_3RD_PARTY_DIR)
    set(Fetch3rdPartyDir "${OPENVDS_3RD_PARTY_DIR}")
  else()
    set(Fetch3rdPartyDir "${Fetch3rdPartyDirInternal}/../3rdparty")
  endif()
  get_filename_component(thirdParty "${Fetch3rdPartyDir}/${name}-${version}/${dir_prefix}" ABSOLUTE)
  file(MAKE_DIRECTORY ${thirdParty})
  set(DEST_FILE ${thirdParty}/${dest})
  set(${name}_SOURCE_FILE ${DEST_FILE} PARENT_SCOPE)
  set(${name}_SOURCE_DIR  ${Fetch3rdPartyDir}/${name}-${version} PARENT_SCOPE)
  set(${name}_VERSION ${version} PARENT_SCOPE)
  if (NOT (EXISTS ${DEST_FILE}))
    file(DOWNLOAD ${url}
      ${DEST_FILE}
      SHOW_PROGRESS
      EXPECTED_HASH ${url_hash}
      )
  endif()

endmacro()

macro(Fetch3rdParty_Package name version url url_hash)
  if (OPENVDS_3RD_PARTY_DIR)
    set(Fetch3rdPartyDir "${OPENVDS_3RD_PARTY_DIR}")
  else()
    set(Fetch3rdPartyDir "${Fetch3rdPartyDirInternal}/../3rdparty")
  endif()
    get_filename_component(thirdParty "${Fetch3rdPartyDir}" ABSOLUTE)
    set(SRC_DIR ${thirdParty}/${name}-${version})
    set(${name}_SOURCE_DIR ${SRC_DIR} PARENT_SCOPE)
    set(${name}_VERSION ${version} PARENT_SCOPE)
  if (NOT (EXISTS ${SRC_DIR}))
    FetchContent_Populate(${name}
      URL ${url}
      URL_HASH ${url_hash}
      SOURCE_DIR ${SRC_DIR}
      SUBBUILD_DIR ${thirdParty}/CMakeArtifacts/${name}-sub-${version}
      BINARY_DIR ${thirdParty}/CMakeArtifacts/${name}-${version})
  endif()
endmacro()

macro(Fetch3rdParty_Git name version url tag)
  if (OPENVDS_3RD_PARTY_DIR)
    set(Fetch3rdPartyDir "${OPENVDS_3RD_PARTY_DIR}")
  else()
    set(Fetch3rdPartyDir "${Fetch3rdPartyDirInternal}/../3rdparty")
  endif()
    get_filename_component(thirdParty "${Fetch3rdPartyDir}" ABSOLUTE)
    set(SRC_DIR ${thirdParty}/${name}-${version})
    set(${name}_SOURCE_DIR ${SRC_DIR} PARENT_SCOPE)
    set(${name}_VERSION ${version} PARENT_SCOPE)
  if (NOT (EXISTS ${SRC_DIR}))
    FetchContent_Populate(${name}
      GIT_REPOSITORY ${url}
      GIT_TAG ${tag}
      GIT_SHALLOW ON
      SOURCE_DIR ${SRC_DIR}
      SUBBUILD_DIR ${thirdParty}/CMakeArtifacts/${name}-sub-${version}
      BINARY_DIR ${thirdParty}/CMakeArtifacts/${name}-${version})
  endif()
endmacro()


function(Fetch3rdParty)
  set(FETCHCONTENT_QUIET OFF)
  include(FetchContent)

  Fetch3rdParty_Git(aws-cpp-sdk           1.9.197a   https://github.com/aws/aws-sdk-cpp.git                                               1.9.197)
  Fetch3rdParty_Package(gtest             1.10.0     https://github.com/google/googletest/archive/release-1.10.0.tar.gz                   SHA256=9dc9157a9a1551ec7a7e43daea9a694a0bb5fb8bec81235d8a1e6ef64c716dcb)
  Fetch3rdParty_Package(benchmark         1.6.1     https://github.com/google/benchmark/archive/refs/tags/v1.6.1.tar.gz                   SHA256=6132883bc8c9b0df5375b16ab520fac1a85dc9e4cf5be59480448ece74b278d4)
  Fetch3rdParty_Package(jsoncpp           1.8.4      https://github.com/open-source-parsers/jsoncpp/archive/1.8.4.tar.gz                  MD5=fa47a3ab6b381869b6a5f20811198662)
  Fetch3rdParty_Package(fmt               7.1.3      https://github.com/fmtlib/fmt/archive/7.1.3.tar.gz                                   SHA256=5cae7072042b3043e12d53d50ef404bbb76949dad1de368d7f993a15c8c05ecc)
  Fetch3rdParty_Package(cpprestapi        2.10.16    https://github.com/microsoft/cpprestsdk/archive/v2.10.16.tar.gz                      SHA256=3d75e17c7d79131320438f2a15331f7ca6281c38c0e2daa27f051e290eeb8681)
  Fetch3rdParty_Package(azure-storage-cpp 7.5.0      https://github.com/Azure/azure-storage-cpp/archive/v7.5.0.tar.gz                     SHA256=446a821d115949f6511b7eb01e6a0e4f014b17bfeba0f3dc33a51750a9d5eca5)
  Fetch3rdParty_Package(pybind11          2.6.2      https://github.com/pybind/pybind11/archive/v2.6.2.tar.gz                             SHA256=8ff2fff22df038f5cd02cea8af56622bc67f5b64534f1b83b9f133b8366acff2)
  Fetch3rdParty_Package(curl              7.73.0     https://github.com/curl/curl/releases/download/curl-7_73_0/curl-7.73.0.tar.gz        SHA256=ba98332752257b47b9dea6d8c0ad25ec1745c20424f1dd3ff2c99ab59e97cf91)
  Fetch3rdParty_Package(libuv             1.34.1     https://github.com/libuv/libuv/archive/v1.34.1.tar.gz                                SHA256=e3e0105c9b26e181e0547607cb6893462beb0c652674c3795766b2e5555288b3)
  Fetch3rdParty_Package(zlib              1.2.12     http://zlib.net/zlib-1.2.12.tar.gz                                                   SHA256=91844808532e5ce316b3c010929493c0244f3d37593afd6de04f71821d5136d9)
  Fetch3rdParty_Package(libressl          3.5.0      https://cdn.openbsd.org/pub/OpenBSD/LibreSSL/libressl-3.5.0.tar.gz                   SHA256=f01d4f76191558158a06afbdc2405fefd5b02540a197ab2546c840e06a5c0fb7)
  Fetch3rdParty_Package(absl              20200225.2 https://codeload.github.com/abseil/abseil-cpp/tar.gz/20200225.2                      SHA256=f41868f7a938605c92936230081175d1eae87f6ea2c248f41077c8f88316f111)
  Fetch3rdParty_Package(crc32c            1.1.1      https://codeload.github.com/google/crc32c/tar.gz/1.1.1                               SHA256=a6533f45b1670b5d59b38a514d82b09c6fb70cc1050467220216335e873074e8)
  Fetch3rdParty_Package(google-cloud-cpp  1.14.0     https://codeload.github.com/googleapis/google-cloud-cpp/tar.gz/v1.14.0               SHA256=839b2d4dcb36a671734dac6b30ea8c298bbeaafcf7a45ee4a7d7aa5986b16569)
  Fetch3rdParty_Package(libxml2           2.9.12a    https://codeload.github.com/GNOME/libxml2/tar.gz/v2.9.12                             SHA256=8a4ddd706419c210b30b8978a51388937fd9362c34fc9a3d69e4fcc6f8055ee0)
  Fetch3rdParty_Package(azure-sdk-for-cpp 12.0.0b11  https://codeload.github.com/Azure/azure-sdk-for-cpp/tar.gz/azure-storage-blobs_12.0.0-beta.11 SHA256=b111636335340e3e7a5675351216dde606b8345b9906ed2f42ff8a794f5f2375)
  Fetch3rdParty_Package(dms               3633f2030  https://community.opengroup.org/osdu/platform/domain-data-mgmt-services/seismic/seismic-dms-suite/seismic-store-cpp-lib/-/archive/3633f20302970e41341fe70e6bc2a573b380bd17/seismic-store-cpp-lib-master.tar.gz SHA256=8186a0ef533b9254834c421ec90b80a1569cb60d800ef0e351d1dfeba6e20e2c)
  #Fetch3rdParty_Git(dms                   git        git@community.opengroup.org:osdu/platform/domain-data-mgmt-services/seismic/seismic-dms-suite/seismic-store-cpp-lib.git master)
  Fetch3rdParty_File(testng  java         6.14.3 jar https://repo1.maven.org/maven2/org/testng/testng/6.14.3/testng-6.14.3.jar            MD5=9f17a8f9e99165e148c42b21f4b63d7c)
  Fetch3rdParty_File(jcommander java      1.72 jar   https://repo1.maven.org/maven2/com/beust/jcommander/1.72/jcommander-1.72.jar         MD5=9fde6bc0ba1032eceb7267fd1ad1657b)
  Fetch3rdParty_FileTarget(google_nlohmann google/cloud/storage/internal nlohmann_json.hpp 3.4.0  https://raw.githubusercontent.com/nlohmann/json/v3.4.0/single_include/nlohmann/json.hpp MD5=27f3760c1d3a0fff7d8a2407d8db8f9d)
  Fetch3rdParty_Package(cmakerc           e7ba9e     https://github.com/vector-of-bool/cmrc/archive/e7ba9e9417960b2a5cefc9e79e8af8b06bfde3d1.zip SHA256=75c1263bb37b8bae159bacb4da10fd2eb50b9c04118901218b6a817b9d0fa757)
  Fetch3rdParty_Package(cxxopts           3.0.0      https://github.com/jarro2783/cxxopts/archive/refs/tags/v3.0.0.tar.gz SHA256=36f41fa2a46b3c1466613b63f3fa73dc24d912bc90d667147f1e43215a8c6d00)
  Fetch3rdParty_Package(sse2neon          1.5.1      https://github.com/DLTcollab/sse2neon/archive/refs/tags/v1.5.1.tar.gz SHA256=4001e2dfb14fcf3831211581ed83bcc83cf6a3a69f638dcbaa899044a351bb2a)


endfunction()

