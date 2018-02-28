cmake_minimum_required(VERSION 3.10)

add_subdirectory(glider)
add_subdirectory(zorro)

file (GLOB ZE_FILES ./*.cpp )

add_definitions(-DZVM_STATIC_MATRIX)

add_executable(ze ${ZE_FILES} )
if(MSVC)
  add_definitions(-DWIN32_LEAN_AND_MEAN -DVC_EXTRALEAN -DNOMINMAX )
  set_target_properties(uitest PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
endif()
target_link_libraries(ze glider-ui zorro)
