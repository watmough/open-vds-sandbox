set(CMAKE_DEBUG_POSTFIX "")

set(OPT_STABLE_API "")

if(SKBUILD)
  set(PY_INSTALL_DIR ${SKBUILD_PLATLIB_DIR}/openvds)
  set(PY_HEADER_DIR ${SKBUILD_HEADERS_DIR})

  if(NOT "${SKBUILD_SABI_VERSION}" STREQUAL "")
    set(OPT_WITH_SOABI "WITH_SOABI;USE_SABI;${SKBUILD_SABI_VERSION}")
  endif()
else()
  set(PY_INSTALL_DIR python/openvds)
  set(PY_HEADER_DIR $<INSTALL_INTERFACE:include>)
  message(WARNING "Enable SKBUILD for python wheel-builds - Recommended way to build wheel: `python -m build -w`")
endif()


pybind11_add_module(core MODULE ${OPT_STABLE_API}
    core.cpp
    PyGlobal.cpp
    PyGlobal.h
    PyMetadataKey.cpp
    PyMetadataKey.h
    PyKnownMetadata.cpp
    PyKnownMetadata.h
    PyMetadataAccess.cpp
    PyMetadataAccess.h
    PyMetadataContainer.cpp
    PyMetadataContainer.h
    PyVolumeData.cpp
    PyVolumeData.h
    PyVolumeDataAccess.cpp
    PyVolumeDataAccess.h
    PyVolumeDataAccessManager.cpp
    PyVolumeDataAccessManager.h
    PyVolumeDataAxisDescriptor.cpp
    PyVolumeDataAxisDescriptor.h
    PyVolumeDataChannelDescriptor.cpp
    PyVolumeDataChannelDescriptor.h
    PyVolumeDataLayout.cpp
    PyVolumeDataLayout.h
    PyVolumeDataLayoutDescriptor.cpp
    PyVolumeDataLayoutDescriptor.h
    PyVolumeSampler.cpp
    PyVolumeSampler.h
    PyGlobalState.cpp
    PyGlobalState.h
    PyExceptions.cpp
    PyExceptions.h
    PyCoordinateTransformer.cpp
    PyCoordinateTransformer.h
    PyIJKCoordinateTransformer.cpp
    PyIJKCoordinateTransformer.h
    PyError.cpp
    PyError.h
    PyLog.cpp
    PyLog.h
)

if (MSVC AND (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
  openvds_target_compile_options(core BEFORE PRIVATE "SHELL:/imsvc\ \"${PYBIND11_INCLUDE_DIR}\" /imsvc\ \"${PYTHON_INCLUDE_DIRS}\"")
else ()
  target_include_directories(core SYSTEM BEFORE PRIVATE ${PYBIND11_INCLUDE_DIR})
endif()

if(WIN32)
  # In case we have the Python debug library installed it will try to pragma-link with the non-debug library. 
  # We therefore need to add the library dir to the LIBPATH.
  target_link_options(core PRIVATE "/LIBPATH:${Python3_LIBRARY_DIRS}")
endif()

set_target_properties(core
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/openvds
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/openvds
  VS_DEBUGGER_COMMAND "${Python3_EXECUTABLE}"
  VS_DEBUGGER_ENVIRONMENT "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}/..;$ENV{PYTHONPATH}"
  )

list(APPEND pythonsources ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py)
list(APPEND pythonsources ${CMAKE_CURRENT_SOURCE_DIR}/api.py)
list(APPEND pythonsources ${CMAKE_CURRENT_SOURCE_DIR}/volumedataaccess.py)
list(APPEND pythonsources ${CMAKE_CURRENT_SOURCE_DIR}/volumedataaccessors.py)
list(APPEND pythonsources ${CMAKE_CURRENT_SOURCE_DIR}/asyncmethods.py)

add_custom_target(core_py_files ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${pythonsources} $<TARGET_FILE_DIR:core>)

copyDllForTarget(core)
setWarningFlagsForTarget(core)

target_link_libraries(core PRIVATE openvds)

if (OpenMP_CXX_FOUND)
  target_link_libraries(core PRIVATE OpenMP::OpenMP_CXX)
endif()

if(APPLE)
  set_target_properties(core PROPERTIES INSTALL_RPATH "@loader_path")
else()
  set_target_properties(core PROPERTIES INSTALL_RPATH "$ORIGIN")
endif()

install(TARGETS core LIBRARY CONFIGURATIONS Release RelWithDebInfo MinSizeRel DESTINATION ${PY_INSTALL_DIR})
install(TARGETS core LIBRARY CONFIGURATIONS Debug DESTINATION python/Debug/openvds)
install(FILES ${pythonsources} CONFIGURATIONS Release RelWithDebInfo MinSizeRel DESTINATION ${PY_INSTALL_DIR})
install(FILES ${pythonsources} CONFIGURATIONS Debug DESTINATION python/Debug/openvds)

if (NOT PYTHON_DISABLE_INSTALL_3RD_PARTY)
  set(PYINSTALL_TARGETS "openvds")
  get_property(runtime_3rdparty GLOBAL PROPERTY OPENVDS_RUNTIME_LIBS)
  install(FILES ${runtime_3rdparty} CONFIGURATIONS Release RelWithDebInfo MinSizeRel DESTINATION ${PY_INSTALL_DIR})
  install(FILES ${runtime_3rdparty} CONFIGURATIONS Debug DESTINATION python/Debug/openvds)
  install(TARGETS ${PYINSTALL_TARGETS} CONFIGURATIONS Release RelWithDebInfo MinSizeRel
    RUNTIME DESTINATION ${PY_INSTALL_DIR}
    LIBRARY DESTINATION ${PY_INSTALL_DIR}
    PUBLIC_HEADER DESTINATION ${PY_HEADER_DIR}
  )
  install(TARGETS ${PYINSTALL_TARGETS} CONFIGURATIONS Debug
    RUNTIME DESTINATION python/Debug/openvds
    LIBRARY DESTINATION python/Debug/openvds
    PUBLIC_HEADER DESTINATION $<INSTALL_INTERFACE:include>
  )
  set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION ${PY_INSTALL_DIR})
  include (InstallRequiredSystemLibraries)
endif()
