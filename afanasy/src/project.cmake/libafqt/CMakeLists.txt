file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafqt/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafqt/*.h")

set(CMAKE_CXX_STANDARD 11)

find_package(Qt6 QUIET COMPONENTS Core Network Gui)
if (Qt6_FOUND)
	message(STATUS "Qt version ${Qt6Core_VERSION}")
else()
	find_package(Qt5 REQUIRED COMPONENTS Core Network Gui)
	message(STATUS "Qt version ${Qt5Core_VERSION}")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(afqt ${src} ${inc})

if(${CMAKE_VERSION} VERSION_LESS "3.20.0")
	if (Qt6_FOUND)
		target_link_libraries(afqt afanasy ${QT_LIBRARIES} ${Qt6Core_LIBRARIES} ${Qt6Network_LIBRARIES} ${Qt6Gui_LIBRARIES})
	else()
		target_link_libraries(afqt afanasy ${QT_LIBRARIES} ${Qt5Core_LIBRARIES} ${Qt5Network_LIBRARIES} ${Qt5Gui_LIBRARIES})
	endif()
else()
	target_link_libraries(afqt afanasy Qt::Core Qt::Network Qt::Gui)
endif()

if( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )
   set_target_properties(afqt PROPERTIES COMPILE_FLAGS $ENV{AF_ADD_CFLAGS})
endif( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )

