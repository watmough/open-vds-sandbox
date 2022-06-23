set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "We dont need to build the unit tests" FORCE)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "We dont need to build the unit tests" FORCE)
set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "We dont need to build the unit tests" FORCE)
set(BENCHMARK_ENABLE_ASSEMBLY_TESTS OFF CACHE BOOL "We dont need to build the unit tests" FORCE)
add_subdirectory(${benchmark_SOURCE_DIR} ${PROJECT_BINARY_DIR}/benchmark)

add_executable(BenchmarkRootTarget RootBenchmarkExecutable.cpp)
set_target_properties(BenchmarkRootTarget PROPERTIES FOLDER tests/benchmark)
copyDllForTarget(BenchmarkRootTarget)

function(add_benchmark_executable name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name} PRIVATE openvds benchmark::benchmark benchmark::benchmark_main)
  addSystemHeadersToTarget(${name} "${include_3rdparty}")
  add_dependencies(${name} BenchmarkRootTarget)
  set_target_properties(${name} PROPERTIES FOLDER tests/benchmark)
  target_sources(${name} PRIVATE "main.cpp")
  target_include_directories(${name} PRIVATE ${COMMON_DIR})
endfunction()

add_benchmark_executable(simple_benchmark
  simple_benchmark.cpp
)
