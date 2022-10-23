//------------------------------------------------------------------------------
// ircctcpparser.cpp
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef id14670887_AF0E_4EAD_9A495B9E413C6723
#define id14670887_AF0E_4EAD_9A495B9E413C6723

#include "dptr.h"

#include <QObject>
#include <QString>

class IRCNetworkAdapter;

class IRCCtcpParser : public QObject
{
	Q_OBJECT

public:
	enum CtcpEcho
	{
		PrintAsNormalMessage,
		DisplayInServerTab,
		DisplayThroughGlobalMessage,
		DontShow
	};

	enum MessageType
	{
		/**
		 * @brief A question is being asked through PRIVMSG.
		 */
		Request,
		/**
		 * @brief An answer is being sent through NOTICE.
		 */
		Reply
	};

	IRCCtcpParser(IRCNetworkAdapter *network, const QString &sender,
		const QString &recipient, const QString &msg, MessageType msgType);
	~IRCCtcpParser() override;

	CtcpEcho echo() const;
	bool parse();
	const QString &printable() const;
	const QString &reply() const;

private:
	DPtr<IRCCtcpParser> d;

	bool isCommand(const QString &candidate);
	bool isCtcp() const;
	/**
	 * @brief Fill d->command and d->params.
	 */
	void tokenizeMsg();
	QString typeToName() const;
};


#endif
