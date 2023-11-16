file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../watch/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../watch/*.h")

find_package(Qt6 COMPONENTS Widgets Multimedia)
if (NOT Qt6_FOUND)
	find_package(Qt5 REQUIRED COMPONENTS Widgets Multimedia)
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(afwatch ${src} ${inc})

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

target_link_libraries(afwatch afqt Qt::Multimedia Qt::Widgets $ENV{AF_EXTRA_LIBS} )
