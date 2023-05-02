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

SDK_10_4=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.4u.sdk
SDK_10_9=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk

declare IP2CFile="$(pwd)/IpToCountry.dat"
if [[ ! -f "$IP2CFile" ]]
then
	echo 'Provide IpToCountry.dat in the current working directory.' >&2
	exit 1
fi

if [ -z $QT5PATH ]
then
	echo "Building Qt4 version."
	QT_VERSION=4
else
	echo "Building Qt5 version."
	QT_VERSION=5
fi

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
echo "Building with $NUM_CPU_CORES jobs!"
echo

mkdir -p Doomseeker/Doomseeker.app/Contents/MacOS
mkdir Doomseeker/Doomseeker.app/Contents/Frameworks
mkdir Doomseeker/Doomseeker.app/Contents/Resources

# Build
mkdir Doomseeker/build
cd Doomseeker/build
if [ -z $QT5PATH ]
then
	export MACOSX_DEPLOYMENT_TARGET=10.4
	echo CC="${CC:-/usr/bin/gcc-4.0}" CXX="${CXX:-/usr/bin/g++-4.0}" cmake ../../.. -DFORCE_QT4=YES "$@" || exit
	CC="${CC:-/usr/bin/gcc-4.0}" CXX="${CXX:-/usr/bin/g++-4.0}" cmake ../../.. -DFORCE_QT4=YES "$@" || exit
	cmake ../../.. -DCMAKE_OSX_ARCHITECTURES="ppc;i386" -DCMAKE_OSX_DEPLOYMENT_TARGET=$MACOSX_DEPLOYMENT_TARGET -DCMAKE_OSX_SYSROOT=${SDK_10_4} -DCMAKE_BUILD_TYPE=Release -DFORCE_QT4=YES -DCMAKE_SKIP_RPATH=ON -DDOOMSEEKER_IP2C_DAT="$IP2CFile" "$@" || exit
else
	export MACOSX_DEPLOYMENT_TARGET=10.9
	CMAKE_PREFIX_PATH=$QT5PATH cmake ../../.. "$@" || exit
	CMAKE_PREFIX_PATH=$QT5PATH cmake ../../.. -DCMAKE_OSX_ARCHITECTURES="x86_64" -DCMAKE_OSX_DEPLOYMENT_TARGET=$MACOSX_DEPLOYMENT_TARGET -DCMAKE_OSX_SYSROOT=$SDK_10_9 -DCMAKE_BUILD_TYPE=Release -DDOOMSEEKER_IP2C_DAT="$IP2CFile" "$@" || exit
fi
make -j $NUM_CPU_CORES || exit
./doomseeker --version-json version.js
plutil -convert xml1 version.js
DISPLAYVERSION=`/usr/libexec/PlistBuddy -c Print:doomseeker:display-version version.js`
cd ..

# Detect Qt installation
QTCORE_STRING=`otool -L build/doomseeker | grep QtCore.framework`
QTPATH=`echo $QTCORE_STRING | sed 's,QtCore.framework.*,,'`

QTNTPATH=$QTPATH
if [ "$QT5PATH" ]
then
	QTPATH="$QT5PATH/lib/"
fi

QTPLPATH=${QTPATH}../
if [ -z "$QTPATH" ]
then
	QTPATH=/Library/Frameworks/
	QTPLPATH=/Developer/Applications/Qt/
fi

if [ -d "${QTPATH}../hybridlib" ]
then
	# Blzut3 merged PowerPC Qt 4.7 and i386 Qt 4.8 into a single library which
	# works when compiled against 4.7. This pulls in the proper library.
	QTPATH=`echo $QTPATH | sed 's,/lib,/hybridlib,'`
	#QTPLPATH=`echo $QTPLPATH | sed 's,/plugins,/hybridplugins,'`
fi

echo "Qt Located: $QTPATH"
echo "Qt Plugins: $QTPLPATH"
echo

MODULES=`otool -L build/doomseeker | grep -o 'Qt[A-Za-z]\{1,\}.framework' | sed 's/.framework//'`
if [ "$QT5PATH" ]
then
	# Few more modules used by cocoa plugin
	MODULES="$MODULES QtDBus QtPrintSupport"
