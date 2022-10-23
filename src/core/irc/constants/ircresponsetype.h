//------------------------------------------------------------------------------
// ircresponsetype.h
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
// Copyright (C) 2011 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __IRCRESPONSETYPE_H__
#define __IRCRESPONSETYPE_H__

#include <QString>

/**
 * @brief Defines types of IRC network response message.
 *
 * Types are compliant to the response types defined by RFC 1459.
 */
class IRCResponseType
{
public:
	/**
	 * @brief Represents types defined by RFC 1459.
	 *
	 * In order to learn what each type represents please refer to
	 * RFC 1459 document.
	 */
	enum MsgType
	{
		// === IMPORTANT!!! ===
		// Remember to fix toRfcString() if values here change!
		// Also IRCResponseParser::parseMessage() must be
		// fixed.

		/**
		 * @brief Type unknown to this IRC client.
		 *
		 * Messages with Invalid types may still be valid IRC messages
		 * that are complaint with the RFC 1459 document. However they
		 * may not be supported by this client.
		 */
		Invalid = 0,

		/**
		 * @brief 001 - sent when client connects.
		 *
		 * This is the first thing delivered after successful client
		 * registration.
		 */
		HelloClient,

		// === Here are types with no text representation ===

		/**
		 * @brief 005 - all sorts of server flags.
		 */
		RPLISupport,

		/**
		 * @brief 251 - how many users on how many servers
		 *
		 * @see RPLLUserMe
		 */
		RPLLUserClient,

		/// 252 - how many ops
		RPLLUserOp,

		/// 253 - how many unknown connections
		RPLLUserUnknown,

		/// 254 - how many channels,
		RPLLUserChannels,

		/// 301
		RPLAway,

		/**
		 * @brief 307 - no idea what this is, but we'll treat it
		 *        the same way we treat RPLWhoIsSpecial.
		 */
		RPLWhoIsRegnick,

		/**
		 * @brief 255 - how many clients on how many servers,
		 *
		 * Server must send that and RPLLUserClient (251). Other values
		 * in between are sent only if they are not zero.
		 */
		RPLLUserMe,

		/// 311
		RPLWhoIsUser,

		/// 312
		RPLWhoIsServer,

		/// 313
		RPLWhoIsOperator,

		/// 317
		RPLWhoIsIdle,

		/// 318
		RPLEndOfWhoIs,

		/// 319
		RPLWhoIsChannels,

		/**
		 * @brief 320 - it's inconclusive what this code means.
		 *
		 * This source https://www.alien.net.au/irc/irc2numerics.html
		 * specified that this can have more than one meaning.
		 * I'm not sure what to expect here, but this message will
		 * be treated the same as a generic WHOIS reply.
		 */
		RPLWhoIsSpecial,

		/// 328
		RPLChannelUrl,

		/// 329
		RPLCreationTime,

		/// 330
		RPLWhoIsAccount,

		/// 332
		RPLTopic,

		/// 333
		RPLTopicWhoTime,

		/// 335 - non-standard
		RPLWhoIsBot,

		/// 338
		RPLWhoIsActually,

		/// 353 - names list for a channel
		RPLNamReply,

		/// 366 - end of names list
		RPLEndOfNames,

		/// 372 - message of the day
		RPLMOTD,

		/// 375 - start of the message of the day
		RPLMOTDStart,

		/// 376 - end of the message of the day
		RPLEndOfMOTD,

		/// 378
		RPLWhoIsHost,

		/// 379
		RPLWhoIsModes,

		// === Here are errors with no text representation ===

		/// 401
		ERRNoSuchNick,

		/// 404
		ERRCannotSendToChan,

		/// 432
		ERRErroneousNickname,
		/// 433
		ERRNicknameInUse,

		ERRChannelIsFull, // 471
		ERRInviteOnlyChan, // 473
		ERRBannedFromChan, // 474
		ERRBadChannelKey, // 475
		ERRBadChannelMask, // 476
		ERRNoChanModes, // 477

		/// 482
		ERRChanOpPrivIsNeeded,

		/// 671
		RPLWhoIsSecure,

		// === Here are types which convert to their string counterparts
		// === letter-by-letter

		Join,
		Kick,
		Kill,
		Mode,
		Nick,
		Notice,
		Part,
		Ping,
		PrivMsg,
		Quit,
		Topic,

		/// Not a real type, denotes number of all types.
		NUM_TYPES
	};

	/**
	 * @brief Creates IRCResponseType objects taking numeric value as the
	 *        more important here.
	 *
	 * The MsgType, returned by type(), in the created object may still
	 * point to Invalid value but the numericValue() will be set to whatever
	 * was specified as the parameter of this method.
	 */
	static IRCResponseType fromIntegerResponseValue(int responseType);

	/**
	 * @brief If type can be represented as an integer, this will convert
	 *        it.
	 *
	 * Some IRC message types are represented by words like KILL or PING,
	 * but some are represented by numbers like 001, 311, 401, etc.
	 * This method will convert MsgType value to numeric value, if such
	 * value can be found.
	 *
	 * Internally, type is converted to string using the toRfcString()
	 * method and then that string is converted to integer.
	 *
	 * @return Either a valid, non-negative number on success, or negative
	 *         value on failure.
	 */
	static int toRfcNumber(MsgType type);

	/**
	 * @brief String representation of specified message type.
	 *
	 * This returns the RFC 1459 representation of the message type!
	 *
	 * @return If type is invalid or unknown, null QString is returned.
	 *         All returned strings are upper-case.
	 */
	static QString toRfcString(MsgType type);

	/**
	 * @brief Returns MsgType basing on typeRepresentation.
	 *
	 * It is either one of the known and implemented types or
	 * <code>Invalid</code> if string cannot be successfuly converted.
	 */
	static MsgType typeFromRfcString(const QString &typeRepresentation);

	/**
	 * @brief Initializes an invalid IRCResponseType object.
	 */
	IRCResponseType();

	/**
	 * @brief Initializes object with specified type.
	 */
	IRCResponseType(MsgType type);

	/**
	 * @brief Initializes object by attempting to convert specified string
	 *        to MsgType through typeFromRfcString().
	 */
	IRCResponseType(const QString &typeRepresentation);

	/**
	 * @brief Check if numeric value is between 200 and 399 (inclusive).
	 *
	 * See: RFC 1459.
	 */
	bool isCommandResponse() const
	{
		return d.numericType >= 200 && d.numericType <= 399;
	}

	/**
	 * @brief Check if numeric value is equal to or above 400.
	 *
	 * See: RFC 1459.
	 */
	bool isErrorMessage() const
	{
		return d.numericType >= 400;
	}

	/**
	 * @brief Response is valid if type is different than Invalid.
	 */
	bool isValid() const
	{
		return d.type != Invalid;
	}

	bool operator==(const IRCResponseType &other) const;
	bool operator!=(const IRCResponseType &other) const;

	/**
	 * @brief If message type can be represented as number,
	 *        this will contain its value.
	 *
	 * Numeric types values are stored to easilly distinct a message family.
	 * For example all errors start with 400 and above.
	 */
	int numericType() const
	{
		return d.numericType;
	}

	/**
	 * @brief String representation of the message type.
	 *
	 * This returns the RFC 1459 representation of the message type!
	 */
	QString toRfcString() const
	{
		return toRfcString(d.type);
	}

	MsgType type() const
	{
		return d.type;
	}

private:
	class PrivData
	{
	public:
		/**
		 * @brief If message type can be represented as number,
		 *        this will contain its value.
		 */
		int numericType;
		MsgType type;
	};

	PrivData d;
};

#endif
