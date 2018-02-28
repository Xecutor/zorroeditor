cmake_minimum_required(VERSION 3.10)

cmake_policy(SET CMP0003 NEW)

if(VCPKG_ROOT)
  include(${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake)
endif()

add_subdirectory(deps)

file (GLOB ZE_FILES ./*.cpp )

add_definitions(-DZVM_STATIC_MATRIX)

add_executable(ze ${ZE_FILES} )
if(MSVC)
  target_compile_definitions(ze PRIVATE -DWIN32_LEAN_AND_MEAN -DVC_EXTRALEAN -DNOMINMAX )
  set_target_properties(ze PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
endif()

target_link_libraries(ze glider-ui zorro)
