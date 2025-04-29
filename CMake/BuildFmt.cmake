function(BuildFmt)
  set(BUILD_SHARED_LIBS OFF)
  set(CMAKE_CXX_STANDARD 11)
  if (MSVC)
    # This avoids propagating the /utf8 command line switch up to targets that depend on fmt.
	  # On other compilers, UTF8 is on enabeled by default.
  	set(FMT_UNICODE OFF)
  endif()
  add_subdirectory(${fmt_SOURCE_DIR} ${PROJECT_BINARY_DIR}/fmt_${fmt_VERSION} EXCLUDE_FROM_ALL)
  set_target_properties(fmt PROPERTIES FOLDER ExternalProjectTargets/fmt)
endfunction()
