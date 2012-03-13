file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")

find_package(Qt4 COMPONENTS QtCore QtXml REQUIRED)

include(${QT_USE_FILE})
include_directories(${PYTHON_INCLUDE_PATH})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/")
include_directories("${afanasy_SOURCE_DIR}/libafanasy/src")
include_directories("${afanasy_SOURCE_DIR}/libafapi/src")


add_library(pyaf SHARED ${src} ${inc})

target_link_libraries(pyaf afapi)
set_target_properties(pyaf PROPERTIES PREFIX "")
if(APPLE)
	set_target_properties(pyaf PROPERTIES SUFFIX ".so")
endif(APPLE)
if(WIN32)
	set_target_properties(pyaf PROPERTIES SUFFIX ".pyd")
endif(WIN32)
install(TARGETS pyaf RUNTIME DESTINATION bin LIBRARY DESTINATION bin)