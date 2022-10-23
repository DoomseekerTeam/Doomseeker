//------------------------------------------------------------------------------
// ircnetworkadapter.cpp
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
#include "irc/configuration/chatnetworkscfg.h"
#include "irc/configuration/ircconfig.h"
#include "irc/entities/ircuserprefix.h"
#include "irc/ircchanneladapter.h"
#include "irc/ircglobal.h"
#include "irc/ircisupportparser.h"
#include "irc/ircmessageclass.h"
#include "irc/ircprivadapter.h"
#include "irc/ircrequestparser.h"
#include "irc/ircresponseparser.h"
#include "irc/ircuserinfo.h"
#include "irc/ircuserlist.h"
#include "irc/ops/ircdelayedoperationban.h"
#include "ircnetworkadapter.h"
#include "log.h"
#include <QDateTime>

IRCNetworkAdapter::IRCNetworkAdapter(IRCNetworkConnectionInfo connectionInfo)
{
	this->connectionInfo = connectionInfo;
	this->bIsJoining = false;
	this->bEmitAllIRCMessages = false;

	pIrcSocketSignalsAdapter = new IRCSocketSignalsAdapter(this);
	ircISupportParser = new IRCISupportParser();
	ircResponseParser = new IRCResponseParser(this);
	ircClient.connectSocketSignals(pIrcSocketSignalsAdapter);

	QObject::connect(&ircClient, SIGNAL(ircServerResponse(const QString&)),
		this, SLOT(ircServerResponse(const QString&)));

	// Request parser
	QObject::connect(&ircRequestParser, SIGNAL(echoPrivmsg(const QString&,const QString&)),
		this, SLOT(echoPrivmsg(const QString&,const QString&)));

	QObject::connect(&ircRequestParser, SIGNAL(query(const QString&)),
		this, SLOT(openNewAdapter(const QString&)));

	// Response parser begins here.
	QObject::connect(ircResponseParser, SIGNAL(helloClient(const QString&)),
		this, SLOT(helloClient(const QString&)));

	QObject::connect(ircResponseParser, SIGNAL(iSupportReceived(QString)),
		SLOT(appendISupportLine(QString)));

	QObject::connect(ircResponseParser, SIGNAL(kick(const QString&,const QString&,const QString&,const QString&)),
		this, SLOT(kick(const QString&,const QString&,const QString&,const QString&)));

	QObject::connect(ircResponseParser, SIGNAL(kill(const QString&,const QString&)),
		this, SLOT(kill(const QString&,const QString&)));

	QObject::connect(ircResponseParser, SIGNAL(modeInfo(const QString&,const QString&,const QString&)),
		this, SLOT(modeInfo(const QString&,const QString&,const QString&)));

	QObject::connect(ircResponseParser, SIGNAL(namesListReceived(const QString&,const QStringList&)),
		this, SLOT(namesListReceived(const QString&,const QStringList&)));

	QObject::connect(ircResponseParser, SIGNAL(namesListEndReceived(const QString&)),
		this, SLOT(namesListEndReceived(const QString&)));

	QObject::connect(ircResponseParser, SIGNAL(nicknameInUse(const QString&)),
		this, SLOT(nicknameInUse(const QString&)));

	QObject::connect(ircResponseParser, SIGNAL(noSuchNickname(const QString&)),
		this, SLOT(noSuchNickname(const QString&)));

	QObject::connect(ircResponseParser, SIGNAL (parseError(const QString&)),
		this, SLOT(parseError(const QString&)));

	// This connect must be direct as it might interfere with other operations
	// of printing done in the window.
	QObject::connect(ircResponseParser, SIGNAL(print(const QString&,const QString&)),
		this, SLOT(print(const QString&,const QString&)), Qt::DirectConnection);
	QObject::connect(ircResponseParser,
		SIGNAL(printWithClass(const QString&,const QString&,const IRCMessageClass&)),
		this, SLOT(printWithClass(const QString&,const QString&,const IRCMessageClass&)),
		Qt::DirectConnection);
	QObject::connect(ircResponseParser, SIGNAL(printToNetworksCurrentChatBox(QString,IRCMessageClass)),
		SIGNAL(messageToNetworksCurrentChatBox(QString,IRCMessageClass)));

	QObject::connect(ircResponseParser, SIGNAL (privMsgReceived(const QString&,const QString&,const QString&)),
		this, SLOT(privMsgReceived(const QString&,const QString&,const QString&)));
	QObject::connect(ircResponseParser, SIGNAL (privMsgLiteralReceived(QString,QString,IRCMessageClass)),
		this, SLOT(printMsgLiteral(QString,QString,IRCMessageClass)));

	QObject::connect(ircResponseParser, SIGNAL (sendPongMessage(const QString&)),
		this, SLOT(sendPong(const QString&)));

	QObject::connect(ircResponseParser, SIGNAL (userChangesNickname(const QString&,const QString&)),
		this, SLOT(userChangesNickname(const QString&,const QString&)));

	QObject::connect(ircResponseParser, SIGNAL(userIdleTime(QString,int)),
		this, SLOT(userIdleTime(QString,int)));

	QObject::connect(ircResponseParser, SIGNAL (userJoinsChannel(const QString&,const QString&,const QString&)),
		this, SLOT(userJoinsChannel(const QString&,const QString&,const QString&)));

	QObject::connect(ircResponseParser,
		SIGNAL(userModeChanged(const QString&,const QString&,const QList<char>&,const QList<char>&)),
		this,
		SLOT(userModeChanged(const QString&,const QString&,const QList<char>&,const QList<char>&)));

	QObject::connect(ircResponseParser, SIGNAL(userNetworkJoinDateTime(QString,QDateTime)),
		this, SLOT(userNetworkJoinDateTime(QString,QDateTime)));

	QObject::connect(ircResponseParser, SIGNAL (userPartsChannel(const QString&,const QString&,const QString&)),
		this, SLOT(userPartsChannel(const QString&,const QString&,const QString&)));

	QObject::connect(ircResponseParser, SIGNAL (userQuitsNetwork(const QString&,const QString&)),
		this, SLOT(userQuitsNetwork(const QString&,const QString&)));

	QObject::connect(ircResponseParser, SIGNAL (whoIsUser(const QString&,const QString&,const QString&,const QString&)),
		this, SLOT(whoIsUser(const QString&,const QString&,const QString&,const QString&)));
}

