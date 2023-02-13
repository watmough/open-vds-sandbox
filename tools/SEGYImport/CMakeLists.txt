add_executable(SEGYImport
  SEGYImport.cpp
  TraceInfo2DManager.cpp
  TraceInfo2DManager.h
  )

target_link_libraries(SEGYImport PRIVATE Threads::Threads openvds segyutils jsoncpp_lib_static fmt::fmt)

setCompilerFlagsForTools(SEGYImport)

if(MSVC_VERSION LESS_EQUAL 1916)
  set_source_files_properties(SEGYImport.cpp PROPERTIES COMPILE_FLAGS /bigobj)
endif()

if(WIN32)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} setargv.obj")
endif()
