file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafanasy/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafanasy/*.h")

find_package(Qt4 COMPONENTS QtCore QtXml QtNetwork REQUIRED)
include(${QT_USE_FILE})

add_library(afanasy ${src} ${inc})
if(UNIX AND NOT APPLE)
   set_target_properties(afanasy PROPERTIES COMPILE_FLAGS "-fPIC")
endif(UNIX AND NOT APPLE)
target_link_libraries(afanasy ${QT_LIBRARIES} ${PYTHON_LIBRARIES})

add_definitions(-DCGRU_REVISION=$ENV{CGRU_REVISION})
