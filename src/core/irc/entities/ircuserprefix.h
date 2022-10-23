//------------------------------------------------------------------------------
// ircuserprefix.h
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
#ifndef id068647E2_4561_423C_AC0AE4C8499B2EF8
#define id068647E2_4561_423C_AC0AE4C8499B2EF8

#include "dptr.h"

#include <QList>
#include <QString>

/**
 * @brief One-to-one association of visible prefixes to user mode.
 *
 * For example: Ops get mode 'o' and prefix '@', voiced users get mode 'v'
 * and prefix '+'. Various servers define various different user modes.
 */
class IRCUserPrefix
{
public:
	/**
	 * @brief User modes: op, half-op and voice.
	 *
	 * Should be used when server doesn't provide its own PREFIX
	 * information.
	 */
	static IRCUserPrefix mkDefault();

	IRCUserPrefix();
	IRCUserPrefix(const IRCUserPrefix &other);
	~IRCUserPrefix();
	IRCUserPrefix &operator=(const IRCUserPrefix &other);

	void assignPrefix(char mode, char prefix);
	QString cleanNickname(const QString &nickname) const;
	int compare(char mode1, char mode2) const;
	bool hasMode(char mode) const;
	bool hasPrefix(char prefix) const;
	char modeFromNickname(const QString &nickname) const;
	char modeForPrefix(char prefix) const;
	char prefixForMode(char mode) const;
	/**
	 * @brief Out of list of possible modes picks mode with highest
	 *        priority.
	 */
	char topMostMode(const QList<char> &candidates) const;

private:
	DPtr<IRCUserPrefix> d;
};

#endif
