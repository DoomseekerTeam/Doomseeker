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
# Copyright (C) 2020 "geNia" <geniann93@gmail.com>
#------------------------------------------------------------------------------

set(PLUGIN_NAME zandronumq)
set(PLUGIN_VERSION 20)
include(../PluginHeader.cmake)

set(ZANDRONUMQ_FILES
	createserverdialogpages/flagsid.cpp
	createserverdialogpages/flagspagevaluecontroller1.cpp
	createserverdialogpages/flagspage.cpp
	createserverdialogpages/votingsetupwidget.cpp
	huffman/bitreader.cpp
	huffman/bitwriter.cpp
	huffman/huffcodec.cpp
	huffman/huffman.cpp
	enginezandronumqconfigbox.cpp
	huffmanqt.cpp
	huffmanudpsocket.cpp
	zandronumqbroadcast.cpp
	zandronumq1dmflags.cpp
	zandronumqengineplugin.cpp
	zandronumqgameexefactory.cpp
	zandronumqgamehost.cpp
	zandronumqgameinfo.cpp
	zandronumqgamerunner.cpp
	zandronumqgamesettings.cpp
	zandronumqmasterclient.cpp
	zandronumqrconprotocol.cpp
	zandronumqserver.cpp
	zandronumqserverdmflagsparser.cpp
)

set(HEADER_FILES
	createserverdialogpages/flagspage.h
	createserverdialogpages/votingsetupwidget.h
	createserverdialogpages/flagsid.h
	createserverdialogpages/flagspagevaluecontroller1.h
	huffman/bitreader.h
	huffman/bitwriter.h
	huffman/codec.h
	huffman/huffcodec.h
	huffman/huffman.h
	enginezandronumqconfigbox.h
	huffmanqt.h
	huffmanudpsocket.h
	zandronumqbroadcast.h
	zandronumq1dmflags.h
	zandronumqengineplugin.h
	zandronumqgameexefactory.h
	zandronumqgamehost.h
	zandronumqgameinfo.h
	zandronumqgamerunner.h
	zandronumqgamesettings.h
	zandronumqmasterclient.h
	zandronumqrconprotocol.h
	zandronumqserver.h
	zandronumqserverdmflagsparser.h
)

set(TRANSLATIONS_FILES
	translations/zandronumq_ca_ES.ts
	translations/zandronumq_es_ES.ts
	translations/zandronumq_pl_PL.ts
)

include(../Translation.cmake)

add_library(${PLUGIN_NAME}
	MODULE
	${ZANDRONUMQ_FILES}
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
