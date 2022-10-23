//------------------------------------------------------------------------------
// doomseekerfilepaths.cpp
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
#include "doomseekerfilepaths.h"

#include "datapaths.h"
#include "strings.hpp"
#include <QFile>

static const QString DATA_SEARCH_PREFIX = "data";

DataPaths *DoomseekerFilePaths::pDataPaths = nullptr;

const QString DoomseekerFilePaths::CACERTS_FILENAME = "cacerts.pem";
const QString DoomseekerFilePaths::IP2C_DATABASE_FILENAME = "IpToCountry.dat";
const QString DoomseekerFilePaths::IP2C_QT_SEARCH_PATH = DATA_SEARCH_PREFIX + ":" + IP2C_DATABASE_FILENAME;
const QString DoomseekerFilePaths::TEMP_SERVER_CONFIG_FILENAME = "tmpserver.cfg";
const QString DoomseekerFilePaths::INI_FILENAME = "doomseeker.ini";
const QString DoomseekerFilePaths::IRC_INI_FILENAME = "doomseeker-irc.ini";
const QString DoomseekerFilePaths::PASSWORD_INI_FILENAME = "doomseeker-password.ini";

QString DoomseekerFilePaths::cacerts()
{
	QStringList paths = pDataPaths->staticDataSearchDirs(CACERTS_FILENAME);
	for (const QString &path : paths)
	{
		if (QFile(path).exists())
			return path;
	}
	return QString();
}

QString DoomseekerFilePaths::ini()
{
	return joinIfNeitherEmpty(pDataPaths->programsDataDirectoryPath(), INI_FILENAME);
}

QString DoomseekerFilePaths::ircIni()
{
	return joinIfNeitherEmpty(pDataPaths->programsDataDirectoryPath(), IRC_INI_FILENAME);
}

QString DoomseekerFilePaths::ip2cDatabaseAny()
{
	for (const QString &searchPath : QDir::searchPaths(DATA_SEARCH_PREFIX))
	{
		QString path = Strings::combinePaths(searchPath, IP2C_DATABASE_FILENAME);
		if (QFile(path).exists())
			return path;
	}
	return QString();
}

QString DoomseekerFilePaths::ip2cDatabase()
{
	return joinIfNeitherEmpty(pDataPaths->localDataLocationPath(), IP2C_DATABASE_FILENAME);
}

QString DoomseekerFilePaths::joinIfNeitherEmpty(const QString &left, const QString &right)
{
	if (left.trimmed().isEmpty() || right.trimmed().isEmpty())
		return QString();
	return left + "/" + right;
}

QString DoomseekerFilePaths::passwordIni()
{
	return joinIfNeitherEmpty(pDataPaths->programsDataDirectoryPath(), PASSWORD_INI_FILENAME);
}

QString DoomseekerFilePaths::tempServerConfig()
{
	return joinIfNeitherEmpty(pDataPaths->programsDataDirectoryPath(), TEMP_SERVER_CONFIG_FILENAME);
}

QString DoomseekerFilePaths::updatePackagesStorageDir()
{
	return pDataPaths->localDataLocationPath(DataPaths::UPDATE_PACKAGES_DIR_NAME);
}
