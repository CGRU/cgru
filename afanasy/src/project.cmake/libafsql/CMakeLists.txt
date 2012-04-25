file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafsql/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafsql/*.h")

include_directories(${PostgreSQL_INCLUDE_DIRS})

find_package(PostgreSQL)


if( PostgreSQL_FOUND )
	include_directories( ${PostgreSQL_INCLUDE_DIRS})
else( PostgreSQL_FOUND ) 
	add_definitions( -DNO_POSTGRESQL )
endif( PostgreSQL_FOUND )

add_library( afsql ${src} ${inc})

target_link_libraries( afsql afanasy ${PostgreSQL_LIBRARIES})
