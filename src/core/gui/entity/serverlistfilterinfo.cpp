//------------------------------------------------------------------------------
// serverlistfilterinfo.cpp
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
#include "serverlistfilterinfo.h"

ServerListFilterInfo::ServerListFilterInfo()
{
	bEnabled = true;
	bShowEmpty = true;
	bShowFull = true;
	bShowOnlyValid = false;
	lockedServers = Doomseeker::Indifferent;
	maxPing = 0;
	testingServers = Doomseeker::Indifferent;
}

void ServerListFilterInfo::copy(const ServerListFilterInfo &other)
{
	bEnabled = other.bEnabled;
	bShowEmpty = other.bShowEmpty;
	bShowFull = other.bShowFull;
	bShowOnlyValid = other.bShowOnlyValid;
	gameModes = other.gameModes;
	gameModesExcluded = other.gameModesExcluded;
	lockedServers = other.lockedServers;
	maxPing = other.maxPing;
	serverName = other.serverName.trimmed();
	testingServers = other.testingServers;

	copyTrimmed(this->wads, other.wads);
	copyTrimmed(this->wadsExcluded, other.wadsExcluded);
}

void ServerListFilterInfo::copyTrimmed(QStringList &target, const QStringList &source) const
{
	target.clear();
	for (QString element : source)
	{
		element = element.trimmed();
		if (!element.isEmpty())
			target << element;
	}
}

bool ServerListFilterInfo::isFilteringAnything() const
{
	if (!serverName.isEmpty())
		return true;

	if (!bEnabled)
		return false;

	if (!bShowEmpty || !bShowFull)
		return true;

	if (bShowOnlyValid || maxPing > 0)
		return true;

	if (!gameModes.isEmpty()
		|| !gameModesExcluded.isEmpty()
		|| !wads.isEmpty()
		|| !wadsExcluded.isEmpty())
	{
		return true;
	}

	if (lockedServers != Doomseeker::Indifferent
		|| testingServers != Doomseeker::Indifferent)
	{
		return true;
	}

	return false;
}

QString ServerListFilterInfo::toString() const
{
	QString ret = "";

	ret += QString("bEnabled: ") + (bEnabled ? "Yes" : "No") + "\n";
	ret += QString("bShowEmpty: ") + (bShowEmpty ? "Yes" : "No") + "\n";
	ret += QString("bShowFull: ") + (bShowFull ? "Yes" : "No") + "\n";
	ret += QString("bShowOnlyValid: ") + (bShowOnlyValid ? "Yes" : "No") + "\n";
	ret += QString("GameModes: ") + gameModes.join(",") + "\n";
	ret += QString("GameModes Excluded: ") + gameModesExcluded.join(",") + "\n";
	ret += QString("LockedServers: %1\n").arg(lockedServers);
	ret += QString("MaxPing: ") + QString::number(maxPing) + "\n";
	ret += QString("ServerName: ") + serverName + "\n";
	ret += QString("Testing servers: %1\n").arg(testingServers);
	ret += QString("WADs: ") + wads.join(",") + "\n";
	ret += QString("WADs Excluded: ") + wadsExcluded.join(",") + "\n";

	return ret;
}
