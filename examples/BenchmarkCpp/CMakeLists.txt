
add_executable(benchmark-cpp benchmark-cpp.cpp)
target_link_libraries(benchmark-cpp openvds::openvds fmt)
set_target_properties(benchmark-cpp PROPERTIES FOLDER examples)

target_compile_definitions(benchmark-cpp PRIVATE -DTEST_URL="${TEST_URL}")
string(REPLACE ";" "\\\\;" TEST_CONNECTION_ESCAPED "${TEST_CONNECTION}")
target_compile_definitions(benchmark-cpp PRIVATE -DTEST_CONNECTION="${TEST_CONNECTION_ESCAPED}")

setWarningFlagsForTarget(benchmark-cpp)
copyDllForTarget(benchmark-cpp)
