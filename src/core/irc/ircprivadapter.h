//------------------------------------------------------------------------------
// ircprivadapter.h
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
#ifndef __IRCPRIVADAPTER_H__
#define __IRCPRIVADAPTER_H__

#include "irc/ircchatadapter.h"

/**
 *	@brief Class type that is used for private conversations with other
 *	users directly.
 */
class IRCPrivAdapter : public IRCChatAdapter
{
	Q_OBJECT

public:
	IRCPrivAdapter(IRCNetworkAdapter *pNetwork, const QString &recipient);

	AdapterType adapterType() const override
	{
		return PrivAdapter;
	}

	void userChangesNickname(const QString &oldNickname, const QString &newNickname) override;
	void userJoins(const QString &nickname, const QString &fullSignature) override;
	void userLeaves(const QString &nickname, const QString &farewellMessage, IRCQuitType quitType) override;
	void userModeChanges(const QString &nickname,
		const QList<char> &addedFlags, const QList<char> &removedFlags) override
	{
		Q_UNUSED(nickname);
		Q_UNUSED(addedFlags);
		Q_UNUSED(removedFlags);
		// This is ignored here.
	}
};

#endif
