file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafsql/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafsql/*.h")

find_package(Qt4 COMPONENTS QtCore QtSql REQUIRED)

include(${QT_USE_FILE})
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../libafsql/src/")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../libafanasy/src")

add_library(afsql ${src} ${inc})
target_link_libraries(afsql afanasy ${QT_LIBRARIES})