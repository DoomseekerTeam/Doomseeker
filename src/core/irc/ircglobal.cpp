//------------------------------------------------------------------------------
// ircglobal.cpp
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
#include "ircglobal.h"

#include <QChar>

bool IRCGlobal::isChannelDenotingCharacter(char character)
{
	return character == '&' || character == '#'
		|| character == '+' || character == '!';
}

bool IRCGlobal::isChannelName(const QString &name)
{
	if (name.isEmpty())
		return false;

	QChar c = name[0];
	return isChannelDenotingCharacter(c.toLatin1());
}

QString IRCGlobal::toIrcLower(const QString &str)
{
	QString lowered = str.toLower();

	lowered.replace('[', '{');
	lowered.replace(']', '}');
	lowered.replace('\\', '|');

	return lowered;
}
