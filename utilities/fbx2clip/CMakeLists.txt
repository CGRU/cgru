cmake_minimum_required(VERSION 2.6.4)
project(fbx2clip)

set(CMAKE_BUILD_TYPE "Release")

include_directories( "$ENV{FBXSDK_PATH}/include")
link_directories("$ENV{FBXSDK_PATH}/lib")

file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

add_executable(fbx2clip ${src} ${inc})

set_target_properties(fbx2clip PROPERTIES LINKER_LANGUAGE C)

target_link_libraries(fbx2clip libfbxsdk_gcc4_x64.a libpthread.so libdl.so)
