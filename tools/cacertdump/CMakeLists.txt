# Public Domain. Authored by Zalewa <zalewapl@gmail.com>, 2017.
cmake_minimum_required(VERSION 3.5...4.0)
if(CMAKE_VERSION VERSION_LESS 3.12)
	cmake_policy(VERSION ${CMAKE_VERSION})
endif()

project(cacertdump)

set(Qt_PACKAGE "Qt5" CACHE STRING "Qt package name")

find_package(${Qt_PACKAGE} COMPONENTS Core Network REQUIRED)
add_executable(cacertdump cacertdump.cpp)
target_link_libraries(cacertdump ${Qt_PACKAGE}::Core ${Qt_PACKAGE}::Network)
