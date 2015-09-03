cmake_minimum_required (VERSION 2.6)
project (samp-nodejs-plugin)

# Set vars
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib)

# Set include directories
include_directories("${INCLUDE_DIR}")

# Set defines
add_definitions(-DSUBHOOK_STATIC)

# Add all source files
if(WIN32)
    set(SRC_PATTERN ${SRC_DIR}/*.cpp ${SRC_DIR}/*.def ${SRC_DIR}/*.c ${INCLUDE_DIR}/*.cpp ${INCLUDE_DIR}/*.def ${INCLUDE_DIR}/*.c)
else()
    set(SRC_PATTERN ${SRC_DIR}/*.cpp ${SRC_DIR}/*.c ${INCLUDE_DIR}/*.cpp ${INCLUDE_DIR}/*.def ${INCLUDE_DIR}/*.c)
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

if(WIN32)
FOREACH(CONF ${CMAKE_CONFIGURATION_TYPES})
STRING(TOUPPER "${CONF}" CONF)
SET("CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONF}" "${CMAKE_SOURCE_DIR}/bin/plugins")
SET("CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONF}" "${CMAKE_SOURCE_DIR}/bin/plugins")
IF(WIN32)
  SET("CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONF}" "${CMAKE_SOURCE_DIR}/bin/plugins")
ELSE()
  SET("CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONF}" "$${CMAKE_SOURCE_DIR}/bin/plugins")
ENDIF()
ENDFOREACH()

SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
SET (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /SAFESEH:NO")
SET (CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /SAFESEH:NO")

endif()

# Link libraries and create binary
add_library(samp-nodejs-plugin MODULE ${SRC_FILES})
target_link_libraries(samp-nodejs-plugin ${LIB_FILES})
