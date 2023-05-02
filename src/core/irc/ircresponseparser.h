//------------------------------------------------------------------------------
// ircresponseparser.h
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
#ifndef __IRCRESPONSEPARSER_H__
#define __IRCRESPONSEPARSER_H__

#include "dptr.h"
#include "irc/entities/ircresponseparseresult.h"

#include <QDateTime>
#include <QObject>
#include <QString>

class IRCMessageClass;
class IRCNetworkAdapter;

class IRCResponseParser : public QObject
{
	Q_OBJECT

public:
	IRCResponseParser(IRCNetworkAdapter *network);
	~IRCResponseParser() override;

	/**
	 * @brief Parses the message received from the network.
	 *
	 * Parsing a message will result in emitting an appropriate signal.
	 * For example is message says that some user was kicked, a kick()
	 * signal will be emitted.
	 *
	 * @return Instance of IRCResponseParseResult with defined type of
	 *         the message and info if message was actually parsed or not.
	 */
	IRCResponseParseResult parse(const QString &message);

signals:
	/**
	 * @brief Emitted when user successfully joins the network.
	 *
	 * @param nickname
	 *     Nickname with which the user has just registered.
	 */
	void helloClient(const QString &nickname);

	void kick(const QString &channel, const QString &byWhom, const QString &whoIsKicked, const QString &reason);
	void kill(const QString &victim, const QString &comment);

	void iSupportReceived(const QString &line);

	/**
	 * @brief Carries info about MODE for display.
	 */
	void modeInfo(const QString &channel, const QString &whoSetThis, const QString &modeParams);
	void namesListReceived(const QString &channel, const QStringList &names);
	void namesListEndReceived(const QString &channel);
	void nicknameInUse(const QString &nickname);
	void noSuchNickname(const QString &nickname);
	void parseError(const QString &error);

	/**
	 * @brief Tells the network adapter to print a message.
	 *
	 * @param printWhat
	 *      This will be printed.
	 * @param printWhere
	 *      Name of target adapter where the message should be printed. If
	 *      empty, message is printed directly through the
	 *      IRCNetworkAdapter itself.
	 */
	void print(const QString &printWhat, const QString &printWhere);
	/**
	 * @brief Same as print(), but allows to specify message class.
	 */
	void printWithClass(const QString &printWhat, const QString &printWhere, const IRCMessageClass &msgClass);
	void printToNetworksCurrentChatBox(const QString &printWhat, const IRCMessageClass &msgClass);

	void privMsgReceived(const QString &recipient, const QString &sender, const QString &content);
	/**
	 * @brief Create chat window if necessary and display message
	 *        'as is' without further string gluing.
	 */
	void privMsgLiteralReceived(const QString &recipient, const QString &content, const IRCMessageClass &msgClass);
	void sendPongMessage(const QString &sendWhere);
	void userChangesNickname(const QString &oldNickname, const QString &newNickname);
	void userIdleTime(const QString &nick, int secondsIdle);
	void userJoinsChannel(const QString &channel, const QString &nickname, const QString &fullSignature);
	void userNetworkJoinDateTime(const QString &nick, const QDateTime &timeOfJoin);
	void userModeChanged(const QString &channel, const QString &nickname,
		const QList<char> &flagsAdded, const QList<char> &flagsRemoved);
	void userPartsChannel(const QString &channel, const QString &nickname, const QString &farewellMessage);
	void userQuitsNetwork(const QString &nickname, const QString &farewellMessage);

	/**
	 * @brief Emitted with response 311 RPL_WHOISUSER.
	 */
	void whoIsUser(const QString &nickname, const QString &user, const QString &hostName, const QString &realName);

private:
	enum FlagModes
	{
		FlagModeAdd,
		FlagModeRemove,
		FlagModeError
	};

	DPtr<IRCResponseParser> d;

	bool isPrefixIgnored() const;
	QString joinAndTrimColonIfNecessary(const QStringList &strList) const;

	/**
	 * @brief Will return FlagModeError if the given character cannot be
	 * translated.
	 */
	FlagModes getFlagMode(char c);

	IRCResponseParseResult parseMessage();

	void parsePrivMsgOrNotice();
	/**
	 * @brief Will emit userModeChanged() for each character in flagsString.
	 *
	 * This method will "damage" the passed nicknames parameter.
	 * Use with care.
	 */
	void parseUserModeMessage(const QString &channel, QString flagsString, QStringList &nicknames);

	QString &trimColonIfNecessary(QString &str) const;
};

#endif
