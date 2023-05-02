//------------------------------------------------------------------------------
// ircisupportparser.cpp
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
#include "ircisupportparser.h"

#include "irc/entities/ircuserprefix.h"
#include <QByteArray>
#include <QStringList>

DClass<IRCISupportParser>
{
public:
	class State
	{
	public:
		IRCUserPrefix userPrefixes;

		State()
		{
			userPrefixes = IRCUserPrefix::mkDefault();
		}
	};

	QStringList iSupportParams;
	State state;
};

DPointered(IRCISupportParser)

IRCISupportParser::IRCISupportParser()
{
}

IRCISupportParser::~IRCISupportParser()
{
}

void IRCISupportParser::appendLine(const QString &line)
{
	for (const QString &element : line.split(" "))
	{
		if (element == ":are")
		{
			// Start of ":are supported by this server".
			// We don't need this.
			break;
		}
		d->iSupportParams << element;
	}
}

QString IRCISupportParser::findValue(const QString &key)
{
	for (const QString &element : d->iSupportParams)
	{
		if (element.startsWith(key))
			return element.mid(key.length());
	}
	return QString();
}

void IRCISupportParser::parse()
{
	d->state = PrivData<IRCISupportParser>::State();
	parsePrefix();
}

void IRCISupportParser::parsePrefix()
{
	QString value = findValue("PREFIX=");
	if (value.isNull())
		return;
	QRegExp regex(R"(\((\S+)\)(\S+)$)");
	regex.setMinimal(true);
	if (regex.indexIn(value) < 0)
		return;
	QByteArray modes = regex.cap(1).toUtf8();
	QByteArray prefixes = regex.cap(2).toUtf8();
	if (modes.size() != prefixes.size())
		return;
	IRCUserPrefix userPrefixes;
	for (int i = 0; i < modes.size(); ++i)
		userPrefixes.assignPrefix(modes[i], prefixes[i]);
	d->state.userPrefixes = userPrefixes;
}

const IRCUserPrefix &IRCISupportParser::userPrefixes() const
{
	return d->state.userPrefixes;
}
