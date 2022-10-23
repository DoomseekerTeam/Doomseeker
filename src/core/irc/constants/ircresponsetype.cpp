//------------------------------------------------------------------------------
// ircresponsetype.cpp
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
#include "ircresponsetype.h"

IRCResponseType::IRCResponseType()
{
	d.type = Invalid;
	d.numericType = -1;
}

IRCResponseType::IRCResponseType(MsgType type)
{
	d.type = type;
	d.numericType = toRfcNumber(type);
}

IRCResponseType::IRCResponseType(const QString &type)
{
	d.type = typeFromRfcString(type);

	// Get numeric type from string.
	bool bOk = false;
	int numericType = type.toInt(&bOk);
	if (!bOk)
		numericType = -1;
	d.numericType = numericType;
}

IRCResponseType IRCResponseType::fromIntegerResponseValue(int responseType)
{
	// Response type is always a 3-digit number. Conver the passed integer with
	// leading zeros if necessary and then pass it to typeFromRfcString().

	// The result here is d.numericType always set to responseType, and the
	// d.type set to either Invalid or appropriate value if the number was
	// recognized.
	QString strResponseType = QString("%1").arg(responseType, 3, 10, QChar('0'));

	IRCResponseType newIRCResponseType;
	newIRCResponseType.d.type = typeFromRfcString(strResponseType);
	newIRCResponseType.d.numericType = responseType;

	return newIRCResponseType;
}

bool IRCResponseType::operator==(const IRCResponseType &other) const
{
	return d.type == other.d.type;
}

bool IRCResponseType::operator!=(const IRCResponseType &other) const
{
	return !((*this) == other);
}

int IRCResponseType::toRfcNumber(MsgType type)
{
	if (type == Invalid)
		return -1;

	QString str = toRfcString(type);

	bool bOk = false;
	int val = str.toInt(&bOk);

	return bOk ? val : -1;
}

QString IRCResponseType::toRfcString(MsgType type)
{
	switch (type)
	{
	case HelloClient:
		return "001";

	case RPLISupport:
		return "005";

	case RPLLUserClient:
		return "251";

	case RPLLUserOp:
		return "252";

	case RPLLUserUnknown:
		return "253";

	case RPLLUserChannels:
		return "254";

	case RPLLUserMe:
		return "255";

	case RPLAway:
		return "301";

	case RPLWhoIsRegnick:
		return "307";

	case RPLWhoIsUser:
		return "311";

	case RPLWhoIsServer:
		return "312";

	case RPLWhoIsOperator:
		return "313";

	case RPLWhoIsIdle:
		return "317";

	case RPLEndOfWhoIs:
		return "318";

	case RPLWhoIsChannels:
		return "319";

	case RPLWhoIsSpecial:
		return "320";

	case RPLChannelUrl:
		return "328";

	case RPLCreationTime:
		return "329";

	case RPLWhoIsAccount:
		return "330";

	case RPLTopic:
		return "332";

	case RPLTopicWhoTime:
		return "333";

	case RPLWhoIsBot:
		return "335";

	case RPLWhoIsActually:
		return "338";

	case RPLNamReply:
		return "353";

	case RPLEndOfNames:
		return "366";

	case RPLMOTD:
		return "372";

	case RPLMOTDStart:
		return "375";

	case RPLEndOfMOTD:
		return "376";

	case RPLWhoIsHost:
		return "378";

	case RPLWhoIsModes:
		return "379";

	case ERRNoSuchNick:
		return "401";

	case ERRCannotSendToChan:
		return "404";

	case ERRErroneousNickname:
		return "432";

	case ERRNicknameInUse:
		return "433";

	case ERRChannelIsFull:
		return "471";

	case ERRInviteOnlyChan:
		return "473";

	case ERRBannedFromChan:
		return "474";

	case ERRBadChannelKey:
		return "475";

	case ERRBadChannelMask:
		return "476";

	case ERRNoChanModes:
		return "477";

	case ERRChanOpPrivIsNeeded:
		return "482";

	case RPLWhoIsSecure:
		return "671";

	case Join:
		return "JOIN";

	case Kick:
		return "KICK";

	case Kill:
		return "KILL";

	case Mode:
		return "MODE";

	case Nick:
		return "NICK";

	case Notice:
		return "NOTICE";

	case Part:
		return "PART";

	case Ping:
		return "PING";

	case PrivMsg:
		return "PRIVMSG";

	case Quit:
		return "QUIT";

	case Topic:
		return "TOPIC";

	case Invalid:
	default:
		return QString();
	}
}

IRCResponseType::MsgType IRCResponseType::typeFromRfcString(const QString &typeRepresentation)
{
	QString strTypeUpper = typeRepresentation.trimmed().toUpper();

	// Compare passed string against all known types.
	for (int i = 0; i < NUM_TYPES; ++i)
	{
		auto enumType = (MsgType) i;
		QString strCurrent = toRfcString(enumType);
		if (strTypeUpper == strCurrent)
			return enumType;
	}

	return Invalid;
}
