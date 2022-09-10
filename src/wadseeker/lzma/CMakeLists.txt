set( LZMA_FILES
	C/7zAlloc.c
	C/7zArcIn.c
	C/7zBuf.c
	C/7zBuf2.c
	C/7zCrc.c
	C/7zCrcOpt.c
	C/7zDec.c
	C/7zFile.c
	C/7zStream.c
	C/Aes.c
	C/AesOpt.c
	C/Alloc.c
	C/Bcj2.c
	C/Bra.c
	C/Bra86.c
	C/BraIA64.c
	C/CpuArch.c
	C/Delta.c
	C/DllSecur.c
	C/LzFind.c
	C/LzFindOpt.c
	C/Lzma2Dec.c
	C/Lzma2DecMt.c
	C/Lzma2Enc.c
	C/Lzma86Dec.c
	C/Lzma86Enc.c
	C/LzmaDec.c
	C/LzmaEnc.c
	C/LzmaLib.c
	C/Ppmd7.c
	C/Ppmd7Dec.c
	C/Ppmd7Enc.c
	C/Sha256.c
	C/Sha256Opt.c
	C/Sort.c
	C/Xz.c
	C/XzCrc64.c
	C/XzCrc64Opt.c
	C/XzDec.c
	C/XzEnc.c
	C/XzIn.c  )

if (WIN32)
	set (LZMA_FILES
		${LZMA_FILES}
		C/Bcj2Enc.c
		C/LzFindMt.c
		C/MtCoder.c
		C/MtDec.c
		C/Threads.c
		)
endif ()

add_library( lzma STATIC ${LZMA_FILES})
if (NOT WIN32)
	target_compile_definitions( lzma PRIVATE "-D_7ZIP_ST" )
endif ()

target_include_directories( lzma INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/C )

if( CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang" )
	target_compile_options( lzma PRIVATE -Wall -Wextra -fomit-frame-pointer)
	# [Doomseeker] MinGW-w64 + clang will not compile with -fPIC
	if (NOT WIN32)
		target_compile_options( lzma PRIVATE -fPIC )
	endif()
endif()
