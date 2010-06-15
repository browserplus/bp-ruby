#####
# A CMake build file for the RubyInterpreter service
# 
# Author: Lloyd Hilaiel
# (c) Yahoo 2009, all rights reserved
#####

# I'm a modern man
cmake_minimum_required(VERSION 2.6)

# pull in build configuration
INCLUDE(BuildConfigs.cmake)

SET (serviceName RubyInterpreter)

PROJECT(${serviceName}Service)

SET (outputDir "${CMAKE_CURRENT_BINARY_DIR}/${serviceName}")

# build up the "platform identifier"
SET(arch "i386")
IF (APPLE)
   execute_process(COMMAND uname -p
                   OUTPUT_VARIABLE arch
                   ERROR_QUIET
                   OUTPUT_STRIP_TRAILING_WHITESPACE)

   execute_process(COMMAND uname -r
                   OUTPUT_VARIABLE ver
                   ERROR_QUIET
                   OUTPUT_STRIP_TRAILING_WHITESPACE)

   execute_process(COMMAND uname -s
                   OUTPUT_VARIABLE sysUpper
                   ERROR_QUIET
                   OUTPUT_STRIP_TRAILING_WHITESPACE)

   string(TOLOWER ${sysUpper} sys) 

   SET( platIdent "${arch}-${sys}${ver}" )

   MESSAGE("platform: ${platIdent}")
ELSEIF(UNIX)
   execute_process(COMMAND uname -m
                   OUTPUT_VARIABLE arch
                   ERROR_QUIET
                   OUTPUT_STRIP_TRAILING_WHITESPACE)

   execute_process(COMMAND uname -s
                   OUTPUT_VARIABLE sysUpper
                   ERROR_QUIET
                   OUTPUT_STRIP_TRAILING_WHITESPACE)

   string(TOLOWER ${sysUpper} sys) 

   SET( platIdent "${arch}-${sys}" )

   MESSAGE("platform: ${platIdent}")
ELSE ()
   # hard coded for now!
   SET( platIdent "i386-mswin32_90" )
ENDIF ()

# looser syntax (that's my patch!)
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS 1)

# default to a debug build.  This gets us symbols by default under
# the cmake makefile generator
IF (NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE "Debug")
ENDIF ()

SET(extDir "${CMAKE_CURRENT_SOURCE_DIR}/../external")
SET(extLibDir "${extDir}/dist/lib/${CMAKE_BUILD_TYPE}")
SET(extIncDir "${extDir}/dist/include/ruby19")
SET(CPPTOOLS_DIR "${extDir}/bp-service-tools/CppTools/src")

INCLUDE_DIRECTORIES(
  "${extIncDir}/${platIdent}"
  "${extIncDir}"
  "${extDir}/bp-service-tools/CppTools/external/bpsdk/include"
  "${CPPTOOLS_DIR}"
  "${CMAKE_CURRENT_SOURCE_DIR}"
)

SET(rubyLibPath ${extLibDir})
LINK_DIRECTORIES(${rubyLibPath})

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

# find the path to the ruby library we'll link
MESSAGE("looking in ${rubyLibPath}")
FIND_LIBRARY(rubyDylib NAMES ruby msvcr90-ruby191
             PATHS ${rubyLibPath}
             NO_DEFAULT_PATH
             NO_SYSTEM_ENVIRONMENT_PATH)
GET_FILENAME_COMPONENT(rubyDylibName ${rubyDylib} NAME)

TARGET_LINK_LIBRARIES(${serviceName} ${rubyDylibName})

# create service directory
FILE(MAKE_DIRECTORY ${outputDir})

IF (APPLE)
#  ADD_CUSTOM_COMMAND(TARGET ${serviceName} POST_BUILD
#                     COMMAND strip -x \"${outputDir}/${ServiceLibrary}\")
ENDIF (APPLE)



SET(stdlibDest "${outputDir}/stdlib")

