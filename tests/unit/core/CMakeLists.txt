add_unit_test(datetime LIBRARIES doomseeker_core)
add_unit_test(gamedemo LIBRARIES doomseeker_core)
add_unit_test(modreader
	LIBRARIES doomseeker_core
	SOURCES _testdata/modreader/resources.qrc
)
add_unit_test(strings LIBRARIES doomseeker_core)
add_unit_test(templatedpathresolver LIBRARIES doomseeker_core)
add_unit_test(utf8splitter LIBRARIES doomseeker_core)

add_subdirectory(ini)
add_subdirectory(irc)
add_subdirectory(pathfinder)
add_subdirectory(serverapi)
