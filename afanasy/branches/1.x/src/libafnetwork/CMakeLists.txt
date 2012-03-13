file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

find_package(Qt4 COMPONENTS QtCore QtNetwork REQUIRED)

include(${QT_USE_FILE})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/")
include_directories("${afanasy_SOURCE_DIR}/libafanasy/src")

add_library(afnetwork ${src} ${inc})
target_link_libraries(afnetwork afanasy  ${QT_LIBRARIES})