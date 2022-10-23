//------------------------------------------------------------------------------
// ircprivadapter.cpp
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
#include "irc/ircglobal.h"
#include "irc/ircmessageclass.h"
#include "irc/ircuserinfo.h"
#include "ircprivadapter.h"

IRCPrivAdapter::IRCPrivAdapter(IRCNetworkAdapter *pNetwork, const QString &recipient)
	: IRCChatAdapter(pNetwork, recipient)
{
}

void IRCPrivAdapter::userChangesNickname(const QString &oldNickname, const QString &newNickname)
{
	if (recipientName.compare(oldNickname, Qt::CaseInsensitive) == 0)
	{
		emit messageWithClass(tr("This user changed nickname from %1 to %2")
			.arg(oldNickname, newNickname), IRCMessageClass::ChannelAction);
		setRecipient(newNickname);
	}
}

void IRCPrivAdapter::userJoins(const QString &nickname, const QString &fullSignature)
{
	Q_UNUSED(nickname);
	Q_UNUSED(fullSignature);
	// Ignore. This has no sensible application here.
}

void IRCPrivAdapter::userLeaves(const QString &nickname, const QString &farewellMessage, IRCQuitType quitType)
{
	// Make sure that this user is the recipient of this adapter.
	IRCUserInfo recipientUserInfo(recipientName, network());
	if (recipientUserInfo.isSameNickname(nickname))
	{
		QString message = "";

		switch (quitType)
		{
		case IRCChatAdapter::NetworkKill:
			message = tr("This user connection has been killed. (KILL: %1)").arg(farewellMessage);
			break;

		case IRCChatAdapter::NetworkQuit:
			message = tr("This user has left the network. (QUIT: %1)").arg(farewellMessage);
			break;

		default:
			emit error(tr("Unhandled IRCQuitType in IRCPrivAdapter::userLeaves()"));
			break;
		}

		if (!message.isEmpty())
			emit messageWithClass(message, IRCMessageClass::NetworkAction);
	}
}
