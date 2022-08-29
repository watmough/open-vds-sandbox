include(CheckCXXCompilerFlag)
# Check for __builtin_prefetch support in the compiler.
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("
int main() {
  char data = 0;
  const char* address = &data;
  __builtin_prefetch(address, 0, 0);
  return 0;
}
"  HAVE_BUILTIN_PREFETCH)

# Check for _mm_prefetch support in the compiler.
include(CheckCXXSourceCompiles)
check_cxx_source_compiles("
#if defined(_MSC_VER)
#include <intrin.h>
#else  // !defined(_MSC_VER)
#include <xmmintrin.h>
#endif  // defined(_MSC_VER)

int main() {
  char data = 0;
  const char* address = &data;
  _mm_prefetch(address, _MM_HINT_NTA);
  return 0;
}
"  HAVE_MM_PREFETCH)

# Check for SSE4.2 support in the compiler.
set(OLD_CMAKE_REQURED_FLAGS ${CMAKE_REQUIRED_FLAGS})
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} /arch:AVX")
else(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -msse4.2")
endif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
check_cxx_source_compiles("
#if defined(_MSC_VER)
#include <intrin.h>
#else  // !defined(_MSC_VER)
#include <cpuid.h>
#include <nmmintrin.h>
#endif  // defined(_MSC_VER)

int main() {
  _mm_crc32_u8(0, 0); _mm_crc32_u32(0, 0);
#if defined(_M_X64) || defined(__x86_64__)
   _mm_crc32_u64(0, 0);
#endif // defined(_M_X64) || defined(__x86_64__)
  return 0;
}
"  HAVE_SSE42)
set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQURED_FLAGS})

# Check for ARMv8 w/ CRC and CRYPTO extensions support in the compiler.
set(OLD_CMAKE_REQURED_FLAGS ${CMAKE_REQUIRED_FLAGS})
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # TODO(pwnall): Insert correct flag when VS gets ARM CRC32C support.
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} /arch:NOTYET")
else(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_REQUIRED_FLAGS "${CMAKE_REQUIRED_FLAGS} -march=armv8-a+crc+crypto")
endif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
check_cxx_source_compiles("
#include <arm_acle.h>
#include <arm_neon.h>

int main() {
  __crc32cb(0, 0); __crc32ch(0, 0); __crc32cw(0, 0); __crc32cd(0, 0);
  vmull_p64(0, 0);
  return 0;
}
" HAVE_ARM64_CRC32C)
set(CMAKE_REQUIRED_FLAGS ${OLD_CMAKE_REQURED_FLAGS})

# Check for strong getauxval() support in the system headers.
check_cxx_source_compiles("
#include <arm_acle.h>
#include <arm_neon.h>
#include <sys/auxv.h>

int main() {
  getauxval(AT_HWCAP);
  return 0;
}
" HAVE_STRONG_GETAUXVAL)

# Check for weak getauxval() support in the compiler.
check_cxx_source_compiles("
unsigned long getauxval(unsigned long type) __attribute__((weak));
#define AT_HWCAP 16

int main() {
  getauxval(AT_HWCAP);
  return 0;
}
" HAVE_WEAK_GETAUXVAL)

configure_file(
  "${crc32c_SOURCE_DIR}/src/crc32c_config.h.in"
  "${CMAKE_CURRENT_BINARY_DIR}/include/crc32c/crc32c_config.h"
)
include_directories("${CMAKE_CURRENT_BINARY_DIR}/include")

# ARM64 CRC32C code is built separately, so we don't accidentally compile
# unsupported instructions into code that gets run without ARM32 support.
add_library(crc32c_arm64 OBJECT "")
target_sources(crc32c_arm64
  PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/include/crc32c/crc32c_config.h"
    "${crc32c_SOURCE_DIR}/src/crc32c_arm64.cc"
    "${crc32c_SOURCE_DIR}/src/crc32c_arm64.h"
)
if(HAVE_ARM64_CRC32C)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # TODO(pwnall): Insert correct flag when VS gets ARM64 CRC32C support.
    target_compile_options(crc32c_arm64 PRIVATE "/arch:NOTYET")
  else(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(crc32c_arm64 PRIVATE "-march=armv8-a+crc+crypto")
  endif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
endif(HAVE_ARM64_CRC32C)

# SSE4.2 code is built separately, so we don't accidentally compile unsupported
# instructions into code that gets run without SSE4.2 support.
add_library(crc32c_sse42 OBJECT "")
target_sources(crc32c_sse42
  PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/include/crc32c/crc32c_config.h"
    "${crc32c_SOURCE_DIR}/src/crc32c_sse42.cc"
    "${crc32c_SOURCE_DIR}/src/crc32c_sse42.h"
)
if(HAVE_SSE42)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(crc32c_sse42 PRIVATE "/arch:AVX")
  else(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(crc32c_sse42 PRIVATE "-msse4.2")
  endif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
endif(HAVE_SSE42)

# CMake only enables PIC by default in SHARED and MODULE targets.
if(BUILD_SHARED_LIBS)
  set_property(TARGET crc32c_sse42 PROPERTY POSITION_INDEPENDENT_CODE TRUE)
endif(BUILD_SHARED_LIBS)

add_library(crc32c STATIC
    $<TARGET_OBJECTS:crc32c_arm64>
    $<TARGET_OBJECTS:crc32c_sse42>
)
target_sources(crc32c
  PRIVATE
  "${CMAKE_CURRENT_BINARY_DIR}/include/crc32c/crc32c_config.h"
    "${crc32c_SOURCE_DIR}/src/crc32c_arm64.h"
    "${crc32c_SOURCE_DIR}/src/crc32c_arm64_linux_check.h"
    "${crc32c_SOURCE_DIR}/src/crc32c_internal.h"
    "${crc32c_SOURCE_DIR}/src/crc32c_portable.cc"
    "${crc32c_SOURCE_DIR}/src/crc32c_prefetch.h"
    "${crc32c_SOURCE_DIR}/src/crc32c_read_le.h"
    "${crc32c_SOURCE_DIR}/src/crc32c_round_up.h"
    "${crc32c_SOURCE_DIR}/src/crc32c_sse42.h"
    "${crc32c_SOURCE_DIR}/src/crc32c_sse42_check.h"
    "${crc32c_SOURCE_DIR}/src/crc32c.cc"
  PUBLIC
    "${crc32c_SOURCE_DIR}/include/crc32c/crc32c.h"
)

target_include_directories(crc32c
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  $<BUILD_INTERFACE:${crc32c_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

setCommonTargetProperties(crc32c)
