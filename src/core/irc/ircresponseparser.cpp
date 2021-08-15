//------------------------------------------------------------------------------
// ircresponseparser.cpp
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
#include "ircresponseparser.h"

#include "irc/constants/ircresponsetype.h"
#include "irc/ircctcpparser.h"
#include "irc/ircglobal.h"
#include "irc/ircmessageclass.h"
#include "irc/ircnetworkadapter.h"
#include "irc/ircuserinfo.h"
#include "log.h"
#include "patternlist.h"
#include "strings.hpp"
#include <cassert>
#include <QDateTime>
#include <QRegularExpression>
#include <QStringList>

DClass<IRCResponseParser>
{
public:
	IRCNetworkAdapter *network;
	QString prefix;
	QString sender;
	QString type;
	QStringList params;
};

DPointered(IRCResponseParser)

IRCResponseParser::IRCResponseParser(IRCNetworkAdapter *network)
{
	d->network = network;
}

IRCResponseParser::~IRCResponseParser()
{
}

IRCResponseParser::FlagModes IRCResponseParser::getFlagMode(char c)
{
	switch (c)
	{
	case '+':
		return FlagModeAdd;

	case '-':
		return FlagModeRemove;

	default:
		return FlagModeError;
	}
}

bool IRCResponseParser::isPrefixIgnored() const
{
	return d->network->ignoredUsersPatterns().isExactMatchAny(d->prefix);
}

QString IRCResponseParser::joinAndTrimColonIfNecessary(const QStringList &strList) const
{
	QString joined = strList.join(" ");
	return this->trimColonIfNecessary(joined);
}

IRCResponseParseResult IRCResponseParser::parse(const QString &message)
{
	QString formattedMessage = message.trimmed();

	QRegularExpression prefixRegExp("(^:\\S+\\s)(.*)");
	auto match = prefixRegExp.match(formattedMessage);

	QString prefix = match.captured(1);
	QString remainder = formattedMessage.mid(prefix.length());

	d->prefix = Strings::triml(prefix, ":").trimmed();

	// Obtain message sender from the prefix.
	int indexExclamation = prefix.indexOf('!');
	if (indexExclamation > 0)
		d->sender = d->prefix.left(indexExclamation);
	else
		d->sender = d->prefix;

	QStringList msgParameters = remainder.split(" ");
	if (!msgParameters.isEmpty())
	{
		d->type = msgParameters.takeFirst();
		d->params = msgParameters;

		return parseMessage();
	}

	// Return invalid result.
	return IRCResponseParseResult();
}

