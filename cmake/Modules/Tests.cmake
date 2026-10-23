#------------------------------------------------------------------------------
# Tests.cmake
#------------------------------------------------------------------------------
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301  USA
#------------------------------------------------------------------------------
# Copyright (C) 2026 "Zalewa" <zalewapl@gmail.com>
#------------------------------------------------------------------------------

get_filename_component(_unit_test_root "${CMAKE_CURRENT_LIST_DIR}/../../tests/unit" ABSOLUTE)

#[[
Add a Qt Test executable and register it with CTest.

    add_unit_test(<subject>
        [LIBRARIES <libraries...>]
        [SOURCES <files...>])

Use <subject> to select tst_<subject>.cpp in the calling directory.
Use LIBRARIES to specify production libraries or targets to link.
Use SOURCES to include additional sources or Qt resource collections;
resolve relative paths against the calling directory.

Creates unit_test_<path>_<subject> target. <path> is derived from the calling
directory relative to tests/unit/ alongside the cmake/ directory, replacing
path separators with underscores. Registers unit.<path>.<subject> with CTest,
replacing path separators with dots.

Links ${Qt_PACKAGE}::Test and enable AUTOMOC and AUTORCC. Labels the CTest
entry with unit and the first component of <path>.

Sets a 30-second timeout.
]]
function(add_unit_test subject)
	cmake_parse_arguments(args "" "" "LIBRARIES;SOURCES" ${ARGN})
	if(args_UNPARSED_ARGUMENTS)
		message(FATAL_ERROR "Unknown unit-test arguments: ${args_UNPARSED_ARGUMENTS}")
	endif()

	file(RELATIVE_PATH test_directory "${_unit_test_root}"
		"${CMAKE_CURRENT_SOURCE_DIR}")
	string(REPLACE "/" "_" target_namespace "${test_directory}")
	string(REPLACE "/" "." test_namespace "${test_directory}")
	string(REGEX MATCH "^[^/]+" component "${test_directory}")
	set(target "unit_test_${target_namespace}_${subject}")
	set(test_name "unit.${test_namespace}.${subject}")

	add_executable(${target} tst_${subject}.cpp ${args_SOURCES})
	set_target_properties(${target} PROPERTIES AUTOMOC ON AUTORCC ON)
	if(WIN32)
		# Keep test executables beside the project's runtime DLLs.
		set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
	endif()
	target_link_libraries(${target}
	PRIVATE
		${args_LIBRARIES}
		${Qt_PACKAGE}::Test
	)

	add_test(NAME ${test_name} COMMAND ${target})
	set_tests_properties(${test_name} PROPERTIES LABELS "unit;${component}" TIMEOUT 30)
endfunction()
