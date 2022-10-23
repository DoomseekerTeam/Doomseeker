#------------------------------------------------------------------------------
# InstallHeaders.cmake
#------------------------------------------------------------------------------
#
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
#
#------------------------------------------------------------------------------
# Copyright (C) 2017 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
#------------------------------------------------------------------------------

# Custom install script that pulls header files which are considered public.
# These will contain the string WADSEEKER_API somewhere in them.

file(GLOB_RECURSE HEADER_FILES RELATIVE ${CMAKE_CURRENT_LIST_DIR} "${CMAKE_CURRENT_LIST_DIR}/*.h")
foreach(FILENAME ${HEADER_FILES})
	file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/${FILENAME}" FILE_CONTENTS LIMIT_COUNT 1 REGEX ".*WADSEEKER_API.*")
	if(FILE_CONTENTS)
		get_filename_component(FILE_PATH "${CMAKE_CURRENT_LIST_DIR}/${FILENAME}" DIRECTORY)
		file(RELATIVE_PATH FILE_RELPATH ${CMAKE_CURRENT_LIST_DIR} ${FILE_PATH})
		if(NOT ${FILE_RELPATH} EQUAL "")
			set(FILE_RELPATH "/${FILE_RELPATH}")
		endif()
		file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/${FILENAME}" DESTINATION "${CMAKE_INSTALL_PREFIX}/include/wadseeker${FILE_RELPATH}")
	endif()
endforeach()
