cmake_minimum_required( VERSION 2.8.12 )

set( SOURCES
    blocksort.c
    bzlib.c
    compress.c
    crctable.c
    decompress.c
    huffman.c
    randtable.c )
add_library( bz2 STATIC ${SOURCES} )
target_include_directories( bz2 INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} )
target_compile_definitions( bz2 PRIVATE -DBZ_NO_STDIO )

if( CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang" )
	target_compile_options( bz2 PRIVATE -Wall -Wextra -fomit-frame-pointer )
	# [Doomseeker] MinGW-w64 + clang will not compile with -fPIC
	if (NOT WIN32)
		target_compile_options(bz2 PRIVATE -fPIC)
	endif()
endif()

if (MSVC)
    # [Doomseeker] doomseeker.exe in Visual Studio needs /MD, but updater.exe needs /MT.
    add_library( bz2_mt STATIC ${SOURCES} )
    target_compile_options( bz2_mt PRIVATE "/MT$<$<CONFIG:Debug>:d>" )
    target_compile_definitions( bz2_mt PRIVATE -DBZ_NO_STDIO )
    target_include_directories( bz2_mt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR} )
endif()

add_library(BZip2::BZip2 ALIAS bz2)
