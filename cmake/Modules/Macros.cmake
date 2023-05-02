# ------------------------------------------------------------------------------
# Macros.cmake
# ------------------------------------------------------------------------------
#
#  This library is free software; you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public
#  License as published by the Free Software Foundation; either
#  version 2.1 of the License, or (at your option) any later version.
#
#  This library is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this library; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
#  02110-1301  USA
#
# ------------------------------------------------------------------------------
#  Copyright (C) 2017 "Zalewa" <zalewapl@gmail.com>
# ------------------------------------------------------------------------------

# mt/md replacers excellently borrowed from
# http://stackoverflow.com/a/14172871/1089357

set(Macros_CompilerFlags
	CMAKE_CXX_FLAGS
	CMAKE_CXX_FLAGS_DEBUG
	CMAKE_CXX_FLAGS_MINSIZEREL
	CMAKE_CXX_FLAGS_RELEASE
	CMAKE_CXX_FLAGS_RELWITHDEBINFO
	CMAKE_C_FLAGS
	CMAKE_C_FLAGS_DEBUG
	CMAKE_C_FLAGS_MINSIZEREL
	CMAKE_C_FLAGS_RELEASE
	CMAKE_C_FLAGS_RELWITHDEBINFO
)

macro(msvc_mt)
	foreach(CompilerFlag ${Macros_CompilerFlags})
		string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
	endforeach()
endmacro()

macro(msvc_md)
	foreach(CompilerFlag ${Macros_CompilerFlags})
		string(REPLACE "/MT" "/MD" ${CompilerFlag} "${${CompilerFlag}}")
	endforeach()
endmacro()

macro(save_flags)
	foreach(CompilerFlag ${Macros_CompilerFlags})
		set(STORE_${CompilerFlag} ${${CompilerFlag}})
	endforeach()
endmacro()

macro(load_flags)
	foreach(CompilerFlag ${Macros_CompilerFlags})
		set(${CompilerFlag} ${STORE_${CompilerFlag}})
	endforeach()
endmacro()

# Find <package> that ensures target definitions.
# Maintains forward-compatibility with official CMake Find modules.

macro(find_package_ZLIB)
	find_package(ZLIB ${ARGV0})
	# https://github.com/Kitware/CMake/blob/745b56f58c8147aa6015a918f3bfd19abc807b48/Modules/FindZLIB.cmake#L122
	if(NOT TARGET ZLIB::ZLIB)
		add_library(ZLIB::ZLIB UNKNOWN IMPORTED)
		set_target_properties(ZLIB::ZLIB PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${ZLIB_INCLUDE_DIRS}")

		if(ZLIB_LIBRARY_RELEASE)
			set_property(TARGET ZLIB::ZLIB APPEND PROPERTY
				IMPORTED_CONFIGURATIONS RELEASE)
			set_target_properties(ZLIB::ZLIB PROPERTIES
				IMPORTED_LOCATION_RELEASE "${ZLIB_LIBRARY_RELEASE}")
		endif()

		if(ZLIB_LIBRARY_DEBUG)
			set_property(TARGET ZLIB::ZLIB APPEND PROPERTY
				IMPORTED_CONFIGURATIONS DEBUG)
			set_target_properties(ZLIB::ZLIB PROPERTIES
				IMPORTED_LOCATION_DEBUG "${ZLIB_LIBRARY_DEBUG}")
		endif()

		if(NOT ZLIB_LIBRARY_RELEASE AND NOT ZLIB_LIBRARY_DEBUG)
			set_property(TARGET ZLIB::ZLIB APPEND PROPERTY
				IMPORTED_LOCATION "${ZLIB_LIBRARY}")
		endif()
	endif()
endmacro()

macro(find_package_BZip2)
	find_package(BZip2 ${ARGV0})
	# If CMake won't define BZip2::BZip2 target for us,
	# we need to do it ourselves.
	# https://github.com/Kitware/CMake/blob/745b56f58c8147aa6015a918f3bfd19abc807b48/Modules/FindBZip2.cmake#L63
	if (BZIP2_FOUND AND NOT TARGET BZip2::BZip2)
		add_library(BZip2::BZip2 UNKNOWN IMPORTED)
		set_target_properties(BZip2::BZip2 PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${BZIP2_INCLUDE_DIRS}")

		if(BZIP2_LIBRARY_RELEASE)
			set_property(TARGET BZip2::BZip2 APPEND PROPERTY
				IMPORTED_CONFIGURATIONS RELEASE)
			set_target_properties(BZip2::BZip2 PROPERTIES
				IMPORTED_LOCATION_RELEASE "${BZIP2_LIBRARY_RELEASE}")
		endif()

		if(BZIP2_LIBRARY_DEBUG)
			set_property(TARGET BZip2::BZip2 APPEND PROPERTY
				IMPORTED_CONFIGURATIONS DEBUG)
			set_target_properties(BZip2::BZip2 PROPERTIES
				IMPORTED_LOCATION_DEBUG "${BZIP2_LIBRARY_DEBUG}")
		endif()

		if(NOT BZIP2_LIBRARY_RELEASE AND NOT BZIP2_LIBRARY_DEBUG)
			set_property(TARGET BZip2::BZip2 APPEND PROPERTY
				IMPORTED_LOCATION "${BZIP2_LIBRARY}")
		endif()
	endif()
endmacro()

macro(find_package_wadseeker)
	find_package(wadseeker CONFIG ${ARGV0})
	#Probably not needed. The config should do it's job.
	if (wadseeker_FOUND AND NOT TARGET Wadseeker::wadseeker)
		add_library(Wadseeker::wadseeker UNKNOWN IMPORTED)
		set_target_properties(Wadseeker::wadseeker PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${wadseeker_INCLUDE_DIRS}")

		if(wadseeker_LIBRARY_RELEASE)
			set_property(TARGET Wadseeker::wadseeker APPEND PROPERTY
				IMPORTED_CONFIGURATIONS RELEASE)
			set_target_properties(Wadseeker::wadseeker PROPERTIES
				IMPORTED_LOCATION_RELEASE "${wadseeker_LIBRARY_RELEASE}")
		endif()

		if(W_LIBRARY_DEBUG)
			set_property(TARGET Wadseeker::wadseeker APPEND PROPERTY
				IMPORTED_CONFIGURATIONS DEBUG)
			set_target_properties(Wadseeker::wadseeker PROPERTIES
				IMPORTED_LOCATION_DEBUG "${wadseeker_LIBRARY_DEBUG}")
		endif()

		if(NOT wadseeker_LIBRARY_RELEASE AND NOT wadseeker_LIBRARY_DEBUG)
			set_property(TARGET Wadseeker::wadseeker APPEND PROPERTY
				IMPORTED_LOCATION "${wadseeker_LIBRARY}")
		endif()
	endif()
endmacro()