#!/bin/bash
#------------------------------------------------------------------------------
# mergemacdmg.sh
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

# Merges the Qt4 and Qt5 version of Doomseeker
# Usage:
#  ./mergemacdmg.sh <qt4path> <qt5path>

QT4=$1
shift
QT5=$1
shift

if [[ -z $QT4 || -z $QT5 ]]; then
	echo 'Pass path to Qt4 and Qt5 installs.' >&2
	exit 1
fi

rm -rf Qt4 Qt5 Doomseeker.app Doomseeker

CC="${QT4_CC}" CXX="${QT4_CXX}" ./makemacdmg.sh -DQT_QMAKE_EXECUTABLE="$QT4/bin/qmake" "$@" || exit
mkdir Qt4
mv Doomseeker*.dmg Qt4/
mv Doomseeker.app Qt4/

QT5PATH="$QT5" CC="${QT5_CC}" CXX="${QT5_CXX}" ./makemacdmg.sh "$@" || exit
mkdir Qt5
mv Doomseeker*.dmg Qt5/
mv Doomseeker.app Qt5/

cp -a Qt4/Doomseeker.app Doomseeker.app
cp -a Qt5/Doomseeker.app/Contents/Frameworks/Qt* Doomseeker.app/Contents/Frameworks/

function split_qt4_universal()
{
	declare File=$1
	shift
	echo "Splitting $File"

	lipo $File -thin ppc -output $File.ppc
	if [ ! -f $File.ppc ]
	then
		lipo $File -thin ppc7400 -output $File.ppc
	fi
	lipo $File -thin i386 -output $File.i386
	rm $File
}

function merge_qt5()
{
	declare File=$1
	shift
	declare Qt5File=$1
	shift

	declare Parts=()

	cp $Qt5File $File.x86_64
	for i in $File.*
	do
		Parts+=(-arch ${i##*.} $i)
	done
	echo lipo -create ${Parts[@]} -output $File
	lipo -create ${Parts[@]} -output $File
	rm $File.*
}

function flatten_framework()
{
	declare Framework=$1
	shift
	echo "---- Flattening $Framework ----"

	declare Qt4Framework=$Framework/Versions/4
	declare Qt5Framework=$Framework/Versions/5

	find $Framework -mindepth 1 -maxdepth 1 -not -type d | xargs rm -f

	if [[ ! -d $Qt4Framework ]]
	then
		mv -v $Qt5Framework/* $Framework
		rm -vrf $Framework/Versions
		return
	fi

	mv -v $Qt4Framework/* $Framework
	find $Qt5Framework -type f | while read -r Filename
	do
		Filename=${Filename/*Versions\/5\//}
		echo $Filename
		mkdir -p $(dirname $Framework/$Filename)
		if [[ -f $Framework/$Filename ]] && file $Qt5Framework/$Filename | grep Mach > /dev/null; then
			split_qt4_universal $Framework/$Filename
			merge_qt5 $Framework/$Filename $Qt5Framework/$Filename
		else
			mv $Qt5Framework/$Filename $Framework/$Filename
		fi
	done
	# Contents directory not allowed with code signing
	rm -vrf $Framework/Versions $Framework/Contents
}

function mergebin()
{
	split_qt4_universal $1
	merge_qt5 $1 Qt5/$1
}

# Merge our binaries
mergebin Doomseeker.app/Contents/Frameworks/libwadseeker.1.dylib
mergebin Doomseeker.app/Contents/MacOS/doomseeker
mergebin Doomseeker.app/Contents/MacOS/updater
for i in `ls Doomseeker.app/Contents/MacOS/engines`
do
	mergebin Doomseeker.app/Contents/MacOS/engines/$i
done

# Flatten and merge frameworks for signing
for i in Doomseeker.app/Contents/Frameworks/Qt*
do
	flatten_framework $i
done

# Copy new plugin directories
(
	cd Qt5
	find Doomseeker.app/Contents/plugins -type d -exec mkdir -p "../{}" \;
)

# Merge common plugins
for i in `comm -12 <(cd Qt4; find Doomseeker.app/Contents/plugins -name *.dylib | sort) <(cd Qt5; find Doomseeker.app/Contents/plugins -name *.dylib | sort)`
do
	mergebin $i
done
# Copy new plugins
for i in `comm -13 <(cd Qt4; find Doomseeker.app/Contents/plugins -name *.dylib | sort) <(cd Qt5; find Doomseeker.app/Contents/plugins -name *.dylib | sort)`
do
	cp Qt5/$i $i
done

exit 0
./createmacdmg.sh `(cd Qt5;echo *.dmg)`
