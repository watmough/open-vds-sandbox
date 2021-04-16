configure_file("connection_defs.txt" "${CMAKE_CURRENT_BINARY_DIR}/connection_defs.py" @ONLY)

function(add_python_test scope test_file)
  if (UNIX)
    set(separator ":")
  else ()
    set(separator ";")
  endif()

  string(REGEX REPLACE "\.py$" "" test_name ${test_file})

  add_test(NAME "python.${scope}.${test_name}"
    COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=$<TARGET_FILE_DIR:core>/..${separator}${CMAKE_CURRENT_BINARY_DIR}${separator}$<TARGET_FILE_DIR:openvds_python_test>/..${separator}${CMAKE_CURRENT_SOURCE_DIR}"
      ${Python3_EXECUTABLE} ${test_file}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endfunction()

function(add_python_pytest scope dir)
endfunction()

add_python_test("python" "basic_tests/basictest.py")
add_python_test("python" "basic_tests/createtest.py")
add_python_test("python" "basic_tests/VolumeDataAccessor.py")
add_python_test("python" "basic_tests/RequestVolumeDataSubset.py")
  
if (UNIX)
  set(separator ":")
else ()
  set(separator ";")
endif()

add_test(NAME "python.pytests.pytests"
  COMMAND ${CMAKE_COMMAND} -E env "PYTHONPATH=$<TARGET_FILE_DIR:core>/..${separator}${CMAKE_CURRENT_BINARY_DIR}${separator}$<TARGET_FILE_DIR:openvds_python_test>/..${separator}${CMAKE_CURRENT_SOURCE_DIR}"
  ${Python3_EXECUTABLE} -m pytest -s
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/pytests
  )

add_subdirectory(binding)
