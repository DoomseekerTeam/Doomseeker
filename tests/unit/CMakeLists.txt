function(add_doomseeker_unit_test subject)
	cmake_parse_arguments(PARSE_ARGV 1 args "" "" "LIBRARIES")
	if(args_UNPARSED_ARGUMENTS)
		message(FATAL_ERROR "Unknown unit-test arguments: ${args_UNPARSED_ARGUMENTS}")
	endif()

	file(RELATIVE_PATH test_directory "${Doomseeker_SOURCE_DIR}/tests/unit"
		"${CMAKE_CURRENT_SOURCE_DIR}")
	string(REPLACE "/" "_" target_namespace "${test_directory}")
	string(REPLACE "/" "." test_namespace "${test_directory}")
	string(REGEX MATCH "^[^/]+" component "${test_directory}")
	set(target "doomseeker_test_${target_namespace}_${subject}")
	set(test_name "unit.${test_namespace}.${subject}")

	add_executable(${target} tst_${subject}.cpp)
	set_target_properties(${target} PROPERTIES AUTOMOC ON)
	target_link_libraries(${target}
	PRIVATE
		${args_LIBRARIES}
		${Qt_PACKAGE}::Test
	)

	add_test(NAME ${test_name} COMMAND ${target})
	set_tests_properties(${test_name} PROPERTIES LABELS "unit;${component}" TIMEOUT 30)
endfunction()

add_subdirectory(core)
