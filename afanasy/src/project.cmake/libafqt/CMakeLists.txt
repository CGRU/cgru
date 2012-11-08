file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafqt/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafqt/*.h")

find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
include(${QT_USE_FILE})

set(moc_hdr
 "${CMAKE_CURRENT_SOURCE_DIR}/../../libafqt/qserver.h"
 "${CMAKE_CURRENT_SOURCE_DIR}/../../libafqt/qserverthread.h"
 "${CMAKE_CURRENT_SOURCE_DIR}/../../libafqt/qthreadclient.h"
 "${CMAKE_CURRENT_SOURCE_DIR}/../../libafqt/qthreadclientsend.h"
 "${CMAKE_CURRENT_SOURCE_DIR}/../../libafqt/qthreadclientup.h"
)

qt4_wrap_cpp(moc_src ${moc_hdr})

add_library(afqt ${src} ${inc} ${moc_src})
target_link_libraries(afqt afanasy  ${QT_LIBRARIES} )
