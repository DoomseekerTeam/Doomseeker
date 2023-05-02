#------------------------------------------------------------------------------
# PluginHeader.cmake
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
# Copyright (C) 2010 - 2017 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
#------------------------------------------------------------------------------

# Generic Plugin CMakeList.txt
# Usage:
#  - Set PLUGIN_NAME to the name of your plugin set(PLUGIN_NAME skulltag)
#  - Include this file using include(../PluginHeader.cmake)
#  - add_library(${PLUGIN_NAME} MODULE ${STUFF})
#  - Include the footer file include(../PluginFooter.cmake)

if(MSVC)
	set(CMAKE_SHARED_MODULE_PREFIX "lib")
endif()

find_package(Qt5 COMPONENTS Widgets)

if(NOT Qt5_FOUND)
	message(FATAL_ERROR "Qt5 not found. Please make sure to set the Qt5_DIR variable")
else()
	find_package(Qt5 COMPONENTS Core Gui LinguistTools Multimedia Network Xml REQUIRED)
	set(QT_LIBRARIES Qt5::Widgets Qt5::Multimedia Qt5::Network Qt5::Xml Qt5::Gui Qt5::Core)
endif()

message(STATUS "Setting up plugin ${PLUGIN_NAME}. Source dir:\n"
	"    ${CMAKE_CURRENT_SOURCE_DIR}")
