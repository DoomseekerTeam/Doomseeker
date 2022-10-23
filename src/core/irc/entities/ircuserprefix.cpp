//------------------------------------------------------------------------------
// ircuserprefix.cpp
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
#include "ircuserprefix.h"

#include <QMap>

class IRCModePrefix
{
public:
	char mode;
	char prefix;

	IRCModePrefix(char mode, char prefix)
	{
		this->mode = mode;
		this->prefix = prefix;
	}
};


DClass<IRCUserPrefix>
{
public:
	QList<IRCModePrefix> map;
};

DPointered(IRCUserPrefix)

IRCUserPrefix::IRCUserPrefix()
{
}

IRCUserPrefix::IRCUserPrefix(const IRCUserPrefix &other)
{
	d = other.d;
}

IRCUserPrefix::~IRCUserPrefix()
{
}

IRCUserPrefix &IRCUserPrefix::operator=(const IRCUserPrefix &other)
{
	d = other.d;
	return *this;
}

void IRCUserPrefix::assignPrefix(char mode, char prefix)
{
	d->map << IRCModePrefix(mode, prefix);
}

QString IRCUserPrefix::cleanNickname(const QString &nickname) const
{
	if (!nickname.isEmpty())
	{
		if (hasPrefix(nickname[0].toLatin1()))
			return nickname.mid(1);
	}
	return nickname;
}

int IRCUserPrefix::compare(char mode1, char mode2) const
{
	if (mode1 == mode2)
		return 0;
	for (const IRCModePrefix &candidate : d->map)
	{
		if (candidate.mode == mode1)
			return -1;
		else if (candidate.mode == mode2)
			return 1;
	}
	// Neither was found so we treat them as equal.
	return 0;
}

bool IRCUserPrefix::hasMode(char mode) const
{
	return prefixForMode(mode) != 0;
}

bool IRCUserPrefix::hasPrefix(char prefix) const
{
	return modeForPrefix(prefix) != 0;
}

IRCUserPrefix IRCUserPrefix::mkDefault()
{
	IRCUserPrefix obj;
	obj.assignPrefix('o', '@');
	obj.assignPrefix('h', '%');
	obj.assignPrefix('v', '+');
	return obj;
}

char IRCUserPrefix::modeFromNickname(const QString &nickname) const
{
	if (!nickname.isEmpty())
	{
		if (hasPrefix(nickname[0].toLatin1()))
			return modeForPrefix(nickname[0].toLatin1());
	}
	return 0;
}

char IRCUserPrefix::modeForPrefix(char prefix) const
{
	for (const IRCModePrefix &candidate : d->map)
	{
		if (candidate.prefix == prefix)
			return candidate.mode;
	}
	return 0;
}

char IRCUserPrefix::prefixForMode(char mode) const
{
	for (const IRCModePrefix &candidate : d->map)
	{
		if (candidate.mode == mode)
			return candidate.prefix;
	}
	return 0;
}

char IRCUserPrefix::topMostMode(const QList<char> &candidates) const
{
	char highest = 0;
	for (char candidate : candidates)
	{
		if (compare(highest, candidate) > 0)
			highest = candidate;
	}
	return highest;
}
