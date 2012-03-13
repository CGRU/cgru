file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

find_package(Qt4 COMPONENTS QtCore QtXml QtNetwork QtSql REQUIRED)

include(${QT_USE_FILE})
include_directories(${PYTHON_INCLUDE_PATH})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/")
include_directories("${afanasy_SOURCE_DIR}/libafanasy/src")
include_directories("${afanasy_SOURCE_DIR}/libafapi/src")
include_directories("${afanasy_SOURCE_DIR}/libafnetwork/src")
include_directories("${afanasy_SOURCE_DIR}/libafsql/src")

add_executable(afcmd ${src} ${inc})
target_link_libraries(afcmd afapi afnetwork afsql)
install(TARGETS afcmd RUNTIME DESTINATION bin)
