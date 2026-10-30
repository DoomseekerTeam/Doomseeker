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

# Compile the real plugin implementation to satisfy OdamexServer's dependencies
# without test doubles or loading a plugin module.
add_plugin_unit_test(odamex odamexserver
	SOURCES
		"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexengineplugin.cpp"
		"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexgamehost.cpp"
		"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexgameinfo.cpp"
		"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexgamemode.cpp"
		"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexgamerunner.cpp"
		"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexmasterclient.cpp"
		"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexserver.cpp"
		_testdata/odamexserver/resources.qrc
)
target_compile_definitions(unit_test_plugins_odamex_odamexserver
	PRIVATE DOOMSEEKER_STATIC_PLUGINS)

add_plugin_unit_test(odamex odamexgamemode
	SOURCES
		"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexgamemode.cpp"
		"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexgamemode.h"
)
