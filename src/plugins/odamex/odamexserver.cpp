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
#include <algorithm>
#include <cmath>
#include <limits>

#include "odamexcvar.h"
#include "odamexengineplugin.h"
#include "odamexgamehost.h"
#include "odamexgameinfo.h"
#include "odamexgamemode.h"
#include "odamexgamerunner.h"
#include "odamexqueryreader.h"
#include "plugins/engineplugin.h"
#include "serverapi/playerslist.h"

// Require the next field to be present and propagate errors from previous reads.
#define REQUIRE_BYTES(size) if (inStream.status() != QDataStream::Ok || in.remaining() < (size)) \
	{ \
		return RESPONSE_BAD; \
	}

// Server challenge, max supported engine version and max protocol version
constexpr quint8 MAX_KNOWN_PROTOCOL_VERSION = 7;
#define SERVER_CHALLENGE    0x02, 0x10, 0x01, 0xAD, 0xFF, 0xFF, 0xFF, 0xFF, MAX_KNOWN_PROTOCOL_VERSION, 0, 0, 0, 0, 0, 0, 0

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
	for (auto iter = cvars.constBegin(); iter != cvars.constEnd(); ++iter)
	{
		cvarList += QString("<li>%1 %2</li>")
			.arg(iter.key().toHtmlEscaped(), iter.value().toString().toHtmlEscaped());
	}
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

	REQUIRE_BYTES(12); // Response code, engine version and negotiated protocol.
	const quint32 response = in.readQUInt32();
	if ((response & 0xFFF00000) != 0xAD000000)
		return RESPONSE_BAD;

	unsigned int version = in.readQUInt32();
	short version_major = version / 256;
	short version_minor = (version % 256) / 10;
	short version_patch = (version % 256) % 10;
	QString strVersion = QString("%1.%2.%3").arg(version_major).arg(version_minor).arg(version_patch);

	unsigned int protocolVersion = in.readQUInt32();
	if (protocolVersion >= 1)
	{
		REQUIRE_BYTES(8);
		in.skipRawData(8); // Echoed token and the server's actual protocol.
	}

	// Do not impose an upper protocol-version bound here: protocol 8 or later
	// may remain compatible with this layout and work without a client update.
	// The >= checks select known fields, not an exclusive list of supported
	// version numbers. A higher version alone is not a reason to reject a reply.
	if (protocolVersion >= 7)
	{
		static const QString notFound = "HEAD-HASH-NOTFOUND";
		const QString revision = OdamexQuery::readCString(in);
		if (notFound != revision && !revision.isEmpty())
			strVersion += QString(" %1").arg(revision);
	}
	else
	{
		REQUIRE_BYTES(4);
		strVersion += QString(" r%1").arg(in.readQUInt32());
	}

	REQUIRE_BYTES(1); // CVar count.
	setGameVersion(strVersion);

	// Empty strings and zero-valued limits may be omitted from full replies.
	// Clear their extracted properties as well as the raw CVar map below (#4584).
	setEmail(QString());
	setName(QString());
	setScoreLimit(0);
	setWebSite(QString());
	setAdditionalWebSites(QStringList());
	setTimeLimit(0);
	setTimeLeft(0);
	cvars.clear();

	float replyTimeLimit = 0.0f;
	unsigned int replyScoreLimit = 0;
	short cvarCount = in.readQUInt8();
	while (cvarCount-- > 0)
	{
		const QString cvarName = OdamexQuery::readCString(in);
		REQUIRE_BYTES(1); // Type byte, or the start of a legacy string value.

		OdamexCvar::CvarType type = OdamexCvar::CvarType::String;
		if (protocolVersion >= 5)
			type = static_cast<OdamexCvar::CvarType>(in.readQUInt8());

		const auto interpretation = OdamexCvar::integerInterpretationFor(cvarName, type);
		const QVariant cvarValue = OdamexCvar::readCvarValue(in, type, interpretation);
		REQUIRE_BYTES(1); // Next CVar or password hash.

		if (cvarName == "sv_email")
			setEmail(cvarValue.toString());
		else if (cvarName == "sv_hostname")
			setName(cvarValue.toString());
		else if (cvarName == "sv_maxplayers")
			setMaxPlayers((quint8)OdamexCvar::valueToInt(cvarValue));
		else if (cvarName == "sv_maxclients")
			setMaxClients((quint8)OdamexCvar::valueToInt(cvarValue));
		else if (cvarName == "sv_scorelimit")
			replyScoreLimit = OdamexCvar::valueToUInt(cvarValue);
		else if (cvarName == "sv_website")
			setWebSite(cvarValue.toString());
		else if (cvarName == "sv_downloadsites")
			setAdditionalWebSites(cvarValue.toString().split(' '));
		else if (cvarName == "sv_timelimit")
		{
			bool ok;
			replyTimeLimit = cvarValue.toFloat(&ok);
			if (!ok || !std::isfinite(replyTimeLimit) || replyTimeLimit < 0.0f)
				return RESPONSE_BAD;

			// The API stores whole minutes. Round up so a positive limit is not
			// displayed as unlimited. Cap the result at the largest value that
			// unsigned short can store.
			const float maxMinutes = std::numeric_limits<unsigned short>::max();
			setTimeLimit(static_cast<unsigned short>(std::min(std::ceil(replyTimeLimit), maxMinutes)));
		}
		else
			cvars.insert(cvarName, cvarValue);
	}

	setGameMode(OdamexGameMode::fromServerSettings(cvars, maxPlayers()));

	// Odamex 12.3.0 (common/g_gametype.cpp, G_UsesFraglimit/G_UsesScorelimit)
	// selects the limit by base game type, even for variants with lives or sides.
	// A server can advertise both settings (e.g. 20 frags and the default 5 CTF
	// points).
	switch (OdamexCvar::valueToUInt(cvars.value("sv_gametype")))
	{
	case OdamexGameMode::BASE_DEATHMATCH:
	case OdamexGameMode::BASE_TEAM_DEATHMATCH:
		setScoreLimit(OdamexCvar::valueToUInt(cvars.value("sv_fraglimit")));
		break;
	case OdamexGameMode::BASE_CTF:
		setScoreLimit(replyScoreLimit);
		break;
	default:
		break;
	}

	if (webSite().isEmpty() && !additionalWebSites().isEmpty())
	{
		auto list = additionalWebSites();
		setWebSite(list.takeFirst());
		setAdditionalWebSites(list);
	}

	const bool locked = protocolVersion >= 4 ? !OdamexQuery::readMd5(in).isEmpty()
		: !OdamexQuery::readCString(in).isEmpty();
	REQUIRE_BYTES(1); // Map string.
	setLocked(locked);
	setMap(OdamexQuery::readCString(in));

	// In protocol 6 and later, whether the reply contains the two bytes for
	// "time remaining" is separate from how the time limit is displayed.
	//
	// | Received time limit | Odamex rounds to | Time-remaining bytes included? | Doomseeker shows |
	// +---------------------+------------------+--------------------------------+------------------+
	// | 0.1 minutes         | 0                | No                             | 1 minute         |
	// | 0.5 minutes         | 1                | Yes                            | 1 minute         |
	// | 1.1 minutes         | 1                | Yes                            | 2 minutes        |
	//
	// Doomseeker rounds **up for display** because its API only stores whole minutes.
	// We don't want a near-zero, but still positive limit to appear as "unlimited."
	//
	// But when reading the packet, we must follow **Odamex's rule**. For `0.1`, the
	// displayed value is `1`, but the time-remaining bytes are absent. Using the
	// displayed value to decide what to read would consume bytes belonging to the
	// next section and misread the rest of the reply.
	if (protocolVersion < 6 || std::round(replyTimeLimit) != 0.0f)
	{
		REQUIRE_BYTES(2);
		setTimeLeft(in.readQUInt16());
	}

	// A reply may advertise fewer teams or switch to a non-team game. Clear
	// scores that are not supplied by this snapshot as well as replacing points.
	auto &teamScores = scoresMutable();
	std::fill(teamScores.begin(), teamScores.end(), 0);
	int teamCount = 0;
	if (protocolVersion < 5 || gameMode().isTeamGame())
	{
		REQUIRE_BYTES(1);
		teamCount = in.readQUInt8();
		if (teamCount > teamScores.size())
			return RESPONSE_BAD;
		for (int i = 0; i < teamCount; ++i)
		{
			OdamexQuery::readCString(in); // Team name.
			REQUIRE_BYTES(6);
			in.skipRawData(4); // Team color.
			teamScores[i] = in.readQInt16();
		}
	}

	REQUIRE_BYTES(1);
	dehPatches.clear();
	short patchCount = in.readQUInt8();
	while (patchCount-- > 0)
	{
		const QString patch = OdamexQuery::readCString(in);
		REQUIRE_BYTES(0); // The patch name must be NUL-terminated.
		dehPatches << patch;
	}

	REQUIRE_BYTES(1);
	clearWads();
	const int wadCount = in.readQUInt8();
	for (int i = 0; i < wadCount; ++i)
	{
		const QString wad = OdamexQuery::readCString(in);
		QByteArray hash;
		if (protocolVersion >= 4)
			hash = OdamexQuery::readMd5(in);
		else
			OdamexQuery::readCString(in); // Legacy textual hash, not stored.
		REQUIRE_BYTES(0); // The WAD name and hash must both be complete.
		if (i >= 2)
		{
			PWad pwad(wad, false);
			pwad.addChecksum(hash, QCryptographicHash::Md5);
			addWad(pwad);
		}
		else if (i == 1)
			setIwad(wad);
	}

	REQUIRE_BYTES(1);
	clearPlayersList();
	short playerCount = in.readQUInt8();
	const bool teamGame = gameMode().isTeamGame();
	const bool hasPlayerTeam = protocolVersion < 5 || teamGame;
	const int colorBytes = protocolVersion >= 2 ? 4 : 0;
	while (playerCount-- > 0)
	{
		const QString playerName = OdamexQuery::readCString(in);
		// Ping, time in game, spectator, frags, kills and deaths total 11 bytes.
		REQUIRE_BYTES(colorBytes + (hasPlayerTeam ? 1 : 0) + 11);
		if (colorBytes != 0)
			in.skipRawData(colorBytes);
		const int teamIndex = hasPlayerTeam ? in.readQUInt8() : Player::TEAM_NONE;
		const quint16 ping = in.readQUInt16();
		in.skipRawData(2);
		const quint8 spectator = in.readQUInt8();
		if (spectator > 1)
			return RESPONSE_BAD;
		const int score = in.readQInt16();
		in.skipRawData(4);

		Player::PlayerTeam team = Player::TEAM_NONE;
		if (teamGame)
		{
			// Odamex 12.3.0 (common/teamdef.h) sends 4 for TEAM_NONE.
			// A spectator may also retain a preference for an inactive team;
			// only assign a team if the reply actually includes it.
			constexpr int odamexNoTeam = 4;
			if (teamIndex < teamCount)
				team = static_cast<Player::PlayerTeam>(teamIndex);
			else if (teamIndex != odamexNoTeam && teamIndex != Player::TEAM_NONE
				&& !(spectator && teamIndex < teamScores.size()))
				return RESPONSE_BAD;
		}
		addPlayer(Player(playerName, score, ping, team, spectator != 0));
	}

	// Ignore data after the declared player list. Servers can append fields
	// without changing the layout we understand or bumping the protocol version.
	return inStream.status() == QDataStream::Ok ? RESPONSE_GOOD : RESPONSE_BAD;
}

QByteArray OdamexServer::createSendRequest()
{
	// This construction and cast to (char*) removes warnings from MSVC.
	const unsigned char challenge[] = {SERVER_CHALLENGE};

	QByteArray challengeByteArray((char *)challenge, 16);
	return challengeByteArray;
}
