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
# Copyright (C) 2009 "Zalewa" <zalewapl@gmail.com>
#------------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8.12)
project(Wadseeker)

if(POLICY CMP0042)
	cmake_policy(SET CMP0042 NEW)
endif()

if (MSVC)
	set(CMAKE_SHARED_LIBRARY_PREFIX "lib")
endif ()

# Multiarch support - CMAKE_INSTALL_LIBDIR will be set by Doomseeker's CMake
# or we will get it from GNUInstallDirs.
if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
	if(UNIX AND NOT APPLE)
		include(GNUInstallDirs)
	else()
		set(CMAKE_INSTALL_LIBDIR "lib")
	endif()
endif()

# C++ standard
if(CMAKE_VERSION VERSION_LESS "3.1")
	if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	endif ()
else ()
		set(CMAKE_CXX_STANDARD 11)
		set(CXX_STANDARD_REQUIRED ON)
endif ()

add_subdirectory(lzma)

include(${CMAKE_SOURCE_DIR}/cmake/Modules/Macros.cmake)

find_package(Qt5 COMPONENTS Core)

if(NOT Qt5_FOUND)
	message(FATAL_ERROR "Qt5 not found. Please make sure to set the Qt5_DIR variable")
else()
	find_package(Qt5 COMPONENTS LinguistTools Network REQUIRED)
	set(QT_LIBRARIES Qt5::Core Qt5::Network)
