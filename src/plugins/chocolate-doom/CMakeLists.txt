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
# Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
#------------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.12)

set(PLUGIN_NAME chocolatedoom)
set(PLUGIN_VERSION 28)
include(../PluginHeader.cmake)

set(CHOCOLATEDOOM_FILES
	chocolatedoombinaries.cpp
	chocolatedoomengineplugin.cpp
	chocolatedoomgameexefactory.cpp
	chocolatedoomgamehost.cpp
	chocolatedoomgameinfo.cpp
	chocolatedoomgamerunner.cpp
	chocolatedoommasterclient.cpp
	chocolatedoomserver.cpp
)

set(HEADER_FILES
	chocolatedoombinaries.h
	chocolatedoomengineplugin.h
	chocolatedoomgamehost.h
	chocolatedoomgameexefactory.h
	chocolatedoomgamerunner.h
	chocolatedoommasterclient.h
	chocolatedoomserver.h
)

set(TRANSLATIONS_FILES
	translations/chocolatedoom_es_ES.ts
	translations/chocolatedoom_pl_PL.ts
)
set(FILES_TO_TRANSLATE
	${HEADER_FILES}
	${CHOCOLATEDOOM_FILES}
)

include(../Translation.cmake)

add_library(${PLUGIN_NAME} MODULE ${CHOCOLATEDOOM_FILES} ${HEADER_FILES} ${QM_FILES})

include(../PluginFooter.cmake)
