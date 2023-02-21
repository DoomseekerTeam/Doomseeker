#!/bin/bash
#------------------------------------------------------------------------------
# makemacdmg.sh
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
# Copyright (C) 2010 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
#------------------------------------------------------------------------------

declare IP2CFile="$(pwd)/IpToCountry.dat"
if [[ ! -f "$IP2CFile" ]]
then
	echo 'Provide IpToCountry.dat in the current working directory.' >&2
	exit 1
fi

if [[ -z $QT5PATH && -z $QT6PATH ]]
then
	echo 'Provide environment variable QT5PATH or QT6PATH.' >&2
	exit 1
fi

if [[ $QT5PATH ]]; then
	readonly QT_PACKAGE=Qt5
	readonly QT_VERSION=5
	readonly QTPREFIX=$QT5PATH
	readonly OSX_ARCHITECTURES="x86_64"
else
	readonly QT_PACKAGE=Qt6
	readonly QT_VERSION=A
	readonly QTPREFIX=$QT6PATH
	readonly OSX_ARCHITECTURES="x86_64;arm64"
fi

# Find Qt target OS version
readonly QT_MIN_VERSION_TARGET=$(otool -l "$QTPREFIX/lib/QtCore.framework/QtCore" | grep -A 3 LC_VERSION_MIN_MACOSX | awk '$1 == "version" { print $2 }')

# Get the version number
version=$(
	cmake -P /dev/stdin 2>&1 <<-'EOF'
		include("../src/core/versiondefs.cmake")
		message("${VERSION_STRING}")
	EOF
)
if [[ -z $version ]]; then
	echo 'ERROR: Failed to get version number.' >&2
	exit 1
fi

NUM_CPU_CORES=`system_profiler SPHardwareDataType | awk '/Total Number of Cores/ {print $5};'`

echo "Version: $version"
echo "Targeting: $QT_MIN_VERSION_TARGET"
echo "Building with $NUM_CPU_CORES jobs!"
echo

mkdir -p Doomseeker/Doomseeker.app/Contents/MacOS
mkdir Doomseeker/Doomseeker.app/Contents/Frameworks
mkdir Doomseeker/Doomseeker.app/Contents/Resources

# Build
mkdir Doomseeker/build
cd Doomseeker/build
export MACOSX_DEPLOYMENT_TARGET=$QT_MIN_VERSION_TARGET
CMAKE_PREFIX_PATH=$QTPREFIX cmake ../../.. \
	-DCMAKE_OSX_ARCHITECTURES="$OSX_ARCHITECTURES" \
	-DCMAKE_OSX_DEPLOYMENT_TARGET="$MACOSX_DEPLOYMENT_TARGET" \
	-DCMAKE_BUILD_TYPE=Release \
	-DDOOMSEEKER_IP2C_DAT="$IP2CFile" \
	-DQt_PACKAGE="$QT_PACKAGE"
	"$@" || exit

make -j $NUM_CPU_CORES || exit
cd ..

# Detect Qt installation
QTCORE_STRING=`otool -L build/doomseeker | grep QtCore.framework`
QTPATH=`echo $QTCORE_STRING | sed 's,QtCore.framework.*,,'`

QTNTPATH=$QTPREFIX
QTPATH="$QTPREFIX/lib/"
QTPLPATH=${QTPATH}../

echo "Qt Located: $QTPATH"
echo "Qt Plugins: $QTPLPATH"
echo

MODULES=`otool -L build/doomseeker | grep -o 'Qt[A-Za-z]\{1,\}.framework' | sed 's/.framework//'`
# Few more modules used by cocoa plugin
MODULES="$MODULES QtDBus QtPrintSupport"

MODULES_COMMA=`echo $MODULES | sed 's/ /,/g'`

cp {build/,Doomseeker.app/Contents/MacOS/}doomseeker
cp {build/,Doomseeker.app/Contents/Frameworks/}libwadseeker.2.dylib
cp "$IP2CFile" Doomseeker.app/Contents/MacOS/
for i in $MODULES
do
	cp -a ${QTPATH}$i.framework Doomseeker.app/Contents/Frameworks/

	# Remove unneeded development files
	find Doomseeker.app/Contents/Frameworks/$i.framework -name Headers -or -name '*_debug*' -or -name '*.prl' -or -name Current | xargs rm -rf
	rm -rf Doomseeker.app/Contents/Frameworks/$i.framework/Resources

	rm Doomseeker.app/Contents/Frameworks/$i.framework/$i
	ln -s Versions/$QT_VERSION/$i Doomseeker.app/Contents/Frameworks/$i.framework/$i
	ln -s $QT_VERSION Doomseeker.app/Contents/Frameworks/$i.framework/Versions/Current
