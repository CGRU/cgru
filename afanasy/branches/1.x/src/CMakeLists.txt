cmake_minimum_required(VERSION 2.6.4)
project(afanasy)

set(CMAKE_BUILD_TYPE "Release")

find_package(PythonLibs REQUIRED)

if(UNIX)
	if(APPLE)
		set(CMAKE_OSX_ARCHITECTURES "i386;x86_64;")
		#Changing the order of configurations to be sure release is the first one
		set(CMAKE_CONFIGURATION_TYPES "Release;Debug;MinSizeRel;RelWithDebInfo")
		add_definitions(-DMACOSX)
	else(APPLE)
		add_definitions(-DLINUX)
	endif(APPLE)
endif(UNIX)
if(WIN32)
	add_definitions(-DWINNT)
endif(WIN32)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")

add_subdirectory(libafanasy)
add_subdirectory(libafapi)
#These projects are not currently supported for Windows platform
if(NOT WIN32)
	add_subdirectory(libafnetwork)
	add_subdirectory(libafsql)
	add_subdirectory(server)
    add_subdirectory(cmd)
endif(NOT WIN32)
add_subdirectory(libafqt)
add_subdirectory(render)
add_subdirectory(talk)
add_subdirectory(watch)
add_subdirectory(monitor)
add_subdirectory(libpyaf)
