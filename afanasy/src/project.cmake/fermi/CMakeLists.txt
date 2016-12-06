file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../fermi/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../fermi/*.h")
file(GLOB_RECURSE inc2 RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../fermi/*.hpp")

if( "$ENV{AF_QT_VER}" STREQUAL "5" )
	find_package(Qt5Widgets REQUIRED)
	find_package(Qt5Multimedia REQUIRED)
	find_package(Qt5Quick REQUIRED)
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
else()
	message(FATAL_ERROR "Qt5 only")
endif()

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


add_executable(afermi ${src} ${inc} ${inc2} ${watch_cpp_src} )

if( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )
   set_target_properties(afwatch PROPERTIES COMPILE_FLAGS $ENV{AF_ADD_CFLAGS})
endif( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )

if( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )
   set_target_properties(afwatch PROPERTIES LINK_FLAGS $ENV{AF_ADD_LFLAGS})
endif( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../")

target_link_libraries(afermi afqt ${Qt5Multimedia_LIBRARIES} 
                                  ${Qt5Widgets_LIBRARIES} 
                                  ${Qt5Quick_LIBRARIES} 
                                  $ENV{AF_EXTRA_LIBS} ${Boost_LIBRARIES})


add_custom_command(TARGET afermi POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}/../../fermi/qml" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/qml )
