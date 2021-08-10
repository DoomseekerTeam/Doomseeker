# Public Domain. Authored by Zalewa <zalewapl@gmail.com>, 2017.
cmake_minimum_required(VERSION 3.5...3.21)
project(cacertdump)
find_package(Qt5 COMPONENTS Core Network REQUIRED)
add_executable(cacertdump cacertdump.cpp)
target_link_libraries(cacertdump Qt5::Core Qt5::Network)
