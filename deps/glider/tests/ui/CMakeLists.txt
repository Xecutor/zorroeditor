cmake_minimum_required(VERSION 3.0)

add_executable(uitest uitest.cpp )
if(MSVC)
  set_target_properties(uitest PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
endif()
target_link_libraries(uitest glider-ui)

add_custom_command(
  TARGET uitest POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${tests_data_dir}/FreeMono.ttf $<TARGET_FILE_DIR:uitest>)
