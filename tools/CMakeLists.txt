set(TOOLS_FOLDER "${CMAKE_CURRENT_LIST_DIR}")
function(setCompilerFlagsForTools target)
  set_target_properties(${target} PROPERTIES FOLDER tools)
  setWarningFlagsForTarget(${target})
  copyDllForTarget(${target})

  addSystemHeadersToTarget(${target} "${jsoncpp_SOURCE_DIR}/include")

  install(TARGETS ${target}
    RUNTIME
    DESTINATION ${CMAKE_INSTALL_BINDIR}${LIB_TOOLSET_DIR}
    )

  target_link_libraries(${target} PRIVATE tools_shared)
  set_target_properties(${target} PROPERTIES INSTALL_RPATH "$ORIGIN/../lib${LIBSUFFIX}")
  target_include_directories(${target} PRIVATE ${TOOLS_FOLDER}/Shared ${COMMON_DIR} ${cxxopts_SOURCE_DIR}/include)
endfunction()

add_subdirectory(Shared)

add_subdirectory(SEGYImport)
add_subdirectory(SEGYExport)
add_subdirectory(VDSInfo)
add_subdirectory(VDSCopy)
add_subdirectory(HueBDS)