IRCNetworkAdapter::~IRCNetworkAdapter()
{
	disconnect(gIRCConfig.personal.quitMessage);

	killAllChatWindows();
	delete this->ircResponseParser;
	delete this->ircISupportParser;
	delete this->pIrcSocketSignalsAdapter;
}

void IRCNetworkAdapter::appendISupportLine(const QString &line)
{
	print(line, QString());
	ircISupportParser->appendLine(line);
	ircISupportParser->parse();
}

void IRCNetworkAdapter::banUser(const QString &nickname, const QString &reason, const QString &channel)
{
	auto op = new IRCDelayedOperationBan(this, channel, nickname, this);
	op->setReason(reason);
	op->start();
}

QList<IRCAdapterBase *> IRCNetworkAdapter::childrenAdapters()
{
	QList<IRCAdapterBase *> result;
	for (IRCChatAdapter *adapter : chatWindows.values())
	{
		result << adapter;
	}
	return result;
}

void IRCNetworkAdapter::connect()
{
	emit titleChange();
	ircClient.connect(connectionInfo.networkEntity.address(), connectionInfo.networkEntity.port());
}

void IRCNetworkAdapter::detachChatWindow(const IRCChatAdapter *pAdapter)
{
	chatWindows.remove(pAdapter->recipient().toLower());
}

void IRCNetworkAdapter::disconnect(const QString &farewellMessage)
{
	sendMessage("/quit " + farewellMessage);
	ircClient.disconnect();
}

