include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../libafanasy")

add_executable(test_platform_info_unit
	platform_info_unit_tests.cpp
	../../libafanasy/platform_info.cpp
)

add_executable(test_platform_info_integration
	platform_info_integration_tests.cpp
	../../libafanasy/platform_info.cpp
)

add_test(NAME test_platform_info_unit COMMAND test_platform_info_unit)
add_test(NAME test_platform_info_integration COMMAND test_platform_info_integration)
