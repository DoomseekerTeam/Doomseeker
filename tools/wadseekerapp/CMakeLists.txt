#------------------------------------------------------------------------------
# CMakeLists.txt
#------------------------------------------------------------------------------
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301  USA
#------------------------------------------------------------------------------
# Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
#------------------------------------------------------------------------------
project(WadseekerApp)

cmake_minimum_required(VERSION 3.5.1)  # Ubuntu 16.04

find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(wadseeker CONFIG REQUIRED)

add_executable(wadseeker main.cpp)
set_target_properties(wadseeker PROPERTIES
	CXX_STANDARD 11
	AUTOMOC 1)
target_link_libraries(wadseeker Qt5::Core Wadseeker::wadseeker)
