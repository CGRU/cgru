file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../monitor/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../monitor/*.h")

find_package(Qt4 COMPONENTS QtGui REQUIRED)
include(${QT_USE_FILE})

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

if(WIN32)
   target_link_libraries(afmonitor Ws2_32.lib)
endif(WIN32)

target_link_libraries(afmonitor afqt)
