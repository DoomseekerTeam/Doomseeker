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
#-------------------------------------------------------------------------------
# Copyright (C) 2011 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
#-------------------------------------------------------------------------------

# Regarding Microsoft Visual Studio:
# Creating proper builds with MSVC required setting subsystem to windows,
# otherwise an unwanted console window was created. However when subsystem is
# set to windows MSVC uses WinMain() as the entry point, while MinGW simply
# keeps using the main() function. Also note that since console window is useful
# for debugging, Debug builds still have subsystem set to console.
#
# To solve all the issues WinMain() has been implemented and is now used
# depending on certain defines: if _MSC_VER is defined it means that user is
# attempting to create a build with MSVC. Also if NDEBUG is defined it means
# that user chose one of the Release builds. When both of these conditions are
# met then, and only then, WinMain() is used instead of main() as the
# entry point.
#
# Conclusion: If current MSVC subsystem setup is changed the code may not
# compile, as the proper entry point will be missing.

if (WIN32 OR APPLE)
	set(DEFAULT_NONFREE_INSTALL "yes")
else()
	# On Linux distros default to 'no' to avoid accidental
	# violation of any freedom rules.
	set(DEFAULT_NONFREE_INSTALL "no")
endif()

option(DOOMSEEKER_IP2C_AUTOUPDATE_DEFAULT "Configure if autoupdate of IP2C should be enabled by default in newly created configs" "yes")
configure_file(doomseeker_copts.h.in doomseeker_copts.h)

option(DOOMSEEKER_INSTALL_NONFREE "Install non-free resources?" ${DEFAULT_NONFREE_INSTALL})
set(DOOMSEEKER_IP2C_DAT "" CACHE FILEPATH "Path to IpToCountry.dat; optional")

# Get the revision from vcs
add_custom_target(revision_check ALL
	COMMAND updaterevision ${CMAKE_CURRENT_SOURCE_DIR}/gitinfo.h
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	DEPENDS updaterevision
)

# In case gitinfo.h isn't yet generated create a dummy file so our file listing
# will contain it. We can't just add it to the file listing since then CMake
# would complain that it doesn't exist.
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/gitinfo.h")
	file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/gitinfo.h" "")
endif()

find_package(${Qt_PACKAGE} COMPONENTS Widgets)

if(NOT ${Qt_PACKAGE}Widgets_FOUND)
	message(FATAL_ERROR "${Qt_PACKAGE} not found. Please make sure to set the ${Qt_PACKAGE}_DIR variable")
else()
	find_package(${Qt_PACKAGE} COMPONENTS Core Gui LinguistTools Multimedia Network Xml REQUIRED)
	if (WIN32)
		find_package(${Qt_PACKAGE} COMPONENTS WinExtras REQUIRED)
	endif()
	set(QT_LIBRARIES ${Qt_PACKAGE}::Widgets ${Qt_PACKAGE}::Multimedia ${Qt_PACKAGE}::Network ${Qt_PACKAGE}::Xml ${Qt_PACKAGE}::Gui ${Qt_PACKAGE}::Core)
	if(${Qt_PACKAGE}WinExtras_FOUND)
		list(APPEND QT_LIBRARIES ${Qt_PACKAGE}::WinExtras)
	endif()
	get_target_property(QMAKE_EXEC ${Qt_PACKAGE}::qmake LOCATION)
endif()

if(NOT TARGET ZLIB::ZLIB)
	find_package_ZLIB(REQUIRED)
endif()

if(NOT TARGET Wadseeker::wadseeker AND NOT WADSEEKER_INTERNAL)
	find_package_wadseeker(REQUIRED)
endif()

include("CMakeSpawnFileListing.cmake")
# This hack allows CMake to regenerate the project when spawned
# file listing file changes from outside sources. This covers
# updating local repo and checking out different revisions.
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/CMakeFileListing.cmake"
	"${CMAKE_CURRENT_SOURCE_DIR}/CMakeFileListing.cmake"
	NEWLINE_STYLE LF
)
include("CMakeFileListing.cmake")

