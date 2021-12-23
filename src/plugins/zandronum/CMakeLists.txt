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
# Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
#------------------------------------------------------------------------------

set(PLUGIN_NAME zandronum)
set(PLUGIN_VERSION 45)
include(../PluginHeader.cmake)

set(ZANDRONUM_FILES
	createserverdialogpages/flagsid.cpp
	createserverdialogpages/flagspagevaluecontroller2.cpp
	createserverdialogpages/flagspagevaluecontroller3.cpp
	createserverdialogpages/flagspage.cpp
	createserverdialogpages/votingsetupwidget.cpp
	huffman/bitreader.cpp
	huffman/bitwriter.cpp
	huffman/huffcodec.cpp
	huffman/huffman.cpp
	enginezandronumconfigbox.cpp
	huffmanqt.cpp
	huffmanudpsocket.cpp
	zandronumbinaries.cpp
	zandronumbroadcast.cpp
	zandronum2dmflags.cpp
	zandronum3dmflags.cpp
	zandronumengineplugin.cpp
	zandronumgameexefactory.cpp
	zandronumgamehost.cpp
	zandronumgameinfo.cpp
	zandronumgamerunner.cpp
	zandronumgamesettings.cpp
	zandronummasterclient.cpp
	zandronumrconprotocol.cpp
	zandronumserver.cpp
	zandronumserverdmflagsparser.cpp
)

set(HEADER_FILES
	createserverdialogpages/flagspage.h
	createserverdialogpages/votingsetupwidget.h
	createserverdialogpages/flagsid.h
	createserverdialogpages/flagspagevaluecontroller2.h
	createserverdialogpages/flagspagevaluecontroller3.h
	huffman/bitreader.h
	huffman/bitwriter.h
	huffman/codec.h
	huffman/huffcodec.h
	huffman/huffman.h
	enginezandronumconfigbox.h
	huffmanqt.h
	huffmanudpsocket.h
	zandronumbinaries.h
	zandronumbroadcast.h
	zandronum2dmflags.h
	zandronum3dmflags.h
	zandronumengineplugin.h
	zandronumgameexefactory.h
	zandronumgamehost.h
	zandronumgameinfo.h
	zandronumgamerunner.h
	zandronumgamesettings.h
	zandronummasterclient.h
	zandronumrconprotocol.h
	zandronumserver.h
	zandronumserverdmflagsparser.h
)

set(TRANSLATIONS_FILES
	translations/zandronum_ca_ES.ts
	translations/zandronum_es_ES.ts
	translations/zandronum_pl_PL.ts
)
set(FILES_TO_TRANSLATE
	${HEADER_FILES}
	${ZANDRONUM_UI_FILES}
	${ZANDRONUM_FILES}
)

include(../Translation.cmake)

add_library(${PLUGIN_NAME}
	MODULE
	${ZANDRONUM_FILES}
	${HEADER_FILES}
	${QM_FILES}
)

if (NOT UNIX OR APPLE)
	# Linking Wadseeker is not needed on Linux and only causes
	# the resulting .so to get an RPATH. Windows still needs
	# to link it, though, or otherwise the compilation will fail.
	target_link_libraries(${PLUGIN_NAME} LINK_PRIVATE Wadseeker::wadseeker)
endif()

include(../PluginFooter.cmake)
