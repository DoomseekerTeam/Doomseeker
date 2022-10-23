//------------------------------------------------------------------------------
// ircnetworkadapter.h
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
#ifndef __IRCNETWORKADAPTER_H__
#define __IRCNETWORKADAPTER_H__

#include "irc/ircadapterbase.h"
#include "irc/ircclient.h"
#include "irc/ircnetworkconnectioninfo.h"
#include "irc/ircrequestparser.h"
#include "socketsignalsadapter.h"

#include <QHash>

class IRCChatAdapter;
class IRCISupportParser;
class IRCResponseParser;
class IRCSocketSignalsAdapter;
class IRCUserList;
class IRCUserPrefix;

/**
 * @brief Interprets communication between the client and the IRC server.
 */
class IRCNetworkAdapter : public IRCAdapterBase
{
	Q_OBJECT

	friend class IRCSocketSignalsAdapter;

public:
	IRCNetworkAdapter(IRCNetworkConnectionInfo connectionInfo);
	~IRCNetworkAdapter() override;

	AdapterType adapterType() const override
	{
		return NetworkAdapter;
	}

	/**
	 * @brief Checks if client is an operator on a specified channel.
	 */
	bool amIOperator(const QString &channel) const
	{
		return this->isOperator(this->myNickname(), channel);
	}

	/**
	 * @brief Bans specified user from a channel.
	 *
	 * The data that is required to deliver a ban is contained inside the
	 * string returned by /whois query.
	 *
	 * This will create a IRCDelayedOperation object and first send
	 * /whois <nickname> query. When the /whois returns the delayed
	 * operations are searched for pending bans. This is when
	 * bans are delivered.
	 *
	 * For end-user this effect should be almost completely invisible.
	 *
	 * @param nickName
	 *     Victim's nick.
	 * @param reason
	 *     Reason for ban (this will be delivered to /kick message).
	 * @param channel
	 *     Channel from which the user will be banned.
	 */
	void banUser(const QString &nickname, const QString &reason, const QString &channel);

	QList<IRCAdapterBase *> childrenAdapters();
	void connect();
	const IRCNetworkConnectionInfo &connection() const
	{
		return this->connectionInfo;
	}

	/**
	 * @brief Detaches the specified IRCChatAdapter instance from this
	 *        network @b without deleting it.
	 */
	void detachChatWindow(const IRCChatAdapter *pAdapter);

	void disconnect(const QString &farewellMessage);

	/**
	 * @brief Implemented to support direct communication between client
	 *        and server.
	 *
	 * All messages that do not begin with '/' character will be ignored
	 * here. In fact if the '/' character is missing an error() signal
	 * will be emitted to notify the user of this fact.
	 *
	 * Programmers must remember that although IRC protocol itself
	 * doesn't require clients to prepend the messages with a slash this
	 * class always does - the slash character is stripped, then the
	 * remainder of the message is sent 'as is'.
	 *
	 * @param pOrigin
	 *     If this is not null the IRCNetworkAdapter will attempt to pass
	 *     some message and error signals through this pOrigin. Otherwise
	 *     these signals will be sent directly.
	 */
	void doSendMessage(const QString &message, IRCAdapterBase *pOrigin) override;

	bool hasRecipient(const QString &recipient) const;

	const PatternList &ignoredUsersPatterns() const;

	/**
	 * @brief Checks if pAdapter equals this or is one of
	 *        chat windows of this network.
	 */
	bool isAdapterRelated(const IRCAdapterBase *pAdapter) const;
	bool isConnected() const
	{
		return ircClient.isConnected();
	}
	bool isMyNickname(const QString &nickname) const;

	/**
	 * @brief Checks if user is an operator on a given channel.
	 *
	 * This will work only for known channels (ie. the ones the client
	 * is registered on).
	 *
	 * @return True if given nickname has op privileges on given channel.
	 */
	bool isOperator(const QString &nickname, const QString &channel) const;

	void killAllChatWindows();

	const QString &myNickname() const
	{
		return connectionInfo.nick;
	}

	IRCNetworkAdapter *network() override
	{
		return this;
	}

	IRCResponseParser *responseParser()
	{
		return ircResponseParser;
	}

	/**
	 * @brief Sets channel flags.
	 *
	 * Flags are set using following command:
	 * @code
	 * /mode <channel> [-|+]<flag> <nickname> command
	 * @endcode
	 */
	void setChannelMode(const QString &channel, const QString &nickname, const QString &flag, bool bSet);

	void sendCtcp(const QString &nickname, const QString &command);

	/**
	 * @see bEmitAllIRCMessages
	 */
	void setEmitAllIRCMessagesEnabled(bool b)
	{
		this->bEmitAllIRCMessages = b;
	}

	void setNetworkEntity(const IRCNetworkEntity &network);

	QString title() const override;

