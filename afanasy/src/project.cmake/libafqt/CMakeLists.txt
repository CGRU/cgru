file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafqt/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafqt/*.h")

if( "$ENV{AF_QT_VER}" STREQUAL "5" )
	message("Building with Qt5.")
	find_package(Qt5Core REQUIRED)
	find_package(Qt5Network REQUIRED)
	find_package(Qt5Gui REQUIRED)

	set(CMAKE_AUTOMOC ON)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)

	add_library(afqt ${src} ${inc} ${moc_src})
	target_link_libraries(afqt afanasy ${Qt5Core_LIBRARIES} ${Qt5Network_LIBRARIES} ${Qt5Gui_LIBRARIES})
else()
	find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
	include(${QT_USE_FILE})
	set(moc_hdr
	"${CMAKE_CURRENT_SOURCE_DIR}/../../libafqt/qthreadclient.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../libafqt/qthreadclientsend.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/../../libafqt/qthreadclientup.h"
	)
	qt4_wrap_cpp(moc_src ${moc_hdr})
	add_library(afqt ${src} ${inc} ${moc_src})
	target_link_libraries(afqt afanasy  ${QT_LIBRARIES} )
endif()

if( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )
   set_target_properties(afqt PROPERTIES COMPILE_FLAGS $ENV{AF_ADD_CFLAGS})
endif( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )

