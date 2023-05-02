//------------------------------------------------------------------------------
// ircchatadapter.h
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
#ifndef __IRCCHATADAPTER_H__
#define __IRCCHATADAPTER_H__

#include "irc/ircadapterbase.h"
#include <QStringList>

class IRCNetworkAdapter;

/**
 * @brief Handles chatting through IRC.
 *
 * This class covers both channel and private messaging. This can be done
 * because the IRC protocol as stated in RFC 1459 doesn't see a difference
 * between transmitting messages between channel and user or between user and
 * user directly.
 */
class IRCChatAdapter : public IRCAdapterBase
{
public:
	enum IRCQuitType
	{
		ChannelPart,
		NetworkKill,
		NetworkQuit
	};

	/**
	 * @param pNetwork
	 *     A pointer to the network this channel or user exists on.
	 * @param recipient
	 *     Full name of the entity this adapter will be talking to.
	 */
	IRCChatAdapter(IRCNetworkAdapter *pNetwork, const QString &recipient);
	virtual ~IRCChatAdapter() override;

	/**
	 * All messages starting with '/' character will be passed to the
	 * parent network directly. Also the pOrigin parameter for the parent
	 * network will be set to point to 'this'.
	 *
	 * All other messages will be sent as:
	 * @code
	 * /PRIVMSG <recipient> :<message>
	 * @endcode
	 */
	void doSendMessage(const QString &message, IRCAdapterBase *pOrigin) override;

	/**
	 * @brief Emits message() signal formatting it to present sender's message.
	 */
	virtual void emitChatMessage(const QString &sender, const QString &content);

	IRCNetworkAdapter *network() override
	{
		return this->pNetwork;
	}

	QString recipient() const override
	{
		return this->recipientName;
	}

	/**
	 * @brief Sets IRCNetworkAdapter for this chat window. This adapter
	 * is not detached from the old network.
	 */
	void setNetwork(IRCNetworkAdapter *pNetwork);

	/**
	 * @brief For chat adapters this will return recipientName.
	 */
	QString title() const override;

	/**
	 * @brief Use this to register the fact that user has changed
	 * his/hers nickname.
	 */
	virtual void userChangesNickname(const QString &oldNickname, const QString &newNickname) = 0;

	/**
	 * @brief Use this to register the fact that user has joined the chat.
	 */
	virtual void userJoins(const QString &nickname, const QString &fullSignature) = 0;

	/**
	 * @brief Use this to register the fact that user has left the chat.
	 */
	virtual void userLeaves(const QString &nickname, const QString &farewellMessage, IRCQuitType quitType) = 0;

	/**
	 * @brief Use this to register the fact that user MODE flags have changed.
	 */
	virtual void userModeChanges(const QString &nickname,
		const QList<char> &addedFlags, const QList<char> &removedFlags) = 0;

protected:
	IRCNetworkAdapter *pNetwork;
	QString recipientName;

	/**
	 * Makes sure the sent message will not exceed the IRC character limit.
	 *
	 * @param words
	 *     A list of words from which a "sentence" will be build. This
	 *     list is directly modified and only unused words are left when
	 *     this method is done processing.
	 */
	QString extractMessageLine(QStringList &words, int maxLength);

	void sendChatMessage(const QString &message);
	void setRecipient(const QString &name);
};


#endif
