#!/bin/bash
#------------------------------------------------------------------------------
# createmacdmg.sh
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
# Copyright (C) 2015 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
#------------------------------------------------------------------------------

# Create/Mount the image
IMAGENAME=$1
mkdir image
hdiutil create -size 300m -srcfolder image -volname Doomseeker -fs HFS+ -format UDRW $IMAGENAME
hdiutil attach -readwrite -noverify -noautoopen $IMAGENAME -mountpoint image

mkdir image/.background
cp -a Doomseeker.app image/Doomseeker.app
cp ../media/Doomseeker-DMG-Background.png image/.background/background.png
osascript makemacdmg.applescript

# Finalize
hdiutil detach ./image
hdiutil convert $IMAGENAME -format UDBZ -o tmp$IMAGENAME
mv {tmp,}$IMAGENAME

rm -r image

