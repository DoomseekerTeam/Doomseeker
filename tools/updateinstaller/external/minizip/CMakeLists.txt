project(minizip)

set (SOURCES
 ioapi.c
 unzip.c
 zip.c
)

set (HEADERS
 ioapi.h
 unzip.h
 zip.h
)

add_library(minizip
 ${SOURCES}
 ${HEADERS}
)

if (APPLE)
	# Mac OS X does not have fopen64()
	# and several related functions as the standard fopen()
	# calls are 64bit
	target_compile_definitions(minizip PRIVATE USE_FILE32API)
endif()

target_compile_definitions(minizip PRIVATE HAVE_BZIP2)

if (TARGET ZLIB::ZLIBMT)
	target_link_libraries(minizip PRIVATE ZLIB::ZLIBMT)
else ()
	target_link_libraries(minizip PRIVATE ZLIB::ZLIB)
endif ()
if (TARGET BZip2::BZip2MT)
	target_link_libraries(minizip PRIVATE BZip2::BZip2MT)
else ()
	target_link_libraries(minizip PRIVATE BZip2::BZip2)
endif ()

#if (UNIX)
	# on Mac, link to libbz2 dynamically, on Linux
	# we link statically to libbz2 so that an updater binary
	# build on Debian (where the packaged libbz2 has a SONAME of "libbz2.so.1.0"
	# works on Fedora/openSUSE (where no libbz2.so.1.0 symlink exists)
	#
	# see http://stackoverflow.com/questions/1835489/linking-an-application-to-libbz2-so-1-rather-than-libbz2-so-1-0
	#
#	set(BZ2_LIB_NAME bz2)
#	if (NOT APPLE)
#		set(BZ2_LIB_NAME bz2.a)
#	endif()

#	target_link_libraries(minizip z ${BZ2_LIB_NAME})
#else()
#	target_link_libraries(minizip
#		"${CMAKE_CURRENT_SOURCE_DIR}/../zlib/prebuilt/zlib_static.lib"
#		"${CMAKE_CURRENT_SOURCE_DIR}/../bzip2/libbz2_static.lib"
#	)
#endif()
