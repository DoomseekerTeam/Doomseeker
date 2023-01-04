//------------------------------------------------------------------------------
// serverlistfilterinfo.h
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
// Copyright (C) 2011 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __SERVERLISTFILTERINFO_H__
#define __SERVERLISTFILTERINFO_H__

#include <QString>
#include <QStringList>
#include <QVariant>

#include "gui/entity/showmode.h"

/**
 *	@brief Structure describing server filter.
 */
class ServerListFilterInfo
{
public:
	QString name;
	bool bEnabled;
	bool bShowEmpty;
	bool bShowFull;
	bool bShowOnlyValid;
	bool bShowBannedServers;
	bool bShowTooSoonServers;
	bool bShowNotRespondingServers;
	QStringList gameModes;
	QStringList gameModesExcluded;

	Doomseeker::ShowMode lockedServers;

	/**
	 *	@brief Maximum allowed ping.
	 *
	 *	The default value is 0 and it means that ping filter
	 *	is disabled.
	 */
	unsigned maxPing;
	QString serverName;
	QStringList wads;
	QStringList wadsExcluded;
	Doomseeker::ShowMode testingServers;

	/// This is not a filter but a grouping option.
	bool bPopulatedServersOnTop;

	ServerListFilterInfo();
	ServerListFilterInfo(const ServerListFilterInfo &other)
	{
		copy(other);
	}

	ServerListFilterInfo &operator=(const ServerListFilterInfo &other)
	{
		if (this != &other)
			copy(other);

		return *this;
	}

	/**
	 * @brief Informs if filter will apply to any server.
	 *
	 * @return true is returned if this filter will cause servers to become
	 * hidden from the server table, false if filter will never apply to any
	 * server.
	 */
	bool isFilteringAnything() const;

	/**
	 * @brief Check if the two filters are equivalent in their filterings.
	 *
	 * The equivalent filter results in the same filtering, but can have
	 * a different name.
	 */
	bool isFilteringEquivalent(const ServerListFilterInfo &other) const;

	static ServerListFilterInfo deserialize(const QVariant &variant);
	QVariant serialize() const;

	QString toString() const;

private:
	void copy(const ServerListFilterInfo &other);
	void copyTrimmed(QStringList &target, const QStringList &source) const;
};

#endif
