//------------------------------------------------------------------------------
// ircnetworkentity.cpp
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
#include "ircnetworkentity.h"

#include "irc/chatnetworknamer.h"
#include "patternlist.h"
#include <QVariantMap>

DClass<IRCNetworkEntity>
{
public:
	QString address;
	QStringList autojoinChannels;
	QStringList autojoinCommands;
	bool bAutojoinNetwork;
	QString description;
	PatternList ignoredUsers;
	QString nickservCommand;
	QString nickservPassword;
	QString password;
	unsigned short port;
};

DPointered(IRCNetworkEntity)

IRCNetworkEntity::IRCNetworkEntity()
{
	d->bAutojoinNetwork = false;
	d->port = 6667;
	d->nickservCommand = "/msg nickserv identify %1";
}

IRCNetworkEntity::~IRCNetworkEntity()
{
}

const QString &IRCNetworkEntity::address() const
{
	return d->address;
}

const QStringList &IRCNetworkEntity::autojoinChannels() const
{
	return d->autojoinChannels;
}

QStringList &IRCNetworkEntity::autojoinChannels()
{
	return d->autojoinChannels;
}

const QStringList &IRCNetworkEntity::autojoinCommands() const
{
	return d->autojoinCommands;
}

QStringList &IRCNetworkEntity::autojoinCommands()
{
	return d->autojoinCommands;
}

bool IRCNetworkEntity::isAutojoinNetwork() const
{
	return d->bAutojoinNetwork;
}

bool IRCNetworkEntity::isValid() const
{
	return !description().isEmpty();
}

const QString &IRCNetworkEntity::description() const
{
	return d->description;
}

const QString &IRCNetworkEntity::nickservCommand() const
{
	return d->nickservCommand;
}

const QString &IRCNetworkEntity::nickservPassword() const
{
	return d->nickservPassword;
}

bool IRCNetworkEntity::operator<(const IRCNetworkEntity &other) const
{
	return description().toLower().trimmed() < other.description().toLower().trimmed();
}

bool IRCNetworkEntity::operator==(const IRCNetworkEntity &other) const
{
	return address() == other.address() && port() == other.port();
}

const QString &IRCNetworkEntity::password() const
{
	return d->password;
}

unsigned short IRCNetworkEntity::port() const
{
	return d->port;
}


void IRCNetworkEntity::setAddress(const QString &v)
{
	d->address = v;
}

void IRCNetworkEntity::setAutojoinChannels(const QStringList &v)
{
	d->autojoinChannels = v;
}

void IRCNetworkEntity::setAutojoinCommands(const QStringList &v)
{
	d->autojoinCommands = v;
}

void IRCNetworkEntity::setAutojoinNetwork(bool v)
{
	d->bAutojoinNetwork = v;
}

void IRCNetworkEntity::setDescription(const QString &v)
{
	d->description = ChatNetworkNamer::convertToValidName(v);
}

void IRCNetworkEntity::setNickservCommand(const QString &v)
{
	d->nickservCommand = v;
}

void IRCNetworkEntity::setNickservPassword(const QString &v)
{
	d->nickservPassword = v;
}

void IRCNetworkEntity::setPassword(const QString &v)
{
	d->password = v;
}

void IRCNetworkEntity::setPort(unsigned short v)
{
	d->port = v;
}

const PatternList &IRCNetworkEntity::ignoredUsers() const
{
	return d->ignoredUsers;
}

void IRCNetworkEntity::setIgnoredUsers(const PatternList &val)
{
	d->ignoredUsers = val;
}

IRCNetworkEntity IRCNetworkEntity::deserializeQVariant(const QVariant &var)
{
	QVariantMap map = var.toMap();
	IRCNetworkEntity result;
	result.setAddress(map["address"].toString());
	result.setAutojoinChannels(map["autoJoinChannels"].toStringList());
	result.setAutojoinCommands(map["autoJoinCommands"].toStringList());
	result.setAutojoinNetwork(map["autoJoinNetwork"].toBool());
	result.setDescription(map["description"].toString());
	result.setIgnoredUsers(PatternList::deserializeQVariant(map["ignoredUsers"]));
	result.setNickservCommand(map["nickservCommand"].toString());
	result.setNickservPassword(map["nickservPassword"].toString());
	result.setPassword(map["password"].toString());
	result.setPort(map["port"].toInt());
	return result;
}

QVariant IRCNetworkEntity::serializeQVariant() const
{
	QVariantMap map;
	map["address"] = address();
	map["autoJoinChannels"] = autojoinChannels();
	map["autoJoinCommands"] = autojoinCommands();
	map["autoJoinNetwork"] = isAutojoinNetwork();
	map["description"] = description();
	map["ignoredUsers"] = ignoredUsers().serializeQVariant();
	map["nickservCommand"] = nickservCommand();
	map["nickservPassword"] = nickservPassword();
	map["password"] = password();
	map["port"] = port();
	return map;
}
