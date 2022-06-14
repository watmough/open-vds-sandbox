function(copyDllForTarget target)
    if (WIN32 OR APPLE)
      get_target_property(is_imported openvds::openvds IMPORTED)
      if (is_imported)
        get_target_property(openvds_location_debug openvds::openvds IMPORTED_LOCATION_DEBUG)
        get_target_property(openvds_location_relwithdebinfo openvds::openvds IMPORTED_LOCATION_RELWITHDEBINFO)
        get_target_property(openvds_location_release openvds::openvds IMPORTED_LOCATION_RELEASE)

        if (openvds_location_relwithdebinfo)
          set(optimized_location_file ${openvds_location_relwithdebinfo})
        elseif(openvds_location_release)
          set(optimized_location_file ${openvds_location_release})
        endif()

        if (optimized_location_file)
            get_filename_component(optimized_location ${optimized_location_file} DIRECTORY)
        endif()
        get_filename_component(debug_location ${openvds_location_debug} DIRECTORY)

        file(GLOB runtime_release "${optimized_location}/*.dll")
        file(GLOB runtime_debug "${debug_location}/*.dll")
        foreach(file ${runtime_release})
          add_custom_command(TARGET ${target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<$<NOT:$<CONFIG:Debug>>:${file}> ${CMAKE_SOURCE_DIR}/README.md $<TARGET_FILE_DIR:${target}>)
        endforeach()
        foreach(file ${runtime_debug})
          add_custom_command(TARGET ${target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<$<CONFIG:Debug>:${file}> ${CMAKE_SOURCE_DIR}/README.md $<TARGET_FILE_DIR:${target}>)
        endforeach()
      else()
	list(APPEND COPY_TARGETS "$<TARGET_FILE:openvds>")
	list(APPEND COPY_TARGETS "$<TARGET_FILE:segyutils>")
        if (NOT DISABLE_DMS_IOMANAGER)
	  list(APPEND COPY_TARGETS "$<TARGET_FILE:sdapi>")
        endif()
        add_custom_command(OUTPUT "${target}_copy_vds"
          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${COPY_TARGETS} $<TARGET_FILE_DIR:${target}>
          DEPENDS openvds)
        set_property(SOURCE "${target}_copy_vds"
                     PROPERTY SYMBOLIC ON
                    )
        target_sources(${target} PRIVATE ${target}_copy_vds)
        get_property(runtime_libs GLOBAL PROPERTY OPENVDS_RUNTIME_LIBS)

        foreach(file ${runtime_libs})
          add_custom_command(TARGET ${target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${file} $<TARGET_FILE_DIR:${target}>)
        endforeach()
      endif()
    endif()
endfunction()
