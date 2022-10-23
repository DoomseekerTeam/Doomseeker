//------------------------------------------------------------------------------
// ircadapterbase.h
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
#ifndef __IRCADAPTERBASE_H__
#define __IRCADAPTERBASE_H__

#include <QObject>


class IRCMessageClass;
class IRCNetworkAdapter;
class IRCNetworkEntity;

/**
 * @brief Provides an unified communication interface between a client and
 * IRC network entities.
 */
class IRCAdapterBase : public QObject
{
	Q_OBJECT

public:
	/**
	 * @brief Defines all possible types of IRC adapters.
	 */
	enum AdapterType
	{
		NetworkAdapter,
		ChannelAdapter,
		PrivAdapter
	};

	/**
	 * @brief Destructor emits terminating() signal.
	 */
	virtual ~IRCAdapterBase() override
	{
		emit terminating();
	}

	/**
	 * @brief Gets adapter type for this adapter instance.
	 */
	virtual AdapterType adapterType() const = 0;

	/**
	 * @brief Implement to handle and send a message to the IRC network
	 * entity.
	 *
	 * This is meant for internal use by the IRCAdapterBase derivatives.
	 *
	 * Each implementation treats this method a bit differently.
	 * Please refer to the documentation in the classes that derive from
	 * this one.
	 *
	 * @param pOrigin
	 *     Origin of this message. Can be used to determine where the error
	 *     and message signals should be passed.
	 */
	virtual void doSendMessage(const QString &message, IRCAdapterBase *pOrigin) = 0;

	void emitError(const QString &strError)
	{
		emit error(strError);
	}

	void emitFocusRequest()
	{
		emit focusRequest();
	}

	void emitMessage(const QString &strMessage)
	{
		emit message(strMessage);
	}

	void emitMessageWithClass(const QString &strMessage, const IRCMessageClass &messageClass)
	{
		emit messageWithClass(strMessage, messageClass);
	}

	/**
	 * @brief The idea of the adapter system is that each adapter
	 * is either a network or is a child of a network.
	 *
	 * This method is supposed to return a pointer to a network
	 * to which this adapter belongs.
	 */
	virtual IRCNetworkAdapter *network() = 0;
	const IRCNetworkEntity &networkEntity() const;

	virtual QString recipient() const
	{
		return QString();
	}

	/**
	 * @brief Gets title for this adapter.
	 */
	virtual QString title() const = 0;

public slots:
	void emitMessageToAllChatBoxes(const QString &message, const IRCMessageClass &msgClass);

	void sendMessage(const QString &message)
	{
		doSendMessage(message, nullptr);
	}

signals:
	void error(const QString &error);

	/**
	 * @brief Called when this adapter requests UI focus.
	 */
	void focusRequest();

	void message(const QString &message);
	void messageWithClass(const QString &message, const IRCMessageClass &messageClass);
	void messageToNetworksCurrentChatBox(const QString &message, const IRCMessageClass &msgClass);

	void terminating();

	/**
	 * @brief Can be called when the variable returned by title()
	 * might have changed and the application should be notified of this
	 * fact.
	 */
	void titleChange();
};

#endif
