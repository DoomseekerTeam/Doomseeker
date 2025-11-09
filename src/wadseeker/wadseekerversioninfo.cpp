//------------------------------------------------------------------------------
// wadseekerversioninfo.cpp
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
#include "wadseekerversiondefs.h"
#include "wadseekerversioninfo.h"
#include <QObject>

QString WadseekerVersionInfo::author()
{
	return QObject::tr("The Doomseeker Team");
}

QString WadseekerVersionInfo::description()
{
	return QObject::tr("This library is distributed under the terms of the LGPL v2.1 or later.");
}

QStringList WadseekerVersionInfo::knownWadExtensions()
{
	QStringList list;

	list << "wad";
	list << "pk3";

	return list;
}

bool WadseekerVersionInfo::isSupportedArchiveExtension(const QString &suffix)
{
	QStringList extensions = supportedArchiveExtensions();
	for (const QString &supported : extensions)
	{
		if (suffix.compare(supported, Qt::CaseInsensitive) == 0)
		{
			return true;
		}
	}

	return false;
}

QStringList WadseekerVersionInfo::supportedArchiveExtensions()
{
	QStringList list;

	list << "7z";
	list << "zip";

	return list;
}

QString WadseekerVersionInfo::userAgent()
{
	return "Wadseeker/" + WadseekerVersionInfo::version();
}

QString WadseekerVersionInfo::version()
{
	return VERSION_STRING;
}

QString WadseekerVersionInfo::yearSpan()
{
	return "2009 - 2025";
}
