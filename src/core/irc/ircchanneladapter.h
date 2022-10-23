//------------------------------------------------------------------------------
// ircchanneladapter.h
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
#ifndef __IRCCHANNELADAPTER_H__
#define __IRCCHANNELADAPTER_H__

#include "irc/ircchatadapter.h"
#include <QStringList>

class IRCUserInfo;
class IRCUserList;

/**
 * @brief Class type that is used for conversations within a channel.
 */
class IRCChannelAdapter : public IRCChatAdapter
{
	Q_OBJECT

public:
	IRCChannelAdapter(IRCNetworkAdapter *pNetwork, const QString &recipient);
	~IRCChannelAdapter() override;

	AdapterType adapterType() const override
	{
		return ChannelAdapter;
	}

	/**
	 * @brief Checks if this client is an operator on this channel.
	 */
	bool amIOperator() const;

	/**
	 * @brief Appends a single name to the sorted cachedNames list.
	 *
	 * Ensures that no duplicate names are found on the list.
	 * This will emit nameAdded() signal.
	 */
	void appendNameToCachedList(const QString &name);

	/**
	 * @brief Appends a list of names to cachedNames list. This ensures that no
	 * duplicate names are found on the list.
	 */
	void appendNamesToCachedList(const QStringList &names);

	/**
	 * @brief Bans and kicks user from the channel.
	 *
	 * Op privileges required.
	 * @param nickname
	 *     Victim's nick.
	 * @param reason
	 *     Reason for ban (this will be delivered to /kick message).
	 */
	void banUser(const QString &nickname, const QString &reason);

	/**
	 * @brief Emits cached list of names. This should be called
	 * when end of names list message is received for this channel.
	 */
	void emitCachedNameListUpdated();
	void emitChatMessage(const QString &sender, const QString &content) override;

	bool hasUser(const QString &nickname);

	/**
	 * @brief Checks if user is an operator on this channel.
	 */
	bool isOperator(const QString &nickname) const;

	/**
	 * @brief Kicks user from the channel.
	 *
	 * Op privileges required.
	 * @param nickname
	 *     Victim's nick.
	 * @param reason
	 *     Reason for kick.
	 */
	void kickUser(const QString &nickname, const QString &reason);


	/**
	 * @brief Removes a name from the sortecd cachedNames list.
	 *
	 * This will emit nameRemoved() signal.
	 */
	void removeNameFromCachedList(const QString &name);

	/**
	 * @brief Sets half op mode for given user.
	 */
	void setHalfOp(const QString &nickname, bool bSet);

	/**
	 * @brief Sets op mode for given user.
	 *
	 * Op privileges required.
	 */
	void setOp(const QString &nickname, bool bSet);

	/**
	 * @brief Sets voice mode for given user.
	 *
	 * Op privileges required.
	 */
	void setVoiced(const QString &nickname, bool bSet);

	/**
	 * This will emit nameRemoved() for oldNickname and nameAdded()
	 * for the newNickname.
	 */
	void userChangesNickname(const QString &oldNickname, const QString &newNickname) override;
	void userJoins(const QString &nickname, const QString &fullSignature) override;
	void userLeaves(const QString &nickname, const QString &farewellMessage, IRCQuitType quitType) override;

	/**
	 * this will first emit nameRemoved() for the nickname and then
	 * nameAdded() for the same nickname.
	 */
	void userModeChanges(const QString &nickname, const QList<char> &addedFlags, const QList<char> &removedFlags) override;

signals:
	/**
	 * @brief Emitted when nickname is used in a message.
	 *
	 * This is played for other user's chat messages.
	 */
	void myNicknameUsed();
	void nameAdded(const IRCUserInfo &userInfo);
	void nameListUpdated(const IRCUserList &names);
	void nameRemoved(const IRCUserInfo &userInfo);
	void nameUpdated(const IRCUserInfo &userInfo);

private:
	IRCUserList *users;

	/**
	 * @brief Adds a name to the cachedNames list.
	 *
	 * This ensures that there are no duplicate names and that
	 * the list remains sorted.
	 *
	 * @return True if a name was actually added. False otherwise.
	 */
	bool addName(const QString &name);
};

#endif
