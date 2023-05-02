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
# Copyright (C) 2016 "Zalewa" <zalewapl@gmail.com>
#------------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.12)

set(PLUGIN_NAME srb2)
set(PLUGIN_VERSION 12)
include(../PluginHeader.cmake)

set(SRB2_FILES
	srb2engineplugin.cpp
	srb2gameclientrunner.cpp
	srb2gamehost.cpp
	srb2gameinfo.cpp
	srb2masterclient.cpp
	srb2server.cpp
)

set(HEADER_FILES
	srb2engineplugin.h
	srb2gameclientrunner.h
	srb2gamehost.h
	srb2gameinfo.h
	srb2masterclient.h
	srb2server.h
)

set(TRANSLATIONS_FILES
	translations/srb2_es_ES.ts
	translations/srb2_pl_PL.ts
)
set(FILES_TO_TRANSLATE
	${HEADER_FILES}
	${SRB2_FILES}
)

include(../Translation.cmake)

add_library(${PLUGIN_NAME} MODULE ${SRB2_FILES} ${HEADER_FILES} ${QM_FILES})

include(../PluginFooter.cmake)
