//------------------------------------------------------------------------------
// ircresponseparseresult.cpp
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
#include "ircresponseparseresult.h"

#include "irc/constants/ircresponsetype.h"

// This is returned by IRCResponseParseResult::type() if the result is invalid.
// Do not modify contents of this var!
const IRCResponseType invalidResponseType;

IRCResponseParseResult::IRCResponseParseResult()
{
	d.pResponseType = nullptr;
	d.bWasParsed = false;
}

IRCResponseParseResult::IRCResponseParseResult(const IRCResponseType &responseType, bool bWasParsed)
{
	d.bWasParsed = bWasParsed;
	d.pResponseType = new IRCResponseType(responseType);
}

IRCResponseParseResult::IRCResponseParseResult(const IRCResponseParseResult &other)
{
	d.pResponseType = nullptr;
	copyIn(other);
}

IRCResponseParseResult::~IRCResponseParseResult()
{
	if (d.pResponseType != nullptr)
		delete d.pResponseType;
}

void IRCResponseParseResult::copyIn(const IRCResponseParseResult &other)
{
	d.bWasParsed = other.d.bWasParsed;
	if (d.pResponseType != nullptr)
	{
		delete d.pResponseType;
		d.pResponseType = nullptr;
	}

	if (other.d.pResponseType != nullptr)
		d.pResponseType = new IRCResponseType(*other.d.pResponseType);
}

IRCResponseParseResult &IRCResponseParseResult::operator=(const IRCResponseParseResult &other)
{
	if (this != &other)
		copyIn(other);

	return *this;
}

const IRCResponseType &IRCResponseParseResult::type() const
{
	if (d.pResponseType == nullptr)
		return invalidResponseType;

	return *d.pResponseType;
}
