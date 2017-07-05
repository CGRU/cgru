if( "$ENV{AF_QT_VER}" STREQUAL "5" )
	find_package(Qt5Widgets REQUIRED)
	find_package(Qt5Multimedia REQUIRED)
	find_package(Qt5Quick REQUIRED)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
else()
	message(FATAL_ERROR "Qt5 only")
endif()

if (Qt5Widgets_VERSION VERSION_LESS 5.7.0)
	message("Afermer minimum supported Qt version is 5.7, skipping project.")
	return()
endif()

file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../fermer/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../fermer/*.h")
file(GLOB_RECURSE inc2 RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../fermer/*.hpp")

set (watch_cpp_src 
		"../../watch/wndlistenjob.cpp"
		"../..//watch/itemjobblock.cpp"
		"../..//watch/itemjobtask.cpp"
		"../..//watch/listtasks.cpp"
		"../..//watch/itemmonitor.cpp"
		"../..//watch/itemuser.cpp"
		"../..//watch/wndlistentask.cpp"
		"../..//watch/wndcustomizegui.cpp"
		"../..//watch/ctrljobs.cpp"
		"../..//watch/ctrlsortfilter.cpp"
		"../..//watch/itemnode.cpp"
		"../..//watch/item.cpp"
		"../..//watch/itemjob.cpp"
		"../..//watch/listnodes.cpp"
		"../..//watch/listitems.cpp"
		"../..//watch/listjobs.cpp"
		"../..//watch/listmonitors.cpp"
		"../..//watch/dialog.cpp"
		"../..//watch/blockinfo.cpp"
		"../..//watch/buttonout.cpp"
		"../..//watch/wnd.cpp"
		"../..//watch/wndlist.cpp"
		"../..//watch/listrenders.cpp"
		"../..//watch/buttonpanel.cpp"
		"../..//watch/actionid.cpp"
		"../..//watch/viewitems.cpp"
		"../..//watch/wndcustomdata.cpp"
		"../..//watch/modelitems.cpp"
		"../..//watch/plotter.cpp"
		"../..//watch/buttonmonitor.cpp"
		"../..//watch/wndtext.cpp"
		"../..//watch/modelnodes.cpp"
		"../..//watch/popup.cpp"
		"../..//watch/itemrender.cpp"
		"../..//watch/infoline.cpp"
		"../..//watch/wndtask.cpp"
		"../..//watch/monitorhost.cpp"
		"../..//watch/watch.cpp"
		"../..//watch/listusers.cpp"
		"../..//watch/filewidget.cpp"
		"../..//watch/wndnotifications.cpp"
		"../..//watch/fontwidget.cpp"
		"../..//watch/checkboxwidget.cpp"
		"../..//watch/labelversion.cpp"
		"../..//watch/colorwidget.cpp"
		"../..//watch/numberwidget.cpp"
		"../..//watch/ctrlrenders.cpp"
		"../..//watch/receiver.cpp"
		"../..//watch/ctrlsortfiltermenu.cpp"
		"../..//watch/offlinescreen.cpp"
		 )

if(WIN32)
	# build qt statically http://amin-ahmadi.com/2016/09/22/how-to-build-qt-5-7-statically-using-msvc14-microsoft-visual-studio-2015/
	# https://bugreports.qt.io/browse/QTBUG-55279?page=com.atlassian.jira.plugin.system.issuetabpanels%3Aall-tabpanel
	# KNOWN ISSUE: 
	#           cgru\afanasy\src\project.cmake\fermer\moc_state.cpp(66): error C2589: 'constant': illegal token on right side of '::'
	# please open the moc_state.cpp file and add line 
	#			#undef ERROR
	# after it compiled properly
	link_directories( ${_qt5Core_install_prefix}/plugins/imageformats
					${_qt5Core_install_prefix}/plugins/audio
					${_qt5Core_install_prefix}/plugins/platforms
					${_qt5Core_install_prefix}/lib
					${_qt5Core_install_prefix}/qml/QtGraphicalEffects/private
					${_qt5Core_install_prefix}/qml/QtQuick/Controls
					${_qt5Core_install_prefix}/qml/QtQuick/Layouts
					${_qt5Core_install_prefix}/qml/QtQuick/Controls/Styles/Flat
					${_qt5Core_install_prefix}/qml/QtQuick/Dialogs
					${_qt5Core_install_prefix}/qml/QtQuick/Window.2
					${_qt5Core_install_prefix}/qml/QtQuick/PrivateWidgets
					${_qt5Core_install_prefix}/qml/QtQuick/Extras
					${_qt5Core_install_prefix}/qml/QtQuick/Controls.2
					${_qt5Core_install_prefix}/qml/QtQuick/Controls.2/Material
					${_qt5Core_install_prefix}/qml/QtQuick/Controls.2/Universal
					${_qt5Core_install_prefix}/qml/QtQuick.2
					${_qt5Core_install_prefix}/qml/QtQuick/Templates.2
					${_qt5Core_install_prefix}/qml/QtGraphicalEffects
					${_qt5Core_install_prefix}/qml/Qt/labs/settings
				)
endif(WIN32)

add_executable(afermer ${watch_cpp_src} ${src} ${inc} ${inc2} )

if( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )
   set_target_properties(afermer PROPERTIES COMPILE_FLAGS $ENV{AF_ADD_CFLAGS})
endif( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )

if( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )
   set_target_properties(afermer PROPERTIES LINK_FLAGS $ENV{AF_ADD_LFLAGS})
endif( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../fermer")


if(WIN32)
	target_link_libraries(afermer Ws2_32.lib)
endif(WIN32)

target_link_libraries(afermer afqt ${Qt5Multimedia_LIBRARIES} 
                                  ${Qt5Widgets_LIBRARIES} 
                                  ${Qt5Quick_LIBRARIES} 
                                  $ENV{AF_EXTRA_LIBS} ${Boost_LIBRARIES})


add_custom_command(TARGET afermer POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/../../fermer/qml" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/qml )
