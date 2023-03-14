include(CMake/BuildExternal.cmake)
if (BUILD_ZLIB)
  include(CMake/BuildZlib.cmake)
endif()
if (NOT EMSCRIPTEN)
  include(CMake/BuildLibreSSL.cmake)
  include(CMake/BuildCurl.cmake)
  include(CMake/BuildAwsCrt.cmake)
  include(CMake/BuildJsonCpp.cmake)
  include(CMake/BuildFmt.cmake)
  include(CMake/BuildCppRestSdk.cmake)
  include(CMake/BuildAzure.cmake)
  include(CMake/BuildLibUV.cmake)
  include(CMake/BuildCrc32c.cmake)
  include(CMake/BuildGoogleCloud.cmake)
  include(CMake/BuildLibXML2.cmake)
  include(CMake/BuildAzureSdkForCpp.cmake)
endif()

macro(build3rdparty)
  if (BUILD_ZLIB)
    BuildZlib()
  endif()

  if (NOT EMSCRIPTEN)
    BuildJsonCPP()
    BuildFmt()

    if (Python3_FOUND)
      add_subdirectory(${pybind11_SOURCE_DIR} ${PROJECT_BINARY_DIR}/pybind11_${pybind11_VERSION} EXCLUDE_FROM_ALL)
    endif()
  
    if (NOT DISABLE_GCP_IOMANAGER OR USE_LIBRESSL)
      BuildLibreSSL()
    endif()

    BuildCurl()
    BuildLibUV()

    if (NOT DISABLE_AWS_IOMANAGER)
      BuildAwsCrt()
    endif()

    if (NOT DISABLE_AZURE_IOMANAGER)
      BuildCppRestSdk()
      BuildAzure()
    endif()

    if (NOT DISABLE_GCP_IOMANAGER)
      BuildCrc32c()
    endif()

    if (NOT DISABLE_GCP_IOMANAGER)
      BuildGoogleCloud()
    endif()

    if (NOT DISABLE_AZURESDKFORCPP_IOMANAGER)
      BuildLibXML2()
      BuildAzureSdkForCpp()
    endif()
  
    include(${cmakerc_SOURCE_DIR}/CMakeRC.cmake)

  endif()

  get_property(link_3rdparty GLOBAL PROPERTY OPENVDS_LINK_LIBRARIES)
  list(REVERSE link_3rdparty)
  set_property(GLOBAL PROPERTY OPENVDS_LINK_LIBRARIES "${link_3rdparty}")
endmacro()
