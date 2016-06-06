# Copyright (c) 2016, Jean-Luc Roberts
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# The views and conclusions contained in the software and documentation are those
# of the authors and should not be interpreted as representing official policies,
# either expressed or implied, of the FreeBSD Project.

project(JavaLauncher)

cmake_minimum_required(VERSION 3.0.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(WIN32)
	set(JRE "windows")
elseif(APPLE)
	set(JRE "macosx")
elseif(UNIX)
	set(JRE "linux")
endif()

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set(JRE_STRING "${JRE}-x64")
else()
	set(JRE_STRING "${JRE}-i586")
endif()

configure_file(src/javalauncher_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/javalauncher_config.h @ONLY)

file(GLOB source_files "src/*.c" "src/*.cpp")
include_directories("include")
include_directories("curl/include")
include_directories("libarchive/libarchive")
include_directories(${CMAKE_CURRENT_BINARY_DIR})
file(GLOB header_files "src/*.h")

if(WIN32)
	include_directories("include/win32")
	SET(ZLIB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/zlib/include" CACHE INTERNAL "zlib: Include Directories" FORCE)
	SET(ZLIB_LIBRARIES "${CMAKE_SOURCE_DIR}/zlib/lib/zlibstatic.lib" CACHE INTERNAL "zlib: Library" FORCE)
endif()

add_executable(JavaLauncher ${source_files} ${header_files})
add_subdirectory("curl")
add_definitions(-DCURL_STATICLIB)
target_link_libraries(JavaLauncher libcurl)

add_subdirectory("libarchive")
add_definitions(-DLIBARCHIVE_STATIC)
target_link_libraries(JavaLauncher archive_static)