void IRCNetworkAdapter::doSendMessage(const QString &message, IRCAdapterBase *pOrigin)
{
	if (pOrigin == nullptr)
		pOrigin = this;

	if (!ircClient.isConnected())
	{
		pOrigin->emitError(tr("You are not connected to the network."));
		return;
	}

	IRCRequestParser::IRCRequestParseResult result = ircRequestParser.parse(pOrigin, message);
	QString parsedMessage = ircRequestParser.output();

	switch (result)
	{
	case IRCRequestParser::ErrorInputInsufficientParameters:
		pOrigin->emitError(tr("Insufficient parameters."));
		break;

	case IRCRequestParser::ErrorInputNotPrependedWithSlash:
		emit error(tr("This is a server window. All commands must be prepended with a '/' character."));
		break;

	case IRCRequestParser::ErrorMessageEmpty:
		pOrigin->emitError(tr("Attempted to send empty message."));
		break;

	case IRCRequestParser::ErrorMessageTooLong:
		pOrigin->emitError(tr("Command is too long."));
		break;

	case IRCRequestParser::ErrorChatWindowOnly:
		pOrigin->emitError(tr("Not a chat window."));
		break;

	case IRCRequestParser::Ok:
		ircClient.sendMessage(parsedMessage);
		break;

	case IRCRequestParser::QuitCommand:
		ircClient.sendMessage(parsedMessage);
		emit messageWithClass(tr("Quit"), IRCMessageClass::NetworkAction);
		break;
	}
}

void IRCNetworkAdapter::echoPrivmsg(const QString &recipient, const QString &content)
{
	// We will echo only chat messages for recipients for whom
	// we have windows open.
	if (hasRecipient(recipient))
	{
		const QString &sender = this->myNickname();
		privMsgReceived(recipient, sender, content);
	}
}

IRCChatAdapter *IRCNetworkAdapter::getChatAdapter(const QString &recipient)
{
	if (recipient.isEmpty())
	{
		emit error("Doomseeker error: getChatAdapter() received empty recipient.");
		return nullptr;
	}

	QString recipientLowercase = recipient.toLower();
	if (hasRecipient(recipientLowercase))
		return chatWindows[recipientLowercase];

	return nullptr;
}

const IRCChatAdapter *IRCNetworkAdapter::getChatAdapter(const QString &recipient) const
{
	if (recipient.isEmpty())
		return nullptr;

	QString recipientLowercase = recipient.toLower();
	if (hasRecipient(recipientLowercase))
		return chatWindows[recipientLowercase];

	return nullptr;
}

IRCChatAdapter *IRCNetworkAdapter::getOrCreateNewChatAdapter(const QString &recipient)
{
	IRCChatAdapter *pAdapter = nullptr;

	if (recipient.isEmpty())
	{
		emit error("Doomseeker error: getOrCreateNewChatAdapter() received empty recipient.");
		return nullptr;
	}

	QString recipientLowercase = recipient.toLower();

	if (hasRecipient(recipientLowercase))
		return chatWindows[recipientLowercase];

	#ifndef NDEBUG
	Log::instance << QString("IRCNetworkAdapter::getOrCreateNewChatAdapter() Creating new adapter for recipient: %1").arg(recipientLowercase);
	#endif

	if (IRCGlobal::isChannelName(recipient))
		pAdapter = new IRCChannelAdapter(this, recipient);
	else
		pAdapter = new IRCPrivAdapter(this, recipient);

	chatWindows.insert(recipientLowercase, pAdapter);
	emit newChatWindowIsOpened(pAdapter);

	return pAdapter;
}

bool IRCNetworkAdapter::hasRecipient(const QString &recipient) const
{
	QString recipientLowercase = recipient.toLower();
	return chatWindows.find(recipientLowercase) != chatWindows.end();
}

void IRCNetworkAdapter::helloClient(const QString &nickname)
{
	// This method can only be called
	// when network is in joining state.

	connectionInfo.nick = nickname;
	IRCNetworkEntity &network = connectionInfo.networkEntity;

	gLog << tr("IRC: Successfully registered on network %1 [%2:%3]").arg(network.description(), network.address()).arg(network.port());

	this->bIsJoining = false;

	if (!network.nickservPassword().isEmpty())
	{
		QString messageNickserv = network.nickservCommand();
		messageNickserv = messageNickserv.arg(network.nickservPassword());

		this->sendMessage(messageNickserv);
	}
	for (const QString &command : network.autojoinCommands())
	{
		if (!command.trimmed().isEmpty())
			this->sendMessage(command);
	}

	for (const QString &channel : network.autojoinChannels())
	{
		if (IRCGlobal::isChannelName(channel))
			this->openNewAdapter(channel);
	}

	// Emit this just to be safe.
	emit titleChange();
}

