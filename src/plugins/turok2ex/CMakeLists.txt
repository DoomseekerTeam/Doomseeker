set(PLUGIN_NAME turok2ex)
set(PLUGIN_VERSION 8)
include(../PluginHeader.cmake)

set(TUROK2EX_FILES
	turok2exengineplugin.cpp
	turok2exgamehost.cpp
	turok2exgameinfo.cpp
	turok2exgamerunner.cpp
	turok2exmasterclient.cpp
	turok2exserver.cpp
	crc.cpp
	crc.h
)

set(HEADER_FILES
	turok2exengineplugin.h
	turok2exgamehost.h
	turok2exgameinfo.h
	turok2exgamerunner.h
	turok2exmasterclient.h
	turok2exserver.h
)

set(TRANSLATIONS_FILES
	translations/turok2ex_ca_ES.ts
	translations/turok2ex_es_ES.ts
	translations/turok2ex_pl_PL.ts
)

include(../Translation.cmake)

add_library(${PLUGIN_NAME} MODULE ${TUROK2EX_FILES} ${HEADER_FILES} ${QM_FILES})

include(../PluginFooter.cmake)
