#!/bin/bash
#------------------------------------------------------------------------------
# makesourcepackages.sh
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
# Copyright (C) 2018 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
#------------------------------------------------------------------------------

set -o pipefail

# Send SIGINT to child process not the shell, so if the user sends SIGINT we
# can still roll back our stack and cleanup
trap "kill -INT -$BASHPID" INT

declare SignEnabled=-1
declare SignGpgKey=""

declare Arg
for Arg in "$@"; do
	if [[ $Arg == "--sign" || $Arg =~ ^--sign= ]]; then
		SignEnabled=1
		SignGpgKey=${Arg:7}
	elif [[ $Arg == "--no-sign" ]]; then
		SignEnabled=0
	else
		echo "ERROR: Unknown option $Arg" >&2
		exit 1
	fi
done

if (( SignEnabled < 0 )); then
	echo 'Specify --sign or --no-sign.' >&2
	exit 1
fi

# Process a cmake file and print the value of a given variable
function cmake_extract_var
{
	declare CMVarsFile=$1
	shift
	declare Variable=$1
	shift

	# CMake's message command writes to stderr
	cmake -P /dev/stdin <<-EOF 2>&1
		include($CMVarsFile)
		message(\${$Variable})
	EOF
}

# Build revision_check to pack in revision version information
function create_vcs_info
{
	declare SrcDir=$(realpath "$1")
	shift

	declare BuildDir
	if ! BuildDir=$(mktemp -d -p . build.XXXXXXXXXX); then
		echo 'Failed to create temporary build directory.' >&2
		exit 1
	fi

	(
		cd "$BuildDir" &&
			cmake "$SrcDir" &&
			cmake --build . -- revision_check
	)
	declare Ret=$?

	rm -rf "$BuildDir"
	return "$Ret"
}


function sign_archive
{
	declare FileName=$1
	shift

	if (( ! SignEnabled )); then
		return 0
	fi

	declare -a SignOpts=(-v)
	if [[ $SignGpgKey ]]; then
		SignOpts+=(-u "$SignGpgKey")
	fi

	echo "Signing $FileName..."
	rm -f "$FileName.sig"
	gpg "${SignOpts[@]}" --output "$FileName.sig" --detach-sig "$FileName"
	declare Ret=$?

	if (( Ret != 0 )); then
		echo "Failed to sign $FileName!" >&2
	fi

	return "$Ret"
}

declare doomseekerVersion=$(cmake_extract_var ../src/core/versiondefs.cmake VERSION_STRING)
declare wadseekerVersion=$(cmake_extract_var ../src/wadseeker/wadseekerversiondefs.cmake VERSION_STRING)

# Strip off beta tag
doomseekerVersion=${doomseekerVersion%~*}
wadseekerVersion=${wadseekerVersion%~*}

readonly doomseekerArchiveName="doomseeker-$doomseekerVersion"
readonly wadseekerArchiveName="libwadseeker-$wadseekerVersion"

echo "Doomseeker version: $doomseekerVersion"
echo "wadseeker version: $wadseekerVersion"

if ! hg archive "$doomseekerArchiveName"; then
	echo 'Failed to call hg archive! Is this a mercurial clone?' >&2
	exit 1
fi

if ! create_vcs_info "$doomseekerArchiveName"; then
	echo 'Failed to create vcs revision info file for archive.' >&2
	rm -rf "$doomseekerArchiveName"
	exit 1
fi

declare Error=0
echo 'Creating Doomseeker archive...'
if ! tar -c "$doomseekerArchiveName" --owner=doomseeker --group=doomseeker | xz -9 > "$doomseekerArchiveName.tar.xz"; then
	echo 'Failed to create Doomseeker archive!' >&2
	Error=1
else
	echo 'Adding licence and dependencies for independent building...'
	cp "$doomseekerArchiveName/LICENSE" "$doomseekerArchiveName/src/wadseeker"
	cp "$doomseekerArchiveName/LICENSE.json" "$doomseekerArchiveName/src/wadseeker"
	cp -r "$doomseekerArchiveName/cmake" "$doomseekerArchiveName/src/wadseeker"
	cp -r "$doomseekerArchiveName/dependencies" "$doomseekerArchiveName/src/wadseeker"

	echo 'Creating Wadseeker archive...'
	if ! tar -cC "$doomseekerArchiveName/src" wadseeker --transform "s,^wadseeker,$wadseekerArchiveName," --owner=doomseeker --group=doomseeker | xz -9 > "$wadseekerArchiveName.tar.xz"; then
		echo 'Failed to create Wadseeker archive!' >&2
		Error=1
	fi
fi

if ! sign_archive "$doomseekerArchiveName.tar.xz" || ! sign_archive "$wadseekerArchiveName.tar.xz"; then
	Error=1
fi

rm -rf "$doomseekerArchiveName"

exit "$Error"
