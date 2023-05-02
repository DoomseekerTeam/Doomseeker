//------------------------------------------------------------------------------
// ircdelayedoperationban.cpp
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
#include "ircdelayedoperationban.h"

#include "irc/entities/ircuserprefix.h"
#include "irc/ircnetworkadapter.h"
#include "irc/ircresponseparser.h"

DClass<IRCDelayedOperationBan>
{
public:
	QString channel;
	QString nickname;
	QString reason;
	IRCNetworkAdapter *network;

	QString cleanNickname() const
	{
		return network->userPrefixes().cleanNickname(nickname);
	}
};

DPointered(IRCDelayedOperationBan)

IRCDelayedOperationBan::IRCDelayedOperationBan(IRCNetworkAdapter *network,
	const QString &channel, const QString &nickname, QObject *parent)
	: IRCDelayedOperation(parent)
{
	d->channel = channel;
	d->nickname = nickname;
	d->network = network;
	this->connect(d->network->responseParser(),
		SIGNAL(whoIsUser(QString,QString,QString,QString)),
		SLOT(onWhoIsUser(QString,QString,QString)));
}

IRCDelayedOperationBan::~IRCDelayedOperationBan()
{
}

void IRCDelayedOperationBan::start()
{
	d->network->sendMessage(QString("/whois %1").arg(d->cleanNickname()));
}

void IRCDelayedOperationBan::onWhoIsUser(const QString &nickname, const QString &user,
	const QString &hostName)
{
	Q_UNUSED(nickname);
	Q_UNUSED(user);
	QString banString = "*!*@" + hostName;
	d->network->sendMessage(QString("/mode %1 +b %2").arg(d->channel, banString));
	d->network->sendMessage(QString("/kick %1 %2 %3").arg(d->channel, d->nickname, d->reason));
	this->deleteLater();
}

void IRCDelayedOperationBan::setReason(const QString &reason)
{
	d->reason = reason;
}
