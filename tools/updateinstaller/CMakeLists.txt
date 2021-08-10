project(updater)

cmake_minimum_required(VERSION 3.5...3.21)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# If the SIGN_UPDATER option is enabled, the updater.exe
# binary is digitally signed on Windows after it has been built.
# A script called sign-updater.bat must exist in the PATH or
# the directory where updater.exe is built. This will be run
# with the path of the binary to sign it.
option(SIGN_UPDATER OFF)

include_directories(external)
include_directories(external/TinyThread/source)

if (WIN32)
	include_directories(external/win32cpp/include)

	# [Doomseeker]
	# Link the updater binary statically with the Visual C++ runtime
	# so that the executable can function standalone.
	# Note: it's very important that all library dependencies
	# are compiled with /MT flag.
	include(${CMAKE_SOURCE_DIR}/cmake/Modules/Macros.cmake)
	msvc_mt()
else()
	# optimize for reduced code size
	set(CMAKE_CXX_FLAGS_RELEASE "-Os")
	set(CMAKE_C_FLAGS_RELEASE "-Os")
endif()

#[BL] This stuff isn't needed since we take care of this in the root CMakeLists.
#if (APPLE)
	# Build the updater as a dual 32/64bit binary.  If only one architecture
	# is required, removing the other architecture will reduce the size
	# of the updater binary
#	set(CMAKE_OSX_ARCHITECTURES i386;x86_64)

	# Build the updater so that it works on OS X 10.5 and above.
#	set(CMAKE_OSX_DEPLOYMENT_TARGET 10.5)

	# Set the SDK which the updater is built against.
	# The available SDK versions are those which exist in /Developer/SDKs/
#	set(CMAKE_OSX_SDK 10.7)

#	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
#	set(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX${CMAKE_OSX_SDK}.sdk")
#endif()

add_subdirectory(src)
add_subdirectory(external/AnyOption)
add_subdirectory(external/minizip)
add_subdirectory(external/tinyxml)
add_subdirectory(external/TinyThread)

