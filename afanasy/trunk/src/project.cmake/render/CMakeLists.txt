file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../render/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../render/*.h")

find_package(Qt4 COMPONENTS QtCore QtXml REQUIRED)

include(${QT_USE_FILE})
#include_directories(${PYTHON_INCLUDE_PATH})
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../render/src/")
#include_directories("${afanasy_SOURCE_DIR}/../libafanasy/src")
#include_directories("${afanasy_SOURCE_DIR}/../libafqt/src")

set(moc_hdr
 "${CMAKE_CURRENT_SOURCE_DIR}/../../render/childprocess.h"
 "${CMAKE_CURRENT_SOURCE_DIR}/../../render/qobject.h"
 "${CMAKE_CURRENT_SOURCE_DIR}/../../render/taskprocess.h"
)

qt4_wrap_cpp(moc_src ${moc_hdr})

add_executable(afrender ${src} ${inc} ${moc_src})
target_link_libraries(afrender afqt)
