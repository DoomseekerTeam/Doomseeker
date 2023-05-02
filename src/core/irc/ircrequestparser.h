//------------------------------------------------------------------------------
// ircrequestparser.h
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
#ifndef __IRCREQUESTPARSER_H__
#define __IRCREQUESTPARSER_H__

#include "dptr.h"

#include <QObject>
#include <QString>

class IRCAdapterBase;
class IRCNetworkAdapter;

/**
 * @brief Parses request and interprets them in a way that emulates
 *        mIRC (or any even slightly sane IRC client for that matter).
 *
 * The above statement means that a non-RFC compliant request form that users
 * grew accustomed to will be understood correctly by this parser and
 * converted to RFC 1459 compliant form.
 *
 * For example:
 * - If RFC command begins with '/' character this character will be always
 *   trimmed. If command doesn't begin with '/' character error will be
 *   returned.
 *   @b Example: "/who <nickname>" will be converted to "who <nickname>"
 * - ':' characters will be inserted into some commands.
 *   @b Example: "/part #channel I am leaving!" to
 *   "part #channel :I am leaving!"
 *
 * For more information refer to parse() method.
 */
class IRCRequestParser : public QObject
{
	Q_OBJECT

public:
	enum IRCRequestParseResult
	{
		Ok,
		ErrorMessageTooLong,
		ErrorMessageEmpty,
		ErrorInputNotPrependedWithSlash,
		ErrorInputInsufficientParameters,
		ErrorChatWindowOnly,
		QuitCommand
	};

	IRCRequestParser();
	~IRCRequestParser() override;

	const QString &output() const;

	/**
	 * @brief Parses input string and returns it through output string.
	 *        Additional information is passed through return value.
	 *
	 * @param pAdapter
	 *     Adapter that sends the message.
	 * @param input
	 *     Input message in common format. See IRCRequestParser
	 *     description.
	 *
	 * @return
	 * This method will:
	 * - Check if message fits in the RFC 1459 max message length. If not
	 *   ErrorMessageTooLong is returned.
	 * - Return ErrorMessageEmpty if input is empty or consists only of
	 *   whitespace.
	 * - Check if input starts with '/' character.
	 *   If not ErrorInputNotPrependedWithSlash is returned.
	 * - Return ErrorInputInsufficientParameters if parsed command expects
	 *   more parameters to work correctly.
	 * - ErrorChatWindowOnly is returned if command applies only to
	 *   chat windows.
	 * - QuitCommand is returned if "/quit" command is used.
	 * - Ok is returned if it is ok to send the output message. Parsed
	 *   message can be extracted through output() accessor.
	 */
	IRCRequestParseResult parse(IRCAdapterBase *pAdapter, QString input);

signals:
	/**
	 * @brief Echoes back all PRIVMSG commands.
	 *
	 * @param recipient
	 *     Recipient of the message. This is extracted
	 *     directly from the privmsg request.
	 * @param messageContent
	 *     Content of the message.
	 */
	void echoPrivmsg(const QString &recipient, const QString &content);

	/**
	 * @brief Emitted when "/query" alias is used.
	 *
	 * This signal will only be emitted if the query
	 * parameter specifies a valid user name.
	 *
	 * @param who
	 *     A clean nickname.
	 */
	void query(const QString &who);

private:
	DPtr<IRCRequestParser> d;

	IRCRequestParseResult buildOutput();
	IRCNetworkAdapter *network();
	/**
	 * @brief Check if d->output is not longer than max message length.
	 */
	bool isOutputTooLong() const;
};

#endif
