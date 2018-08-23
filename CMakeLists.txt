cmake_minimum_required(VERSION 3.0)

add_library( kst STATIC src/Logger.cpp src/Format.cpp)

if(MSVC)
<<<<<<< HEAD
  target_compile_definitions(kst PRIVATE -D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_WARNINGS)
  target_compile_options(kst PRIVATE /wd4710 /wd4514 /wd4820)
=======
  target_compile_definitions(kst PRIVATE -D_CRT_NONSTDC_NO_DEPRECATE)
  target_link_libraries(kst ws2_32)
>>>>>>> 2cac7c46a925e7b9b0b83c563625c637a1309776
endif()


target_include_directories(kst PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/kst>
)