const PatternList &IRCNetworkAdapter::ignoredUsersPatterns() const
{
	return connection().networkEntity.ignoredUsers();
}

void IRCNetworkAdapter::ircServerResponse(const QString &message)
{
	IRCResponseParseResult result = ircResponseParser->parse(message);

	if (this->bEmitAllIRCMessages || !result.wasParsed())
		emit this->message(message.trimmed().replace("\n", "\\n"));

	if (!result.isValid())
		emit this->error(tr("Invalid parse result for message: %1").arg(message));
}

bool IRCNetworkAdapter::isAdapterRelated(const IRCAdapterBase *pAdapter) const
{
	if (this == pAdapter)
		return true;

	QList<IRCChatAdapter *> adaptersList = chatWindows.values();
	for (IRCChatAdapter *pChatWindow : adaptersList)
	{
		if (pChatWindow == pAdapter)
			return true;
	}

	return false;
}

bool IRCNetworkAdapter::isMyNickname(const QString &nickname) const
{
	IRCUserInfo myUserInfo(this->connectionInfo.nick, this);
	return myUserInfo.isSameNickname(nickname);
}

bool IRCNetworkAdapter::isOperator(const QString &nickname, const QString &channel) const
{
	if (IRCGlobal::isChannelName(channel))
	{
		const auto pChannelAdapter = (const IRCChannelAdapter *) this->getChatAdapter(channel);
		if (pChannelAdapter != nullptr)
			return pChannelAdapter->isOperator(nickname);
	}

	return false;
}

void IRCNetworkAdapter::kick(const QString &channel, const QString &byWhom, const QString &whoIsKicked, const QString &reason)
{
	if (hasRecipient(channel))
	{
		auto pAdapter = (IRCChannelAdapter *) this->getOrCreateNewChatAdapter(channel);

		if (isMyNickname(whoIsKicked))
		{
			this->emitMessageWithClass(tr("You have been kicked from channel %1 by %2 (Reason: %3)").arg(channel, byWhom, reason), IRCMessageClass::ChannelAction);
			killChatWindow(channel);
		}
		else
		{
			pAdapter->emitMessageWithClass(tr("%1 was kicked by %2 (%3)").arg(whoIsKicked, byWhom, reason), IRCMessageClass::ChannelAction);
			pAdapter->removeNameFromCachedList(whoIsKicked);
		}
	}
}

void IRCNetworkAdapter::kill(const QString &victim, const QString &comment)
{
	emit message(QString("Connection for user %1 was killed. (%2)").arg(victim, comment));

	// We need to iterate through EVERY adapter and notify them
	// about this quit.
	// Implementation of each adapter should recognize if this quit actually
	// has anything to do with that adapter.
	QList<IRCChatAdapter *> adaptersList = chatWindows.values();
	for (IRCChatAdapter *pAdapter : adaptersList)
	{
		pAdapter->userLeaves(victim, comment, IRCChatAdapter::NetworkKill);
	}
}

void IRCNetworkAdapter::killAllChatWindows()
{
	QList<IRCChatAdapter *> pWindows = chatWindows.values();
	for (IRCChatAdapter *pAdapter : pWindows)
	{
		// Make sure that the adapter destructor won't call the
		// detachChatWindow() method or the program will be shot to oblivion.
		pAdapter->setNetwork(nullptr);
		delete pAdapter;
	}

	chatWindows.clear();
}

void IRCNetworkAdapter::killChatWindow(const QString &recipient)
{
	if (hasRecipient(recipient))
	{
		IRCChatAdapter *pAdapter = getChatAdapter(recipient);
		chatWindows.remove(recipient.toLower());

		// Make sure that the adapter destructor won't call the
		// detachChatWindow() method or the program will be shot to oblivion.
		pAdapter->setNetwork(nullptr);
		delete pAdapter;
	}
}

void IRCNetworkAdapter::modeInfo(const QString &channel, const QString &whoSetThis, const QString &modeParams)
{
	auto pAdapter = (IRCChannelAdapter *) this->getOrCreateNewChatAdapter(channel);
	pAdapter->emitMessageWithClass(tr("%1 sets mode: %2").arg(whoSetThis, modeParams), IRCMessageClass::ChannelAction);
}

