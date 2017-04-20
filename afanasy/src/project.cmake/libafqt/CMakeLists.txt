file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafqt/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafqt/*.h")

set(CMAKE_CXX_STANDARD 11)

if( "$ENV{AF_QT_VER}" STREQUAL "5" )
	find_package(Qt5Core REQUIRED)
	find_package(Qt5Network REQUIRED)
	find_package(Qt5Gui REQUIRED)
	message(STATUS "Qt version ${Qt5Core_VERSION}")
else()
	find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
	include(${QT_USE_FILE})
	message(STATUS "Qt version ${QT_INST_MAJOR_VERSION}.${QT_INST_MINOR_VERSION}.${QT_INST_PATCH_VERSION}")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(afqt ${src} ${inc})
target_link_libraries(afqt afanasy ${QT_LIBRARIES} ${Qt5Core_LIBRARIES} ${Qt5Network_LIBRARIES} ${Qt5Gui_LIBRARIES})

if( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )
   set_target_properties(afqt PROPERTIES COMPILE_FLAGS $ENV{AF_ADD_CFLAGS})
endif( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )

