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

if( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )
   set_target_properties(afrender PROPERTIES COMPILE_FLAGS $ENV{AF_ADD_CFLAGS})
endif( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )

if( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )
   set_target_properties(afrender PROPERTIES LINK_FLAGS $ENV{AF_ADD_LFLAGS})
endif( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )

target_link_libraries(afrender afqt)
