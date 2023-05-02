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
project(Doomseeker)

# Multiarch support
if(UNIX AND NOT APPLE)
	include(GNUInstallDirs)
else()
	# CMAKE_ prefix used here to match the variable set by GNUInstallDirs.
	# Values currently don't matter outside of Linux.
	if(NOT DEFINED CMAKE_INSTALL_LIBDIR)
		set(CMAKE_INSTALL_LIBDIR "lib")
	endif()
	if(NOT DEFINED CMAKE_INSTALL_DATAROOTDIR)
		set(CMAKE_INSTALL_DATAROOTDIR "share")
	endif()
endif()

# We need a newer CPack version to get desirable results.
# Note: CMake 3.11+ recommended for best results
if(NOT CMAKE_VERSION VERSION_LESS "3.6.0")
	set(USE_CPACK ON)
else()
	set(USE_CPACK OFF)
endif()

if(USE_CPACK)
	# Right now all we support is DEB, but we'll probably want to make this a
	# user option later?
	set(CPACK_GENERATOR "DEB")
	set(CPACK_PACKAGE_CONTACT "First Last <example@localhost.localdomain>" CACHE STRING "Contact info for archive maintainer.")
	# We encode the install prefix into the binary, so we should use the same prefix when packaging.
	set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
	set(CPACK_STRIP_FILES ON)
	set(CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)

	set(CPACK_DEB_COMPONENT_INSTALL ON)
	set(CPACK_DEBIAN_FILE_NAME "DEB-DEFAULT")
	set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://doomseeker.drdteam.org")
	set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON CACHE BOOL "Perform automatic dependency listing when generating deb packages. Disable this to generate intermediate packages to solve chicken and egg problem.")
	set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION ON)
	set(CPACK_DEBIAN_COMPRESSION_TYPE "xz")

	include(CPackComponent)

	# Helper macro to get variables set by cpack_add_component propagated back up to the root.
	macro(parent_scope_component COMP)
		string(TOUPPER ${COMP} COMP_UPPER)
		foreach(VARNAME "DISPLAY_NAME" "DESCRIPTION" "GROUP" "DEPENDS" "INSTALL_TYPES" "ARCHIVE_FILE" "HIDDEN" "REQUIRED" "DISABLED" "DOWNLOADED" "PLIST")
			set(CPACK_COMPONENT_${COMP_UPPER}_${VARNAME} ${CPACK_COMPONENT_${COMP_UPPER}_${VARNAME}} PARENT_SCOPE)
		endforeach()
	endmacro()
endif()

if(WIN32)
	set(FORCE_INTERNAL_ZLIB YES)
	set(FORCE_INTERNAL_BZIP2 YES)
	set(FORCE_INTERNAL_WADSEEKER YES)
else()
	option(FORCE_INTERNAL_ZLIB "Use internal zlib" OFF)
	option(FORCE_INTERNAL_BZIP2 "Use internal bzip2" OFF)
	option(FORCE_INTERNAL_WADSEEKER "Use internal wadseeker" ON)
endif()

include(${CMAKE_SOURCE_DIR}/cmake/Modules/Macros.cmake)

if(NOT FORCE_INTERNAL_ZLIB)
	find_package_ZLIB()
endif()
if(NOT FORCE_INTERNAL_BZIP2)
	find_package_BZip2()
endif()
if(NOT FORCE_INTERNAL_WADSEEKER)
	find_package_wadseeker()
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

if ( wadseeker_FOUND AND NOT FORCE_INTERNAL_WADSEEKER )
	message( STATUS "Using system Wadseeker library" )
else( wadseeker_FOUND AND NOT FORCE_INTERNAL_WADSEEKER )
	message( STATUS "Using internal Wadseeker library" )
	set( WADSEEKER_INTERNAL YES )
	add_subdirectory( src/wadseeker )
	set( WADSEEKER_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/wadseeker" )
	set( WADSEEKER_INCLUDE_DIR "${WADSEEKER_INCLUDE_DIRS}" )
endif()

if(MSVC AND NOT MSVC90 AND NOT MSVC12)
	message(WARNING
		"Full automatic packaging is only supported for Microsoft Visual C++ 2008 and 2013. "
		"Remember that redist needs to be accessible both by the main program "
		"and by the engines/ directory. Copy over Manifest and redist DLLs to "
		"engines/ directory if you do not plan on installing redist on target "
		"machines."
	)
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

if(WIN32)
	set(WITH_AUTOUPDATES 1)
	add_subdirectory(tools/updateinstaller)
endif()

add_subdirectory(tools/updaterevision)

add_subdirectory(src/core)
add_subdirectory(src/plugins)

message(STATUS "'INSTALL' will dump files to \"${CMAKE_INSTALL_PREFIX}\"")

if(USE_CPACK)
	include(CPack)
endif()
