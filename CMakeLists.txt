cmake_minimum_required(VERSION 2.6)

if(CMAKE_BUILD_TYPE MATCHES Debug)
add_definitions(-DDEBUG)
set(CMAKE_CXX_FLAGS -Wall)
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_definitions(-DZVM_STATIC_MATRIX)
add_definitions(-I/usr/include)

#set(CMAKE_CXX_FLAGS -pg)

if(APPLE)
add_definitions(-stdlib=libc++)
include_directories(SYSTEM "/usr/local/include")
link_directories("/usr/local/lib")
endif(APPLE)

if(WIN32)
if(CMAKE_CXX_COMPILER MATCHES g[+][+][.]exe)
set(CMAKE_EXE_LINKER_FLAGS -Wl,--enable-auto-import)
endif()
endif(WIN32)

if(MSVC)
include_directories("d:/Work/CPP/kst/include")
link_directories("d:/Work/CPP/kst/")
else()
add_definitions(-std=c++11)
#set(CMAKE_EXE_LINKER_FLAGS -stdlib=libc++)
endif(MSVC)



add_library(zorro STATIC
ZVMOps.cpp
ZorroParser.cpp
FileReader.cpp
CodeGenerator.cpp
ZorroVM.cpp
Debug.cpp
ZMap.cpp
ZSet.cpp
ZMemory.cpp
ZorroLexer.cpp
ZVMStd.cpp
ZString.cpp
Symbolic.cpp
SynTree.cpp
MacroExpander.cpp
Debugger.cpp
#HeapTracer.cpp
)
add_executable( zorro-bin zorro.cpp )
set_target_properties(zorro-bin PROPERTIES OUTPUT_NAME zorro )
target_link_libraries( zorro-bin zorro kst)
