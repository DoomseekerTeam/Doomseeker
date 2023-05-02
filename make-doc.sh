#!/bin/bash
#------------------------------------------------------------------------------
# make-doc.sh
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
#------------------------------------------------------------------------------
# Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
#------------------------------------------------------------------------------
set -e

if [ $# -lt 1 ]; then
	echo "Usage: `basename $0` <doxyconffile> [-d]"
	echo "    -d - debug"
	exit 1
fi

doxyfile=$1
tmpdoxyfile="/tmp/doomseekerdoc.doxycfg"
outputdir="/tmp/doomseekerdoc"
versiondefs="src/core/versiondefs.cmake"
project="doomseeker"
if [[ $doxyfile == *"wadseeker"* ]]; then
	project="wadseeker"
	versiondefs="src/wadseeker/wadseekerversiondefs.cmake"
fi
debug=0

if [ $# -ge 2 ]; then
	if [ $2 == '-d' ]; then
		debug=1
	fi
fi

if [ ! -f $versiondefs ]; then
	echo "file $versiondefs doesn't exist."
	echo "make sure you run this script from repository root directory"
	exit 2
fi

function strip_whitespace {
	stripped=`echo -n $1 | sed -e 's/^[[:space:]].*//' | sed -e 's/[[:space:]]$//'`
	echo -n $stripped
}

# final sed trims whitespace and quotation
version=`grep VERSION_STRING "$versiondefs" | cut -d'"' -f2`
version=`strip_whitespace $version`
cp "$1" "$tmpdoxyfile"
sed -i -r 's/PROJECT_NUMBER\s*=.*/PROJECT_NUMBER = '"$version"'/' $tmpdoxyfile
sed -i -r 's:OUTPUT_DIRECTORY\s*=.*:OUTPUT_DIRECTORY = '"$outputdir"':' $tmpdoxyfile
if [ $debug -ne 0 ]; then
	sed -i -r 's/HTML_FILE_EXTENSION\s*=.*/HTML_FILE_EXTENSION = .html/' $tmpdoxyfile
	sed -i -r 's/HTML_HEADER\s*=.*//' $tmpdoxyfile
	sed -i -r 's/HTML_FOOTER\s*=.*//' $tmpdoxyfile
fi

if [ $debug -eq 0 ]; then
	if [ -d "$outputdir/html" ]; then
		rm -rf "$outputdir/html"
	fi
fi

mkdir -p $outputdir
doxygen "$tmpdoxyfile"
exitcode=$?

if [ $debug -eq 0 ]; then
	pushd $outputdir
	pkg_name="${project}_${version}"
	if [ -d "./$pkg_name" ]; then
		rm -rf "./$pkg_name"
	fi
	mv html $pkg_name
	tar -cvzf "$pkg_name.tar.gz" "$pkg_name"
	popd
	echo "Created doc package $pkg_name.tar.gz"
fi

if [ $exitcode -eq 0 ]; then
	rm "$tmpdoxyfile"
	echo "Doc created at '$outputdir'"
else
	exit $exitcode
fi
