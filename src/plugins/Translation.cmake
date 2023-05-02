#------------------------------------------------------------------------------
# Translation.cmake
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
# Copyright (C) 2015 "Zalewa" <zalewapl@gmail.com>
#------------------------------------------------------------------------------
message(STATUS "    ${PLUGIN_NAME} provides translations")
if (UPDATE_TRANSLATIONS)
	qt5_create_translation(TS_FILES
		${FILES_TO_TRANSLATE}
		${TRANSLATIONS_FILES}
	)
	add_custom_target(${PLUGIN_NAME}_translations_target DEPENDS ${TS_FILES})
	message(STATUS "    make ${PLUGIN_NAME}_translations_target")
else ()
	add_custom_target(${PLUGIN_NAME}_translations_target)
endif ()

qt5_add_translation(QM_FILES ${TRANSLATIONS_FILES})