IRCResponseParseResult IRCResponseParser::parseMessage()
{
	IRCResponseType responseType(d->type);
	IRCResponseType::MsgType enumType = responseType.type();

	switch (enumType)
	{
	case IRCResponseType::HelloClient:
	{
		QString nickname = d->params.takeFirst();

		emit helloClient(nickname);
		break;
	}

	case IRCResponseType::RPLAway:
	{
		d->params.takeFirst(); // Own nick.
		QString nickname = d->params.takeFirst();
		QString reason = joinAndTrimColonIfNecessary(d->params);
		emit printToNetworksCurrentChatBox(tr("User %1 is away: %2").arg(nickname, reason),
			IRCMessageClass::NetworkAction);
		break;
	}

	case IRCResponseType::RPLWhoIsUser:
	{
		// First param is unnecessary
		d->params.takeFirst();

		// Extract user info.
		QString nickname = d->params.takeFirst();
		QString user = d->params.takeFirst();
		QString hostName = d->params.takeFirst();
		QString realName = joinAndTrimColonIfNecessary(d->params);

		emit whoIsUser(nickname, user, hostName, realName);
		break;
	}

	case IRCResponseType::RPLWhoIsRegnick:
	case IRCResponseType::RPLWhoIsServer:
	case IRCResponseType::RPLWhoIsOperator:
	case IRCResponseType::RPLWhoIsHost:
	case IRCResponseType::RPLWhoIsModes:
	case IRCResponseType::RPLWhoIsSpecial:
	case IRCResponseType::RPLWhoIsBot:
	case IRCResponseType::RPLWhoIsSecure:
	case IRCResponseType::RPLWhoIsActually:
	case IRCResponseType::RPLEndOfWhoIs:
	{
		d->params.takeFirst(); // Own nick.
		emit printToNetworksCurrentChatBox(d->params.join(" "), IRCMessageClass::NetworkAction);
		break;
	}

	case IRCResponseType::RPLWhoIsIdle:
	{
		d->params.takeFirst(); // Own nick.
		QString nick = d->params.takeFirst();
		int secondsIdle = d->params.takeFirst().toInt();
		emit userIdleTime(nick, secondsIdle);
		if (d->params.first().toInt() != 0)
		{
			int joinedOn = d->params.takeFirst().toInt();
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
			emit userNetworkJoinDateTime(nick, QDateTime::fromSecsSinceEpoch(joinedOn));
#else
			emit userNetworkJoinDateTime(nick, QDateTime::fromTime_t(joinedOn));
#endif
		}
		break;
	}

	case IRCResponseType::RPLWhoIsChannels:
	{
		d->params.takeFirst(); // Own nick.
		QString nick = d->params.takeFirst();
		QString channels = joinAndTrimColonIfNecessary(d->params);
		emit printToNetworksCurrentChatBox(tr("%1 is on channels: %2").arg(nick, channels),
			IRCMessageClass::NetworkAction);
		break;
	}

	case IRCResponseType::RPLWhoIsAccount:
	{
		d->params.takeFirst(); // Own nick.
		QString nick = d->params.takeFirst();
		QString account = d->params.takeFirst();
		QString message = joinAndTrimColonIfNecessary(d->params);
		emit printToNetworksCurrentChatBox(QString("%1 %2 %3").arg(nick, message, account),
			IRCMessageClass::NetworkAction);
		break;
	}

	case IRCResponseType::RPLISupport:
	{
		d->params.takeFirst(); // Own nickname.
		emit iSupportReceived(d->params.join(" "));
		break;
	}

	case IRCResponseType::RPLTopic:
	{
		d->params.takeFirst(); // Own nickname.
		QString channel = d->params.takeFirst();
		QString topic = joinAndTrimColonIfNecessary(d->params);
		QString msg = tr("Topic: %1").arg(topic);
		emit printWithClass(msg, channel, IRCMessageClass(IRCMessageClass::ChannelAction));
		break;
	}

	case IRCResponseType::RPLTopicWhoTime:
	{
		d->params.takeFirst(); // Own nickname.
		QString channel = d->params.takeFirst();
		QString who = d->params.takeFirst();
		qint64 timestampSeconds = d->params.takeFirst().toLongLong();
		QDateTime date = QDateTime::fromMSecsSinceEpoch(timestampSeconds * 1000);
		QString msg = tr("Topic set by %1 on %2.").arg(who, date.toString("yyyy-MM-dd hh:mm:ss"));
		emit printWithClass(msg, channel, IRCMessageClass(IRCMessageClass::ChannelAction));
		break;
	}

	case IRCResponseType::RPLChannelUrl:
	{
		d->params.takeFirst(); // Own nickname.
		QString channel = d->params.takeFirst();
		QString url = joinAndTrimColonIfNecessary(d->params);
		emit printWithClass(tr("URL: %1").arg(url), channel, IRCMessageClass::ChannelAction);
		break;
	}

	case IRCResponseType::RPLCreationTime:
	{
		d->params.takeFirst(); // Own nickname.
		QString channel = d->params.takeFirst();
		QString time = joinAndTrimColonIfNecessary(d->params);
		emit printWithClass(tr("Created time: %1").arg(time), channel,
			IRCMessageClass::ChannelAction);
		break;
	}

	case IRCResponseType::RPLNamReply:
	{
		// Namelists.

		// Attempt to extract the channel name. For some reason
		// irc.skulltag.net returns a '=' character between the message type
		// signature and the channel name. RFC 1459 doesn't say anything about
		// such behavior, at least not in the chapter 6. We should protect
		// ourselves from such unwelcome surprises.
		QString channelName = "";
		while (!IRCGlobal::isChannelName(channelName) && !d->params.isEmpty())
		{
			channelName = d->params.takeFirst();
		}

		if (channelName.isEmpty())
		{
			emit parseError(tr("RPLNamReply: Received names list but no channel name."));
			return IRCResponseParseResult();
		}

		// Remaining values will be user names. Send them all as a strings list.
		// Remember to remove the ":" character from the first name.
		if (!d->params.isEmpty())
			d->params[0] = d->params[0].remove(0, 1);

		emit namesListReceived(channelName, d->params);
		break;
	}

	case IRCResponseType::RPLEndOfNames:
	{
		QString channel = d->params[1];
		emit namesListEndReceived(channel);
		break;
	}

	case IRCResponseType::RPLMOTDStart:
	case IRCResponseType::RPLMOTD:
	case IRCResponseType::RPLEndOfMOTD:
	{
		// First param is username, drop it.
		d->params.takeFirst();

		if (enumType == IRCResponseType::RPLMOTDStart)
		{
			// We will print additional separator line if it's a start of
			// the MOTD.
			emit print("\n----------------------", QString());
		}

		emit print(joinAndTrimColonIfNecessary(d->params), QString());

		if (enumType == IRCResponseType::RPLEndOfMOTD)
		{
			// Again, we will print additional separator line if it's the
			// end of the MOTD.
			emit print("----------------------\n", QString());
		}

		break;
	}

	// Extract correct message and print it.
	case IRCResponseType::RPLLUserClient:
	case IRCResponseType::RPLLUserMe:
	{
		// Drop the first param.
		d->params.takeFirst();

		// Join and print the rest.
		emit print(joinAndTrimColonIfNecessary(d->params), QString());
		break;
	}

	// Extract correct message and print it.
	case IRCResponseType::RPLLUserOp:
	case IRCResponseType::RPLLUserUnknown:
	case IRCResponseType::RPLLUserChannels:
	{
		// Drop the first param.
		d->params.takeFirst();

		// Here the first param is always an integer, and colon is located
		// afterwards.
		QString number = d->params.takeFirst();
		emit print(number + " " + joinAndTrimColonIfNecessary(d->params), QString());
		break;
	}

	case IRCResponseType::ERRNoSuchNick:
	{
		// Drop the first param.
		d->params.takeFirst();

		// This is the real nickname.
		QString nickname = d->params.takeFirst();

		emit noSuchNickname(nickname);
		break;
	}

	case IRCResponseType::ERRErroneousNickname:
	{
		// Own nickname.
		d->params.takeFirst();
		QString badNick = d->params.takeFirst();
		QString msg = tr("Erroneous nickname: %1").arg(badNick);
		if (d->params.join(" ").compare(":Erroneous nickname", Qt::CaseInsensitive) != 0)
			msg += tr(" (%1)").arg(joinAndTrimColonIfNecessary(d->params));
		emit printToNetworksCurrentChatBox(msg, IRCMessageClass::Error);
		break;
	}

	case IRCResponseType::ERRNicknameInUse:
	{
		// Drop the first param.
		d->params.takeFirst();

		QString nickname = d->params.takeFirst();

		emit nicknameInUse(nickname);
		break;
	}

	case IRCResponseType::ERRChannelIsFull:
	case IRCResponseType::ERRInviteOnlyChan:
	case IRCResponseType::ERRBannedFromChan:
	case IRCResponseType::ERRBadChannelKey:
	case IRCResponseType::ERRBadChannelMask:
	case IRCResponseType::ERRNoChanModes:
	case IRCResponseType::ERRCannotSendToChan:
	case IRCResponseType::ERRChanOpPrivIsNeeded:
	{
		d->params.takeFirst(); // User
		QString channel = d->params.takeFirst();
		QString reason = joinAndTrimColonIfNecessary(d->params);
		switch (enumType)
		{
		case IRCResponseType::ERRChannelIsFull:
		case IRCResponseType::ERRInviteOnlyChan:
		case IRCResponseType::ERRBannedFromChan:
		case IRCResponseType::ERRBadChannelKey:
			emit printToNetworksCurrentChatBox(tr("%1: %2").arg(channel, reason),
				IRCMessageClass::Error);
			break;
		default:
			emit printWithClass(reason, channel, IRCMessageClass::ChannelAction);
			break;
		}

		break;
	}

	case IRCResponseType::Join:
	{
		QString channel = d->params[0];
		trimColonIfNecessary(channel);

		emit userJoinsChannel(channel, d->sender, d->prefix);
		break;
	}

	case IRCResponseType::Kick:
	{
		QString channel = d->params.takeFirst();
		QString whoIsKicked = d->params.takeFirst();

		QString reason = joinAndTrimColonIfNecessary(d->params);

		emit kick(channel, d->sender, whoIsKicked, reason);
		break;
	}

	case IRCResponseType::Kill:
	{
		QString victim = d->params.takeFirst();
		QString comment = joinAndTrimColonIfNecessary(d->params);
		emit kill(victim, comment);
		break;
	}

	case IRCResponseType::Mode:
	{
		QString channel = d->params.takeFirst();
		QString flagsString = d->params.takeFirst();

		// If there are no more params left on the list then this modes
		// are for the channel itself. Otherwise they are for the users.
		if (!d->params.isEmpty())
		{
			emit modeInfo(channel, d->sender, flagsString + " " + d->params.join(" "));
			parseUserModeMessage(channel, flagsString, d->params);
		}

		break;
	}

	case IRCResponseType::Nick:
	{
		QString oldNickname = d->sender;
		QString newNickname = d->params[0];
		trimColonIfNecessary(newNickname);

		emit userChangesNickname(oldNickname, newNickname);
		break;
	}

	case IRCResponseType::Part:
	{
		QString farewellMessage = QString();
		QString channel = d->params[0];

		if (d->params.size() > 1)
		{
			d->params.pop_front();

			farewellMessage = joinAndTrimColonIfNecessary(d->params);
		}

		emit userPartsChannel(channel, d->sender, farewellMessage);
		break;
	}

	case IRCResponseType::Ping:
	{
		QString pongToWhom = d->params[0];

		emit sendPongMessage(pongToWhom);
		break;
	}

	case IRCResponseType::PrivMsg:
	case IRCResponseType::Notice:
		parsePrivMsgOrNotice();
		break;

	case IRCResponseType::Quit:
	{
		QString farewellMessage = QString();
		farewellMessage = joinAndTrimColonIfNecessary(d->params);

		emit userQuitsNetwork(d->sender, farewellMessage);
		break;
	}

	case IRCResponseType::Topic:
	{
		QString channel = d->params.takeFirst();
		QString topic = joinAndTrimColonIfNecessary(d->params);
		QString msg = tr("New topic set by user %1:\n%2").arg(d->sender, topic);
		emit printWithClass(msg, channel, IRCMessageClass(IRCMessageClass::ChannelAction));
		break;
	}

	case IRCResponseType::Invalid:
	{
		// Messages below 100 are some generic server responses to connect
		// event.
		if (responseType.numericType() > 1)
		{
			emit print(joinAndTrimColonIfNecessary(d->params), "");
			return IRCResponseParseResult(responseType, true);
		}

		return IRCResponseParseResult(responseType, false);
	}

	default:
		emit parseError(tr(
			"IRCResponseParser: Type '%1' was recognized but there has been no parse code implemented for it.\
(yep, it's a bug in the application!)"
			).arg(d->type));
		return IRCResponseParseResult(responseType, true);
	}

	return IRCResponseParseResult(responseType, true);
}

