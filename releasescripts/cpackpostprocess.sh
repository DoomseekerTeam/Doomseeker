#!/bin/bash
#------------------------------------------------------------------------------
# cpackpostprocess.sh
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
# Copyright (C) 2017 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
#------------------------------------------------------------------------------
#
# This script does a recursive scan of the current directory and finds files
# named CPackComponent.version which we create. These files give the package
# name and what we think the version should be. Next it finds the latest
# package in the current directory with a matching name and corrects the
# control file and the file name.

set -e

if [[ ! -f CMakeCache.txt ]]
then
	echo 'Run this script from the build directory after running cpack.' >&2
	exit 1
fi

# Set to 1 if the IpToCountry.dat file is found. With the current setup we can
# assume that we stumble across it while doing version fixup instead of having
# a pass specifically for it.
declare FoundIP2CDat=0

declare -A FixupVersions

while read -r FileName
do
	while read -r -a Entry
	do
		# Only modify the latest matching package so we can keep the old packages in the directory.
		if DebName=$(ls -t | grep -m1 "${Entry[0]}_.*\\.deb")
		then
			# Convert to full path so we can access it from the temp directory
			DebName=$(realpath "$DebName")
			TempDir=$(mktemp -d)

			pushd . > /dev/null
			cd "$TempDir"

			# Unpack the deb
			ar -x "$DebName"
			mkdir control
			cd control
			tar xf ../control.tar.gz

			# While we're at it: Look for IPToCountry.dat
			if tar -tf ../data.tar.xz --wildcards '*/IpToCountry.dat' &>/dev/null; then
				FoundIP2CDat=1
			fi

			# Sanity check our package against what we thought the name was
			if grep -qP "^Package: ${Entry[0]}\$" control
			then
				# Grab arch out of the control file since it's easier than parsing the filename
				Arch=$(grep -oP '(?<=Architecture: )(.*)$' control)
				DestName="${Entry[0]}_${Entry[1]}_$Arch.deb"

				sed -i "s/^Version:.*\$/Version: ${Entry[1]}/" control

				# Repack our control
				fakeroot tar c * | gzip -9 > ../control.tar.gz
				cd ..
				rm -rf control

				# Build new deb
				fakeroot ar rc "$DestName" debian-binary control.tar.gz data.tar.*

				# We want to delete the old file and show what the new file is
				# named. Easiest way to do that is to overwrite the old file
				# and move it to the new name letting mv show if anything
				# has changed or not.
				popd > /dev/null
				mv "$TempDir/$DestName" "$DebName"
				mv -v "$DebName" "$DestName" || true
			else
				echo "ERROR: Package for ${Entry[0]} didn't match ($DebName)" >&2
				popd > /dev/null
			fi
			rm -rf "$TempDir"
		fi
	done < "$FileName"
done < <(find . -name 'CPackComponent.version')

if (( ! FoundIP2CDat )); then
	echo 'ERROR: Failed to find IPToCountry.dat. Unless the omission was intentional check DOOMSEEKER_IP2C_DAT in your CMake config.' >&2
	exit 1
fi
