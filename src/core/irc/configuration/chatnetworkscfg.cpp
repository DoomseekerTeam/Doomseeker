//------------------------------------------------------------------------------
// chatnetworkscfg.cpp
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "chatnetworkscfg.h"

#include "ini/inisection.h"
#include "irc/chatlogs.h"
#include "irc/configuration/ircconfig.h"
#include "strings.hpp"
#include "patternlist.h"
#include <QString>

const QString ChatNetworksCfg::SECTIONS_NAMES_PREFIX = "Network.";


QList<IRCNetworkEntity> ChatNetworksCfg::autoJoinNetworks() const
{
	QList<IRCNetworkEntity> result;
	for (const IRCNetworkEntity &network : networks())
	{
		if (network.isAutojoinNetwork())
			result << network;
	}

	return result;
}

bool ChatNetworksCfg::isAnyNetworkOnAutoJoin() const
{
	for (const IRCNetworkEntity &network : networks())
	{
		if (network.isAutojoinNetwork())
			return true;
	}

	return false;
}

IRCNetworkEntity ChatNetworksCfg::lastUsedNetwork() const
{
	QString networkName = ini().section("LastUsedNetwork").value("Description").toString();
	for (const IRCNetworkEntity &network : networks())
	{
		if (network.description() == networkName)
			return network;
	}
	return IRCNetworkEntity();
}

void ChatNetworksCfg::setLastUsedNetwork(const IRCNetworkEntity &network)
{
	// LastUsedNetwork section had more data in the past. To prevent
	// obscuring of the .ini file with this old data, we'll delete the
	// section and promptly recreate it.
	ini().deleteSection("LastUsedNetwork");
	ini().section("LastUsedNetwork").setValue("Description", network.description());
}

QList<IRCNetworkEntity> ChatNetworksCfg::networks() const
{
	QList<IRCNetworkEntity> result;
	for (const IniSection &section : allNetworksSections())
	{
		result << loadNetwork(section);
	}
	return result;
}

void ChatNetworksCfg::setNetworks(const QList<IRCNetworkEntity> &networks)
{
	// Erase all previously stored networks.
	// We need to rebuild these sections from scratch.
	clearNetworkSections();
	int id = 0;
	for (const IRCNetworkEntity &network : networks)
	{
		saveNetwork(networkSection(id), network);
		++id;
	}
}

IRCNetworkEntity ChatNetworksCfg::network(const QString &description)
{
	for (const IRCNetworkEntity &network : networks())
	{
		if (network.description() == description)
			return network;
	}
	return IRCNetworkEntity();
}

void ChatNetworksCfg::removeNetwork(const IRCNetworkEntity &network)
{
	QList<IRCNetworkEntity> networks = this->networks();
	QMutableListIterator<IRCNetworkEntity> it(networks);
	while (it.hasNext())
	{
		IRCNetworkEntity &candidate = it.next();
		if (candidate.description() == network.description())
		{
			it.remove();
			break;
		}
	}
	setNetworks(networks);
}

bool ChatNetworksCfg::replaceNetwork(const QString &oldDescription, const IRCNetworkEntity &newNetwork, QWidget *errorDisplayParentWidget)
{
	if (!ChatLogs().renameNetwork(errorDisplayParentWidget, oldDescription, newNetwork.description()))
		return false;
	QList<IRCNetworkEntity> networks = this->networks();
	QMutableListIterator<IRCNetworkEntity> it(networks);
	while (it.hasNext())
	{
		IRCNetworkEntity &network = it.next();
		if (network.description() == oldDescription)
		{
			network = newNetwork;
			break;
		}
	}
	setNetworks(networks);
	return true;
}

void ChatNetworksCfg::clearNetworkSections()
{
	for (const IniSection &section : allNetworksSections())
	{
		ini().deleteSection(section.name());
	}
}

IRCNetworkEntity ChatNetworksCfg::loadNetwork(const IniSection &section) const
{
	IRCNetworkEntity network;
	network.setAddress(section["Address"]);
	network.setAutojoinNetwork(section["bAutojoinNetwork"]);
	network.setAutojoinChannels(static_cast<QString>(section["AutojoinChannels"])
		.split(" ", Qt::SkipEmptyParts));
	network.setAutojoinCommands(section.value("AutojoinCommands").toStringList());
	network.setDescription(section["Description"]);
	network.setIgnoredUsers(PatternList::deserializeQVariant(section.value("IgnoredUsers")));
	network.setNickservCommand(section["NickservCommand"]);
	network.setNickservPassword(section["NickservPassword"]);
	network.setPassword(section["Password"]);
	network.setPort(section["Port"]);
	return network;
}

void ChatNetworksCfg::saveNetwork(IniSection section, const IRCNetworkEntity &network)
{
	section["Address"] = network.address();
	section["bAutojoinNetwork"] = network.isAutojoinNetwork();
	section["AutojoinChannels"] = network.autojoinChannels().join(" ");
	section["AutojoinCommands"].setValue(network.autojoinCommands());
	section["Description"] = network.description();
	section.setValue("IgnoredUsers", network.ignoredUsers().serializeQVariant());
	section["NickservCommand"] = network.nickservCommand();
	section["NickservPassword"] = network.nickservPassword();
	section["Password"] = network.password();
	section["Port"] = network.port();
}

QVector<IniSection> ChatNetworksCfg::allNetworksSections() const
{
	return ini().sectionsArray("^" + SECTIONS_NAMES_PREFIX);
}

IniSection ChatNetworksCfg::networkSection(int id)
{
	return ini().section(SECTIONS_NAMES_PREFIX + QString::number(id));
}

Ini &ChatNetworksCfg::ini() const
{
	return *gIRCConfig.ini();
}
