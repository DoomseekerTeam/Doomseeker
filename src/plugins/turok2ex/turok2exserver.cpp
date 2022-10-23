//------------------------------------------------------------------------------
// Turok2Exserver.cpp
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
// Copyright (C) 2017 "Edward Richardson" <Edward850@crantime.org>
//------------------------------------------------------------------------------
#include "turok2exserver.h"

#include <QBuffer>
#include <QDataStream>

#include "crc.h"
#include "datastreamoperatorwrapper.h"
#include "plugins/engineplugin.h"
#include "serverapi/playerslist.h"
#include "turok2exengineplugin.h"
#include "turok2exgamehost.h"
#include "turok2exgameinfo.h"
#include "turok2exgamerunner.h"

/// Macro that checks the readRequest() validity.
#define CHECK_POS if (!in.hasRemaining()) \
	{ \
		return RESPONSE_BAD; \
	}

#define CHECK_POS_OFFSET(offset) if (in.remaining() < (offset)) \
	{ \
		return RESPONSE_BAD; \
	}

Turok2ExServer::Turok2ExServer(const QHostAddress &address, unsigned short port)
	: Server(address, port), protocol(0)
{
	set_readRequest(&Turok2ExServer::readRequest);
	set_createSendRequest(&Turok2ExServer::createSendRequest);
}

GameClientRunner *Turok2ExServer::gameRunner()
{
	return new Turok2ExGameClientRunner(
		self().toStrongRef().staticCast<Turok2ExServer>());
}

EnginePlugin *Turok2ExServer::plugin() const
{
	return Turok2ExEnginePlugin::staticInstance();
}

static uint64_t read7bitEncode(DataStreamOperatorWrapper &in)
{
	uint64_t o = 0;
	if (!in.hasRemaining())
		return 0;
	uint8_t encByte = in.readQUInt8();
	int shift = 0;
	// Read next byte, decode it and shift it into postion
	o |= uint64_t(encByte & 0x7F) << (7 * shift);
	// 0x80 (0b10000000) is the flag bit, which marks that the next byte continues the current value
	while (encByte & 0x80)
	{
		shift++;
		if (!in.hasRemaining())
			return 0;
		encByte = in.readQUInt8();
		o |= uint64_t(encByte & 0x7F) << (7 * shift);
	}
	return o;
}

static QString readString(DataStreamOperatorWrapper &in)
{
	QString o = "";
	if (!in.hasRemaining())
		return "";
	int len = (int)read7bitEncode(in);

	for (int c = 0; c < len; c++)
	{
		if (!in.hasRemaining())
			return "";
		o += (char)in.readQInt8();
	}

	return o;
}

Server::Response Turok2ExServer::readRequest(const QByteArray &data)
{
	QByteArray decryptData = data;

	if (!decryptPacket(decryptData, DOOMSEEKEY))
		return RESPONSE_BAD;

	QBuffer ioBuffer;
	ioBuffer.setData(decryptData);
	ioBuffer.open(QIODevice::ReadOnly);
	QDataStream inStream(&ioBuffer);
	inStream.setByteOrder(QDataStream::LittleEndian);
	DataStreamOperatorWrapper in(&inStream);

	CHECK_POS in.skipRawData(4); // 4 bytes are the CRC from the decryption

	// Check the response code
	CHECK_POS if (in.readQUInt8() != NETM_INFO2)
		return RESPONSE_BAD;

	CHECK_POS QString name = readString(in);
	CHECK_POS QString map = readString(in);
	CHECK_POS uint8_t gamemode = in.readQUInt8();
	CHECK_POS uint8_t players = in.readQUInt8();
	CHECK_POS uint8_t maxPlayers = in.readQUInt8();
	CHECK_POS QString version = readString(in);
	CHECK_POS QString KPFkey = readString(in);
	CHECK_POS QString modlist = readString(in);

	setGameVersion(version);
	setName(name);
	setMaxPlayers(maxPlayers);
	setMaxClients(maxPlayers);
	setMap(map);
	setIwad("game.kpf"); // This is pretty much static

	if (gamemode > Turok2ExGameInfo::MODE_SINGLEPLAYER && gamemode < Turok2ExGameInfo::MODE_NUMMODES)
		setGameMode(plugin()->gameModes()[gamemode - 1]);
	else
		setGameMode(GameMode::mkUnknown());

	if (modlist.length() > 0)
	{
		QStringList modlistArray = modlist.split('\n');
		for (short i = 0; i < modlistArray.count(); i++)
		{
			if (modlistArray[i].length())
				addWad(modlistArray[i]);
		}
	}

	clearPlayersList();
	int playerCount = players;
	while (playerCount-- > 0)
	{
		CHECK_POS QString playerName = readString(in);
		CHECK_POS uint8_t points = in.readQUInt8();
		CHECK_POS uint8_t ping = in.readQUInt8();

		addPlayer(Player(playerName, points, ping));
	}

	CHECK_POS QString email = readString(in);
	CHECK_POS QString website = readString(in);
	setEmail(email);
	setWebSite(website);

	return RESPONSE_GOOD;
}

QByteArray Turok2ExServer::createSendRequest()
{
	// This construction and cast to (char*) removes warnings from MSVC.
	const unsigned char challenge[] = { NETM_INFO2, DOOMSEEDTA };
	QByteArray challengeByteArray((char *)challenge, sizeof(challenge));
	return encryptPacket(challengeByteArray, GENERICKEY);
}

QByteArray Turok2ExServer::encryptPacket(QByteArray &message, uint32_t key)
{
	QByteArray challenge;

	uint32_t crc = kexCRC::Mem32(message.data(), message.length(), 0);
	challenge[0] = (crc >> 24) & 0xFF;
	challenge[1] = (crc >> 16) & 0xFF;
	challenge[2] = (crc >> 8) & 0xFF;
	challenge[3] = crc & 0xFF;

	uint8_t keya[4];
	keya[0] = (key >> 24) & 0xFF;
	keya[1] = (key >> 16) & 0xFF;
	keya[2] = (key >> 8) & 0xFF;
	keya[3] = key & 0xFF;
	for (int i = 0; i < message.length(); i++)
		challenge[i + 4] = message[i] ^ keya[i % sizeof(keya)];

	return challenge;
}

bool Turok2ExServer::decryptPacket(QByteArray &message, uint32_t key)
{
	if (message.length() <= 4)
		return false;

	// I have no idea why I have to grab the 4 bytes explicitly to correctly read the CRC
	uint8_t byte1 = message[0];
	uint8_t byte2 = message[1];
	uint8_t byte3 = message[2];
	uint8_t byte4 = message[3];
	uint32_t crc = (byte1 << 24) | (byte2 << 16) | (byte3 << 8) | byte4;

	uint8_t keya[4];
	keya[0] = (key >> 24) & 0xFF;
	keya[1] = (key >> 16) & 0xFF;
	keya[2] = (key >> 8) & 0xFF;
	keya[3] = key & 0xFF;

	for (int i = 4; i < message.length(); i++)
		message[i] = message[i] ^ keya[i % sizeof(keya)];

	if (crc == kexCRC::Mem32(&message.data()[4], message.length() - 4, 0))
		return true;
	return false;
}
