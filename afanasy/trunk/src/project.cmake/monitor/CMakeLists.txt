file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../monitor/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../monitor/*.h")

find_package(Qt4 COMPONENTS QtCore QtXml QtGui REQUIRED)

include(${QT_USE_FILE})
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../monitor/src/")
#include_directories("${afanasy_SOURCE_DIR}/../libafanasy/src")
#include_directories("${afanasy_SOURCE_DIR}/../libafqt/src")

set(moc_hdr
 "${CMAKE_CURRENT_SOURCE_DIR}/../../monitor/monitorwindow.h"
)

qt4_wrap_cpp(moc_src ${moc_hdr})

add_executable(afmonitor ${src} ${inc} ${moc_src})


if( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )
   set_target_properties(afmonitor PROPERTIES COMPILE_FLAGS $ENV{AF_ADD_CFLAGS})
endif( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )

if( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )
   set_target_properties(afmonitor PROPERTIES LINK_FLAGS $ENV{AF_ADD_LFLAGS})
endif( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )

target_link_libraries(afmonitor afqt)
