//------------------------------------------------------------------------------
// capsqt.cpp
//------------------------------------------------------------------------------
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301  USA
//------------------------------------------------------------------------------
// Copyright (C) 2023 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "capsqt.h"

#include <QtGlobal>
#include <QString>
#include <QStringList>

// Fix "error: does not have any field named 'gnu_dev_major'"
// https://bugzilla.redhat.com/show_bug.cgi?id=130601
#undef major
#undef minor

namespace
{
struct VersionNumber
{
	int major;
	int minor;

	VersionNumber(int major = 0, int minor = 0) : major(major), minor(minor) {}

	/**
	 * @return
	 * - negative if this is lesser
	 * - 0 if both are equal
	 * - positive if this is greater
	 */
	int compare(const VersionNumber &other) const
	{
		int major = this->major - other.major;
		if (major != 0)
			return major;
		return this->minor - other.minor;
	}
};
}

static const VersionNumber &runtimeQtVersion()
{
	static const VersionNumber version = [](){
		QString versionQt = qVersion();
		QStringList tokens = versionQt.split(".");
		VersionNumber version;
		if (tokens.size() >= 1)
		{
			version.major = tokens[0].toInt();
		}
		if (tokens.size() >= 2)
		{
			version.minor = tokens[1].toInt();
		}
		return version;
	}();
	return version;
}

bool CapsQt::canCssCellBorders()
{
	static const VersionNumber minVersion(5, 14);
	static bool can = runtimeQtVersion().compare(minVersion) >= 0;
	return can;
}
