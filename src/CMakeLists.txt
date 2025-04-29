add_subdirectory(OpenVDS)

if (NOT EMSCRIPTEN AND NOT BUILD_FOR_SANDBOX)
  add_subdirectory(SEGYUtils)
endif()
