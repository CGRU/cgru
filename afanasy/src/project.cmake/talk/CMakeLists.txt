file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../talk/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../talk/*.h")

find_package(Qt4 COMPONENTS QtGui REQUIRED)
include(${QT_USE_FILE})

set(moc_hdr
 "${CMAKE_CURRENT_SOURCE_DIR}/../../talk/dialog.h"
 "${CMAKE_CURRENT_SOURCE_DIR}/../../talk/editor.h"
 "${CMAKE_CURRENT_SOURCE_DIR}/../../talk/userslist.h"
 "${CMAKE_CURRENT_SOURCE_DIR}/../../talk/textview.h"
 "${CMAKE_CURRENT_SOURCE_DIR}/../../talk/tray.h"
)

qt4_wrap_cpp(moc_src ${moc_hdr})

add_executable(aftalk ${src} ${inc} ${moc_src})
if( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )
   set_target_properties(aftalk PROPERTIES COMPILE_FLAGS $ENV{AF_ADD_CFLAGS})
endif( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )

if( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )
   set_target_properties(aftalk PROPERTIES LINK_FLAGS $ENV{AF_ADD_LFLAGS})
endif( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )

if(WIN32)
   set_target_properties(aftalk PROPERTIES WIN32_EXECUTABLE 1)
   target_link_libraries(aftalk Ws2_32.lib)
endif(WIN32)

target_link_libraries(aftalk afqt)
