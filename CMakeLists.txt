cmake_minimum_required(VERSION 2.8.12)

# Set extension name here
set(TARGET_NAME parallel_python)
set(EXTENSION_NAME ${TARGET_NAME}_extension)
set(LOADABLE_EXTENSION_NAME ${TARGET_NAME}_loadable_extension)

project(${TARGET_NAME})
include_directories(src/include)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)


include_directories(pmload/include)
include_directories(pmload/dyld/lsl)

set(MACH_LOADER_SOURCES
        pmload/dyld/common/MachOAnalyzer.cpp pmload/dyld/common/MachOFile.cpp
        pmload/dyld/common/MachOLoaded.cpp pmload/dyld/common/MachOLayout.cpp
        pmload/dyld/common/Diagnostics.cpp pmload/dyld/common/ClosureFileSystemPhysical.cpp)


set(EXTENSION_SOURCES src/parallel_python_extension.cpp pmload/pmload.cpp ${MACH_LOADER_SOURCES})

build_static_extension(${TARGET_NAME} ${EXTENSION_SOURCES})
build_loadable_extension(${TARGET_NAME} " " ${EXTENSION_SOURCES})



install(
  TARGETS ${EXTENSION_NAME}
  EXPORT "${DUCKDB_EXPORT_SET}"
  LIBRARY DESTINATION "${INSTALL_LIB_DIR}"
  ARCHIVE DESTINATION "${INSTALL_LIB_DIR}")