done
cp -a {${QTPLPATH},Doomseeker.app/Contents/}plugins
find Doomseeker.app/Contents -name '*.dSYM' | xargs rm -rf
find Doomseeker.app/Contents/plugins -name '*_debug*' | xargs rm -rf
cp -R {build/,Doomseeker.app/Contents/MacOS/}engines
cp -R {build/,Doomseeker.app/Contents/MacOS/}translations
cp {${QTPLPATH},Doomseeker.app/Contents/MacOS/}translations/qt_ca.qm
cp {${QTPLPATH},Doomseeker.app/Contents/MacOS/}translations/qt_es.qm
cp {${QTPLPATH},Doomseeker.app/Contents/MacOS/}translations/qt_pl.qm
cp {${QTPLPATH},Doomseeker.app/Contents/MacOS/}translations/qtbase_ca.qm
cp {${QTPLPATH},Doomseeker.app/Contents/MacOS/}translations/qtbase_es.qm
cp {${QTPLPATH},Doomseeker.app/Contents/MacOS/}translations/qtbase_pl.qm
cp {${QTPLPATH},Doomseeker.app/Contents/MacOS/}translations/qtmultimedia_ca.qm
cp {${QTPLPATH},Doomseeker.app/Contents/MacOS/}translations/qtmultimedia_es.qm
cp {${QTPLPATH},Doomseeker.app/Contents/MacOS/}translations/qtmultimedia_pl.qm
cp {../../media/,Doomseeker.app/Contents/}Info.plist
cp {../../media/,Doomseeker.app/Contents/Resources/}icon-osx.icns
cp {../../media/,Doomseeker.app/Contents/Resources/}qt.conf

/usr/libexec/PlistBuddy -c "Set :CFBundleVersion $version" Doomseeker.app/Contents/Info.plist
/usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString $version" Doomseeker.app/Contents/Info.plist

# Get a list of Qt plugins so that we can relink them.
QTPLUGINS_LIST=''
for i in `ls Doomseeker.app/Contents/plugins`
do
	for j in `ls Doomseeker.app/Contents/plugins/${i}`
	do
		QTPLUGINS_LIST="$QTPLUGINS_LIST Doomseeker.app/Contents/plugins/${i}/${j}"
	done
done

RELINK_LIST="`ls Doomseeker.app/Contents/{MacOS/{doomseeker,engines/*.so},Frameworks/libwadseeker.2.dylib}` $QTPLUGINS_LIST"
for i in $RELINK_LIST
do
	if otool -L "$i" | awk '/libwadseeker.2.dylib/{if(!($0 ~ /\.app/)){print $1;found=1}}END{exit !found}' > /dev/null
	then
		install_name_tool -change $(otool -L "$i" | awk '/libwadseeker.2.dylib/{if(!($0 ~ /\.app/)){print $1;exit 0}}') @executable_path/../Frameworks/libwadseeker.2.dylib $i
	fi
done
for i in $MODULES
do
	install_name_tool -id {@executable_path/../,Doomseeker.app/Contents/}Frameworks/${i}.framework/Versions/$QT_VERSION/$i
	for j in `otool -L Doomseeker.app/Contents/Frameworks/${i}.framework/Versions/$QT_VERSION/$i | awk '/architecture/{if(arch)exit 0;arch=1;next}/Qt[A-Za-z]*\.framework/{print $1}'`
	do
		current=`<<<"$j" sed "s,$QTNTPATH,,"`
		install_name_tool -change ${QTNTPATH}${current} @executable_path/../Frameworks/${current} Doomseeker.app/Contents/Frameworks/${i}.framework/Versions/$QT_VERSION/$i
	done
done

install_name_tool -rpath "$PWD/build" @executable_path/../Frameworks Doomseeker.app/Contents/MacOS/doomseeker

rm -r build

cd ..
rm -rf Doomseeker.app
mv Doomseeker/Doomseeker.app Doomseeker.app
rm -r Doomseeker

echo 'Doomseeker.app has been created, please sign, notarize, and then run createmacdmg.sh'
