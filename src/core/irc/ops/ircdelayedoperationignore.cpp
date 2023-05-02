//------------------------------------------------------------------------------
// ircdelayedoperationignore.cpp
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
#include "ircdelayedoperationignore.h"

#include "irc/configuration/chatnetworkscfg.h"
#include "irc/entities/ircnetworkentity.h"
#include "irc/entities/ircuserprefix.h"
#include "irc/ircnetworkadapter.h"
#include "irc/ircnetworkconnectioninfo.h"
#include "irc/ircresponseparser.h"
#include "patternlist.h"
#include <QInputDialog>

DClass<IRCDelayedOperationIgnore>
{
public:
	bool showPatternPopup;
	IRCNetworkAdapter *network;
	QString nickname;
	QWidget *parentWidget;

	QString cleanNickname() const
	{
		return network->userPrefixes().cleanNickname(nickname);
	}

	QString networkDescription() const
	{
		return network->connection().networkEntity.description();
	}
};

DPointered(IRCDelayedOperationIgnore)


IRCDelayedOperationIgnore::IRCDelayedOperationIgnore(QWidget *parent,
	IRCNetworkAdapter *network, const QString &nickname)
	: IRCDelayedOperation(parent)
{
	d->showPatternPopup = false;
	d->network = network;
	d->nickname = nickname;
	d->parentWidget = parent;
	this->connect(d->network->responseParser(),
		SIGNAL(whoIsUser(QString,QString,QString,QString)),
		SLOT(onWhoIsUser(QString,QString,QString)));
}

IRCDelayedOperationIgnore::~IRCDelayedOperationIgnore()
{
}

void IRCDelayedOperationIgnore::start()
{
	d->network->sendMessage(QString("/whois %1").arg(d->cleanNickname()));
}

void IRCDelayedOperationIgnore::onWhoIsUser(const QString &nickname, const QString &user,
	const QString &hostName)
{
	QString ignorePattern = "*!*@" + hostName;
	if (d->showPatternPopup)
	{
		QString label = tr("Ignore user %1 (username=%2) on network %3:").arg(
			nickname, user, d->networkDescription());
		ignorePattern = QInputDialog::getText(d->parentWidget, tr("IRC - Ignore user"),
			label, QLineEdit::Normal, ignorePattern);
	}
	if (!ignorePattern.trimmed().isEmpty())
	{
		ChatNetworksCfg cfg;
		IRCNetworkEntity network = cfg.network(d->networkDescription());
		PatternList patterns = network.ignoredUsers();
		patterns << QRegExp(ignorePattern, Qt::CaseInsensitive, QRegExp::Wildcard);
		network.setIgnoredUsers(patterns);
		cfg.replaceNetwork(network.description(), network, d->parentWidget);
		d->network->setNetworkEntity(network);
	}
	this->deleteLater();
}

void IRCDelayedOperationIgnore::setShowPatternPopup(bool b)
{
	d->showPatternPopup = b;
}
