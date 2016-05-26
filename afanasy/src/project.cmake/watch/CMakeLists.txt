file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../watch/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../watch/*.h")

if( "$ENV{AF_QT_VER}" STREQUAL "5" )
	find_package(Qt5Widgets REQUIRED)
	find_package(Qt5Multimedia REQUIRED)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
else()
	find_package(Qt4 COMPONENTS QtGui REQUIRED)
	include(${QT_USE_FILE})
	set(moc_hdr
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/actionid.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/buttonmonitor.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/buttonpanel.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/checkboxwidget.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/colorwidget.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/ctrljobs.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/ctrlrenders.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/ctrlsortfilter.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/dialog.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/filewidget.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/fontwidget.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/labelversion.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/listitems.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/listjobs.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/listmonitors.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/listnodes.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/listrenders.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/listtasks.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/listusers.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/modelnodes.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/numberwidget.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/wndcustomdata.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/wndcustomizegui.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/wndtask.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/item.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/itemjobtask.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../watch/itemjobblock.h"
	)
	qt4_wrap_cpp(moc_src ${moc_hdr})
endif()

add_executable(afwatch ${src} ${inc} ${moc_src})

if( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )
   set_target_properties(afwatch PROPERTIES COMPILE_FLAGS $ENV{AF_ADD_CFLAGS})
endif( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )

if( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )
   set_target_properties(afwatch PROPERTIES LINK_FLAGS $ENV{AF_ADD_LFLAGS})
endif( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )

if(WIN32)
   set_target_properties(afwatch PROPERTIES WIN32_EXECUTABLE 1)
   target_link_libraries(afwatch Ws2_32.lib)
endif(WIN32)

if(APPLE)
   find_library(SYSTEMCONFIGURATION_FRAMEWORK SystemConfiguration)
   find_library(SECURITY_FRAMEWORK Security)
   target_link_libraries(afwatch ${SYSTEMCONFIGURATION_FRAMEWORK}
	   ${SECURITY_FRAMEWORK})
endif(APPLE)

#target_link_libraries(afwatch afqt $ENV{AF_EXTRA_LIBS} )
target_link_libraries(afwatch afqt ${Qt5Multimedia_LIBRARIES} ${Qt5Widgets_LIBRARIES} $ENV{AF_EXTRA_LIBS} )
