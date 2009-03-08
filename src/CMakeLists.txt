#####
# A CMake build file for the RubyInterpreter service
# 
# Author: Lloyd Hilaiel
# (c) Yahoo 2009, all rights reserved
#####

# I'm a modern man
cmake_minimum_required(VERSION 2.6)

# set up global BrowserPlus build flags
INCLUDE(BuildConfigs.cmake)

SET (serviceName RubyInterpreter)

PROJECT(${serviceName}Service)

SET (outputDir "${CMAKE_CURRENT_BINARY_DIR}/${serviceName}")

# looser syntax (that's my patch!)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)

# default to a debug build.  This gets us symbols by default under
# the cmake makefile generator
IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "Debug")
ENDIF ()

INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}/../external/ruby/ruby_build_output/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/../external/bpsdk/include"
)

LINK_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}/../external/ruby/ruby_build_output/lib"
)

SET(SRCS service.cpp)

ADD_LIBRARY(${serviceName} MODULE ${SRCS})

TARGET_LINK_LIBRARIES(${serviceName} ruby-static)

# create service directory
FILE(MAKE_DIRECTORY ${outputDir})

# copy in manifest
GET_TARGET_PROPERTY(loc ${serviceName} LOCATION)
GET_FILENAME_COMPONENT(ServiceLibrary "${loc}" NAME)
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/manifest.json"
               "${outputDir}/manifest.json")  
ADD_CUSTOM_COMMAND(TARGET ${serviceName} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                           \"${loc}\" \"${outputDir}\")
IF (APPLE)
  ADD_CUSTOM_COMMAND(TARGET ${serviceName} POST_BUILD
                     COMMAND strip -x \"${outputDir}/${ServiceLibrary}\")
ENDIF (APPLE)

SET(stdlibDest "${outputDir}/stdlib")
SET(stdlibSource
    "${CMAKE_CURRENT_SOURCE_DIR}/../external/ruby/ruby_build_output/lib/ruby/1.9.1")

IF (APPLE)
    SET(RM_CMD rm -rf)
ELSE ()
    SET(RM_CMD "del /f /q")
ENDIF ()


ADD_CUSTOM_COMMAND(
    TARGET ${serviceName} POST_BUILD
    COMMAND ${RM_CMD} \"${stdlibDest}\"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        \"${stdlibSource}\" \"${stdlibDest}\"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            \"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt\"
            \"${outputDir}\"
)