//------------------------------------------------------------------------------
// updaterinfoparser.h
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
#ifndef DOOMSEEKER_UPDATER_UPDATERINFOPARSER_H
#define DOOMSEEKER_UPDATER_UPDATERINFOPARSER_H

#include "dptr.h"

#include <QByteArray>
#include <QList>
#include <QVariantMap>

class UpdatePackage;

class UpdaterInfoParser : public QObject
{
	Q_OBJECT

public:
	UpdaterInfoParser();
	~UpdaterInfoParser() override;

	const QList<UpdatePackage> &packages() const;

	/**
	 * @brief Parses updater info JSON and sets certain internal
	 *        properties which can then be accessed through getters.
	 *
	 * Information on all parsed packages can be obtained through
	 * packages() getter.
	 *
	 * @return The returned value is an error code defined in
	 *         AutoUpdater::ErrorCode. AutoUpdater::ErrorCode::EC_Ok
	 *         is returned on success.
	 */
	int parse(const QByteArray &json);

private:
	DPtr<UpdaterInfoParser> d;

	int parsePackageNode(const QString &packageName, const QVariantMap &map);
};

#endif
