//------------------------------------------------------------------------------
// doomseekerfilepaths.h
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
#ifndef __DOOMSEEKERFILEPATHS_H__
#define __DOOMSEEKERFILEPATHS_H__

#include "global.h"
#include <QString>

class DataPaths;

/**
 * @ingroup group_pluginapi
 * @brief Static class responsible for retrieving paths to data files used
 *        by Doomseeker.
 *
 * This uses a static instance of DataPaths class through pDataPaths member,
 * which should be specified during init phase.
 */
class MAIN_EXPORT DoomseekerFilePaths
{
public:
	static const QString CACERTS_FILENAME;
	static const QString IP2C_DATABASE_FILENAME;

	/// @deprecated not used anymore; delete
	static const QString IP2C_QT_SEARCH_PATH;

	static const QString TEMP_SERVER_CONFIG_FILENAME;
	static const QString INI_FILENAME;
	static const QString IRC_INI_FILENAME;
	static const QString PASSWORD_INI_FILENAME;

	static DataPaths *pDataPaths;

	static QString cacerts();
	static QString ini();
	static QString ircIni();
	/**
	 * This is any IP2C database available in the order of usual
	 * loading preference.
	 *
	 * @return The return value can vary depending on where
	 * in the file system the file resides. If it's nowhere
	 * to be found, empty string is returned.
	 */
	static QString ip2cDatabaseAny();
	/**
	 * This always points to the location of the locally downloaded
	 * database regardless if the file exists or not.
	 */
	static QString ip2cDatabase();
	static QString passwordIni();
	static QString tempServerConfig();
	static QString updatePackagesStorageDir();

private:
	static QString joinIfNeitherEmpty(const QString &left, const QString &right);
};

#endif