fi
MODULES_COMMA=`echo $MODULES | sed 's/ /,/g'`

cp {build/,Doomseeker.app/Contents/MacOS/}doomseeker
cp {build/,Doomseeker.app/Contents/Frameworks/}libwadseeker.1.dylib
cp "$IP2CFile" Doomseeker.app/Contents/MacOS/
for i in $MODULES
do
	cp -a ${QTPATH}$i.framework Doomseeker.app/Contents/Frameworks/

	# Qt4 Content directory should probably be Resources
	if [[ -z "$QT5PATH" ]] && [[ -d Doomseeker.app/Contents/Frameworks/$i.framework/Contents ]]
	then
		mv Doomseeker.app/Contents/Frameworks/$i.framework/{Contents,Resources}
	fi

	# Remove unneeded development files
	find Doomseeker.app/Contents/Frameworks/$i.framework -name Headers -or -name '*_debug*' -or -name '*.prl' -or -name Current | xargs rm -rf
	rm -rf Doomseeker.app/Contents/Frameworks/$i.framework/Resources
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
/usr/libexec/PlistBuddy -c "Set :CFBundleLongVersionString $DISPLAYVERSION" Doomseeker.app/Contents/Info.plist

# For non-Qt5 remove any ppc64 or x86_64 libraries there may be.
if [ -z "$QT5PATH" ]
then
	BINFILES=`find Doomseeker.app -type f -exec sh -c "file -I '{}' | grep -q 'octet-stream'" \; -print`
	for i in $BINFILES
	do
		if lipo -info $i 2> /dev/null | grep -E '(ppc64|x86_64)' > /dev/null
		then
			lipo -remove ppc64 -remove x86_64 $i -output $i
		fi
	done
fi

# Get a list of Qt plugins so that we can relink them.
QTPLUGINS_LIST=''
for i in `ls Doomseeker.app/Contents/plugins`
do
	for j in `ls Doomseeker.app/Contents/plugins/${i}`
	do
		QTPLUGINS_LIST="$QTPLUGINS_LIST Doomseeker.app/Contents/plugins/${i}/${j}"
	done
done

RELINK_LIST="`ls Doomseeker.app/Contents/{MacOS/{doomseeker,engines/*.so},Frameworks/libwadseeker.1.dylib}` $QTPLUGINS_LIST"
for i in $RELINK_LIST
do
	if otool -L "$i" | awk '/libwadseeker.1.dylib/{if(!($0 ~ /\.app/)){print $1;found=1}}END{exit !found}' > /dev/null
	then
		install_name_tool -change $(otool -L "$i" | awk '/libwadseeker.1.dylib/{if(!($0 ~ /\.app/)){print $1;exit 0}}') @executable_path/../Frameworks/libwadseeker.1.dylib $i
	fi
done
for i in $MODULES
do
	install_name_tool -id {@executable_path/../,Doomseeker.app/Contents/}Frameworks/${i}.framework/$i
	for j in `otool -L Doomseeker.app/Contents/Frameworks/${i}.framework/Versions/$QT_VERSION/$i | awk '/architecture/{if(arch)exit 0;arch=1;next}/Qt[A-Za-z]*\.framework/{print $1}'`
	do
		current=`<<<"$j" sed "s,$QTNTPATH,,"`
		install_name_tool -change ${QTNTPATH}${current} @executable_path/../Frameworks/${current/Versions\/$QT_VERSION\//} Doomseeker.app/Contents/Frameworks/${i}.framework/Versions/$QT_VERSION/$i
	done
	for j in $RELINK_LIST
	do
		install_name_tool -change ${QTNTPATH}${i}.framework/Versions/$QT_VERSION/$i @executable_path/../Frameworks/${i}.framework/$i $j
	done
done

rm -r build

cd ..
mv Doomseeker/Doomseeker.app Doomseeker.app
rm -r Doomseeker

./createmacdmg.sh Doomseeker-${version}${tag}.dmg
