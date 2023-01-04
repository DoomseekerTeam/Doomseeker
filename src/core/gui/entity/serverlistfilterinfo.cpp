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
	bShowBannedServers = true;
	bShowTooSoonServers = true;
	bShowNotRespondingServers = true;
	lockedServers = Doomseeker::Indifferent;
	maxPing = 0;
	testingServers = Doomseeker::Indifferent;
}

void ServerListFilterInfo::copy(const ServerListFilterInfo &other)
{
	name = other.name;
	bEnabled = other.bEnabled;
	bShowEmpty = other.bShowEmpty;
	bShowFull = other.bShowFull;
	bShowOnlyValid = other.bShowOnlyValid;
	bShowBannedServers = other.bShowBannedServers;
	bShowTooSoonServers = other.bShowTooSoonServers;
	bShowNotRespondingServers = other.bShowNotRespondingServers;
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

	if (bShowOnlyValid
		|| !bShowBannedServers
		|| !bShowTooSoonServers
		|| !bShowNotRespondingServers)
	{
		return true;
	}

	if (maxPing > 0)
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

bool ServerListFilterInfo::isFilteringEquivalent(const ServerListFilterInfo &other) const
{
	return
		bEnabled == other.bEnabled &&
		bShowEmpty == other.bShowEmpty &&
		bShowFull == other.bShowFull &&
		bShowOnlyValid == other.bShowOnlyValid &&
		bShowBannedServers == other.bShowBannedServers &&
		bShowTooSoonServers == other.bShowTooSoonServers &&
		bShowNotRespondingServers == other.bShowNotRespondingServers &&
		gameModes == other.gameModes &&
		gameModesExcluded == other.gameModesExcluded &&
		lockedServers == other.lockedServers &&
		maxPing == other.maxPing &&
		serverName == other.serverName &&
		wads == other.wads &&
		wadsExcluded == other.wadsExcluded &&
		testingServers == other.testingServers
		;
}


ServerListFilterInfo ServerListFilterInfo::deserialize(const QVariant &variant)
{
	ServerListFilterInfo obj;

	QVariantMap in = variant.toMap();
	obj.name = in["name"].toString();
	obj.bEnabled = in["bEnabled"].toBool();
	obj.bShowEmpty = in["bShowEmpty"].toBool();
	obj.bShowFull = in["bShowFull"].toBool();
	obj.bShowOnlyValid = in["bShowOnlyValid"].toBool();
	obj.bShowBannedServers = in["bShowBannedServers"].toBool();
	obj.bShowTooSoonServers = in["bShowTooSoonServers"].toBool();
	obj.bShowNotRespondingServers = in["bShowNotRespondingServers"].toBool();
	obj.gameModes = in["GameModes"].toStringList();
	obj.gameModesExcluded = in["GameModesExcluded"].toStringList();
	obj.lockedServers = static_cast<Doomseeker::ShowMode>(in["LockedServers"].toInt());
	obj.maxPing = in["MaxPing"].toUInt();
	obj.serverName = in["ServerName"].toString();
	obj.testingServers = static_cast<Doomseeker::ShowMode>(in["TestingServers"].toInt());
	obj.wads = in["WADs"].toStringList();
	obj.wadsExcluded = in["WADsExcluded"].toStringList();
	return obj;
}

QVariant ServerListFilterInfo::serialize() const
{
	QVariantMap out;
	out["name"] = this->name;
	out["bEnabled"] = this->bEnabled;
	out["bShowEmpty"] = this->bShowEmpty;
	out["bShowFull"] = this->bShowFull;
	out["bShowOnlyValid"] = this->bShowOnlyValid;
	out["bShowBannedServers"] = this->bShowBannedServers;
	out["bShowTooSoonServers"] = this->bShowTooSoonServers;
	out["bShowNotRespondingServers"] = this->bShowNotRespondingServers;
	out["GameModes"] = this->gameModes;
	out["GameModesExcluded"] = this->gameModesExcluded;
	out["LockedServers"] = this->lockedServers;
	out["MaxPing"] = this->maxPing;
	out["ServerName"] = this->serverName;
	out["TestingServers"] = this->testingServers;
	out["WADs"].setValue(this->wads);
	out["WADsExcluded"].setValue(this->wadsExcluded);
	return out;
}

QString ServerListFilterInfo::toString() const
{
	QString ret = "";

	ret += QString("name: %1\n").arg(name);
	ret += QString("bEnabled: ") + (bEnabled ? "Yes" : "No") + "\n";
	ret += QString("bShowEmpty: ") + (bShowEmpty ? "Yes" : "No") + "\n";
	ret += QString("bShowFull: ") + (bShowFull ? "Yes" : "No") + "\n";
	ret += QString("bShowOnlyValid: ") + (bShowOnlyValid ? "Yes" : "No") + "\n";
	ret += QString("bShowBannedServers: ") + (bShowBannedServers ? "Yes" : "No") + "\n";
	ret += QString("bShowTooSoonServers: ") + (bShowTooSoonServers ? "Yes" : "No") + "\n";
	ret += QString("bShowNotRespondingServers: ") + (bShowNotRespondingServers ? "Yes" : "No") + "\n";

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
