file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafanasy/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libafanasy/*.h")

add_library(afanasy ${src} ${inc})
if(UNIX AND NOT APPLE)
   set_target_properties(afanasy PROPERTIES COMPILE_FLAGS "-fPIC")
endif(UNIX AND NOT APPLE)
target_link_libraries(afanasy ${PYTHON_LIBRARIES})

add_definitions(-DCGRU_REVISION=$ENV{CGRU_REVISION})
