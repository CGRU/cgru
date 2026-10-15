include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../libafanasy")

add_executable(test_platform_info_unit
	platform_info_unit_tests.cpp
	../../libafanasy/platform_info.cpp
)

add_executable(test_platform_info_integration
	platform_info_integration_tests.cpp
	../../libafanasy/platform_info.cpp
)

add_executable(test_blockdata_json
	blockdata_json_tests.cpp
)
if( NOT "$ENV{AF_ADD_CFLAGS}" STREQUAL "" )
	set_target_properties(test_blockdata_json PROPERTIES COMPILE_FLAGS "$ENV{AF_ADD_CFLAGS}")
endif()
if( NOT "$ENV{AF_ADD_LFLAGS}" STREQUAL "" )
	set_target_properties(test_blockdata_json PROPERTIES LINK_FLAGS "$ENV{AF_ADD_LFLAGS}")
endif()
if(WIN32)
	target_link_libraries(test_blockdata_json Ws2_32.lib Iphlpapi.lib)
endif(WIN32)
target_link_libraries(test_blockdata_json afanasy $ENV{AF_EXTRA_LIBS})

add_test(NAME test_platform_info_unit COMMAND test_platform_info_unit)
add_test(NAME test_platform_info_integration COMMAND test_platform_info_integration)
add_test(NAME test_blockdata_json COMMAND test_blockdata_json)
