#------------------------------------------------------------------------------
# CMakeLists.txt
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

#[[
Add a plugin unit-test executable linked to Doomseeker core and the plugin's
shared build configuration.

    add_plugin_unit_test(<plugin-target> <subject>
        [LIBRARIES <libraries...>]
        [SOURCES <files...>])

Pass LIBRARIES and SOURCES through to add_unit_test(). Use SOURCES to compile
selected plugin implementation files without linking the plugin module or
its entry point.
]]
function(add_plugin_unit_test plugin subject)
	cmake_parse_arguments(PARSE_ARGV 2 args "" "" "LIBRARIES;SOURCES")
	if(args_UNPARSED_ARGUMENTS)
		message(FATAL_ERROR "Unknown plugin unit-test arguments: ${args_UNPARSED_ARGUMENTS}")
	endif()

	add_unit_test(${subject}
		LIBRARIES doomseeker_core ${plugin}_build_config ${args_LIBRARIES}
		SOURCES ${args_SOURCES}
	)
endfunction()

if(TARGET chocolatedoom)
	add_subdirectory(chocolate-doom)
endif()
if(TARGET odamex)
	add_subdirectory(odamex)
endif()
