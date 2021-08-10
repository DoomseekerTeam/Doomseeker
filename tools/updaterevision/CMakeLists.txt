if( WIN32 )
	if( MSVC_VERSION GREATER 1399 )
		# VC 8+ adds a manifest automatically to the executable. We need to
		# merge ours with it.
		set( MT_MERGE ON )
	else( MSVC_VERSION GREATER 1399 )
		set( TRUSTINFO trustinfo.rc )
	endif()
endif()

add_executable( updaterevision updaterevision.c ${TRUSTINFO} )

if( MT_MERGE )
	add_custom_command(TARGET updaterevision POST_BUILD
		COMMAND mt -inputresource:$<TARGET_FILE:updaterevision> -manifest "${CMAKE_CURRENT_SOURCE_DIR}/trustinfo.txt" -outputresource:$<TARGET_FILE:updaterevision> -nologo
		COMMENT "Embedding trustinfo into updaterevision" )
endif()
