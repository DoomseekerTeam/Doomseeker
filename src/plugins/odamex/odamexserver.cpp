//------------------------------------------------------------------------------
// odamexserver.cpp
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
#include "odamexserver.h"

#include <QBuffer>
#include <QCryptographicHash>
#include <QDataStream>

#include "datastreamoperatorwrapper.h"
#include "odamexengineplugin.h"
#include "odamexgamehost.h"
#include "odamexgameinfo.h"
#include "odamexgamemode.h"
#include "odamexgamerunner.h"
#include "plugins/engineplugin.h"
#include "serverapi/playerslist.h"

/// Macro that checks the readRequest() validity.
#define CHECK_POS if (!in.hasRemaining()) \
	{ \
		return RESPONSE_BAD; \
	}

#define CHECK_POS_OFFSET(offset) if (in.remaining() < (offset)) \
	{ \
		return RESPONSE_BAD; \
	}

// Server challenge, max supported engine version and max protocol version
#define SERVER_CHALLENGE    0x02, 0x10, 0x01, 0xAD, 0xFF, 0xFF, 0xFF, 0xFF, 7, 0, 0, 0, 0, 0, 0, 0

#define SPECTATOR_INFO      0x01020304
#define EXTRA_INFO          0x01020305

OdamexServer::OdamexServer(const QHostAddress &address, unsigned short port)
	: Server(address, port), protocol(0)
{
	set_customDetails(&OdamexServer::customDetails);
	set_readRequest(&OdamexServer::readRequest);
	set_createSendRequest(&OdamexServer::createSendRequest);
}

QString OdamexServer::customDetails()
{
	if (cvars.empty())
		return "";

	QString cvarList("<ul>");
	QMap<QString, QString>::ConstIterator iter = cvars.constBegin();
	do
	{
		cvarList += QString("<li>%1 %2</li>").arg(iter.key()).arg(iter.value());
	} while (++iter != cvars.constEnd());
	return cvarList + "</ul>";
}

GameClientRunner *OdamexServer::gameRunner()
{
	return new OdamexGameClientRunner(
		self().toStrongRef().staticCast<OdamexServer>());
}

EnginePlugin *OdamexServer::plugin() const
{
	return OdamexEnginePlugin::staticInstance();
}

