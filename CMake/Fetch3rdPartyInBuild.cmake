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

  Fetch3rdParty_Git(aws-cpp-sdk           1.9.336    https://github.com/aws/aws-sdk-cpp.git                                               1.9.336)
  Fetch3rdParty_Package(gtest             1.12.1     https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz         SHA256=81964fe578e9bd7c94dfdb09c8e4d6e6759e19967e397dbea48d1c10e45d0df2)
  Fetch3rdParty_Package(benchmark         1.6.1     https://github.com/google/benchmark/archive/refs/tags/v1.6.1.tar.gz                   SHA256=6132883bc8c9b0df5375b16ab520fac1a85dc9e4cf5be59480448ece74b278d4)
  Fetch3rdParty_Package(jsoncpp           1.8.4      https://github.com/open-source-parsers/jsoncpp/archive/1.8.4.tar.gz                  MD5=fa47a3ab6b381869b6a5f20811198662)
  Fetch3rdParty_Package(fmt               9.1.0      https://github.com/fmtlib/fmt/archive/9.1.0.tar.gz                                   SHA256=5dea48d1fcddc3ec571ce2058e13910a0d4a6bab4cc09a809d8b1dd1c88ae6f2)
  Fetch3rdParty_Package(cpprestapi        2.10.16    https://github.com/microsoft/cpprestsdk/archive/v2.10.16.tar.gz                      SHA256=3d75e17c7d79131320438f2a15331f7ca6281c38c0e2daa27f051e290eeb8681)
  Fetch3rdParty_Package(azure-storage-cpp 7.5.0      https://github.com/Azure/azure-storage-cpp/archive/v7.5.0.tar.gz                     SHA256=446a821d115949f6511b7eb01e6a0e4f014b17bfeba0f3dc33a51750a9d5eca5)
  Fetch3rdParty_Package(pybind11          2.10.0     https://github.com/pybind/pybind11/archive/v2.10.0.tar.gz                            SHA256=eacf582fa8f696227988d08cfc46121770823839fe9e301a20fbce67e7cd70ec)
  Fetch3rdParty_Package(curl              7.85.0     https://github.com/curl/curl/releases/download/curl-7_85_0/curl-7.85.0.tar.gz        SHA256=78a06f918bd5fde3c4573ef4f9806f56372b32ec1829c9ec474799eeee641c27)
  Fetch3rdParty_Package(libuv             1.44.2     https://github.com/libuv/libuv/archive/v1.44.2.tar.gz                                SHA256=e6e2ba8b4c349a4182a33370bb9be5e23c51b32efb9b9e209d0e8556b73a48da)
  Fetch3rdParty_Package(zlib              1.2.12     http://zlib.net/zlib-1.2.12.tar.gz                                                   SHA256=91844808532e5ce316b3c010929493c0244f3d37593afd6de04f71821d5136d9)
  Fetch3rdParty_Package(libressl          3.5.3      https://cdn.openbsd.org/pub/OpenBSD/LibreSSL/libressl-3.5.3.tar.gz                   SHA256=3ab5e5eaef69ce20c6b170ee64d785b42235f48f2e62b095fca5d7b6672b8b28)
  Fetch3rdParty_Package(absl              20200225.2 https://codeload.github.com/abseil/abseil-cpp/tar.gz/20200225.2                      SHA256=f41868f7a938605c92936230081175d1eae87f6ea2c248f41077c8f88316f111)
  Fetch3rdParty_Package(crc32c            1.1.1      https://codeload.github.com/google/crc32c/tar.gz/1.1.1                               SHA256=a6533f45b1670b5d59b38a514d82b09c6fb70cc1050467220216335e873074e8)
  Fetch3rdParty_Package(google-cloud-cpp  1.14.0     https://codeload.github.com/googleapis/google-cloud-cpp/tar.gz/v1.14.0               SHA256=839b2d4dcb36a671734dac6b30ea8c298bbeaafcf7a45ee4a7d7aa5986b16569)
  Fetch3rdParty_Package(libxml2           2.9.12a    https://codeload.github.com/GNOME/libxml2/tar.gz/v2.9.12                             SHA256=8a4ddd706419c210b30b8978a51388937fd9362c34fc9a3d69e4fcc6f8055ee0)
  Fetch3rdParty_Package(azure-sdk-for-cpp 12.3.0     https://github.com/Azure/azure-sdk-for-cpp/archive/refs/tags/azure-storage-blobs_12.3.0.tar.gz SHA256=178dbd70b8aeec0ab100e646de69fedae7bba2d035d32f749eb7a535035263c8)
  Fetch3rdParty_Package(dms               8444fbf13  https://community.opengroup.org/osdu/platform/domain-data-mgmt-services/seismic/seismic-dms-suite/seismic-store-cpp-lib/-/archive/8444fbf13693c13ed90ceb578240d70ef19aa11f/seismic-store-cpp-lib-master.tar.gz SHA256=7d4cd9b3f2d2f21be37f028463569c0eb1e0fb8323c35c57df0d524347cb1e50)
  #Fetch3rdParty_Git(dms                   git        git@community.opengroup.org:osdu/platform/domain-data-mgmt-services/seismic/seismic-dms-suite/seismic-store-cpp-lib.git master)
  Fetch3rdParty_File(testng  java         6.14.3 jar https://repo1.maven.org/maven2/org/testng/testng/6.14.3/testng-6.14.3.jar            MD5=9f17a8f9e99165e148c42b21f4b63d7c)
  Fetch3rdParty_File(jcommander java      1.72 jar   https://repo1.maven.org/maven2/com/beust/jcommander/1.72/jcommander-1.72.jar         MD5=9fde6bc0ba1032eceb7267fd1ad1657b)
  Fetch3rdParty_FileTarget(google_nlohmann google/cloud/storage/internal nlohmann_json.hpp 3.4.0  https://raw.githubusercontent.com/nlohmann/json/v3.4.0/single_include/nlohmann/json.hpp MD5=27f3760c1d3a0fff7d8a2407d8db8f9d)
  Fetch3rdParty_Package(cmakerc           e7ba9e     https://github.com/vector-of-bool/cmrc/archive/e7ba9e9417960b2a5cefc9e79e8af8b06bfde3d1.zip SHA256=75c1263bb37b8bae159bacb4da10fd2eb50b9c04118901218b6a817b9d0fa757)
  Fetch3rdParty_Package(cxxopts           3.0.0      https://github.com/jarro2783/cxxopts/archive/refs/tags/v3.0.0.tar.gz SHA256=36f41fa2a46b3c1466613b63f3fa73dc24d912bc90d667147f1e43215a8c6d00)
  Fetch3rdParty_Package(sse2neon          1.5.1      https://github.com/DLTcollab/sse2neon/archive/refs/tags/v1.5.1.tar.gz SHA256=4001e2dfb14fcf3831211581ed83bcc83cf6a3a69f638dcbaa899044a351bb2a)


endfunction()

