file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

find_package(Qt4 COMPONENTS QtCore QtXml REQUIRED)

include(${QT_USE_FILE})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/")
include_directories("${afanasy_SOURCE_DIR}/libafanasy/src")

add_library(afapi ${src} ${inc})
if(UNIX AND NOT APPLE)
   set_target_properties(afapi PROPERTIES COMPILE_FLAGS "-fPIC")
endif(UNIX AND NOT APPLE)
target_link_libraries(afapi afanasy)