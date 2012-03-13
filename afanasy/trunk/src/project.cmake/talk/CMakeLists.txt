file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../talk/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../talk/*.h")

find_package(Qt4 COMPONENTS QtCore QtXml QtGui REQUIRED)

include(${QT_USE_FILE})
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../talk/")
#include_directories("${afanasy_SOURCE_DIR}/../libafanasy/src")
#include_directories("${afanasy_SOURCE_DIR}/../libafqt/src")

set(moc_hdr
 "${CMAKE_CURRENT_SOURCE_DIR}/../../talk/dialog.h"
 "${CMAKE_CURRENT_SOURCE_DIR}/../../talk/editor.h"
 "${CMAKE_CURRENT_SOURCE_DIR}/../../talk/userslist.h"
 "${CMAKE_CURRENT_SOURCE_DIR}/../../talk/textview.h"
 "${CMAKE_CURRENT_SOURCE_DIR}/../../talk/tray.h"
)

qt4_wrap_cpp(moc_src ${moc_hdr})

add_executable(aftalk ${src} ${inc} ${moc_src})
if(WIN32)
   set_target_properties(aftalk PROPERTIES LINK_FLAGS "-Wl,-subsystem,windows")
endif(WIN32)
target_link_libraries(aftalk afqt)
