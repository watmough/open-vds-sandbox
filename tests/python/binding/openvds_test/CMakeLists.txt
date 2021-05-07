set(CMAKE_DEBUG_POSTFIX "")
pybind11_add_module(openvds_python_test MODULE
    openvds_test.cpp
)

if (MSVC AND (CMAKE_CXX_COMPILER_ID STREQUAL "Clang"))
  target_compile_options(openvds_python_test BEFORE PRIVATE "SHELL:/imsvc\ \"${PYBIND11_INCLUDE_DIR}\" /imsvc\ \"${PYTHON_INCLUDE_DIRS}\"")
else ()
  target_include_directories(openvds_python_test SYSTEM BEFORE PRIVATE ${PYBIND11_INCLUDE_DIR})
endif()

get_target_property(fmt_INCLUDE fmt::fmt INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(openvds_python_test PRIVATE ${PROJECT_SOURCE_DIR}/tests ${fmt_INCLUDE} ${COMMON_DIR})

if(WIN32)
  # In case we have the Python debug library installed it will try to pragma-link with the non-debug library. 
  # We therefore need to add the library dir to the LIBPATH.
  target_link_options(openvds_python_test PRIVATE "/LIBPATH:${Python3_LIBRARY_DIRS}")
endif()

set_target_properties(openvds_python_test
  PROPERTIES
  VERSION ${CMAKE_PROJECT_VERSION}
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/openvds_test
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/openvds_test
  )

SET(pythonsources
    ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py
    ${CMAKE_CURRENT_SOURCE_DIR}/api.py
)

add_custom_target(openvds_python_test_py_files ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${pythonsources} $<TARGET_FILE_DIR:openvds_python_test>)

copyDllForTarget(openvds_python_test)
setWarningFlagsForTarget(openvds_python_test)

target_link_libraries(openvds_python_test PRIVATE openvds)

if (OpenMP_CXX_FOUND)
  target_link_libraries(openvds_python_test PRIVATE OpenMP::OpenMP_CXX)
endif()

set_target_properties(openvds_python_test PROPERTIES INSTALL_RPATH "$ORIGIN")