# Translations
set (TRANSLATIONS_FILES
	translations/ca_ES.ts
	translations/es_ES.ts
	translations/pl_PL.ts
)

option (UPDATE_TRANSLATIONS "Update source translation translations/*.ts files (WARNING: make clean will delete the source .ts files! Danger!)")
if (UPDATE_TRANSLATIONS)
	qtx_create_translation(TS_FILES
		${CMAKE_CURRENT_SOURCE_DIR}
		${TRANSLATIONS_FILES}
	)
	add_custom_target(_translations_target_touch
		COMMAND ${CMAKE_COMMAND} -E touch_nocreate ${CMAKE_CURRENT_SOURCE_DIR})
	add_custom_target(translations_target DEPENDS _translations_target_touch ${TS_FILES})
	message(WARNING
		"Update source translation translations/*.ts files (WARNING: make clean will delete the source .ts files! Danger!)"
	)
	message(STATUS "make translations_target\n\n")
else ()
	add_custom_target(translations_target)
endif ()

qtx_add_translation(QM_FILES ${TRANSLATIONS_FILES})

if (WIN32 AND ${Qt_PACKAGE}Widgets_FOUND)
	# Copy Qt5 translations that need to be redistributed with the .exe.
	# WARNING: This should always be defined after qt5_add_translation.
	execute_process(COMMAND ${QMAKE_EXEC} -query QT_INSTALL_TRANSLATIONS
		OUTPUT_VARIABLE QT_TRANSLATIONS_DIR
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	set(QM_FILES
		${QM_FILES}
		${QT_TRANSLATIONS_DIR}/qtbase_ca.qm
		${QT_TRANSLATIONS_DIR}/qtbase_es.qm
		${QT_TRANSLATIONS_DIR}/qtbase_pl.qm
		${QT_TRANSLATIONS_DIR}/qtmultimedia_ca.qm
		${QT_TRANSLATIONS_DIR}/qtmultimedia_es.qm
		${QT_TRANSLATIONS_DIR}/qtmultimedia_pl.qm)
endif ()

# windows.rc
if(WIN32)
	# compile the Windows resource file.  How we do this depends on the compiler.
	set(SOURCE_FILES ${SOURCE_FILES} ${Doomseeker_SOURCE_DIR}/media/windows.rc)
endif()

# Generate versiondefs.h
include("versiondefs.cmake")
configure_file(versiondefs.h.in versiondefs.h)
set(HEADER_FILES "${HEADER_FILES}" versiondefs.h)

## Doomseeker object target
add_library(doomseeker_obj OBJECT
	${Doomseeker_SOURCE_DIR}/media/resources.qrc
	${QM_FILES}
	${UI_FILES}
	${SOURCE_FILES}
	${HEADER_FILES}
)
set_target_properties(doomseeker_obj PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
target_compile_definitions(doomseeker_obj PRIVATE doomseeker_EXPORTS)
target_link_libraries(doomseeker_obj
PRIVATE
	Wadseeker::wadseeker
	ZLIB::ZLIB
	${CMAKE_DL_LIBS}
PUBLIC
	${QT_LIBRARIES}
)

target_include_directories(doomseeker_obj
PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/gui/
	${CMAKE_CURRENT_BINARY_DIR}
PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
)

if(WITH_AUTOUPDATES)
	target_compile_definitions(doomseeker_obj PRIVATE -DWITH_AUTOUPDATES)
endif()

if (MSVC)
	target_compile_definitions(doomseeker_obj PRIVATE -D_CRT_SECURE_NO_WARNINGS)
endif()

target_compile_definitions(doomseeker_obj PRIVATE
	"-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\""
	"-DINSTALL_LIBDIR=\"${CMAKE_INSTALL_LIBDIR}\""
)

# Make sure the vcs version file is created before we attempt to compile
# Doomseeker.
add_dependencies(doomseeker_obj revision_check)

## Doomseeker executable
add_executable(doomseeker
	main.cpp
	main.h
)

target_link_libraries(doomseeker
	PRIVATE Wadseeker::wadseeker
	PUBLIC doomseeker_obj
)

target_compile_definitions(doomseeker PRIVATE
	"-DINSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\""
	"-DINSTALL_LIBDIR=\"${CMAKE_INSTALL_LIBDIR}\""
)

# Copy translations
set(TRANSLATION_FILES_TO_COPY
	${QM_FILES}
	${CMAKE_CURRENT_SOURCE_DIR}/translations/translations.def
)

add_custom_command(TARGET doomseeker PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/translations
)

foreach(FILE_TO_COPY ${TRANSLATION_FILES_TO_COPY})
	add_custom_command(TARGET doomseeker POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different ${FILE_TO_COPY} ${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/translations
	)
endforeach(FILE_TO_COPY)

set_target_properties(doomseeker PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
# Allow plugins to link with doomseeker
set_target_properties(doomseeker PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR} ENABLE_EXPORTS 1)

# Install - procedures and instructions
function(install_all_slot_styles)
	if (DOOMSEEKER_INSTALL_NONFREE)
		install_slot_style("marines")
	endif()
endfunction()

function(install_slot_style name)
	if(UNIX AND NOT APPLE)
		set(prefix "share/doomseeker")
	else()
		set(prefix ".")
	endif()
	foreach(slotpic bot.png open.png player.png specopen.png spectator.png)
		install(FILES ${CMAKE_SOURCE_DIR}/media/slotstyles/${name}/${slotpic}
			DESTINATION ${prefix}/theme/slots/${name}
			COMPONENT doomseeker)
	endforeach()
endfunction()

if(WIN32)
	if(MSVC)
		set_target_properties(doomseeker PROPERTIES
			LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS"
			LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:WINDOWS"
			LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS")
	else()
		set_target_properties(doomseeker PROPERTIES
			LINK_FLAGS_RELEASE "-Wl,-subsystem,windows"
			LINK_FLAGS_MINSIZEREL "-Wl,-subsystem,windows")
	endif()

	# OpenSSL
	set(OPENSSL_LIBEAY32_DLL "" CACHE FILEPATH "Path to libeay32.dll v.1.0.2.1")
	set(OPENSSL_SSLEAY32_DLL "" CACHE FILEPATH "Path to ssleay32.dll v.1.0.2.1")
	install(FILES ${OPENSSL_LIBEAY32_DLL} ${OPENSSL_SSLEAY32_DLL}
		${CMAKE_SOURCE_DIR}/media/cacerts.pem
		DESTINATION . COMPONENT doomseeker)

	# Install program DIRECTLY to directory specified by
	# ${CMAKE_INSTALL_PREFIX}. If ${CMAKE_INSTALL_PREFIX} is set to something
	# stupid like "C:\Program Files\" (this is the CMake's default, BTW)
	# then program.exe will be dumped to "C:\Program Files\program.exe"
	install(TARGETS doomseeker RUNTIME DESTINATION . COMPONENT doomseeker)
	# Install Qt libraries.
	if (${Qt_PACKAGE}Widgets_FOUND)
		# Qt5*.dll
		foreach(qtlib ${QT_LIBRARIES})
			get_target_property(qtlib_location ${qtlib} LOCATION)
			install(FILES ${qtlib_location} DESTINATION . COMPONENT doomseeker)
		endforeach()
		# plugins
		execute_process(COMMAND ${QMAKE_EXEC} -query QT_INSTALL_PLUGINS
			OUTPUT_VARIABLE QT_PLUGINS_DIR
			OUTPUT_STRIP_TRAILING_WHITESPACE)
		install(FILES ${QT_PLUGINS_DIR}/audio/qtaudio_windows.dll DESTINATION audio COMPONENT doomseeker)
		install(FILES ${QT_PLUGINS_DIR}/platforms/qwindows.dll DESTINATION platforms COMPONENT doomseeker)
		# Media
		install_all_slot_styles()
		# ICU (the 26.6 Megabyte bastard)
		#
		# Installation of ICU is optional. If you don't have ICU
		# files, it's possible that your Qt installation doesn't
		# need them. That's very good as ICU files weight about 30
		# Megabytes and Doomseeker makes no use of them.
		execute_process(COMMAND ${QMAKE_EXEC} -query QT_INSTALL_LIBEXECS
			OUTPUT_VARIABLE QT_LIBEXECS_DIR
			OUTPUT_STRIP_TRAILING_WHITESPACE)
		install(FILES
			${QT_LIBEXECS_DIR}/icuin53.dll
			${QT_LIBEXECS_DIR}/icuuc53.dll
			${QT_LIBEXECS_DIR}/icudt53.dll
			DESTINATION . COMPONENT doomseeker OPTIONAL)
	else()
		get_filename_component(QT_DLL_DIR ${QT_QMAKE_EXECUTABLE} PATH)
		set(QT_LIBS_RELEASE
			${QT_DLL_DIR}/QtCore4.dll
			${QT_DLL_DIR}/QtGui4.dll
			${QT_DLL_DIR}/QtNetwork4.dll
			${QT_DLL_DIR}/QtXml4.dll
			)
		install(FILES ${QT_LIBS_RELEASE} DESTINATION . COMPONENT doomseeker)
	endif()
	# Install .ico file. No idea what purpose would this serve but some
	# programs come with their icons in the .ico file so why not.
	install(FILES ${CMAKE_SOURCE_DIR}/media/icon_windows.ico DESTINATION .
		COMPONENT doomseeker
		RENAME doomseeker.ico
	)
	# Install runtime libraries.
	if (MSVC90 OR MSVC12)
		# Install redist and manifest for the program.
		include("${CMAKE_SOURCE_DIR}/cmake/Modules/FindRequiredSystemLibraries.cmake")
		install_programs(. ${REQUIRED_SYSTEM_LIBRARIES})
	elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
		# MinGW-w64 release signature: i686-8.1.0-release-posix-dwarf-rt_v6-rev0
		#
		# Get MinGW-w64 path from CMAKE_LINKER variable because this path
		# is the same regardless if we're using MinGW's GCC or Clang compiled
		# with MinGW-w64.
		get_filename_component(MINGW_PATH ${CMAKE_LINKER} PATH)
		install_programs(.
			${MINGW_PATH}/libgcc_s_dw2-1.dll
			${MINGW_PATH}/libstdc++-6.dll
			${MINGW_PATH}/libwinpthread-1.dll
		)
	endif()
	if (MSVC90)
		# Install manifest for plugins.
		install(FILES ${CMAKE_SOURCE_DIR}/media/manifest_for_engines_directory/Microsoft.VC90.CRT.manifest
			DESTINATION engines
			COMPONENT doomseeker)
	endif()

	# License files.
	# Text files in Windows distribution really should have a .txt extension
	# so rename them all accordingly.
	set(LICENSE_FILES
		LICENSE
	)
	foreach (LICFILE ${LICENSE_FILES})
		install(FILES "${CMAKE_SOURCE_DIR}/${LICFILE}" DESTINATION .
			COMPONENT doomseeker
			RENAME "${LICFILE}.txt")
	endforeach(LICFILE)

	# Translatons
	install(FILES ${TRANSLATION_FILES_TO_COPY} DESTINATION translations COMPONENT doomseeker)
	# Other
	if(DOOMSEEKER_IP2C_DAT)
		install(FILES ${DOOMSEEKER_IP2C_DAT} DESTINATION . RENAME IpToCountry.dat COMPONENT doomseeker)
	endif()
	install(FILES ${CMAKE_SOURCE_DIR}/tools/doomseeker-portable.bat DESTINATION . COMPONENT doomseeker)
	install(FILES ${CMAKE_SOURCE_DIR}/CHANGELOG.md DESTINATION . COMPONENT doomseeker RENAME CHANGELOG.txt)
endif()

if(UNIX AND NOT APPLE)
	install(TARGETS doomseeker RUNTIME DESTINATION bin COMPONENT doomseeker)
	install(FILES ${CMAKE_SOURCE_DIR}/media/icon.png DESTINATION share/icons COMPONENT doomseeker RENAME doomseeker.png)
	install(FILES ${TRANSLATION_FILES_TO_COPY} DESTINATION share/doomseeker/translations COMPONENT doomseeker)
	install(FILES ${CMAKE_SOURCE_DIR}/media/org.drdteam.Doomseeker.desktop DESTINATION share/applications COMPONENT doomseeker)
	install_all_slot_styles()

	if(DOOMSEEKER_IP2C_DAT)
		install(FILES ${DOOMSEEKER_IP2C_DAT} DESTINATION share/doomseeker RENAME IpToCountry.dat COMPONENT doomseeker)
	endif()

	# When not installing to /usr we should use an install rpath.
	if(NOT CMAKE_INSTALL_PREFIX STREQUAL "/usr")
		set_target_properties(doomseeker PROPERTIES INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")
	endif()
endif()

if(USE_CPACK)
	cpack_add_component(doomseeker
		DISPLAY_NAME "Doomseeker"
		DESCRIPTION "Cross-platform browser for Doom servers."
		REQUIRED
		DEPENDS wadseeker)
	parent_scope_component(doomseeker)

	set(CPACK_DEBIAN_DOOMSEEKER_PACKAGE_NAME "doomseeker" PARENT_SCOPE)
	set(CPACK_DEBIAN_DOOMSEEKER_PACKAGE_SUMMARY "Server browser for Doom" PARENT_SCOPE)
	set(CPACK_PACKAGE_VERSION ${VERSION_STRING} PARENT_SCOPE)
	set(CPACK_COMPONENT_DOOMSEEKER_DESCRIPTION "Universal cross-platform server browser designed primarily for Doom.\n This package provides the core program which should be complimented with one or\n more plugins providing support for querying their respective servers." PARENT_SCOPE)

	# Not really necessary, but it cleans up extraneous stuff CPack adds to the version string depending on the CPack version.
	file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/CPackComponent.version" "doomseeker ${VERSION_STRING}\n")
endif()

if(WIN32 OR APPLE)
	find_program(PYTHON_EXEC python3)
	if(NOT AUTOUPDATE_PACKAGE_DIRECTORY)
		set(AUTOUPDATE_PACKAGE_DIRECTORY ${CMAKE_BUILD_DIR})
	endif()
	if(PYTHON_EXEC)
		set(INSTALL_TARGET "install")
		if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
			set(INSTALL_TARGET "install/strip")
		endif()

		add_custom_target(package_beta
			COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
				--target ${INSTALL_TARGET} --config ${CMAKE_CFG_INTDIR}
			COMMAND ${PYTHON_EXEC}
				"${CMAKE_SOURCE_DIR}/tools/updateinstaller/tools/build-update-kit.py"
				-c beta -i "${CMAKE_INSTALL_PREFIX}"
			WORKING_DIRECTORY ${AUTOUPDATE_PACKAGE_DIRECTORY})
		add_custom_target(package_stable
			COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
				--target ${INSTALL_TARGET} --config ${CMAKE_CFG_INTDIR}
			COMMAND ${PYTHON_EXEC}
				"${CMAKE_SOURCE_DIR}/tools/updateinstaller/tools/build-update-kit.py"
				-c stable -i "${CMAKE_INSTALL_PREFIX}"
			WORKING_DIRECTORY ${AUTOUPDATE_PACKAGE_DIRECTORY})
	endif()
endif()