void IRCNetworkAdapter::namesListReceived(const QString &channel, const QStringList &names)
{
	if (this->hasRecipient(channel))
	{
		auto pAdapter = (IRCChannelAdapter *) this->getOrCreateNewChatAdapter(channel);
		pAdapter->appendNamesToCachedList(names);
	}
}

void IRCNetworkAdapter::namesListEndReceived(const QString &channel)
{
	if (this->hasRecipient(channel))
	{
		auto pAdapter = (IRCChannelAdapter *) this->getOrCreateNewChatAdapter(channel);
		pAdapter->emitCachedNameListUpdated();
	}
}

void IRCNetworkAdapter::nicknameInUse(const QString &nickname)
{
	emit messageToNetworksCurrentChatBox(tr("Nickname %1 is already taken.").arg(nickname), IRCMessageClass::Error);
	if (this->bIsJoining)
	{
		const QString &altNick = this->connectionInfo.alternateNick;

		if (this->connectionInfo.nick.compare(altNick, Qt::CaseInsensitive) == 0)
			emit messageWithClass(tr("Both nickname and alternate nickname are taken on this network."), IRCMessageClass::Error);
		else if (altNick.isEmpty())
			emit messageWithClass(tr("No alternate nickname specified."), IRCMessageClass::Error);
		else
		{
			this->connectionInfo.nick = altNick;

			emit messageWithClass(tr("Using alternate nickname %1 to join.").arg(altNick), IRCMessageClass::NetworkAction);
			QString message = QString("/nick %1").arg(altNick);
			sendMessage(message);
		}
	}
}

void IRCNetworkAdapter::noSuchNickname(const QString &nickname)
{
	emit messageToNetworksCurrentChatBox(tr("User %1 is not logged in.").arg(nickname), IRCMessageClass::Error);
}

void IRCNetworkAdapter::openNewAdapter(const QString &recipientName)
{
	if (!isConnected() || recipientName.isEmpty())
		return;

	bool bStandardRoutine = !IRCGlobal::isChannelName(recipientName)
		|| hasRecipient(recipientName);

	if (bStandardRoutine)
	{
		IRCChatAdapter *pAdapter = this->getOrCreateNewChatAdapter(recipientName);
		pAdapter->emitFocusRequest();
	}
	else if (IRCGlobal::isChannelName(recipientName))
		this->sendMessage("/join " + recipientName);
}

void IRCNetworkAdapter::parseError(const QString &error)
{
	emit this->error(tr("IRC parse error: %1").arg(error));
}

void IRCNetworkAdapter::print(const QString &printWhat, const QString &printWhere)
{
	printWithClass(printWhat, printWhere, IRCMessageClass(IRCMessageClass::Normal));
}

void IRCNetworkAdapter::printWithClass(const QString &printWhat,
	const QString &printWhere, const IRCMessageClass &msgClass)
{
	IRCAdapterBase *pAdapter = this;

	if (!printWhere.isEmpty())
	{
		IRCAdapterBase *pAdapterCandidate = getChatAdapter(printWhere);
		if (pAdapterCandidate != nullptr)
			pAdapter = pAdapterCandidate;
	}

	// In case if the target adapter is unknown, the message will still get
	// printed to this adapter.
	if (pAdapter == nullptr)
		this->emitMessageWithClass(tr("FROM %1: %2").arg(printWhere, printWhat), msgClass);
	else
	{
		// If bEmitAllIRCMessages is set to true, the message will be already
		// printed for this adapter in ircServerResponse().
		// There is no need to print it again.
		if ( pAdapter == this && this->bEmitAllIRCMessages )
			return;

		pAdapter->emitMessageWithClass(printWhat, msgClass);
	}
}

void IRCNetworkAdapter::printToCurrentChatBox(const QString &printWhat, const IRCMessageClass &msgClass)
{
	emit messageToNetworksCurrentChatBox(printWhat, msgClass);
}

