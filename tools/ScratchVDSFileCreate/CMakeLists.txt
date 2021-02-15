add_executable(VDSFileCreate
        VDSFileCreate.cpp
  )

target_link_libraries(VDSFileCreate PRIVATE Threads::Threads openvds segyutils jsoncpp_lib_static fmt::fmt)

setCompilerFlagsForTools(VDSFileCreate)
