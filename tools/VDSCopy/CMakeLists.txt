add_executable(VDSCopy VDSCopy.cpp)

target_link_libraries(VDSCopy PUBLIC Threads::Threads openvds fmt::fmt jsoncpp_lib_static)

setCompilerFlagsForTools(VDSCopy)
