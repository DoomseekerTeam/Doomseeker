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
# Copyright (C) 2013 "Zalewa" <zalewapl@gmail.com>
#------------------------------------------------------------------------------

set(PLUGIN_NAME fakeplugin)
set(PLUGIN_VERSION 1)
set(DONT_INSTALL ON)
include(../PluginHeader.cmake)

set(FAKEPLUGIN_FILES
	responder/masterresponder.cpp
	responder/respondercfg.cpp
	responder/serverresponder.cpp
	pluginengineplugin.cpp
	plugingamehost.cpp
	pluginmasterclient.cpp
	pluginserver.cpp
)

set(HEADER_FILES
	responder/awaitingclient.h
	responder/masterresponder.h
	responder/respondercfg.h
	responder/serverresponder.h
	pluginengineplugin.h
	plugingamehost.h
	pluginserver.h
	pluginmasterclient.h
)

add_library(${PLUGIN_NAME} MODULE ${FAKEPLUGIN_FILES} ${HEADER_FILES})

include(../PluginFooter.cmake)
