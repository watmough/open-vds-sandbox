function(GetRootInstallDir var name version)
  set(${var} "${PROJECT_BINARY_DIR}/${name}_${version}_install" PARENT_SCOPE)
endfunction()

function(BuildExternal name version depends source_dir install_libs runtime_libs cmake_args)

  GetRootInstallDir(INSTALL_INT ${name} ${version})

  if (UNIX)
    if (NOT install_libs)
      list(APPEND install_libs ${runtime_libs})
    endif()
  endif()

  get_property(_isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
  if (NOT ${_isMultiConfig})
    set(CMAKE_BUILD_TYPE_ARG "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE};")
  endif()

  set(INSTALL_INT_CONFIG "${INSTALL_INT}/$<CONFIG>")
  get_filename_component(ABS_PATH_INSTALL_INT_CONFIG "${INSTALL_INT_CONFIG}" ABSOLUTE)
  foreach (LIB IN LISTS install_libs)
    list(APPEND BUILDBYPRODUCTS "${ABS_PATH_INSTALL_INT_CONFIG}/${LIB}")
  endforeach()

  set(${name}_INSTALL_INT_CONFIG "${INSTALL_INT_CONFIG}" PARENT_SCOPE)

  foreach (LIB IN LISTS install_libs)
    set_property(GLOBAL APPEND PROPERTY OPENVDS_LINK_LIBRARIES "${ABS_PATH_INSTALL_INT_CONFIG}/${LIB}")
  endforeach()

  foreach (LIB IN LISTS runtime_libs)
    set_property(GLOBAL APPEND PROPERTY OPENVDS_RUNTIME_LIBS "${ABS_PATH_INSTALL_INT_CONFIG}/${LIB}")
  endforeach()

  set_property(GLOBAL APPEND PROPERTY OPENVDS_INCLUDE_LIBRARIES "${INSTALL_INT_CONFIG}/include")
  set_property(GLOBAL APPEND PROPERTY OPENVDS_DEPENDENCY_TARGETS "${name}")

  if (cmake_args)
    set(cmake_arg_complete "${cmake_args};")
  endif()

  set(cmake_arg_complete "${cmake_arg_complete}${CMAKE_BUILD_TYPE_ARG}")
  if (CCACHE_PROGRAM)
    set(cmake_arg_complete "${cmake_arg_complete}-DCMAKE_C_COMPILER_LAUNCHER=ccache;-DCMAKE_CXX_COMPILER_LAUNCHER=ccache;")
  endif()
  set(cmake_arg_complete "${cmake_arg_complete}-DCMAKE_INSTALL_PREFIX=${INSTALL_INT_CONFIG};-DCMAKE_INSTALL_MESSAGE=LAZY;-Wno-dev")
  include(ExternalProject)
  ExternalProject_Add(${name}
    PREFIX ${PROJECT_BINARY_DIR}/${name}_${version}
    SOURCE_DIR ${source_dir}
    BUILD_IN_SOURCE OFF
    USES_TERMINAL_DOWNLOAD ON
    USES_TERMINAL_BUILD ON
    CMAKE_GENERATOR ${CMAKE_GENERATOR}
    CMAKE_GENERATOR_PLATFORM ${CMAKE_GENERATOR_PLATFORM}
    CMAKE_GENERATOR_TOOLSET ${CMAKE_GENERATOR_TOOLSET}
    CMAKE_ARGS ${cmake_arg_complete}
    BUILD_BYPRODUCTS "${BUILDBYPRODUCTS}"
    DEPENDS ${depends})
endfunction()

