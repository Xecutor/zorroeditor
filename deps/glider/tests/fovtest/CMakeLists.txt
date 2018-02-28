cmake_minimum_required(VERSION 3.0)

add_executable(fovtest test.cpp)
target_link_libraries(fovtest glider-core)

add_custom_command(
  TARGET fovtest POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/tests/data/testmap.txt $<TARGET_FILE_DIR:fovtest>
  COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/tests/data/FSEX300.ttf $<TARGET_FILE_DIR:fovtest>)
