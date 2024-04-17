cmake_minimum_required(VERSION 3.8)

project(afqt)
set(CMAKE_CXX_FLAGS "-fPIC -fpermissive")

set(AFQT_INCLUDE_QT_PATH    	"/home/suren_ve/dev/maya/devkit_2023//include/qt/"
								"/home/suren_ve/dev/maya/devkit_2023/include/qt/QtGui/"
								"/home/suren_ve/dev/maya/devkit_2023/include/qt/QtWidgets/"
								"/home/suren_ve/dev/maya/devkit_2023/include/qt/QtCore/"
								"/home/suren_ve/dev/maya/devkit_2023/include/qt/QtMultimedia/"
								"/Volumes/Apps/maya/maya2022/include/Python37/Python/"
								"")
set(AFQT_LIB_PATH           	"/Volumes/Apps/maya/maya2023/lib/")

include_directories(${AFQT_INCLUDE_QT_PATH})
link_directories(${AFQT_LIB_PATH})

file(GLOB SRCS 	*.h *.cpp)
set(REQUIRED_LIBS 					
					Qt5Core 
					Qt5Widgets 
					Qt5Gui
					Qt5Multimedia
					libpython3.9.so)


add_library(afqt SHARED ${SRCS})
target_link_libraries(afqt ${REQUIRED_LIBS} )
#target_link_libraries(afqt afanasy ${QT_LIBRARIES} ${Qt5Core_LIBRARIES} ${Qt5Network_LIBRARIES} ${Qt5Gui_LIBRARIES})
