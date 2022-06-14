function(setCommonTargetProperties target)
  setWarningFlagsForTarget(${target})
  setVersionForTarget(${target})
  setLTOForTarget(${target})
  setCompilerFlagsForTarget(${target})
endfunction()

function(setCommonTestTargetProperties target)
  setWarningFlagsForTarget(${target})
  setLTOForTarget(${target})
  setCompilerFlagsForTarget(${target})
endfunction()

function(setExportedHeadersForTarget target)
  set_target_properties(${target}
    PROPERTIES
    PUBLIC_HEADER "${ARGN}"
  )
endfunction()