	/**
	 * @brief All allowed modes with their nickname prefixes for this
	 *        network.
	 */
	const IRCUserPrefix &userPrefixes() const;

public slots:
	/**
	 * @brief Opens a new chat adapter for specified recipient.
	 *
	 * If specified recipient is a channel a /join command will be sent
	 * to that channel. If recipient is a user a chat window will simply
	 * be opened.
	 *
	 * If adapter is not connected to a network or empty name is specified
	 * this becomes a no-op. Also if such recipient
	 * is already present the window will emit focus signal request.
	 */
	void openNewAdapter(const QString &recipientName);

	void printMsgLiteral(const QString &recipient, const QString &content,
		const IRCMessageClass &msgClass);
	/**
	 * @see IRCResponseParser::print()
	 */
	void print(const QString &printWhat, const QString &printWhere);
	void printWithClass(const QString &printWhat, const QString &printWhere, const IRCMessageClass &msgClass);
	void printToCurrentChatBox(const QString &printWhat, const IRCMessageClass &msgClass);
	void reloadNetworkEntityFromConfig();
	void userPing(const QString &nickname, qint64 ping);

signals:
	/**
	 * @brief Signal emitted when a new chat (priv or channel) is opened
	 *        from this network.
	 */
	void newChatWindowIsOpened(IRCChatAdapter *pWindow);

private:
	/**
	 * @brief If set to true this network adapter is in a state
	 *        of joining to a network.
	 *
	 * Default: false.
	 */
	bool bIsJoining;

	/**
	 * @brief If true, will make the adapter emit message() signal for
	 *        every message it received.
	 *
	 * Such messages are emitted in an unformatted state - in a form they
	 * were received from the server.
	 *
	 * Default: false.
	 */
	bool bEmitAllIRCMessages;

	/**
	 * @brief Stores all chat adapters associated with this network.
	 *
	 * Key values are recipient names. To avoid confusion and errors
	 * all key values are @b lower-case.
	 * Values stored here can be any form of IRCChatAdapter - be it
	 * a private conversation or a channel.
	 */
	QHash<QString, IRCChatAdapter *> chatWindows;
	IRCNetworkConnectionInfo connectionInfo;
	IRCClient ircClient;
	IRCRequestParser ircRequestParser;
	IRCResponseParser *ircResponseParser;
	IRCISupportParser *ircISupportParser;
	IRCSocketSignalsAdapter *pIrcSocketSignalsAdapter;

	/**
	 * @brief Retrieves IRCChatAdapter for specified recipient.
	 */
	IRCChatAdapter *getChatAdapter(const QString &recipient);
	const IRCChatAdapter *getChatAdapter(const QString &recipient) const;

	/**
	 * @brief Creates the new IRCChatAdapter object and immediatelly
	 *        adds it to the chatWindows hashmap.
	 *
	 * @b Note: If such recipient is already registered no new object
	 * is created.
	 *
	 * @return Pointer to the adapter object.
	 */
	IRCChatAdapter *getOrCreateNewChatAdapter(const QString &recipient);

	void killChatWindow(const QString &recipient);

private slots:
	void appendISupportLine(const QString &line);
	void echoPrivmsg(const QString &recipient, const QString &content);
	void helloClient(const QString &nickname);
	void kick(const QString &channel, const QString &byWhom, const QString &whoIsKicked, const QString &reason);
	void kill(const QString &victim, const QString &comment);
	void ircServerResponse(const QString &message);
	void modeInfo(const QString &channel, const QString &whoSetThis, const QString &modeParams);
	void namesListReceived(const QString &channel, const QStringList &names);
	void namesListEndReceived(const QString &channel);
	void nicknameInUse(const QString &nickname);
	void noSuchNickname(const QString &nickname);
	void parseError(const QString &error);

	void privMsgReceived(const QString &recipient, const QString &sender, const QString &content);
	void sendPong(const QString &toWhom);
	void userChangesNickname(const QString &oldNickname, const QString &newNickname);
	void userIdleTime(const QString &nick, int secondsIdle);
	void userJoinsChannel(const QString &channel, const QString &nickname, const QString &fullSignature);
	void userModeChanged(const QString &channel, const QString &nickname,
		const QList<char> &addedFlags, const QList<char> &removedFlags);
	void userNetworkJoinDateTime(const QString &nick, const QDateTime &timeOfJoin);
	void userPartsChannel(const QString &channel, const QString &nickname, const QString &farewellMessage);
	void userQuitsNetwork(const QString &nickname, const QString &farewellMessage);
	void whoIsUser(const QString &nickname, const QString &user, const QString &hostName, const QString &realName);
};

class IRCSocketSignalsAdapter : public SocketSignalsAdapter
{
	Q_OBJECT

public:
	IRCNetworkAdapter *pParent;

	IRCSocketSignalsAdapter(IRCNetworkAdapter *pParent)
	{
		this->pParent = pParent;
	}

public slots:
	void connected() override;
	void disconnected() override;
	void errorReceived(QAbstractSocket::SocketError error) override;
	void infoMessage(const QString &message);
};

#endif
