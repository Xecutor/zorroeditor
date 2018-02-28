cmake_minimum_required(VERSION 3.0)

add_executable(coretest test.cpp)
if(MSVC)
  set_target_properties(coretest PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
endif()
target_link_libraries(coretest glider-core)

add_custom_command(
  TARGET coretest POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/tests/data/FreeMono.ttf      $<TARGET_FILE_DIR:coretest>
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/tests/data/potion_bubbly.png $<TARGET_FILE_DIR:coretest>
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/tests/data/Untitled.png      $<TARGET_FILE_DIR:coretest>
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/tests/data/FSEX300.ttf       $<TARGET_FILE_DIR:coretest>)