endif()

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	if(WIN32)
		set(FORCE_INTERNAL_ZLIB YES)
		set(FORCE_INTERNAL_BZIP2 YES)
	else()
		option(FORCE_INTERNAL_ZLIB "Use internal zlib" OFF)
		option(FORCE_INTERNAL_BZIP2 "Use internal bzip2" OFF)

		include(${CMAKE_SOURCE_DIR}/cmake/Modules/Macros.cmake)

		if(NOT FORCE_INTERNAL_ZLIB)
			find_package_ZLIB()
		endif()
		if(NOT FORCE_INTERNAL_BZIP2)
			find_package_BZip2()
		endif()
	endif()

	if( ZLIB_FOUND AND NOT FORCE_INTERNAL_ZLIB )
		message( STATUS "Using system zlib" )
	else( ZLIB_FOUND AND NOT FORCE_INTERNAL_ZLIB )
		message( STATUS "Using internal zlib" )
		set( ZLIB_INTERNAL YES )
		set(SKIP_INSTALL_ALL ON)
		add_subdirectory( dependencies/zlib )
		unset(SKIP_INSTALL_ALL)
		set( ZLIB_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/zlib" )
		set( ZLIB_INCLUDE_DIR "${ZLIB_INCLUDE_DIRS}" )
		set( ZLIB_LIBRARIES ZLIB::ZLIB )
		set( ZLIB_LIBRARY ZLIB::ZLIB )
	endif()

	if( BZIP2_FOUND AND NOT FORCE_INTERNAL_BZIP2 )
		message( STATUS "Using system bzip2 library" )
	else( BZIP2_FOUND AND NOT FORCE_INTERNAL_BZIP2 )
		message( STATUS "Using internal bzip2 library" )
		set( BZIP2_INTERNAL YES )
		add_subdirectory( dependencies/bzip2 )
		set( BZIP2_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/bzip2" )
		set( BZIP2_INCLUDE_DIR "${BZIP2_INCLUDE_DIRS}" )
		set( BZIP2_LIBRARIES bz2 )
		set( BZIP2_LIBRARY bz2 )
	endif()
else()
	if(NOT TARGET ZLIB::ZLIB)
		find_package_ZLIB(REQUIRED)
	endif()
	if(NOT TARGET BZip2::BZip2)
		find_package_BZip2(REQUIRED)
	endif()
	find_package(Threads REQUIRED)
endif()

include("wadseekerversiondefs.cmake")
configure_file(wadseekerversiondefs.h.in wadseekerversiondefs.h)

set(WADSEEKER_FILES
	entities/checksum.cpp
	entities/hash.cpp
	entities/link.cpp
	entities/modfile.cpp
	entities/modset.cpp
	entities/waddownloadinfo.cpp
	protocols/freedoom/freedoominfoparser.cpp
	protocols/freedoom/freedoomquery.cpp
	protocols/idgames/idgamesclient.cpp
	protocols/idgames/idgamesfile.cpp
	protocols/idgames/idgamesfilematcher.cpp
	protocols/idgames/idgamesreply.cpp
	protocols/idgames/idgamesresult.cpp
	protocols/wadarchive/wadarchiveclient.cpp
	protocols/json.cpp
	protocols/http.cpp
	protocols/networkreply.cpp
	protocols/networkreplysignalwrapper.cpp
	protocols/networkreplytimeouter.cpp
	protocols/urlprovider.cpp
	wadretriever/wadinstaller.cpp
	wadretriever/wadretriever.cpp
	wwwseeker/entities/fileseekinfo.cpp
	wwwseeker/htmlparser.cpp
	wwwseeker/idgames.cpp
	wwwseeker/urlparser.cpp
	wwwseeker/wwwseeker.cpp
	zip/unarchive.cpp
	zip/un7zip.cpp
	zip/untar.cpp
	zip/unzip.cpp
	zip/zipfile.cpp
	filefind.cpp
	freedoom.cpp
	ioutils.cpp
	modinstall.cpp
	wadseeker.cpp
	wadseekerversioninfo.cpp
)

# For the purpose of generation proper project files.
set(HEADER_FILES
	entities/checksum.h
	entities/hash.h
	entities/link.h
	entities/modfile.h
	entities/modset.h
	entities/waddownloadinfo.h
	protocols/freedoom/freedoominfoparser.h
	protocols/freedoom/freedoomquery.h
	protocols/idgames/idgamesclient.h
	protocols/idgames/idgamesfile.h
	protocols/idgames/idgamesfilematcher.h
	protocols/idgames/idgamesreply.h
	protocols/idgames/idgamesresult.h
	protocols/wadarchive/wadarchiveclient.h
	protocols/json.h
	protocols/http.h
	protocols/networkreply.h
	protocols/networkreplysignalwrapper.h
	protocols/networkreplytimeouter.h
	protocols/urlprovider.h
	wadretriever/wadinstaller.h
	wadretriever/wadretriever.h
	wwwseeker/entities/fileseekinfo.h
	wwwseeker/htmlparser.h
	wwwseeker/idgames.h
	wwwseeker/urlparser.h
	wwwseeker/wwwseeker.h
	zip/unarchive.h
	zip/un7zip.h
	zip/untar.h
	zip/unzip.h
	zip/zipfile.h
	filefind.h
	freedoom.h
	ioutils.h
	modinstall.h
	wadseeker.h
	wadseekerexportinfo.h
	wadseekermessagetype.h
	wadseekerversiondefs.h
	wadseekerversioninfo.h
)

add_library(wadseeker SHARED ${WADSEEKER_FILES} ${HEADER_FILES})
if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	add_library(Wadseeker::wadseeker ALIAS wadseeker)
endif()

target_include_directories(wadseeker
PRIVATE
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)

target_link_libraries(wadseeker
LINK_PRIVATE
	${QT_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	BZip2::BZip2
	lzma
	ZLIB::ZLIB)

set_target_properties(wadseeker PROPERTIES
	AUTOMOC ON
	VERSION ${WADSEEKER_LIBVERSION}
	SOVERSION ${WADSEEKER_SOVERSION}
)

if(DEFINED Doomseeker_BINARY_DIR)
	set_target_properties(wadseeker PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${Doomseeker_BINARY_DIR} RUNTIME_OUTPUT_DIRECTORY ${Doomseeker_BINARY_DIR})
endif()

set(TARGETS_EXPORT_NAME "wadseekerTargets")
if(NOT WIN32 AND NOT XCODE_VERSION)
	install(
		TARGETS wadseeker
		EXPORT ${TARGETS_EXPORT_NAME}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT wadseeker NAMELINK_SKIP
		INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	)
	install(
		TARGETS wadseeker
		EXPORT ${TARGETS_EXPORT_NAME}
		LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT wadseeker-dev NAMELINK_ONLY
	)
	install(FILES ${CMAKE_SOURCE_DIR}/LICENSE.json DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/wadseeker COMPONENT wadseeker)
elseif(WIN32)
	install(
		TARGETS wadseeker
		EXPORT ${TARGETS_EXPORT_NAME}
		RUNTIME DESTINATION . COMPONENT wadseeker
		ARCHIVE DESTINATION ./lib COMPONENT wadseeker
		INCLUDES DESTINATION ${CMAKE_INSTALL_PREFIX}/include
	)
	# License files.
	set(LICENSE_FILES
		LICENSE
		LICENSE.json
	)
	foreach (LICFILE ${LICENSE_FILES})
		install(FILES "${CMAKE_SOURCE_DIR}/${LICFILE}" DESTINATION .
			COMPONENT wadseeker
			RENAME "${LICFILE}.txt")
	endforeach(LICFILE)
endif()

install(SCRIPT InstallHeaders.cmake COMPONENT wadseeker-dev)

# We need a newer CPack version to get desirable results.
if(NOT CMAKE_VERSION VERSION_LESS "3.6.0")
	# CPack can autogenerate this file, but since we have a postprocessing step
	# to correct the versions per component we're better off generating it
	# manually. This means there will be a lintian error until that step is run.
	configure_file(deb-extra/shlibs.in ${CMAKE_CURRENT_BINARY_DIR}/shlibs)

	set(CPACK_PACKAGE_VERSION ${VERSION_STRING})

	set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
	set(CPACK_DEBIAN_WADSEEKER_PACKAGE_NAME "libwadseeker1")
	set(CPACK_DEBIAN_WADSEEKER_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/deb-extra/triggers;${CMAKE_CURRENT_BINARY_DIR}/shlibs")
	set(CPACK_DEBIAN_WADSEEKER_PACKAGE_SECTION "libs")
	set(CPACK_COMPONENT_WADSEEKER_DESCRIPTION "Library for locating and downloading Doom mods.\n Searching is done against a given list of common servers including the id\n games archive. Wadseeker is based around Qt core and does not provide a GUI\n itself.")
	set(CPACK_DEBIAN_WADSEEKER-DEV_PACKAGE_NAME "libwadseeker-dev")
	set(CPACK_DEBIAN_WADSEEKER-DEV_PACKAGE_DEPENDS "${CPACK_DEBIAN_WADSEEKER_PACKAGE_NAME}, qtbase5-dev")
	set(CPACK_DEBIAN_WADSEEKER-DEV_PACKAGE_SECTION "libdevel")
	set(CPACK_COMPONENT_WADSEEKER-DEV_DESCRIPTION "Header files for libwadseeker.\n Wadseeker is a library for locating and downloading Doom mods.")
	if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
		set(CPACK_DEBIAN_WADSEEKER_PACKAGE_NAME ${CPACK_DEBIAN_WADSEEKER_PACKAGE_NAME} PARENT_SCOPE)
		set(CPACK_DEBIAN_WADSEEKER_PACKAGE_CONTROL_EXTRA ${CPACK_DEBIAN_WADSEEKER_PACKAGE_CONTROL_EXTRA} PARENT_SCOPE)
		set(CPACK_DEBIAN_WADSEEKER_PACKAGE_SECTION ${CPACK_DEBIAN_WADSEEKER_PACKAGE_SECTION} PARENT_SCOPE)
		set(CPACK_COMPONENT_WADSEEKER_DESCRIPTION ${CPACK_COMPONENT_WADSEEKER_DESCRIPTION} PARENT_SCOPE)
		set(CPACK_DEBIAN_WADSEEKER-DEV_PACKAGE_NAME ${CPACK_DEBIAN_WADSEEKER-DEV_PACKAGE_NAME} PARENT_SCOPE)
		set(CPACK_DEBIAN_WADSEEKER-DEV_PACKAGE_DEPENDS ${CPACK_DEBIAN_WADSEEKER-DEV_PACKAGE_DEPENDS} PARENT_SCOPE)
		set(CPACK_DEBIAN_WADSEEKER-DEV_PACKAGE_SECTION ${CPACK_DEBIAN_WADSEEKER-DEV_PACKAGE_SECTION} PARENT_SCOPE)
		set(CPACK_COMPONENT_WADSEEKER-DEV_DESCRIPTION ${CPACK_COMPONENT_WADSEEKER-DEV_DESCRIPTION} PARENT_SCOPE)

		file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/CPackComponent.version" "${CPACK_DEBIAN_WADSEEKER_PACKAGE_NAME} ${VERSION_STRING}\n${CPACK_DEBIAN_WADSEEKER-DEV_PACKAGE_NAME} ${VERSION_STRING}\n")
	else()
		include(CPack)
		include(CMakePackageConfigHelpers)
		set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/wadseeker")

		set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")
		set(VERSION_CONFIG "${generated_dir}/wadseekerConfigVersion.cmake")
		set(PROJECT_CONFIG "${generated_dir}/wadseekerConfig.cmake")
		configure_package_config_file(wadseekerConfig.cmake.in
			${PROJECT_CONFIG}
			INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
		)
		write_basic_package_version_file (
			${VERSION_CONFIG} COMPATIBILITY SameMajorVersion
			VERSION ${VERSION_STRING}
		)

		install(
			EXPORT ${TARGETS_EXPORT_NAME}
			NAMESPACE ${PROJECT_NAME}::
			DESTINATION ${CONFIG_INSTALL_DIR}
		)
		install(
			FILES ${PROJECT_CONFIG} ${VERSION_CONFIG}
			DESTINATION ${CONFIG_INSTALL_DIR}
		)
	endif()
	cpack_add_component(wadseeker
		DISPLAY_NAME libwadseeker
		DESCRIPTION "Library for automatically locating and downloading mods for Doom."
		REQUIRED)
	if(COMMAND parent_scope_component)
		parent_scope_component(wadseeker)
	endif()
endif()
