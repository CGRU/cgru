file(GLOB_RECURSE src RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libpyaf/*.cpp")
file(GLOB_RECURSE inc RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/../../libpyaf/*.h")

add_library(pyaf SHARED ${src} ${inc})

target_link_libraries(pyaf afapi)
set_target_properties(pyaf PROPERTIES PREFIX "")
if(APPLE)
   set_target_properties(pyaf PROPERTIES SUFFIX ".so")
endif(APPLE)
if(WIN32)
   target_link_libraries(pyaf Ws2_32.lib)
   set_target_properties(pyaf PROPERTIES SUFFIX ".pyd")
endif(WIN32)
install(TARGETS pyaf RUNTIME DESTINATION bin LIBRARY DESTINATION bin)

add_definitions(-DCGRU_REVISION=$ENV{CGRU_REVISION})
