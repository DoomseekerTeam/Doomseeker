//------------------------------------------------------------------------------
// chatnetworknamer.cpp
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
#include "chatnetworknamer.h"

QString ChatNetworkNamer::additionalAllowedChars()
{
	return ".-_";
}

QString ChatNetworkNamer::convertToValidName(const QString &name)
{
	QString result = name;
	for (auto &c : result)
	{
		if (!isValidCharacter(c))
			c = '_';
	}
	return result;
}

bool ChatNetworkNamer::isValidCharacter(const QChar &c)
{
	return c.isLetterOrNumber() || c == ' ' || additionalAllowedChars().contains(c);
}

bool ChatNetworkNamer::isValidName(const QString &name)
{
	if (name.trimmed().isEmpty())
		return false;
	for (const auto &c : name)
	{
		if (!isValidCharacter(c))
			return false;
	}
	return true;
}
