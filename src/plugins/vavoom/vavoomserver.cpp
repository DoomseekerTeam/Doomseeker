//------------------------------------------------------------------------------
// vavoomserver.cpp
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
// Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------
#include "vavoomserver.h"

#include "serverapi/playerslist.h"
#include "vavoomengineplugin.h"
#include "vavoomgameinfo.h"
#include "vavoomgamerunner.h"

#define NET_PROTOCOL_VERSION    1
#define CCREQ_SERVER_INFO       2
#define CCREP_SERVER_INFO       13
#define NETPACKET_CTL           0x80

VavoomServer::VavoomServer(const QHostAddress &address, unsigned short port)
	: Server(address, port)
{
	set_readRequest(&VavoomServer::readRequest);
	set_createSendRequest(&VavoomServer::createSendRequest);

	GameMode unknownMode = VavoomGameInfo::gameModes()[VavoomGameInfo::MODE_UNKNOWN];
	setGameMode(unknownMode);
}

GameClientRunner *VavoomServer::gameRunner()
{
	return new VavoomGameClientRunner(self().toStrongRef().staticCast<VavoomServer>());
}

EnginePlugin *VavoomServer::plugin() const
{
	return VavoomEnginePlugin::staticInstance();
}

Server::Response VavoomServer::readRequest(const QByteArray &data)
{
	fflush(stderr);
	fflush(stdout);
	const char *in = data.data();

	// Check the response code
	int response = READINT8(&in[0]);
	int msgtype = READINT8(&in[1]);
	if (response != NETPACKET_CTL || msgtype != CCREP_SERVER_INFO)
		return RESPONSE_BAD;

	int pos = 2;

	// Server name
	setName(QString(&in[pos + 1]));//, in[pos]);
	pos += name().length() + 2;

	// Map
	setMap(QString(&in[pos + 1]));//, in[pos]);
	pos += map().length() + 2;

	// Players
	clearPlayersList();
	int numPlayers = READINT8(&in[pos++]);
	int maxPlayers = READINT8(&in[pos++]);
	setMaxClients(maxPlayers);
	setMaxPlayers(maxPlayers);
	for (int i = 0; i < numPlayers; i++)
	{
		Player player(tr("Unknown"), 0, 0, Player::TEAM_NONE);
		addPlayer(player);
	}

	if (READINT8(&in[pos++]) != NET_PROTOCOL_VERSION)
		return RESPONSE_BAD;

	// Wads
	clearWads();
	QString wadFile;
	bool iwadSet = false;
	while ((wadFile = QString(&in[pos + 1])) != "")
	{
		pos += wadFile.length() + 2;
		// Vavoom want you to know the exact location of the file on the server
		int lastSlash = wadFile.lastIndexOf(QChar('/')) > wadFile.lastIndexOf(QChar('\\')) ? wadFile.lastIndexOf(QChar('/')) : wadFile.lastIndexOf(QChar('\\'));
		if (lastSlash != -1)
			wadFile = wadFile.mid(lastSlash + 1);

		if (!iwadSet)
		{
			setIwad(wadFile);
			iwadSet = true;
		}
		else
			addWad(wadFile);
	}
	pos += 2;

	return RESPONSE_GOOD;
}

QByteArray VavoomServer::createSendRequest()
{
	// This construction and cast to (char*) removes warnings from MSVC.
	const unsigned char challenge[11] = { NETPACKET_CTL, CCREQ_SERVER_INFO, 6, 'V', 'A', 'V', 'O', 'O', 'M', 0, NET_PROTOCOL_VERSION };

	QByteArray challengeByteArray((char *)challenge, 11);
	return challengeByteArray;
}
