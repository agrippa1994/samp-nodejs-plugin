cmake_minimum_required (VERSION 2.6)
project (samp-nodejs-plugin)

# Set vars
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib)

# Set include directories
include_directories("${INCLUDE_DIR}")

# Add all source files
if(WIN32)
    set(SRC_PATTERN ${SRC_DIR}/*.cpp ${SRC_DIR}/*.def)
else()
    set(SRC_PATTERN ${SRC_DIR}/*.cpp)
endif()

file(GLOB_RECURSE SRC_FILES ${SRC_PATTERN})

# Link libraries
if(WIN32)
    set(LIB_PATTERN ${LIB_DIR}/*.lib)
else()
    set(LIB_PATTERN ${LIB_DIR}/*.a ${LIB_DIR}/*.so)
endif()

file(GLOB_RECURSE LIB_FILES ${LIB_PATTERN})

# Set outpout directory
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/plugins)

# Link libraries and create binary
add_library(samp-nodejs-plugin MODULE ${SRC_FILES})
target_link_libraries(samp-nodejs-plugin ${LIB_FILES})
