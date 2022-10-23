//------------------------------------------------------------------------------
// ircctcpparser.cpp
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
#include "ircctcpparser.h"

#include "irc/ircnetworkadapter.h"
#include "version.h"
#include <QDateTime>
#include <QDebug>
#include <QStringList>

DClass<IRCCtcpParser>
{
public:
	QString command;
	IRCCtcpParser::CtcpEcho echo;
	IRCCtcpParser::MessageType msgType;
	QString msg;
	IRCNetworkAdapter *network;
	QStringList params;
	QString printable;
	QString recipient;
	QString reply;
	QString sender;
};

DPointered(IRCCtcpParser)

IRCCtcpParser::IRCCtcpParser(IRCNetworkAdapter *network, const QString &sender,
	const QString &recipient, const QString &msg, MessageType msgType)
{
	d->echo = DontShow;
	d->msg = msg;
	d->msgType = msgType;
	d->network = network;
	d->recipient = recipient;
	d->sender = sender;
}

IRCCtcpParser::~IRCCtcpParser()
{
}

IRCCtcpParser::CtcpEcho IRCCtcpParser::echo() const
{
	return d->echo;
}

bool IRCCtcpParser::isCommand(const QString &candidate)
{
	return d->command.compare(candidate, Qt::CaseInsensitive) == 0;
}

bool IRCCtcpParser::isCtcp() const
{
	if (d->msg.length() <= 2)
		return false;
	return d->msg[0].unicode() == 0x1 && d->msg[d->msg.length() - 1].unicode() == 0x1;
}

bool IRCCtcpParser::parse()
{
	if (!isCtcp())
		return false;
	tokenizeMsg();
	d->printable = tr("CTCP %1: [%2] %3 %4").arg(typeToName(), d->sender, d->command, d->params.join(" "));
	if (isCommand("action"))
	{
		d->echo = PrintAsNormalMessage;
		d->printable = tr("%1 %2").arg(d->sender, d->params.join(" "));
	}
	else
	{
		if (d->msgType == Request)
		{
			d->echo = DisplayInServerTab;
			if (isCommand("clientinfo"))
				d->reply = "CLIENTINFO ACTION VERSION TIME PING";
			else if (isCommand("version"))
				d->reply = QString("VERSION %1").arg(Version::fullVersionInfoWithOs());
			else if (isCommand("time"))
				d->reply = QString("TIME %1").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
			else if (isCommand("ping"))
				d->reply = QString("PING %1").arg(d->params[0]);
		}
		else if (d->msgType == Reply)
		{
			d->echo = DisplayThroughGlobalMessage;
			if (isCommand("ping"))
			{
				// "ping" CTCP is echoed in server tab as further processing
				// displays user ping in a customized message to currently open
				// network window.
				d->echo = DisplayInServerTab;
				qint64 timestamp = d->params.takeFirst().toLongLong();
				d->network->userPing(d->sender, timestamp);
			}
		}
		else
			qDebug() << "Unknown d->msgType in IRCCtcpParser";
	}
	return true;
}

const QString &IRCCtcpParser::printable() const
{
	return d->printable;
}

const QString &IRCCtcpParser::reply() const
{
	return d->reply;
}

void IRCCtcpParser::tokenizeMsg()
{
	QString stripped = d->msg.mid(1, d->msg.length() - 2);
	QStringList tokens = stripped.split(" ");
	d->command = tokens.takeFirst();
	d->params = tokens;
}

QString IRCCtcpParser::typeToName() const
{
	switch (d->msgType)
	{
	case Request:
		return tr("REQUEST");
	case Reply:
		return tr("REPLY");
	default:
		return tr("????");
	}
}
