//------------------------------------------------------------------------------
// version.cpp
//------------------------------------------------------------------------------
//
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
//
//------------------------------------------------------------------------------
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "version.h"
#include "versiondefs.h"

#include <QSysInfo>

QString Version::changeset()
{
	return HG_REVISION_HASH_STRING;
}

QString Version::name()
{
	return "Doomseeker";
}

QString Version::operatingSystem()
{
	#ifdef Q_OS_WIN32
	return QString("Windows");
	#elif defined(Q_OS_LINUX)
	return QString("Linux");
	#elif defined(Q_OS_DARWIN)
	return QString("MAC");
	#else
	return QSysInfo::prettyProductName();
	#endif
}

QString Version::qtPackageVersion()
{
	return QString("%1-%2").arg(qVersion()).arg("mingw-1");
}

QString Version::revision()
{
	return HG_TIME;
}

unsigned long long Version::revisionNumber()
{
	return HG_REVISION_NUMBER;
}

QString Version::userAgent()
{
	return "Doomseeker/" + versionRevision();
}

QString Version::version()
{
	return VERSION_STRING;
}

QString Version::versionRevision()
{
	if (revision().isEmpty())
	{
		return version();
	}
	else
	{
		return version() + "-" + revision();
	}
}

QString Version::yearSpan()
{
	return "2009 - 2019";
}