Server::Response OdamexServer::readRequest(const QByteArray &data)
{
	QBuffer ioBuffer;
	ioBuffer.setData(data);
	ioBuffer.open(QIODevice::ReadOnly);
	QDataStream inStream(&ioBuffer);
	inStream.setByteOrder(QDataStream::LittleEndian);
	DataStreamOperatorWrapper in(&inStream);

	// Check the response code
	int response = in.readQInt32();
	if ((response & 0xFFF00000) != 0xAD000000)
		return RESPONSE_BAD;

	unsigned int version = in.readQUInt32();
	short version_major = version / 256;
	short version_minor = (version % 256) / 10;
	short version_patch = (version % 256) % 10;
	QString strVersion = QString("%1.%2.%3").arg(version_major).arg(version_minor).arg(version_patch);

	unsigned int protocolVersion = in.readQUInt32();
	if (protocolVersion >= 1)
		in.skipRawData(8);

	CHECK_POS;

	if (protocolVersion >= 7)
	{
		static const QString notFound = "HEAD-HASH-NOTFOUND";
		auto revision = in.readUtf8CString();
		if (notFound != revision && !revision.isEmpty())
			strVersion += QString(" %1").arg(revision);
	}
	else
	{
		strVersion += QString(" r%1").arg(in.readQUInt32());
	}
	setGameVersion(strVersion);

	CHECK_POS;

	cvars.clear();
	short cvarCount = in.readQUInt8();
	while (cvarCount-- > 0)
	{
		auto cvarName = in.readUtf8CString();
		CHECK_POS;

		enum CVarType
		{
			CVAR_BOOL = 1,
			CVAR_INT8,
			CVAR_INT16,
			CVAR_INT32,
			CVAR_FLOAT,
			CVAR_STRING
		};
		CVarType type = CVAR_STRING;
		if (protocolVersion >= 5)
			type = static_cast<CVarType>(in.readQUInt8());

		QString cvarValue;
		switch (type)
		{
		case CVAR_BOOL:
			cvarValue = "1";
			break;
		case CVAR_INT8:
			cvarValue = QString("%1").arg(in.readQInt8());
			break;
		case CVAR_INT16:
			cvarValue = QString("%1").arg(in.readQInt16());
			break;
		case CVAR_INT32:
			cvarValue = QString("%1").arg(in.readQInt32());
			break;
		default:
			cvarValue = in.readUtf8CString();
			break;
		}
		CHECK_POS;

		if (cvarName == "sv_email")
			setEmail(cvarValue);
		else if (cvarName == "sv_hostname")
			setName(cvarValue);
		else if (cvarName == "sv_maxplayers")
			setMaxPlayers((quint8)cvarValue.toInt());
		else if (cvarName == "sv_maxclients")
			setMaxClients((quint8)cvarValue.toInt());
		else if (cvarName == "sv_scorelimit")
			setScoreLimit(cvarValue.toUInt());
		else if (cvarName == "sv_website")
			setWebSite(cvarValue);
		else if (cvarName == "sv_downloadsites")
			setAdditionalWebSites(cvarValue.split(' '));
		else if (cvarName == "sv_timelimit")
			setTimeLimit(cvarValue.toUInt());
		else
			cvars.insert(cvarName, cvarValue);
	}

	/*
	  Define the gamemode.
	  From: https://github.com/odamex/odamex/blob/f0e0416fc42ef06ce8ff1f1c462df6a69fc65fd1/common/g_gametype.cpp#L53
	  Do not use g_gametypename.
	*/
	setGameMode([&]()
	{
		return OdamexGameMode::derived()[([&]()
		{
			unsigned int mode = cvars.value("sv_gametype", "0").toUInt();
			unsigned int lives = cvars.value("g_lives", "0").toUInt();
			bool sides = cvars.value("g_sides", "0").toUInt();
			if (mode == OdamexGameMode::BASE_COOPERATIVE && lives > 0)
				return OdamexGameMode::DERIVED_SURVIVAL;
			else if (mode == OdamexGameMode::BASE_COOPERATIVE)
				return OdamexGameMode::DERIVED_COOPERATIVE;
			else if (mode == OdamexGameMode::BASE_DEATHMATCH && lives > 0)
				return OdamexGameMode::DERIVED_LAST_MARINE_STANDING;
			else if (mode == OdamexGameMode::BASE_DEATHMATCH && maxPlayers() <= 2)
				return OdamexGameMode::DERIVED_DUEL;
			else if (mode == OdamexGameMode::BASE_DEATHMATCH)
				return OdamexGameMode::DERIVED_DEATHMATCH;
			else if (mode == OdamexGameMode::BASE_TEAM_DEATHMATCH && lives > 0)
				return OdamexGameMode::DERIVED_TEAM_LAST_MARINE_STANDING;
			else if (mode == OdamexGameMode::BASE_TEAM_DEATHMATCH)
				return OdamexGameMode::DERIVED_TEAM_DEATHMATCH;
			else if (mode == OdamexGameMode::BASE_CTF && sides)
				return OdamexGameMode::DERIVED_ATTACK_DEFEND_CTF;
			else if (mode == OdamexGameMode::BASE_CTF && lives > 0)
				return OdamexGameMode::DERIVED_LMS_CTF;
			else if (mode == OdamexGameMode::BASE_CTF)
				return OdamexGameMode::DERIVED_CAPTURE_THE_FLAG;
			else if (mode == OdamexGameMode::BASE_HORDE && lives > 0)
				return OdamexGameMode::DERIVED_SURVIVAL_HORDE;
			else if (mode == OdamexGameMode::BASE_HORDE)
				return OdamexGameMode::DERIVED_HORDE;
			else
				return OdamexGameMode::DERIVED_COOPERATIVE; // Arbitrary default
		}())];
	}());

	if (webSite().isEmpty() && !additionalWebSites().isEmpty())
	{
		auto list = additionalWebSites();
		setWebSite(list.takeFirst());
		setAdditionalWebSites(list);
	}

	if (protocolVersion >= 4)
	{
		short hashLength = in.readQUInt8();
		in.skipRawData(hashLength);
		setLocked(hashLength > 0);
	}
	else
	{
		auto passwordHash = in.readUtf8CString();
		setLocked(!passwordHash.isEmpty());
	}
	CHECK_POS;

	setMap(in.readUtf8CString());
	CHECK_POS;

	if (protocolVersion >= 6)
	{
		if (timeLimit() != 0)
			setTimeLeft(in.readQUInt16());
	}
	else
		setTimeLeft(in.readQUInt16());

	if (protocolVersion < 5 || gameMode().isTeamGame())
	{
		short teamCount = in.readQUInt8();
		while (teamCount-- > 0)
		{
			CHECK_POS;

			auto teamName = in.readUtf8CString();
			in.skipRawData(6);
		}
	}

	dehPatches.clear();
	short patchCount = in.readQUInt8();
	while (patchCount-- > 0)
	{
		CHECK_POS;

		auto patch = in.readUtf8CString();
		dehPatches << patch;
	}

	clearWads();
	short wadCount = in.readQUInt8();
	for (short i = 0; i < wadCount; i++)
	{
		CHECK_POS;

		auto wad = in.readUtf8CString();
		QByteArray hash = "";
		CHECK_POS;
		if (protocolVersion >= 4)
		{
			short hashLength = in.readQUInt8();
			hash = in.readRaw(hashLength); //Add the hash of the wad.
		}
		else
			in.readUtf8CString();
		if (i >= 2)
		{
			PWad pwad(wad, false);
			pwad.addChecksum(hash, QCryptographicHash::Md5);
			addWad(pwad);
		}
		else if (i == 1)
			setIwad(wad);
	}

	clearPlayersList();
	short playerCount = in.readQUInt8();
	while (playerCount-- > 0)
	{
		CHECK_POS;

		auto playerName = in.readUtf8CString();

		if (protocolVersion >= 2)
		{
			CHECK_POS_OFFSET(4);
			// Player color
			in.skipRawData(4);
		}

		unsigned short teamIndex = Player::TEAM_NONE;
		if (protocolVersion < 5 || gameMode().isTeamGame())
		{
			CHECK_POS_OFFSET(1);
			teamIndex = in.readQUInt8();
		}
		CHECK_POS_OFFSET(11);
		unsigned short ping = in.readQUInt16();
		in.skipRawData(2);
		bool spectator = in.readQUInt8();
		int score = in.readQInt16();
		in.skipRawData(4);

		Player::PlayerTeam team = gameMode().isTeamGame() ? static_cast<Player::PlayerTeam>(teamIndex) : Player::TEAM_NONE;
		Player player(playerName, score, ping, team, spectator);
		addPlayer(player);
	}

	return RESPONSE_GOOD;
}

QByteArray OdamexServer::createSendRequest()
{
	// This construction and cast to (char*) removes warnings from MSVC.
	const unsigned char challenge[] = {SERVER_CHALLENGE};

	QByteArray challengeByteArray((char *)challenge, 16);
	return challengeByteArray;
}
