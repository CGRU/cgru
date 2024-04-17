cmake_minimum_required(VERSION 3.8 FATAL_ERROR)

project(afwatch)
set(CMAKE_CXX_FLAGS "-fPIC -fpermissive -lm ")

# find qt packages
find_package(Qt5Core REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Multimedia REQUIRED)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 20)

add_compile_definitions(LINUX)

#find package for python 3.6.8
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

if (Python3_VERSION_MAJOR STREQUAL "3" AND Python3_VERSION_MINOR STREQUAL "6")
  message(STATUS "Found Python 3.6")
  set(Python3_INCLUDE_DIRS "${Python3_INCLUDE_DIRS}"
      CACHE PATH "Path to Python 3.6.8 headers")
  set(Python3_LIBRARIES "${Python3_LIBRARIES}"
      CACHE PATH "Path to Python 3.6.8 libraries")
else()
  message(FATAL_ERROR "Python 3.6.8 was not found")
endif()

#set(AFWATCH_INCLUDE_PATH       "/home/suren_ve/dev/maya/devkit_2023/include/")

set(AFWATCH_INCLUDE_QT_PATH    	
								"/home/suren_ve/dev/afanasy/afanasy/src/libafqt/"
								"/home/suren_ve/dev/afanasy/afanasy/src/include/"
								"/home/suren_ve/dev/afanasy/afanasy/src/libafanasy/"
								"/home/suren_ve/dev/afanasy/afanasy/src/watch/"
								"/home/suren_ve/dev/General/myRM/vendor/boost_1_80_0/"
								#"/home/suren_ve/dev/maya/devkit_2023//include/qt/"
								#"/home/suren_ve/dev/maya/devkit_2023/include/qt/QtGui/"
								#"/home/suren_ve/dev/maya/devkit_2023/include/qt/QtWidgets/"
								#"/home/suren_ve/dev/maya/devkit_2023/include/qt/QtCore/"
								#"/home/suren_ve/dev/maya/devkit_2023/include/qt/QtMultimedia/"	
								#"/Volumes/Apps/maya/maya2022/include/Python37/Python/"
								"")

#set(AFWATCH_LIB_PATH           	"/Volumes/Apps/maya/maya2023/lib/")

include_directories(${Qt5Declarative_INCLUDE_DIRS}
					${Qt5Widgets_INCLUDE_DIRS}
					${Qt5Core_INCLUDE_DIRS} 
					${Qt5Network_INCLUDE_DIRS} 
					${Qt5Gui_INCLUDE_DIRS}
					${Qt5Multimedia_INCLUDE_DIRS}

					${Python3_INCLUDE_DIRS} 
					${AFWATCH_INCLUDE_QT_PATH}	
									)
#link_directories(${AFWATCH_LIB_PATH})




file(GLOB SRCS 	*.h *.cpp
	"/home/suren_ve/dev/afanasy/afanasy/src/libafqt/*.cpp"
	"/home/suren_ve/dev/afanasy/afanasy/src/libafqt/*.h"
	"/home/suren_ve/dev/afanasy/afanasy/src/libafanasy/*.cpp"
	"/home/suren_ve/dev/afanasy/afanasy/src/libafanasy/msgclasses/*.cpp"
	"/home/suren_ve/dev/afanasy/afanasy/src/libafanasy/common/*.cpp"
	"/home/suren_ve/dev/afanasy/afanasy/src/watch/cinemotion/*.cpp"
	"")

set(REQUIRED_LIBS					
					#Qt5Core 
					#Qt5Widgets 
					#Qt5Gui
					#Qt5Multimedia
					#Qt5Network
					pthread
					#python3.9.so
					)

add_executable(afwatch ${SRCS})
target_link_libraries( afwatch 
						${REQUIRED_LIBS}
						${Qt5Declarative_LIBRARIES}
						${Qt5Widgets_LIBRARIES}
						${Qt5Core_LIBRARIES} 
						${Qt5Network_LIBRARIES} 
						${Qt5Gui_LIBRARIES}
						${Qt5Multimedia_LIBRARIES}
 						${Python3_LIBRARIES} )

#file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../watch/*.cpp")
#file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../watch/*.h")

#set (AF_QT_VER "5")


#set(CMAKE_AUTOMOC ON)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)


#if( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )
#   set_target_properties(afwatch PROPERTIES COMPILE_FLAGS $ENV{AF_ADD_CFLAGS})
#endif( NOT $ENV{AF_ADD_CFLAGS} STREQUAL "" )

#if( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )
#   set_target_properties(afwatch PROPERTIES LINK_FLAGS $ENV{AF_ADD_LFLAGS})
#endif( NOT $ENV{AF_ADD_LFLAGS} STREQUAL "" )


