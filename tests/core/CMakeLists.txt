cmake_minimum_required(VERSION 3.0)

add_executable(coretest test.cpp)
if(MSVC)
  set_target_properties(coretest PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
endif()
target_link_libraries(coretest glider-core)
