#------------------------------------------------------------------------------
# versiondefs.cmake
#------------------------------------------------------------------------------
#
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
#
#------------------------------------------------------------------------------
# Copyright (C) 2017, 2018 The Doomseeker Team
#------------------------------------------------------------------------------

# This is a centralized location to store all version information. Make sure
# to update this file before making a new tag or a new release.

# Info used by version.cpp
set(VERSION_STRING "1.5.2")

# Info used by windows.rc
set(FILEVERSION_DEF "1,5,2,HG_REVISION_NUMBER")
set(PRODUCTVERSION_DEF "1,5,2,HG_REVISION_NUMBER")
set(FILEVERSION_STR_DEF "1.5.2.0")
set(PRODUCTVERSION_STR_DEF "1.5.2.0")
set(LEGALCOPYRIGHT_DEF "The Doomseeker Team 2009 - 2025")
