file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafnetwork/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafnetwork/*.h")

find_package(Qt4 COMPONENTS QtNetwork REQUIRED)

include(${QT_USE_FILE})
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../libafnetwork/src/")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../libafanasy/src")

add_library(afnetwork ${src} ${inc})
target_link_libraries(afnetwork afanasy  ${QT_LIBRARIES})
