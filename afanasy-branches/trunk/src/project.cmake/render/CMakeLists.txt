file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../render/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../render/*.h")

add_executable(afrender ${src} ${inc})

if( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )
   set_target_properties(afrender PROPERTIES COMPILE_FLAGS $ENV{AF_ADD_CFLAGS})
endif( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )

if( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )
   set_target_properties(afrender PROPERTIES LINK_FLAGS $ENV{AF_ADD_LFLAGS})
endif( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )

if(WIN32)
   target_link_libraries(afrender Ws2_32.lib Iphlpapi.lib)
endif(WIN32)

if(APPLE)
   find_library(CORE_FOUNDATION CoreFoundation)
   find_library(IOKIT IOKit)
   set(EXTRA_LIBS ${CORE_FOUNDATION} ${IOKIT} )
endif (APPLE)

target_link_libraries(afrender afanasy ${EXTRA_LIBS} )
