cmake_minimum_required(VERSION 2.6.4)
project(regexp)

set(CMAKE_BUILD_TYPE "Release")

find_package(Qt4 COMPONENTS QtCore QtGui REQUIRED)

file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

include(${QT_USE_FILE})

set(moc_hdr
	"${CMAKE_CURRENT_SOURCE_DIR}/src/dialog.h"
)

qt4_wrap_cpp(moc_src ${moc_hdr})

add_executable(regexp ${src} ${inc} ${moc_src})
target_link_libraries(regexp ${QT_LIBRARIES})