void IRCNetworkAdapter::privMsgReceived(const QString &recipient, const QString &sender, const QString &content)
{
	IRCChatAdapter *pAdapter = this->getOrCreateNewChatAdapter(recipient);
	pAdapter->emitChatMessage(sender, content);
}

void IRCNetworkAdapter::printMsgLiteral(const QString &recipient, const QString &content,
	const IRCMessageClass &msgClass)
{
	this->getOrCreateNewChatAdapter(recipient);
	printWithClass(content, recipient, msgClass);
}

void IRCNetworkAdapter::reloadNetworkEntityFromConfig()
{
	ChatNetworksCfg cfg;
	IRCNetworkEntity entity = cfg.network(connectionInfo.networkEntity.description());
	if (entity.isValid())
		setNetworkEntity(entity);
}

void IRCNetworkAdapter::setNetworkEntity(const IRCNetworkEntity &entity)
{
	IRCNetworkEntity oldEntity = connectionInfo.networkEntity;
	connectionInfo.networkEntity = entity;
	if (oldEntity.description() != entity.description())
		emit titleChange();
}

void IRCNetworkAdapter::sendCtcp(const QString &nickname, const QString &command)
{
	QString msg = QString("/PRIVMSG %1 %2%3%2").arg(nickname).arg(QChar(0x1)).arg(command);
	sendMessage(msg);
}

void IRCNetworkAdapter::sendPong(const QString &toWhom)
{
	QString message = QString("/PONG %1").arg(toWhom);
	sendMessage(message);
}

void IRCNetworkAdapter::setChannelMode(const QString &channel, const QString &nickname, const QString &flag, bool bSet)
{
	QString cleanNickname = userPrefixes().cleanNickname(nickname);

	QString flagPrefixed;
	if (bSet)
		flagPrefixed = "+" + flag.trimmed();
	else
		flagPrefixed = "-" + flag.trimmed();

	QString message = QString("/mode %1 %2 %3").arg(channel, flagPrefixed, cleanNickname);
	this->sendMessage(message);
}

QString IRCNetworkAdapter::title() const
{
	return connectionInfo.networkEntity.description() + " ( " + myNickname() + " )";
}

void IRCNetworkAdapter::userChangesNickname(const QString &oldNickname, const QString &newNickname)
{
	if (isMyNickname(oldNickname))
	{
		emit messageToNetworksCurrentChatBox(tr("Updated own nickname to %1.").arg(newNickname),
			IRCMessageClass::NetworkAction);
		connectionInfo.nick = newNickname;

		emit titleChange();
	}

	QList<IRCChatAdapter *> adaptersList = chatWindows.values();
	for (IRCChatAdapter *pAdapter : adaptersList)
	{
		pAdapter->userChangesNickname(oldNickname, newNickname);
	}

	// MAKE SURE TO SEE IF WE HAVE A CHAT WINDOW OPEN WITH THIS
	// USER AND FIX THE KEY IN THE CHAT WINDOW MAP FOR THIS NETWORK!!!
	if (hasRecipient(oldNickname))
	{
		QString oldNicknameLowercase = oldNickname.toLower();
		QString newNicknameLowercase = newNickname.toLower();

		IRCChatAdapter *pAdapter = getChatAdapter(oldNickname);
		chatWindows.remove(oldNicknameLowercase);
		chatWindows.insert(newNicknameLowercase, pAdapter);
	}
}

void IRCNetworkAdapter::userJoinsChannel(const QString &channel, const QString &nickname, const QString &fullSignature)
{
	if (!isMyNickname(nickname))
	{
		if (hasRecipient(channel))
		{
			auto pChannel = (IRCChannelAdapter *)this->getOrCreateNewChatAdapter(channel);
			pChannel->userJoins(nickname, fullSignature);
		}
	}
	else
	{
		auto pChannel = (IRCChannelAdapter *)this->getOrCreateNewChatAdapter(channel);
		pChannel->emitFocusRequest();
	}
}

void IRCNetworkAdapter::userIdleTime(const QString &nick, int secondsIdle)
{
	QString msg = tr("Last activity of user %1 was %2 ago.").arg(
		nick, Strings::formatTime(secondsIdle));
	emit messageToNetworksCurrentChatBox(msg, IRCMessageClass::NetworkAction);
}

