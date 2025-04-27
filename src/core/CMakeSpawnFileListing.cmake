#------------------------------------------------------------------------------
# CMakeSpawnFileListing.cmake
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
#-------------------------------------------------------------------------------
# Copyright (C) 2015 Zalewa <zalewapl@gmail.com>
#-------------------------------------------------------------------------------

# Collect all Doomseeker's source files via recursive globbing and write
# them out as set() commands to a generated CMake source file.
#
# The collected source files form the 'doomseeker_obj' object library
# target. The main files are excluded from this, because they need
# to be added to the 'doomseeker' executable target instead.
function(append_file_list file_name var_name file_list)
	file(APPEND ${file_name} "set(${var_name}\n")
	list(SORT file_list)
	foreach(file ${file_list})
		file(APPEND ${file_name} "\t${file}\n")
	endforeach(file)
	file(APPEND ${file_name} ")\n")
endfunction()

set(FILE_NAME "CMakeFileListing.cmake")
file(WRITE ${FILE_NAME} "")

file(GLOB_RECURSE UI_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.ui")
append_file_list(${FILE_NAME} "UI_FILES" "${UI_FILES}")

file(GLOB_RECURSE HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.h" "*.hpp")
list(REMOVE_ITEM HEADER_FILES main.h)
append_file_list(${FILE_NAME} "HEADER_FILES" "${HEADER_FILES}")

file(GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp")
list(REMOVE_ITEM SOURCE_FILES main.cpp)
append_file_list(${FILE_NAME} "SOURCE_FILES" "${SOURCE_FILES}")