void IRCResponseParser::parsePrivMsgOrNotice()
{
	if (isPrefixIgnored())
		return;
	QString recipient = d->params.takeFirst();
	if (!IRCGlobal::isChannelName(recipient))
	{
		// If recipient name is not the channel the
		// "recipient" QString will point to this client's user.
		// In order to get a proper recipient we need to use the
		// "sender" QString instead.
		recipient = d->sender;
	}

	// Join the list to form message contents.
	QString content = joinAndTrimColonIfNecessary(d->params);

	IRCResponseType responseType(d->type);
	IRCCtcpParser::MessageType ctcpMsgType = (responseType == IRCResponseType::Notice) ?
		IRCCtcpParser::Reply : IRCCtcpParser::Request;
	IRCCtcpParser ctcp(d->network, d->sender, recipient, content, ctcpMsgType);
	if (ctcp.parse())
	{
		switch (ctcp.echo())
		{
		case IRCCtcpParser::PrintAsNormalMessage:
			emit privMsgLiteralReceived(recipient, ctcp.printable(), IRCMessageClass::Ctcp);
			break;
		case IRCCtcpParser::DisplayInServerTab:
			emit printWithClass(ctcp.printable(), QString(), IRCMessageClass::Ctcp);
			break;
		case IRCCtcpParser::DisplayThroughGlobalMessage:
			emit printToNetworksCurrentChatBox(ctcp.printable(), IRCMessageClass::Ctcp);
			break;
		case IRCCtcpParser::DontShow:
			break;
		default:
			gLog << QString("Unhandled CTCP echo type: %1").arg(ctcp.echo());
			assert(false && "Unhandled CTCP echo type");
			break;
		}
		if (!ctcp.reply().isEmpty() && responseType.type() != IRCResponseType::Notice)
			d->network->sendMessage(QString("/NOTICE %1 %2%3%2").arg(d->sender, QChar(0x1), ctcp.reply()));
	}
	else
	{
		if (responseType == IRCResponseType::PrivMsg)
			emit privMsgReceived(recipient, d->sender, content);
		else if (responseType == IRCResponseType::Notice)
			emit print(tr("[%1]: %2").arg(d->sender, content), recipient);
		else
			emit parseError(tr("Type '%1' was incorrectly parsed in PrivMsg block.").arg(d->type));
	}
}

