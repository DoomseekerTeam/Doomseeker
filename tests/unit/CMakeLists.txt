#[[
Add a Qt Test executable and register it with CTest.

    add_doomseeker_unit_test(<subject>
        [LIBRARIES <libraries...>]
        [SOURCES <files...>])

Use <subject> to select tst_<subject>.cpp in the calling directory.
Use LIBRARIES to specify production libraries or targets to link.
Use SOURCES to include additional sources or Qt resource collections;
resolve relative paths against the calling directory.

Creates doomseeker_test_<path>_<subject> target. <path> is derived from the
calling directory relative to tests/unit/, replacing path separators with
underscores. Registers unit.<path>.<subject> with CTest, replacing path
separators with dots.

Links ${Qt_PACKAGE}::Test and enable AUTOMOC and AUTORCC. Labels the CTest
entry with unit and the first component of <path>.

Sets a 30-second timeout.
]]
function(add_doomseeker_unit_test subject)
	cmake_parse_arguments(PARSE_ARGV 1 args "" "" "LIBRARIES;SOURCES")
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

	add_executable(${target} tst_${subject}.cpp ${args_SOURCES})
	set_target_properties(${target} PROPERTIES AUTOMOC ON AUTORCC ON)
	target_link_libraries(${target}
	PRIVATE
		${args_LIBRARIES}
		${Qt_PACKAGE}::Test
	)

	add_test(NAME ${test_name} COMMAND ${target})
	set_tests_properties(${test_name} PROPERTIES LABELS "unit;${component}" TIMEOUT 30)
endfunction()

add_subdirectory(core)
