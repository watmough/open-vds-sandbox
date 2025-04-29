add_subdirectory(OpenVDS)

if (NOT EMSCRIPTEN)
  add_subdirectory(SEGYUtils)
endif()
