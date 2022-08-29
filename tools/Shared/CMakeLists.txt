add_library(tools_shared OBJECT
  HelpConnection.h
  HelpConnection.cpp)
target_link_libraries(tools_shared PUBLIC help::connection)
setCompilerFlagsForTarget(tools_shared)
