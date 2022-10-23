//------------------------------------------------------------------------------
// updatepackage.h
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_UPDATER_UPDATERPACKAGE_H
#define DOOMSEEKER_UPDATER_UPDATERPACKAGE_H

#include <QString>
#include <QUrl>

class UpdatePackage
{
public:
	/**
	 * @brief Currently installed version, displayed to the user.
	 *
	 * This is set after the package information has been filtered.
	 */
	QString currentlyInstalledDisplayVersion;
	/**
	 * @brief Version displayed to the user.
	 */
	QString displayVersion;
	/**
	 * @brief Name displayed to the user.
	 */
	QString displayName;
	/**
	 * @brief Package download URL.
	 *
	 * This URL must be absolute and contain the protocol.
	 */
	QUrl downloadUrl;
	/**
	 * @brief Updater script download URL.
	 *
	 * This URL must be absolute and contain the protocol.
	 * If not present in the updater info file then downloadUrl is
	 * used with ".xml" appended.
	 */
	QUrl downloadScriptUrl;
	/**
	 * @brief Name of the package (program name or plugin name).
	 */
	QString name;
	/**
	 * @brief Revision used for version comparison.
	 *
	 * This is used to compare versions between the currently installed
	 * program and the available update. If values match then no upgrade
	 * is performed.
	 */
	QString revision;
};

#endif