FILE(GLOB pathToPathnameRb "${extLibDir}/ruby/ruby/1.9.1/pathname.rb")
IF (NOT pathToPathnameRb) 
  FILE(GLOB pathToPathnameRb "${extLibDir}/ruby/1.9.1/pathname.rb")
ENDIF()
GET_FILENAME_COMPONENT(stdlibSource ${pathToPathnameRb} PATH)

SET(extDest "${outputDir}/ext")

FILE(GLOB extSource "${extLibDir}/ruby/1.9.1/${arch}-*")
IF (NOT extSource) 
  FILE(GLOB extSource "${extLibDir}/ruby/ruby/1.9.1/${arch}-*")
ENDIF()

IF (UNIX)
    SET(RM_CMD rm -rf)
ELSE ()
    SET(RM_CMD "del /f /q")
ENDIF ()

GET_FILENAME_COMPONENT( rubyDylibName ${rubyDylib} NAME )

# get location of output service library
GET_TARGET_PROPERTY(loc ${serviceName} LOCATION)

# handle the .lib .dll dichotomy
SET (rubyLibToShip ${rubyDylib})
IF (WIN32) 
  STRING(REGEX REPLACE "lib$" "dll" rubyLibToShipA ${rubyDylib})
  STRING(REGEX REPLACE "/lib/" "/bin/" rubyLibToShip ${rubyLibToShipA})
ENDIF () 

MESSAGE("ruby library to link:  ${rubyDylib}" )
MESSAGE("library to ship:       ${rubyLibToShip}")
MESSAGE("copy extensions from:  ${extSource}")
MESSAGE("copy stdlib from:      ${stdlibSource}")

ADD_CUSTOM_COMMAND(
    TARGET ${serviceName} POST_BUILD
    COMMAND ${RM_CMD} \"${stdlibDest}\"
    COMMAND ${CMAKE_COMMAND} -E echo cp -r
                        \"${stdlibSource}\" \"${stdlibDest}\"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        \"${stdlibSource}\" \"${stdlibDest}\"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            \"${CMAKE_CURRENT_SOURCE_DIR}/ruby/browserplus.rb\"
			\"${stdlibDest}\"
    COMMAND ${RM_CMD} \"${extDest}\"
    COMMAND ${CMAKE_COMMAND} -E echo cp -r
                        \"${extSource}\" \"${extDest}\"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        \"${extSource}\" \"${extDest}\"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            \"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt\"
            \"${outputDir}\"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            \"${rubyLibToShip}\"
            \"${outputDir}\"
)

IF (APPLE)  
  # repeat after me:  I love dylibs
  # this post-build step will help the service find our embedded dynamic ruby
  # dylib, and will help THAT guy find stdlib extensions (.bundles in ext/)
  SET(librubyBefore "libruby.dylib")
  SET(librubyAfter "libruby_bp.dylib")

  ADD_CUSTOM_COMMAND(
      TARGET ${serviceName} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy \"${outputDir}/${librubyBefore}\" \"${outputDir}/${librubyAfter}\"   
      COMMAND ${CMAKE_COMMAND} -E remove \"${outputDir}/${librubyBefore}\"
      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/libruby_fixup.sh ${loc}
      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/libruby_fixup.sh ${loc}
      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/ext_fixup.sh \"${extDest}\"
      )
ENDIF ()

# copy in manifest
GET_FILENAME_COMPONENT(ServiceLibrary "${loc}" NAME)
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/manifest.json"
               "${outputDir}/manifest.json")  
ADD_CUSTOM_COMMAND(TARGET ${serviceName} POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_if_different
                           \"${loc}\" \"${outputDir}\")

# a custom step to build latest externals
SET (pathToExtBuildRb "${CMAKE_CURRENT_SOURCE_DIR}/../external/build.rb")
ADD_CUSTOM_TARGET(
    BuildExternals ALL
    COMMAND ruby ${pathToExtBuildRb}
    COMMENT "Build 3rd party dependencies")
ADD_DEPENDENCIES( ${serviceName} BuildExternals )
