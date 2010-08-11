#####
# A CMake build file for the RubyInterpreter service.
# 
# 
# Copyright 2009, Yahoo!
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
# 
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in
#     the documentation and/or other materials provided with the
#     distribution.
# 
#  3. Neither the name of Yahoo! nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
# INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
# STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
# IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
# 
# Original Author: Lloyd Hilaiel
#####

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.1)

SET (SERVICE_NAME RubyInterpreter)
PROJECT(${SERVICE_NAME}Service)

INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/../external/bp-service-framework/build_support/BuildConfigs.cmake)

# build up the "platform identifier"
SET(arch "i386")
IF (APPLE)
 EXECUTE_PROCESS(COMMAND uname -p
                 OUTPUT_VARIABLE arch
                 ERROR_QUIET
                 OUTPUT_STRIP_TRAILING_WHITESPACE)
 EXECUTE_PROCESS(COMMAND uname -r
                 OUTPUT_VARIABLE ver
                 ERROR_QUIET
                 OUTPUT_STRIP_TRAILING_WHITESPACE)
 EXECUTE_PROCESS(COMMAND uname -s
                 OUTPUT_VARIABLE sysUpper
                 ERROR_QUIET
                 OUTPUT_STRIP_TRAILING_WHITESPACE)
 STRING(TOLOWER ${sysUpper} sys) 
 SET(platIdent "${arch}-${sys}${ver}")
 MESSAGE("platform: ${platIdent}")
ELSEIF(UNIX)
 EXECUTE_PROCESS(COMMAND uname -m
                 OUTPUT_VARIABLE arch
                 ERROR_QUIET
                 OUTPUT_STRIP_TRAILING_WHITESPACE)
 EXECUTE_PROCESS(COMMAND uname -s
                 OUTPUT_VARIABLE sysUpper
                 ERROR_QUIET
                 OUTPUT_STRIP_TRAILING_WHITESPACE)
 STRING(TOLOWER ${sysUpper} sys) 
 SET(platIdent "${arch}-${sys}")
 MESSAGE("platform: ${platIdent}")
ELSE ()
 # hard coded for now!
 SET(platIdent "i386-mswin32_90")
ENDIF ()

SET(extDir "${CMAKE_CURRENT_SOURCE_DIR}/../external")
SET(extBinDir "${extDir}/dist/bin")
IF ("${CMAKE_BUILD_TYPE}" STREQUAL "CodeCoverage")
  SET(extLibDir "${extDir}/dist/lib/Debug")
ELSE ()
  SET(extLibDir "${extDir}/dist/lib/${CMAKE_BUILD_TYPE}")
ENDIF ()
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

# os specific srcs
IF (WIN32)
   FILE(GLOB OS_SRCS "util/*_Windows.cpp")
ELSE()
   FILE(GLOB OS_SRCS "util/*_Darwin.cpp" "util/*_UNIX.cpp")
ENDIF ()

SET(EXT_SRCS
  "${CPPTOOLS_DIR}/bptypeutil.cpp"
  "${CPPTOOLS_DIR}/bpservicedescription.cpp"
  "${CPPTOOLS_DIR}/bpserviceversion.cpp"
)

#FILE(GLOB_RECURSE HDRS "*.hh")

# find the path to the ruby library we'll link
MESSAGE("looking in ${rubyLibPath}")
FIND_LIBRARY(rubyDylib NAMES ruby msvcr90-ruby191
             PATHS ${rubyLibPath}
             NO_DEFAULT_PATH
             NO_SYSTEM_ENVIRONMENT_PATH)
GET_FILENAME_COMPONENT(rubyDylibName ${rubyDylib} NAME)

SET(SRCS
  service.cpp
  RubyInterpreter.cpp
  RubyWork.cpp
  RubyUtils.cpp
  util/fileutils.cpp
  util/widetoutf8.cpp
  Definition.cpp
  DataMapping.cpp
  BuiltIns.cpp
  ${EXT_SRCS}
  ${OS_SRCS}
)
SET(HDRS RubyInterpreter.hh)
SET(LIBS ${rubyDylibName})

BPAddCppService()

