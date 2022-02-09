include(${PROJECT_SOURCE_DIR}/CMake/CompileInTarget.cmake)
add_executable(HueBDS huebds.cpp Commands.cpp ParserHelper.cpp Util.cpp ${COMMON_DIR}/Base64/Base64.cpp $<TARGET_OBJECTS:hue_bds_objects> ${PROJECT_SOURCE_DIR}/src/OpenVDS/IO/File.cpp ${PROJECT_SOURCE_DIR}/src/OpenVDS/IO/Win_File.cpp ${PROJECT_SOURCE_DIR}/src/OpenVDS/IO/Linux_File.cpp)

setCompilerFlagsForTools(HueBDS)
if (WIN32)
  set_source_files_properties(${PROJECT_SOURCE_DIR}/src/OpenVDS/IO/Linux_File.cpp PROPERTIES HEADER_FILE_ONLY TRUE)
elseif (UNIX)
  set_source_files_properties(${PROJECT_SOURCE_DIR}/src/OpenVDS/IO/Win_File.cpp PROPERTIES HEADER_FILE_ONLY TRUE)
endif()

setCompilerFlagsForTools(HueBDS)

target_link_libraries(HueBDS PRIVATE hue_bds_objects)
target_include_directories(HueBDS PRIVATE ${PROJECT_SOURCE_DIR}/src/OpenVDS ${PROJECT_SOURCE_DIR}/src/OpenVDS/BulkDataStore)
target_compile_definitions(HueBDS PRIVATE _CRT_SECURE_NO_WARNINGS=1 OPENVDS_STATIC_DEFINE)
compileInTarget(HueBDS fmt::fmt)
compileInTarget(HueBDS jsoncpp_lib_static)
target_compile_options(HueBDS PRIVATE ${DISABLE_WARNING_FLAG})