void IRCResponseParser::parseUserModeMessage(const QString &channel, QString flagsString, QStringList &nicknames)
{
	// For each flag character there should be one nickname on the list.
	// If there are less nicknames than characters we will simply abort.
	// Of course add/subtract characters are not counted here.

	// The first character should always define the flagMode.
	FlagModes flagMode = getFlagMode(flagsString[0].toLatin1());

	if (flagMode == FlagModeError)
	{
		emit parseError(tr("MODE flags string from IRC server are incorrect: \"%1\". "
			"Information for channel \"%2\" might not be correct anymore.")
				.arg(flagsString, channel));
		return;
	}

	for (int i = 1; i < flagsString.size(); ++i)
	{
		char flagChar = flagsString[i].toLatin1();

		FlagModes tmpFlagMode = getFlagMode(flagChar);
		if (tmpFlagMode == FlagModeError)
		{
			if (nicknames.empty())
				return;

			QList<char> addedFlags;
			QList<char> removedFlags;

			QString name = nicknames[0];

			switch (flagMode)
			{
			case FlagModeAdd:
				addedFlags << flagChar;
				break;

			case FlagModeRemove:
				removedFlags << flagChar;
				break;

			default:
				emit parseError(tr("IRCResponseParser::parseUserModeMessage(): "
					"wrong FlagMode. Information for channel \"%2\" might not be correct anymore."));
				return;
			}

			emit userModeChanged(channel, name, addedFlags, removedFlags);
			// Drop a name from the list and continue.
			nicknames.pop_front();
		}
		else
		{
			flagMode = tmpFlagMode;
			continue;
		}
	}
}

QString &IRCResponseParser::trimColonIfNecessary(QString &str) const
{
	if (!str.isEmpty() && str[0] == ':')
		str.remove(0, 1);

	return str;
}
