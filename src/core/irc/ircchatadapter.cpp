//------------------------------------------------------------------------------
// ircchatadapter.cpp
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
#include "ircchatadapter.h"

#include "irc/ircnetworkadapter.h"
#include "utf8splitter.h"

IRCChatAdapter::IRCChatAdapter(IRCNetworkAdapter *pNetwork, const QString &recipient)
{
	this->pNetwork = pNetwork;
	this->recipientName = recipient;
}

IRCChatAdapter::~IRCChatAdapter()
{
	if (this->pNetwork != nullptr)
	{
		// Prevent the situation where this->pNetwork is
		// NULLified while we still may need it.
		IRCNetworkAdapter *pTmpNetwork = this->pNetwork;

		pTmpNetwork->detachChatWindow(this);
	}
}

void IRCChatAdapter::doSendMessage(const QString &message, IRCAdapterBase *pOrigin)
{
	Q_UNUSED(pOrigin);
	// If network is null and we can still send messages this might be a bug
	// in the application.
	if (pNetwork == nullptr)
	{
		emit error("This chat window is not attached to any network. This is probably a bug in Doomseeker.");
		return;
	}

	// Trim whitespaces to make sure the '/' character gets detected properly.
	QString messageTrimmed = message.trimmed();

	if (messageTrimmed.startsWith("/"))
	{
		// This is a IRC comand. Send it as is to the network.
		pNetwork->doSendMessage(messageTrimmed, this);
	}
	else
	{
		// This is a chat message. We need to process this before
		// sending.
		sendChatMessage(messageTrimmed);
	}
}

void IRCChatAdapter::emitChatMessage(const QString &sender, const QString &content)
{
	emit message(QString("<%1>: %2").arg(sender, content));
}

QString IRCChatAdapter::extractMessageLine(QStringList &words, int maxLength)
{
	QByteArray sentence = "";
	while (!words.isEmpty())
	{
		QByteArray word = words.takeFirst().toUtf8();

		// Remember to add the spacebar into the equation.
		if (sentence.length() + word.length() + 1 > maxLength)
		{
			// If there is no valid way of splitting the sentence; if
			// the word itself is longer than maxLength we have to split
			// the word. Dash will be appended to the end of such word.
			if (sentence.isEmpty())
			{
				// We need to split the word.
				QList<QByteArray> splitWordTokens = Utf8Splitter().split(word, maxLength);

				sentence = splitWordTokens.takeFirst();
				sentence += '-';

				QString wordRemainder = "";
				for (const QByteArray &wordToken : splitWordTokens)
				{
					wordRemainder += QString::fromUtf8(wordToken.constData(), wordToken.size());
				}
				words.prepend(wordRemainder);
			}
			else
			{
				// Put the word back on the list, we'll come back to it
				// in the next line of the message.
				words.prepend(QString::fromUtf8(word.constData(), word.size()));
			}

			break;
		}

		sentence += word + " ";
	}
	return QString::fromUtf8(sentence.constData(), sentence.size()).trimmed();
}

void IRCChatAdapter::sendChatMessage(const QString &message)
{
	// Here we will split too long messages to make sure they don't
	// exceed the 512 character limit as stated in RFC 1459.

	// NOTE: Messages are echoed back through IRCRequestParser

	QString ircCall = QString("/PRIVMSG %1 ").arg(recipientName);
	int maxLength = IRCClient::SAFE_MESSAGE_LENGTH - ircCall.toUtf8().length();
	QStringList wordLines = message.split("\n");

	for (QString line : wordLines)
	{
		QStringList words = line.split(" ");

		while (!words.isEmpty())
		{
			QString sentence = extractMessageLine(words, maxLength);
			pNetwork->doSendMessage(ircCall + sentence, this);
		}
	}
}

void IRCChatAdapter::setNetwork(IRCNetworkAdapter *pNetwork)
{
	this->pNetwork = pNetwork;
}

void IRCChatAdapter::setRecipient(const QString &name)
{
	this->recipientName = name;
	emit titleChange();
}

QString IRCChatAdapter::title() const
{
	return recipientName;
}
