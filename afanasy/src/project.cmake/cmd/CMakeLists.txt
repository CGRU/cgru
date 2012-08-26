file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../cmd/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../cmd/*.h")

if( PostgreSQL_FOUND )
	include_directories( ${PostgreSQL_INCLUDE_DIRS})
	link_directories( ${PostgreSQL_LIBRARY_DIRS})
else( PostgreSQL_FOUND )
	add_definitions( -DNO_POSTGRESQL )
endif( PostgreSQL_FOUND )

add_executable(afcmd ${src} ${inc})

if( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )
   set_target_properties(afcmd PROPERTIES LINK_FLAGS $ENV{AF_ADD_CFLAGS})
endif( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )

if( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )
   set_target_properties(afcmd PROPERTIES LINK_FLAGS $ENV{AF_ADD_LFLAGS})
endif( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )

if(WIN32)
   target_link_libraries(afcmd Ws2_32.lib Iphlpapi.lib)
endif(WIN32)

target_link_libraries(afcmd afsql)
