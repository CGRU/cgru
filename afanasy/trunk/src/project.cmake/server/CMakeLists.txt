file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../server/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../server/*.h")

find_package(Qt4 COMPONENTS QtCore QtXml QtNetwork QtSql REQUIRED)

include(${QT_USE_FILE})
#include_directories(${PYTHON_INCLUDE_PATH})
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../server/src/")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../libafanasy/src")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../libafnetwork/src")
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../libafsql/src")

add_executable(afserver ${src} ${inc})
target_link_libraries(afserver afnetwork afsql)
