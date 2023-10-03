cmake_minimum_required(VERSION 3.24)

project(pmload)

if(POLICY CMP0063)
  cmake_policy(SET CMP0063 NEW)
endif()

include_directories(include)
include_directories(dyld/lsl)

set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)
set(CMAKE_VERBOSE_MAKEFILE TRUE)

set(CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Wunused -Wnarrowing -Wunused -Wuninitialized  -fsanitize=address -fsanitize=undefined"
)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")

add_library(
  dyld_common_static STATIC
  dyld/common/MachOAnalyzer.cpp dyld/common/MachOFile.cpp
  dyld/common/MachOLoaded.cpp dyld/common/MachOLayout.cpp
  dyld/common/Diagnostics.cpp dyld/common/ClosureFileSystemPhysical.cpp)

set_property(TARGET dyld_common_static PROPERTY C_STANDARD 99)
set_property(TARGET dyld_common_static PROPERTY CXX_STANDARD 20)

add_library(pmload STATIC pmload.cpp)
target_link_libraries(pmload dyld_common_static)


