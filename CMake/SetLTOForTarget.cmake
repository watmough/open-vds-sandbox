function(setLTOForTarget target)
  if (NOT DISABLE_LTO)
    set_target_properties(${target}
      PROPERTIES
      INTERPROCEDURAL_OPTIMIZATION_RELEASE ON
    )
  endif()
endfunction()
