//------------------------------------------------------------------------------
// zandronummasterclient.cpp
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
// Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net> (skulltagmasterclient.cpp)
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include <QBuffer>
#include <QByteArray>
#include <QDataStream>
#include <QUdpSocket>

#include "datastreamoperatorwrapper.h"
#include "global.h"
#include "huffman/huffman.h"
#include "zandronumengineplugin.h"
#include "zandronummasterclient.h"
#include "zandronumserver.h"

#define MASTER_CHALLENGE                5660028
#define MASTER_PROTOCOL_VERSION         2
#define MASTER_RESPONSE_GOOD            0
#define MASTER_RESPONSE_BANNED          3
#define MASTER_RESPONSE_BAD             4
#define MASTER_RESPONSE_WRONGVERSION    5
#define MASTER_RESPONSE_SERVER          1
#define MASTER_RESPONSE_END             2
#define MASTER_RESPONSE_BEGINPART       6
#define MASTER_RESPONSE_ENDPART         7
#define MASTER_RESPONSE_SERVERBLOCK     8

#define RETURN_BAD_IF_NOT_ENOUGH_DATA(min_amount_of_data_required) \
	{ \
		if (in.remaining() < (min_amount_of_data_required)) \
		return RESPONSE_BAD; \
	}

ZandronumMasterClient::ZandronumMasterClient() : MasterClient()
{
	set_masterBanHelp(&ZandronumMasterClient::masterBanHelp);
}

QByteArray ZandronumMasterClient::createServerListRequest()
{
	const unsigned char challenge[6] = {WRITEINT32_DIRECT(unsigned char, MASTER_CHALLENGE), WRITEINT16_DIRECT(unsigned char, MASTER_PROTOCOL_VERSION)};
	unsigned char challengeOut[12];
	int out = 12;
	HUFFMAN_Encode(challenge, challengeOut, 6, &out);
	return QByteArray(reinterpret_cast<char *>(challengeOut), out);
}

QString ZandronumMasterClient::masterBanHelp() const
{
	return tr("You may contact Zandronum staff about this through "
		"IRC: <b>irc.zandronum.com #zandronum</b> or on "
		"the forum: <a href=\"https://zandronum.com/forum\">https://zandronum.com/forum.</a>");
}

const EnginePlugin *ZandronumMasterClient::plugin() const
{
	return ZandronumEnginePlugin::staticInstance();
}

MasterClient::Response ZandronumMasterClient::readMasterResponse(const QByteArray &data)
{
	const char *packetEncoded = data.data();
	int packetDecodedSize = 2000 + data.size();
	char *packetDecoded = new char[packetDecodedSize];

	HUFFMAN_Decode(reinterpret_cast<const unsigned char *>(packetEncoded),
		reinterpret_cast<unsigned char *>(packetDecoded),
		data.size(), &packetDecodedSize);

	if (packetDecodedSize <= 0)
	{
		delete[] packetDecoded;
		return RESPONSE_BAD;
	}

	QByteArray packetDecodedByteArray(packetDecoded, packetDecodedSize);
	delete[] packetDecoded;

	QBuffer ioBuffer(&packetDecodedByteArray);
	ioBuffer.open(QIODevice::ReadOnly);
	QDataStream inStream(&ioBuffer);
	inStream.setByteOrder(QDataStream::LittleEndian);
	DataStreamOperatorWrapper in(&inStream);

	// Check the response code
	RETURN_BAD_IF_NOT_ENOUGH_DATA(4);
	int response = in.readQInt32();
	if (response == MASTER_RESPONSE_BANNED)
		return RESPONSE_BANNED;
	else if (response == MASTER_RESPONSE_BAD)
		return RESPONSE_WAIT;
	else if (response == MASTER_RESPONSE_WRONGVERSION)
		return RESPONSE_OLD;
	else if (response != MASTER_RESPONSE_BEGINPART)
		return RESPONSE_PENDING;

	RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
	int packetNum = in.readQUInt8();
	if (!(packetsRead & (1 << packetNum))) // Set flag if we haven't read this packet
		packetsRead |= 1 << packetNum;
	else // Already read packet so ignore it.
		return RESPONSE_PENDING;
	if (packetNum + 1 > numPackets) // Packet numbers start at 0
		numPackets = packetNum + 1;

	RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
	quint8 firstByte = in.readQUInt8();
	if (firstByte != MASTER_RESPONSE_ENDPART && firstByte != MASTER_RESPONSE_END)
	{
		unsigned int numServersInBlock = in.readQUInt8();
		while (numServersInBlock != 0)
		{
			RETURN_BAD_IF_NOT_ENOUGH_DATA(4 + 2); // ip + port

			// [Zalewa] Remember: it's a very bad idea to pass reads
			// directly to function calls because function calls are
			// resolved in reverse order (in MSVC at least).
			quint8 ip1 = in.readQUInt8();
			quint8 ip2 = in.readQUInt8();
			quint8 ip3 = in.readQUInt8();
			quint8 ip4 = in.readQUInt8();

			QString ip = QString("%1.%2.%3.%4").
				arg(ip1, 1, 10, QChar('0')).
				arg(ip2, 1, 10, QChar('0')).
				arg(ip3, 1, 10, QChar('0')).
				arg(ip4, 1, 10, QChar('0'));

			for (unsigned int i = 0; i < numServersInBlock; i++)
			{
				quint16 port = in.readQUInt16();
				ZandronumServer *server = new ZandronumServer(QHostAddress(ip),
					port);
				registerNewServer(ServerPtr(server));
			}

			RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
			numServersInBlock = in.readQUInt8();
		}
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		firstByte = in.readQUInt8();
	}

	if (firstByte == MASTER_RESPONSE_END)
		readLastPacket = true;
	if (readLastPacket) // See if we read every packet.
	{
		if (packetsRead == static_cast<unsigned>((1 << numPackets) - 1))
		{
			emit listUpdated();
			return RESPONSE_GOOD;
		}
	}

	return RESPONSE_PENDING;
}

void ZandronumMasterClient::refreshStarts()
{
	// Make sure we have an empty list.
	emptyServerList();
	readLastPacket = false;
	numPackets = 0;
	packetsRead = 0;

	MasterClient::refreshStarts();
}
