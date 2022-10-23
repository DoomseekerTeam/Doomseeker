//------------------------------------------------------------------------------
// ircchanneladapter.cpp
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
#include "irc/configuration/ircconfig.h"
#include "irc/ircglobal.h"
#include "irc/ircmessageclass.h"
#include "irc/ircnetworkadapter.h"
#include "irc/ircuserinfo.h"
#include "irc/ircuserlist.h"
#include "ircchanneladapter.h"

IRCChannelAdapter::IRCChannelAdapter(IRCNetworkAdapter *pNetwork, const QString &recipient)
	: IRCChatAdapter(pNetwork, recipient)
{
	users = new IRCUserList();
}

IRCChannelAdapter::~IRCChannelAdapter()
{
	if (this->pNetwork != nullptr)
		sendMessage("/part " + this->recipientName + " " + gIRCConfig.personal.quitMessage);

	delete users;
}

bool IRCChannelAdapter::amIOperator() const
{
	const QString &myNickname = pNetwork->myNickname();
	return isOperator(myNickname);
}

void IRCChannelAdapter::appendNameToCachedList(const QString &name)
{
	if (users->appendNameToCachedList(IRCUserInfo(name, network())))
	{
		IRCUserInfo user = users->userCopy(name);
		emit nameAdded(user);
	}
}

void IRCChannelAdapter::appendNamesToCachedList(const QStringList &names)
{
	for (const QString &name : names)
	{
		appendNameToCachedList(name);
	}
}

void IRCChannelAdapter::banUser(const QString &nickname, const QString &reason)
{
	pNetwork->banUser(nickname, reason, this->recipientName);
}

void IRCChannelAdapter::emitCachedNameListUpdated()
{
	emit nameListUpdated(*users);
}

void IRCChannelAdapter::emitChatMessage(const QString &sender, const QString &content)
{
	// Ensure that all nickname artifacts are preserved.
	const IRCUserInfo *pUserInfo = users->user(sender);

	QString actualSenderName = sender;
	if (pUserInfo != nullptr)
		actualSenderName = pUserInfo->prefixedName();

	// Check if content has our nickname.
	// (do not play sounds for our own messages)
	const QString &myNickname = pNetwork->myNickname();
	if (content.contains(myNickname, Qt::CaseInsensitive)
		&& sender.compare(myNickname, Qt::CaseInsensitive) != 0)
	{
		emit myNicknameUsed();
	}

	IRCChatAdapter::emitChatMessage(actualSenderName, content);
}

bool IRCChannelAdapter::hasUser(const QString &nickname)
{
	return users->hasUser(nickname);
}

bool IRCChannelAdapter::isOperator(const QString &nickname) const
{
	const IRCUserInfo *pUser = users->user(nickname);
	if (pUser != nullptr)
		return pUser->isOp();

	return false;
}

void IRCChannelAdapter::kickUser(const QString &nickname, const QString &reason)
{
	if (hasUser(nickname))
	{
		QString cleanNickname = IRCUserInfo(nickname, pNetwork).cleanNickname();
		this->sendMessage(QString("/kick %1 %2 %3").arg(this->recipientName, cleanNickname, reason));
	}
}

void IRCChannelAdapter::removeNameFromCachedList(const QString &name)
{
	IRCUserInfo user = users->userCopy(name);

	if (!users->removeNameFromCachedList(name))
		emit error(QString(R"(Attempted to remove name "%1" from the "%2" channel's name list but no such name is on the list.)").arg(name, this->recipientName));
	else
		emit nameRemoved(user);
}

void IRCChannelAdapter::setHalfOp(const QString &nickname, bool bSet)
{
	pNetwork->setChannelMode(this->recipientName, nickname, "h", bSet);
}

void IRCChannelAdapter::setOp(const QString &nickname, bool bSet)
{
	pNetwork->setChannelMode(this->recipientName, nickname, "o", bSet);
}

void IRCChannelAdapter::setVoiced(const QString &nickname, bool bSet)
{
	pNetwork->setChannelMode(this->recipientName, nickname, "v", bSet);
}

void IRCChannelAdapter::userChangesNickname(const QString &oldNickname, const QString &newNickname)
{
	if (hasUser(oldNickname))
	{
		IRCUserInfo oldName = users->userCopy(oldNickname);

		users->changeNick(oldNickname, newNickname);
		emit nameRemoved(oldName);
		emit nameAdded(users->userCopy(newNickname));

		emit messageWithClass(tr("%1 is now known as %2").arg(oldNickname, newNickname),
			IRCMessageClass::ChannelAction);
	}
}

void IRCChannelAdapter::userJoins(const QString &nickname, const QString &fullSignature)
{
	appendNameToCachedList(nickname);

	emit messageWithClass(tr("User %1 [%2] has joined the channel.").arg(nickname, fullSignature),
		IRCMessageClass::ChannelAction);
}

void IRCChannelAdapter::userLeaves(const QString &nickname, const QString &farewellMessage, IRCQuitType quitType)
{
	if (!hasUser(nickname))
	{
		// Nothing to do here. This user was not even on the channel.
		return;
	}

	removeNameFromCachedList(nickname);

	switch (quitType)
	{
	case IRCChatAdapter::ChannelPart:
		emit messageWithClass(tr("User %1 has left the channel. (PART: %2)").arg(nickname, farewellMessage),
			IRCMessageClass::ChannelAction);
		break;

	case IRCChatAdapter::NetworkKill:
		emit messageWithClass(tr("Connection for user %1 has been killed. (KILL: %2)").arg(nickname, farewellMessage),
			IRCMessageClass::NetworkAction);
		break;

	case IRCChatAdapter::NetworkQuit:
		emit messageWithClass(tr("User %1 has quit the network. (QUIT: %2)").arg(nickname, farewellMessage),
			IRCMessageClass::NetworkAction);
		break;

	default:
		emit error(tr("Unknown quit type from user %1.").arg(nickname));
		break;
	}
}

void IRCChannelAdapter::userModeChanges(const QString &nickname,
	const QList<char> &addedFlags, const QList<char> &removedFlags)
{
	const IRCUserInfo *pUserInfo = this->users->user(nickname);
	if (pUserInfo != nullptr)
	{
		IRCUserInfo newUserInfo = *pUserInfo;
		for (char mode : addedFlags)
		{
			newUserInfo.setMode(mode);
		}
		for (char mode : removedFlags)
		{
			newUserInfo.unsetMode(mode);
		}

		this->users->setUserModes(nickname, newUserInfo.modes());
		emit nameUpdated(newUserInfo);
	}
}
