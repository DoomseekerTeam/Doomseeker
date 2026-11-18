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

# Compile the real plugin implementation for reply tests without test doubles
# or loading a plugin module.
set(odamex_query_sources
	"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexcvar.cpp"
	"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexengineplugin.cpp"
	"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexgamehost.cpp"
	"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexgameinfo.cpp"
	"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexgamemode.cpp"
	"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexgamerunner.cpp"
	"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexmasterclient.cpp"
	"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexserver.cpp"
)
add_plugin_unit_test(odamex odamexserver
	SOURCES ${odamex_query_sources} _testdata/odamexserver/resources.qrc
)
add_plugin_unit_test(odamex odamexmasterclient SOURCES ${odamex_query_sources})
foreach(test odamexserver odamexmasterclient)
	target_compile_definitions(unit_test_plugins_odamex_${test} PRIVATE DOOMSEEKER_STATIC_PLUGINS)
endforeach()

add_plugin_unit_test(odamex odamexcvar
	SOURCES
		"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexcvar.cpp"
)

add_plugin_unit_test(odamex odamexgamemode
	SOURCES
		"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexcvar.cpp"
		"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexgamemode.cpp"
		"${Doomseeker_SOURCE_DIR}/src/plugins/odamex/odamexgamemode.h"
)
