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
# Copyright (C) 2011 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
#------------------------------------------------------------------------------

set(PLUGIN_LIST "")
macro(add_doomseeker_plugin PLUGIN_NAME)
	set(PLUGIN_LIST ${PLUGIN_LIST} ${PLUGIN_NAME} PARENT_SCOPE)
endmacro()

# Permit redistribution of our source code without plugins which may violate
# certain distribution polices (particularly in regards to use of trademarks).
macro(add_opt_subdirectory PLUGIN_NAME)
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PLUGIN_NAME}")
		add_subdirectory(${PLUGIN_NAME})
	endif()
endmacro()

add_opt_subdirectory(chocolate-doom)
add_opt_subdirectory(odamex)
add_opt_subdirectory(srb2)
add_opt_subdirectory(zandronum)
add_opt_subdirectory(zandronumq)
add_opt_subdirectory(turok2ex)

# For unmaintained source ports
option(BUILD_LEGACY_PLUGINS "Build plugins for discontinued source ports." OFF)
if(BUILD_LEGACY_PLUGINS)
	add_opt_subdirectory(vavoom)
endif()

option(BUILD_FAKE_PLUGINS "Build plugins that are used for development purposes only." OFF)
if(BUILD_FAKE_PLUGINS)
	add_opt_subdirectory(fakeplugin)
endif()

foreach(PLUGIN ${PLUGIN_LIST})
	string(TOUPPER ${PLUGIN} PLUGIN_UPPER)

	# Multiple layers of add_subdirectory make it difficult to pass variables
	# like the CPack ones up to the root where they're used.
	if(USE_CPACK)
		parent_scope_component(${PLUGIN})

		set(CPACK_DEBIAN_${PLUGIN_UPPER}_PACKAGE_DEPENDS "doomseeker" PARENT_SCOPE)
		set(CPACK_COMPONENT_${PLUGIN_UPPER}_DESCRIPTION "Doomseeker plugin providing support for ${PLUGIN}.\n Provides Doomseeker with an implementation for ${PLUGIN}\n protocols." PARENT_SCOPE)
	endif()
endforeach()
