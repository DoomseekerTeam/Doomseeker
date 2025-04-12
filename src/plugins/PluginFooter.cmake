#------------------------------------------------------------------------------
# PluginFooter.cmake
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

# See PluginHeader.txt

add_doomseeker_plugin(${PLUGIN_NAME})

target_include_directories(${PLUGIN_NAME}
PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR} 
)

target_compile_definitions(${PLUGIN_NAME} PRIVATE "-DPLUGIN_VERSION=${PLUGIN_VERSION}")

target_link_libraries(${PLUGIN_NAME} PUBLIC ${PLUGIN_LIBS} ${QT_LIBRARIES} doomseeker)

set_target_properties(${PLUGIN_NAME} PROPERTIES AUTOMOC ON AUTOUIC ON)

# Installation is on by default unless explicitly disabled.
if (NOT DONT_INSTALL) # if install
	if(UNIX AND NOT APPLE)
		install(TARGETS ${PLUGIN_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/doomseeker/engines COMPONENT ${PLUGIN_NAME})
		if (QM_FILES)
			install(FILES ${QM_FILES} DESTINATION share/doomseeker/translations COMPONENT ${PLUGIN_NAME})
		endif ()
	elseif(WIN32)
		install(TARGETS ${PLUGIN_NAME}
			RUNTIME DESTINATION engines
			LIBRARY DESTINATION engines
			COMPONENT "doomseeker-${PLUGIN_NAME}"
			OPTIONAL
		)
		if (QM_FILES)
			install(FILES ${QM_FILES} DESTINATION translations COMPONENT ${PLUGIN_NAME})
		endif ()
	endif()

	if(USE_CPACK)
		cpack_add_component(${PLUGIN_NAME}
			DISPLAY_NAME "doomseeker-${PLUGIN_NAME}"
			DESCRIPTION "Plugin to allow Doomseeker to query servers for ${PLUGIN_NAME}."
			DEPENDS doomseeker)
		parent_scope_component(${PLUGIN_NAME})

		file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/CPackComponent.version" "doomseeker-${PLUGIN_NAME} ${PLUGIN_VERSION}\n")
	endif()
endif()

add_custom_command(TARGET ${PLUGIN_NAME} PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/engines
)
add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${PLUGIN_NAME}> ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/engines
)

# Copy translations
if (QM_FILES)
	add_custom_command(TARGET ${PLUGIN_NAME} PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/translations
	)

	foreach(FILE_TO_COPY ${QM_FILES})
		add_custom_command(TARGET ${PLUGIN_NAME} POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${FILE_TO_COPY} ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/translations
		)
	endforeach(FILE_TO_COPY)
endif()