SET(stdlibDest "${OUTPUT_DIR}/stdlib")

FILE(GLOB pathToPathnameRb "${extLibDir}/ruby/ruby/1.9.1/pathname.rb")
IF (NOT pathToPathnameRb) 
  FILE(GLOB pathToPathnameRb "${extLibDir}/ruby/1.9.1/pathname.rb")
ENDIF()
GET_FILENAME_COMPONENT(stdlibSource ${pathToPathnameRb} PATH)

SET(extDest "${OUTPUT_DIR}/ext")

FILE(GLOB extSource "${extLibDir}/ruby/1.9.1/${arch}-*")
IF (NOT extSource) 
  FILE(GLOB extSource "${extLibDir}/ruby/ruby/1.9.1/${arch}-*")
ENDIF()

IF (UNIX)
  SET(RM_CMD rm -rf)
ELSE ()
  IF (WIN32) 
    SET(RM_CMD ${CMAKE_COMMAND} -E remove_directory)
  ELSE ()
    SET(RM_CMD "del /f /q")
  ENDIF ()
ENDIF ()

GET_FILENAME_COMPONENT(rubyDylibName ${rubyDylib} NAME)

# get location of output service library
#GET_TARGET_PROPERTY(loc ${SERVICE_NAME} LOCATION)

# handle the .lib .dll dichotomy
SET (rubyLibToShip ${rubyDylib})
IF (WIN32) 
  SET(rubyLibToShipA "${extBinDir}/${rubyDylibName}")
  STRING(REGEX REPLACE "lib$" "dll" rubyLibToShip ${rubyLibToShipA})
ENDIF () 

MESSAGE("ruby library to link:  ${rubyDylib}" )
MESSAGE("library to ship:       ${rubyLibToShip}")
MESSAGE("copy extensions from:  ${extSource}")
MESSAGE("copy stdlib from:      ${stdlibSource}")

ADD_CUSTOM_COMMAND(
    TARGET ${SERVICE_NAME} POST_BUILD
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
            \"${OUTPUT_DIR}\"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            \"${rubyLibToShip}\"
            \"${OUTPUT_DIR}\"
)

IF (APPLE)  
  # repeat after me:  I love dylibs
  # this post-build step will help the service find our embedded dynamic ruby
  # dylib, and will help THAT guy find stdlib extensions (.bundles in ext/)
  SET(librubyBefore "libruby.dylib")
  SET(librubyAfter "libruby_bp.dylib")
  ADD_CUSTOM_COMMAND(
      TARGET ${SERVICE_NAME} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy \"${OUTPUT_DIR}/${librubyBefore}\" \"${OUTPUT_DIR}/${librubyAfter}\"   
      COMMAND ${CMAKE_COMMAND} -E remove \"${OUTPUT_DIR}/${librubyBefore}\"
      COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/libruby_fixup.sh | tr -d '\r' > ${CMAKE_CURRENT_SOURCE_DIR}/libruby_fixup_unix.sh
      COMMAND cat ${CMAKE_CURRENT_SOURCE_DIR}/ext_fixup.sh | tr -d '\r' > ${CMAKE_CURRENT_SOURCE_DIR}/ext_fixup_unix.sh
      COMMAND chmod 777 ${CMAKE_CURRENT_SOURCE_DIR}/libruby_fixup_unix.sh
      COMMAND chmod 777 ${CMAKE_CURRENT_SOURCE_DIR}/ext_fixup_unix.sh
      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/libruby_fixup_unix.sh ${loc}
      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/libruby_fixup_unix.sh ${loc}
      COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/ext_fixup_unix.sh \"${extDest}\"
      COMMAND ${CMAKE_COMMAND} -E remove \"${CMAKE_CURRENT_SOURCE_DIR}/libruby_fixup_unix.sh\"
      COMMAND ${CMAKE_COMMAND} -E remove \"${CMAKE_CURRENT_SOURCE_DIR}/ext_fixup_unix.sh\"
      COMMAND ${CMAKE_COMMAND} -E copy_if_different \"${loc}\" \"${OUTPUT_DIR}\"
  )
ENDIF ()

