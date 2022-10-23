//------------------------------------------------------------------------------
// chocolatedoomserver.cpp
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
#include "chocolatedoomserver.h"

#include "chocolatedoomengineplugin.h"
#include "chocolatedoomgamehost.h"
#include "chocolatedoomgameinfo.h"
#include "chocolatedoomgamerunner.h"
#include "global.h"
#include "serverapi/playerslist.h"

#define NET_PACKET_TYPE_QUERY           0, 13
#define NET_PACKET_TYPE_QUERY_RESPONSE  14

ChocolateDoomServer::ChocolateDoomServer(const QHostAddress &address, unsigned short port)
	: Server(address, port), serverState(0), game(0), gameMission(0)
{
	set_createSendRequest(&ChocolateDoomServer::createSendRequest);
	set_readRequest(&ChocolateDoomServer::readRequest);
}

GameClientRunner *ChocolateDoomServer::gameRunner()
{
	return new ChocolateDoomGameClientRunner(
		self().toStrongRef().staticCast<ChocolateDoomServer>());
}

EnginePlugin *ChocolateDoomServer::plugin() const
{
	return ChocolateDoomEnginePlugin::staticInstance();
}

Server::Response ChocolateDoomServer::readRequest(const QByteArray &data)
{
	static const char *playerNames[4] =
	{
		"Green",
		"Indigo",
		"Brown",
		"Red"
	};

	const char *in = data.data();

	// Check the response code
	int response = READINT8(&in[1]);
	if (response != NET_PACKET_TYPE_QUERY_RESPONSE)
		return RESPONSE_BAD;

	setGameVersion(QString(&in[2]));
	int pos = 2 + gameVersion().length() + 1;

	serverState = READINT8(&in[pos++]);
	unsigned int numPlayers = READINT8(&in[pos++]);
	clearPlayersList();
	for (unsigned int i = 0; i < numPlayers; i++)
		addPlayer(Player(playerNames[i < 4 ? i : 4], 0, 0, static_cast<Player::PlayerTeam>(Player::TEAM_NONE), false, false));
	int clients = READINT8(&in[pos++]);
	setMaxClients(clients);
	setMaxPlayers(clients);
	game = READINT8(&in[pos++]);
	gameMission = READINT8(&in[pos++]);
	setName(QString(&in[pos]));

	interpretIwad(gameMission, game);
	return RESPONSE_GOOD;
}

void ChocolateDoomServer::interpretIwad(int mission, int gameMode)
{
	using namespace ChocolateDoom;

	switch (mission)
	{
	case doom:
		if (gameMode == shareware)
			setIwad("doom1.wad");
		else
			setIwad("doom.wad");
		break;
	case doom2:
		setIwad("doom2.wad");
		break;
	case pack_tnt:
		setIwad("tnt.wad");
		break;
	case pack_plut:
		setIwad("plutonia.wad");
		break;
	case pack_chex:
		setIwad("chex.wad");
		break;
	case pack_hacx:
		setIwad("hacx.wad");
		break;
	case heretic:
		if (gameMode == shareware)
			setIwad("heretic1.wad");
		else
			setIwad("heretic.wad");
		break;
	case hexen:
		setIwad("hexen.wad");
		break;
	case strife:
		if (gameMode == shareware)
			setIwad("strife0.wad");
		else
			setIwad("strife1.wad");
		break;
	default:
		setIwad("");
		break;
	}
}

QByteArray ChocolateDoomServer::createSendRequest()
{
	char challenge[2] = { NET_PACKET_TYPE_QUERY };
	QByteArray packet(challenge, 2);
	return packet;
}