void IRCNetworkAdapter::userModeChanged(const QString &channel, const QString &nickname,
	const QList<char> &addedFlags, const QList<char> &removedFlags)
{
	if (hasRecipient(channel))
	{
		IRCChatAdapter *pAdapter = this->getOrCreateNewChatAdapter(channel);
		pAdapter->userModeChanges(nickname, addedFlags, removedFlags);
	}
}

void IRCNetworkAdapter::userNetworkJoinDateTime(const QString &nick, const QDateTime &timeOfJoin)
{
	QString msg = tr("%1 joined the network on %2").arg(nick,
		timeOfJoin.toString("yyyy-MM-dd HH:mm:ss"));
	emit messageToNetworksCurrentChatBox(msg, IRCMessageClass::NetworkAction);
}

void IRCNetworkAdapter::userPartsChannel(const QString &channel, const QString &nickname, const QString &farewellMessage)
{
	if (hasRecipient(channel))
	{
		auto pChannel = (IRCChannelAdapter *)getChatAdapter(channel);

		if (isMyNickname(nickname))
		{
			emit messageWithClass(tr("You left channel %1.").arg(channel), IRCMessageClass::ChannelAction);
			killChatWindow(channel);
		}
		else
			pChannel->userLeaves(nickname, farewellMessage, IRCChatAdapter::ChannelPart);
	}
}

const IRCUserPrefix &IRCNetworkAdapter::userPrefixes() const
{
	return ircISupportParser->userPrefixes();
}

void IRCNetworkAdapter::userQuitsNetwork(const QString &nickname, const QString &farewellMessage)
{
	// We need to iterate through EVERY adapter and notify them
	// about this quit.
	// Implementation of each adapter should recognize if this quit actually
	// has anything to do with that adapter.
	QList<IRCChatAdapter *> adaptersList = chatWindows.values();
	for (IRCChatAdapter *pAdapter : adaptersList)
	{
		pAdapter->userLeaves(nickname, farewellMessage, IRCChatAdapter::NetworkQuit);
	}
}

void IRCNetworkAdapter::userPing(const QString &nickname, qint64 ping)
{
	qint64 delta = QDateTime::currentMSecsSinceEpoch() - ping;
	emit messageToNetworksCurrentChatBox(
		tr("Ping to user %1: %2ms").arg(nickname).arg(delta),
		IRCMessageClass::Ctcp);
}

void IRCNetworkAdapter::whoIsUser(const QString &nickname, const QString &user, const QString &hostName, const QString &realName)
{
	emit messageToNetworksCurrentChatBox(
		QString("%1 %2 %3 %4").arg(nickname, user, hostName, realName),
		IRCMessageClass::NetworkAction);
}

////////////////////////////////////////////////////////////////////////////////

void IRCSocketSignalsAdapter::connected()
{
	pParent->bIsJoining = true;
	pParent->emitMessageWithClass(tr("Connected. Sending registration messages."), IRCMessageClass::NetworkAction);

	IRCNetworkConnectionInfo &connectionInfo = pParent->connectionInfo;

	QString messagePass = "/PASS %1";
	QString messageNick = "/NICK %1";
	QString messageUser = "/USER %1 0 * :%2"; // RFC 2812

	IRCNetworkEntity &network = connectionInfo.networkEntity;

	if (!network.password().isEmpty())
		pParent->sendMessage(messagePass.arg(network.password()));

	pParent->sendMessage(messageNick.arg(connectionInfo.nick));
	pParent->sendMessage(messageUser.arg(connectionInfo.userName).arg(connectionInfo.realName));
}

void IRCSocketSignalsAdapter::disconnected()
{
	pParent->killAllChatWindows();
	gLog << tr("IRC: Disconnected from network %1").arg(pParent->connectionInfo.networkEntity.description());
	emit pParent->message("Disconnected");
}

void IRCSocketSignalsAdapter::errorReceived(QAbstractSocket::SocketError error)
{
	Q_UNUSED(error);
	emit pParent->error(pSocket->errorString());
}

void IRCSocketSignalsAdapter::infoMessage(const QString &message)
{
	gLog << message;
	emit pParent->message(message);
}
