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

SET(CPPTOOLS_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/../external/bp-service-tools/CppTools/src"
)

INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}/../external/ruby/ruby_build_output/include/ruby-1.9.1"
  "${CMAKE_CURRENT_SOURCE_DIR}/../external/bpsdk/include"
  "${CPPTOOLS_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

LINK_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}/../external/ruby/ruby_build_output/lib"
)

SET(EXT_SRCS
  "${CPPTOOLS_DIR}/bptypeutil.cpp"
  "${CPPTOOLS_DIR}/bpservicedescription.cpp"
  "${CPPTOOLS_DIR}/bpserviceversion.cpp"
)

SET(SRCS
  service.cpp
  RubyInterpreter.cpp
  RubyWork.cpp
  RubyUtils.cpp
  util/fileutils.cpp
  Definition.cpp
  DataMapping.cpp
  BuiltIns.cpp
)

# os specific srcs
IF (WIN32)
   FILE(GLOB OS_SRCS "util/*_Windows.cpp")
ELSE()
   FILE(GLOB OS_SRCS "util/*_Darwin.cpp" "util/*_UNIX.cpp")
ENDIF ()

FILE(GLOB_RECURSE HDRS "*.hh")

SET(HDRS RubyInterpreter.hh)
ADD_LIBRARY(${serviceName} MODULE ${EXT_SRCS} ${OS_SRCS} ${SRCS} ${HDRS})

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
"${CMAKE_CURRENT_SOURCE_DIR}/../external/ruby/ruby_build_output/lib/ruby/1.9.1"
)

SET(extDest "${outputDir}/ext")
SET(extSource
"${CMAKE_CURRENT_SOURCE_DIR}/../external/ruby/ruby_build_output/lib/ruby/1.9.1/i386-darwin9.6.0"
)

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
            \"${CMAKE_CURRENT_SOURCE_DIR}/ruby/browserplus.rb\"
			\"${stdlibDest}\"
    COMMAND ${RM_CMD} \"${extDest}\"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        \"${extSource}\" \"${extDest}\"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            \"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt\"
            \"${outputDir}\"
)
