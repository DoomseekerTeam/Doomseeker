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

add_unit_test(datetime LIBRARIES doomseeker_core)
add_unit_test(gamedemo LIBRARIES doomseeker_core)
add_unit_test(modreader
	LIBRARIES doomseeker_core
	SOURCES _testdata/modreader/resources.qrc
)
add_unit_test(strings LIBRARIES doomseeker_core)
add_unit_test(templatedpathresolver LIBRARIES doomseeker_core)
add_unit_test(utf8splitter LIBRARIES doomseeker_core)

add_subdirectory(ini)
add_subdirectory(irc)
add_subdirectory(pathfinder)
add_subdirectory(serverapi)
