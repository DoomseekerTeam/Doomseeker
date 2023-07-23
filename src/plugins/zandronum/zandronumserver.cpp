//------------------------------------------------------------------------------
// zandronumserver.cpp
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "zandronumserver.h"

#include "datastreamoperatorwrapper.h"
#include "global.h"
#include "huffman/huffman.h"
#include "log.h"
#include "zandronumbinaries.h"
#include "zandronumengineplugin.h"
#include "zandronumgamehost.h"
#include "zandronumgameinfo.h"
#include "zandronumgamerunner.h"
#include "zandronumrconprotocol.h"
#include "zandronumserverdmflagsparser.h"

#include <application.h>
#include <templatedpathresolver.h>
#include <ini/inisection.h>
#include <ini/inivariable.h>
#include <serverapi/message.h>
#include <serverapi/playerslist.h>
#include <strings.hpp>

#include <QBuffer>
#include <QCryptographicHash>
#include <QDataStream>
#include <QDateTime>
#include <QFileInfo>
#include <QRegularExpression>

#include <cassert>
#include <cstdint>
#include <utility>

#define SERVER_CHALLENGE      0xC7, 0x00, 0x00, 0x00
#define SERVER_GOOD_SINGLE    5660023
#define SERVER_GOOD_SEGMENTED 5660031
#define SERVER_BANNED         5660025
#define SERVER_WAIT           5660024

static const quint8 SQF1 = 0;
static const quint8 SQF2 = 1;

/// Including the size field.
static const quint16 SEGMENT_SIZE_UNTIL_SIZE_FIELD = 7;

#define CHK_REMAINING(in, size) \
	{ if (in.remaining() < size) { return RESPONSE_BAD; } }

#define RETURN_BAD_IF_NOT_ENOUGH_DATA(size) CHK_REMAINING(in, size)

/**
 * Compares versions of Zandronum.
 */
ZandronumVersion::ZandronumVersion(QString version) : version(version)
{
	auto match = versionExpression.match(version);
	major = match.captured(1).toUShort();
	minor = match.captured(2).toUShort();
	revision = match.captured(3).toUShort();
	build = match.captured(4).toUShort();
	tag = match.captured(5);
	hgRevisionDate = match.captured(6).toUInt();
	hgRevisionTime = match.captured(7).toUShort();
}

bool ZandronumVersion::operator> (const ZandronumVersion &other) const
{
	if (major > other.major ||
		(major == other.major && (minor > other.minor ||
		(minor == other.minor && (revision > other.revision ||
		(revision == other.revision && build > other.build))))))
		return true;
	if ((tag.isEmpty() && !other.tag.isEmpty()) || (tag > other.tag))
		return true;
	if (hgRevisionDate > other.hgRevisionDate)
		return true;
	if (hgRevisionTime > other.hgRevisionTime)
		return true;
	return false;
}

const QRegularExpression ZandronumVersion::versionExpression(R"(^(\d+).(\d+)(?:.(\d+)(?:.(\d+))?)?(?:-([a-zA-Z]*)?)?(?:-r(\d+)(?:-(\d+))?)?)");

////////////////////////////////////////////////////////////////////////////////

TeamInfo::TeamInfo(QString name, QColor color, unsigned int score) :
	teamName(std::move(name)), teamColor(std::move(color)), teamScore(score)
{
}

////////////////////////////////////////////////////////////////////////////////

ZandronumServer::ZandronumServer(const QHostAddress &address, unsigned short port)
	: Server(address, port),
	buckshot(false), instagib(false), teamDamage(0.0f),
	botSkill(0), duelLimit(0), fragLimit(0), pointLimit(0), winLimit(0),
	numTeams(2)
{
	teamInfo[0] = TeamInfo(tr("Blue"), QColor(0, 0, 255), 0);
	teamInfo[1] = TeamInfo(tr("Red"), QColor(255, 0, 0), 0);
	teamInfo[2] = TeamInfo(tr("Green"), QColor(0, 255, 0), 0);
	teamInfo[3] = TeamInfo(tr("Gold"), QColor(255, 255, 0), 0);

	set_createSendRequest(&ZandronumServer::createSendRequest);
	set_readRequest(&ZandronumServer::readRequest);

	connect(this, SIGNAL(updated(ServerPtr,int)), this, SLOT(updatedSlot(ServerPtr,int)));
}

ExeFile *ZandronumServer::clientExe()
{
	return new ZandronumClientExeFile(self().toStrongRef().staticCast<ZandronumServer>());
}

GameClientRunner *ZandronumServer::gameRunner()
{
	return new ZandronumGameClientRunner(self());
}

unsigned int ZandronumServer::millisecondTime()
{
	const QTime time = QTime::currentTime();
	return time.hour() * 360000 + time.minute() * 60000 + time.second() * 1000 + time.msec();
}

QList<GameCVar> ZandronumServer::modifiers() const
{
	QList<GameCVar> result;
	if (instagib)
		result << ZandronumGameInfo::gameModifiers()[1];
	else if (buckshot)
		result << ZandronumGameInfo::gameModifiers()[0];
	return result;
}

EnginePlugin *ZandronumServer::plugin() const
{
	return ZandronumEnginePlugin::staticInstance();
}

Server::Response ZandronumServer::readRequest(const QByteArray &data)
{
	const int BUFFER_SIZE = 6000;
	QByteArray rawReadBuffer;

	// Decompress the response.
	const char *huffmanPacket = data.data();

	int decodedSize = BUFFER_SIZE + data.size();
	char *packetDecodedBuffer = new char[decodedSize];

	HUFFMAN_Decode(reinterpret_cast<const unsigned char *>(huffmanPacket),
		reinterpret_cast<unsigned char *>(packetDecodedBuffer), data.size(),
		&decodedSize);

	if (decodedSize <= 0)
	{
		delete [] packetDecodedBuffer;
		return RESPONSE_BAD;
	}

	// Prepare reading interface.
	QByteArray packetDecoded(packetDecodedBuffer, decodedSize);
	this->currentChallenge.raw += packetDecoded;

	delete [] packetDecodedBuffer;

	QBuffer packetDecodedIo(&packetDecoded);

	packetDecodedIo.open(QIODevice::ReadOnly);
	packetDecodedIo.seek(0);

	QDataStream inStream(&packetDecodedIo);
	inStream.setByteOrder(QDataStream::LittleEndian);

	DataStreamOperatorWrapper in(&inStream);

	// Read and parse.

	// Do the initial sanity check. All packets must be at least 8 bytes big,
	// regardless if the segmented or single-packet response is received.
	if (decodedSize < 8)
	{
		fprintf(stderr, "Data size error when reading server %s:%u."
			" Data size encoded: %u, decoded: %u\n",
			address().toString().toUtf8().constData(), port(),
			data.size(), decodedSize);
		return RESPONSE_BAD;
	}

	// Check the response code
	qint32 response = in.readQInt32();

	// Act according to the response
	Server::Response challengeResponse = RESPONSE_NO_RESPONSE_YET;
	switch (response)
	{
	case SERVER_BANNED:
		return RESPONSE_BANNED;

	case SERVER_WAIT:
		return RESPONSE_WAIT;

	case SERVER_GOOD_SINGLE:
		this->currentChallenge.responseType = ChallengeResponse::SINGLE;
		challengeResponse = readSingleReply(inStream);
		break;

	case SERVER_GOOD_SEGMENTED:
		this->currentChallenge.responseType = ChallengeResponse::SEGMENTED;
		challengeResponse = readSegmentedReply(inStream);
		break;

	default:
		return RESPONSE_BAD;
	}

	if (challengeResponse == RESPONSE_GOOD)
	{
		// Now when the server is fully known, clean-up the PWADs list.
		bool pwadsModified = false;
		QList<PWad> pwads = wads();
		for (unsigned index = 0; index < pwads.size(); index++)
		{
			PWad pwad = pwads[index];
			if (pwad.name().isEmpty())
			{
				pwadsModified = true;
				pwads.removeAt(index);
				--index;
			}
		}
		if (pwadsModified)
		{
			resetPwadsList(pwads);
		}
	}

	return challengeResponse;
}

Server::Response ZandronumServer::readSingleReply(QDataStream &stream)
{
	DataStreamOperatorWrapper in(&stream);

	// Drop the "response timestamp" field.
	RETURN_BAD_IF_NOT_ENOUGH_DATA(4);
	in.readQInt32();

	RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
	setGameVersion(in.readRawUntilByte('\0'));

	return readSqf1(stream);
}

Server::Response ZandronumServer::readSegmentedReply(QDataStream &stream)
{
	DataStreamOperatorWrapper in(&stream);

	// All segments have at least 3 bytes.
	RETURN_BAD_IF_NOT_ENOUGH_DATA(3);

	// Collect all segments.
	//
	// The doc doesn't say if there can be multiple segments in one
	// packet, but there's a segment size field, so anticipate that
	// there can.
	while (in.hasRemaining())
	{
		quint8 segmentNo = in.readQUInt8();
		if (0x80 & segmentNo)
		{
			segmentNo &= 0x7f;
			this->currentChallenge.endSegment = segmentNo;
		}

		RETURN_BAD_IF_NOT_ENOUGH_DATA(2);
		quint16 segmentSize = in.readQUInt16();

		// The segmentSize also includes all the fields that are before.
		if (segmentSize < SEGMENT_SIZE_UNTIL_SIZE_FIELD)
		{
			fprintf(stderr, "Server '%s:%u' sent a segment with too small size"
					"(segment size=%d < minimum=%d).\n",
				address().toString().toUtf8().constData(), port(),
				(int) segmentSize, (int) SEGMENT_SIZE_UNTIL_SIZE_FIELD);
			return RESPONSE_BAD;
		}

		segmentSize -= SEGMENT_SIZE_UNTIL_SIZE_FIELD;

		if (segmentSize > in.remaining())
		{
			fprintf(stderr, "Server '%s:%u' sent a truncated segment "
					"(segment size=%d > remaining=%d).\n",
				address().toString().toUtf8().constData(), port(),
				(int) segmentSize, (int) in.remaining());
			return RESPONSE_BAD;
		}
		if (segmentSize > this->currentChallenge.spaceRemaining())
		{
			fprintf(stderr, "Server '%s:%u' sent too much challenge data.\n",
				address().toString().toUtf8().constData(), port());
			return RESPONSE_BAD;
		}
		this->currentChallenge.insertSegment(segmentNo, in.readRaw(segmentSize));
	}

	if (this->currentChallenge.hasAllSegments())
	{
		// Once all segments have arrived, parse them.
		return readAccumulatedSegments();
	}
	else
	{
		// If we don't have all segments yet, wait for more.
		return RESPONSE_PENDING;
	}
}

Server::Response ZandronumServer::readAccumulatedSegments()
{
	assert(this->currentChallenge.hasAllSegments());
	for (int segmentIdx = 0;
		 segmentIdx < this->currentChallenge.segments.size();
		 ++segmentIdx)
	{
		QByteArray &segment = this->currentChallenge.segments[segmentIdx];

		QBuffer segmentIo(&segment);
		segmentIo.open(QIODevice::ReadOnly);

		QDataStream segmentStream(&segmentIo);
		segmentStream.setByteOrder(QDataStream::LittleEndian);

		DataStreamOperatorWrapper segmentIn(&segmentStream);

		if (segmentIdx == 0)
		{
			// Segment 0 contains the additional "timestamp" and game version fields.

			// Drop the "response timestamp" field.
			CHK_REMAINING(segmentIn, 4);
			segmentIn.readQInt32();

			CHK_REMAINING(segmentIn, 1);
			setGameVersion(segmentIn.readRawUntilByte('\0'));
		}

		// Now parse the flags fields.
		Server::Response sqfResult = readSqf(segmentStream);
		if (sqfResult != RESPONSE_GOOD)
		{
			return sqfResult;
		}
	}

	return RESPONSE_GOOD;
}

Server::Response ZandronumServer::readSqf(QDataStream &stream)
{
	DataStreamOperatorWrapper in(&stream);
	while (in.hasRemaining())
	{
		const quint8 sqfSet = in.readQUInt8();
		Server::Response sqfResult = RESPONSE_NO_RESPONSE_YET;
		switch (sqfSet)
		{
		case SQF1:
			sqfResult = readSqf1(stream);
			break;
		case SQF2:
			sqfResult = readSqf2(stream);
			break;
		default:
			fprintf(stderr, "Server '%s:%u' sent an unknown SQF set %d.\n",
				address().toString().toUtf8().constData(), port(),
				(int) sqfSet);
			return RESPONSE_BAD;
		}
		if (sqfResult != RESPONSE_GOOD)
			return sqfResult;
	}
	// If there's nothing more to read from the packet at this point,
	// we conclude that the read was successful.
	return RESPONSE_GOOD;
}

Server::Response ZandronumServer::readSqf1(QDataStream &stream)
{
	DataStreamOperatorWrapper in(&stream);

	// Flags - set of flags returned by the server. This is compared
	// with known set of flags and the data is read from the packet
	// accordingly. Every flag is removed from this variable after such check.
	// See "if SQF_NAME" for an example.
	RETURN_BAD_IF_NOT_ENOUGH_DATA(4);

	quint32 flags = in.readQUInt32();
	if ((flags & SQF_NAME) == SQF_NAME)
	{
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		setName(in.readRawUntilByte('\0'));
		flags ^= SQF_NAME; // Remove SQF_NAME flag from the variable.
	}

	if ((flags & SQF_URL) == SQF_URL)
	{
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		setWebSite(in.readRawUntilByte('\0'));
		flags ^= SQF_URL;
	}
	if ((flags & SQF_EMAIL) == SQF_EMAIL)
	{
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		setEmail(in.readRawUntilByte('\0'));
		flags ^= SQF_EMAIL;
	}
	if ((flags & SQF_MAPNAME) == SQF_MAPNAME)
	{
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		setMap(in.readRawUntilByte('\0'));
		flags ^= SQF_MAPNAME;
	}

	if ((flags & SQF_MAXCLIENTS) == SQF_MAXCLIENTS)
	{
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		setMaxClients(in.readQUInt8());
		flags ^= SQF_MAXCLIENTS;
	}

	if ((flags & SQF_MAXPLAYERS) == SQF_MAXPLAYERS)
	{
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		setMaxPlayers(in.readQUInt8());
		flags ^= SQF_MAXPLAYERS;
	}

	if ((flags & SQF_PWADS) == SQF_PWADS)
	{
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		qint8 numPwads = in.readQInt8();
		flags ^= SQF_PWADS;
		for (int i = 0; i < numPwads; i++)
		{
			RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
			QString wad = in.readRawUntilByte('\0');
			addWad(wad);
		}
	}

	if ((flags & SQF_GAMETYPE) == SQF_GAMETYPE)
	{
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		qint8 modeCode = in.readQInt8();

		if (modeCode < 0 || modeCode > NUM_ZANDRONUM_GAME_MODES)
		{
			modeCode = NUM_ZANDRONUM_GAME_MODES; // this will set the game mode to unknown
		}

		this->currentChallenge.gameMode = static_cast<ZandronumGameInfo::ZandronumGameMode>(modeCode);
		setGameMode(ZandronumGameInfo::gameModes()[this->currentChallenge.gameMode]);

		RETURN_BAD_IF_NOT_ENOUGH_DATA(2);
		instagib = in.readQInt8() != 0;
		buckshot = in.readQInt8() != 0;

		flags ^= SQF_GAMETYPE;
	}

	if ((flags & SQF_GAMENAME) == SQF_GAMENAME)
	{
		//Useless String
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		in.readRawUntilByte('\0');
	}

	if ((flags & SQF_IWAD) == SQF_IWAD)
	{
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		setIwad(in.readRawUntilByte('\0'));
		flags ^= SQF_IWAD;
	}

	if ((flags & SQF_FORCEPASSWORD) == SQF_FORCEPASSWORD)
	{
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		setLocked(in.readQInt8() != 0);
		flags ^= SQF_FORCEPASSWORD;
	}

	if ((flags & SQF_FORCEJOINPASSWORD) == SQF_FORCEJOINPASSWORD)
	{
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		setLockedInGame(in.readQInt8() != 0);
		flags ^= SQF_FORCEJOINPASSWORD;
	}

	if ((flags & SQF_GAMESKILL) == SQF_GAMESKILL)
	{
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		setSkill(in.readQInt8());
		flags ^= SQF_GAMESKILL;
	}

	if ((flags & SQF_BOTSKILL) == SQF_BOTSKILL)
	{
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		botSkill = in.readQInt8();
		flags ^= SQF_BOTSKILL;
	}

	if ((flags & SQF_LIMITS) == SQF_LIMITS)
	{
		flags ^= SQF_LIMITS;

		RETURN_BAD_IF_NOT_ENOUGH_DATA(2);
		fragLimit = in.readQUInt16();

		// Read timelimit and timeleft,
		// note that if timelimit == 0 then no info
		// about timeleft is sent
		RETURN_BAD_IF_NOT_ENOUGH_DATA(2);
		setTimeLimit(in.readQUInt16());
		if (timeLimit() != 0)
		{
			RETURN_BAD_IF_NOT_ENOUGH_DATA(2);
			setTimeLeft(in.readQUInt16());
		}

		RETURN_BAD_IF_NOT_ENOUGH_DATA(2 + 2 + 2);
		duelLimit = in.readQUInt16();
		pointLimit = in.readQUInt16();
		winLimit = in.readQUInt16();
		switch (this->currentChallenge.gameMode)
		{
		default:
			setScoreLimit(fragLimit);
			break;
		case ZandronumGameInfo::GAMEMODE_LASTMANSTANDING:
		case ZandronumGameInfo::GAMEMODE_TEAMLMS:
			setScoreLimit(winLimit);
			break;
		case ZandronumGameInfo::GAMEMODE_POSSESSION:
		case ZandronumGameInfo::GAMEMODE_TEAMPOSSESSION:
		case ZandronumGameInfo::GAMEMODE_TEAMGAME:
		case ZandronumGameInfo::GAMEMODE_CTF:
		case ZandronumGameInfo::GAMEMODE_ONEFLAGCTF:
		case ZandronumGameInfo::GAMEMODE_SKULLTAG:
		case ZandronumGameInfo::GAMEMODE_DOMINATION:
			setScoreLimit(pointLimit);
			break;
		}
	}

	if ((flags & SQF_TEAMDAMAGE) == SQF_TEAMDAMAGE)
	{
		RETURN_BAD_IF_NOT_ENOUGH_DATA(4);
		teamDamage = in.readFloat();
		flags ^= SQF_TEAMDAMAGE;
	}
	if ((flags & SQF_TEAMSCORES) == SQF_TEAMSCORES)
	{
		// DEPRECATED flag
		for (int i = 0; i < 2; i++)
		{
			RETURN_BAD_IF_NOT_ENOUGH_DATA(2);
			scoresMutable()[i] = in.readQInt16();
		}
		flags ^= SQF_TEAMSCORES;
	}
	if ((flags & SQF_NUMPLAYERS) == SQF_NUMPLAYERS)
	{
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		this->currentChallenge.numPlayers = in.readQUInt8();
		flags ^= SQF_NUMPLAYERS;
	}

	if ((flags & SQF_PLAYERDATA) == SQF_PLAYERDATA)
	{
		// The explicit flag for team info is sent in the segmented
		// reply. If the reply isn't segmented, it must be deduced
		// from the game mode. Team isn't sent in non-team modes.
		bool hasTeamData = gameMode().isTeamGame();
		if (this->currentChallenge.responseType == ChallengeResponse::SEGMENTED)
		{
			RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
			hasTeamData = (in.readQInt8() != 0);
		}

		flags ^= SQF_PLAYERDATA;
		for (int i = 0; i < this->currentChallenge.numPlayers; i++)
		{
			RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
			QString name = in.readRawUntilByte('\0');

			RETURN_BAD_IF_NOT_ENOUGH_DATA(2 + 2 + 1 + 1);
			int score = in.readQInt16();
			int ping = in.readQUInt16();
			bool spectating = in.readQUInt8() != 0;
			bool bot = in.readQUInt8() != 0;

			int team;
			if (hasTeamData)
			{
				RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
				team = in.readQUInt8();
			}
			else
			{
				team = Player::TEAM_NONE;
			}
			// Now there is info on time that the player is
			// on the server. We'll skip it.
			RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
			in.skipRawData(1);

			Player player(name, score, ping, static_cast<Player::PlayerTeam>(team), spectating, bot);
			addPlayer(player);
		}
	}

	if ((flags & SQF_TEAMINFO_NUMBER) == SQF_TEAMINFO_NUMBER)
	{
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		numTeams = in.readQUInt8();
		flags ^= SQF_TEAMINFO_NUMBER;
	}

	if ((flags & SQF_TEAMINFO_NAME) == SQF_TEAMINFO_NAME)
	{
		flags ^= SQF_TEAMINFO_NAME;
		for (unsigned i = 0; i < numTeams && i < ST_MAX_TEAMS; ++i)
		{
			RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
			QString name = in.readRawUntilByte('\0');
			teamInfo[i].setName(tr(name.toUtf8().constData()));
		}
	}
	if ((flags & SQF_TEAMINFO_COLOR) == SQF_TEAMINFO_COLOR)
	{
		flags ^= SQF_TEAMINFO_COLOR;
		// NOTE: This may not be correct
		unsigned forLimit = qMin(numTeams, ST_MAX_TEAMS);

		for (unsigned i = 0; i < forLimit; i++)
		{
			RETURN_BAD_IF_NOT_ENOUGH_DATA(4);
			quint32 colorRgb = in.readQUInt32();
			teamInfo[i].setColor(QColor(colorRgb));
		}
	}
	if ((flags & SQF_TEAMINFO_SCORE) == SQF_TEAMINFO_SCORE)
	{
		flags ^= SQF_TEAMINFO_SCORE;
		unsigned forLimit = qMin(numTeams, ST_MAX_TEAMS);

		for (unsigned i = 0; i < forLimit; i++)
		{
			RETURN_BAD_IF_NOT_ENOUGH_DATA(2);
			qint16 score = in.readQInt16();
			teamInfo[i].setScore(score);
			if (i < MAX_TEAMS) // Transfer to super class score array if possible.
			{
				scoresMutable()[i] = teamInfo[i].score();
			}
		}
	}

	if (in.remaining() != 0 && (flags & SQF_TESTING_SERVER) == SQF_TESTING_SERVER)
	{
		flags ^= SQF_TESTING_SERVER;

		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		setTestingServer(in.readQInt8() != 0);

		// '\0' is read if testingServer == false
		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		testingArchive = in.readRawUntilByte('\0');
	}

	if ((flags & SQF_ALL_DMFLAGS) == SQF_ALL_DMFLAGS)
	{
		flags ^= SQF_ALL_DMFLAGS;
		ZandronumServerDmflagsParser *parser = ZandronumServerDmflagsParser::mkParser(this, &stream);
		if (parser != nullptr)
		{
			setDmFlags(parser->parse());
			delete parser;
		}
	}

	if ((flags & SQF_SECURITY_SETTINGS) == SQF_SECURITY_SETTINGS)
	{
		flags ^= SQF_SECURITY_SETTINGS;

		setSecure(in.readQUInt8() != 0);
	}

	if ((flags & SQF_OPTIONAL_WADS) == SQF_OPTIONAL_WADS)
	{
		flags ^= SQF_OPTIONAL_WADS;

		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		unsigned int numOpts = in.readQUInt8();
		RETURN_BAD_IF_NOT_ENOUGH_DATA(numOpts);

		QList<PWad> pwads = wads();
		while (numOpts--)
		{
			unsigned int index = in.readQInt8();
			if (index < static_cast<unsigned>(pwads.size()))
				pwads.replace(index, PWad(pwads[index].name(), true));
		}

		resetPwadsList(pwads);
	}

	if ((flags & SQF_DEH) == SQF_DEH)
	{
		flags ^= SQF_DEH;

		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		unsigned int numDehs = in.readQUInt8();

		QList<PWad> pwads = wads();
		while (numDehs--)
		{
			RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
			QString deh = in.readRawUntilByte('\0');
			pwads << deh;
		}
		resetPwadsList(pwads);
	}

	if (this->currentChallenge.responseType == ChallengeResponse::SINGLE
		&& (flags & SQF_EXTENDED_INFO) == SQF_EXTENDED_INFO)
	{
		return readSqf2(stream);
	}
	return RESPONSE_GOOD;
}

Server::Response ZandronumServer::readSqf2(QDataStream &stream)
{
	DataStreamOperatorWrapper in(&stream);

	RETURN_BAD_IF_NOT_ENOUGH_DATA(4);
	quint32 flags = in.readQInt32();

	if ((flags & SQF2_PWAD_HASHES) == SQF2_PWAD_HASHES)
	{
		QList<PWad> pwads = wads();
		flags ^= SQF2_PWAD_HASHES;

		RETURN_BAD_IF_NOT_ENOUGH_DATA(1);
		unsigned int numHashes = in.readQInt8();

		if (numHashes > pwads.length())
		{
			return RESPONSE_BAD;
		}

		for (unsigned int index = 0; index < numHashes; index++)
		{
			RETURN_BAD_IF_NOT_ENOUGH_DATA(1);

			PWad pwad(pwads[index].name(),
				pwads[index].isOptional());
			pwad.addChecksum(QByteArray::fromHex(in.readRawUntilByte('\0')),
				QCryptographicHash::Md5);
			pwads.replace(index, pwad);
		}
		resetPwadsList(pwads);
	}

	if ((flags & SQF2_COUNTRY) == SQF2_COUNTRY)
	{
		RETURN_BAD_IF_NOT_ENOUGH_DATA(3);
		QByteArray countryCode = in.readRaw(3);
		setCountry(QString::fromLatin1(countryCode));
	}

	return RESPONSE_GOOD;
}

void ZandronumServer::resetPwadsList(const QList<PWad> &wads)
{
	clearWads();
	for (const PWad &wad : wads)
		addWad(wad);
}

QByteArray ZandronumServer::createSendRequest()
{
	// Clear the current server state and reset the challenge tracker.
	this->currentChallenge = Challenge();
	clearPlayersList();
	clearWads();
	setDmFlags(QList<DMFlagsSection>()); // Basically, clear.
	setLocked(false);
	setLockedInGame(false);
	setMaxClients(0);
	setMaxPlayers(0);
	fragLimit = 0;
	setTimeLimit(0);
	duelLimit = 0;
	pointLimit = 0;
	winLimit = 0;
	setScoreLimit(0);
	setTestingServer(false);
	testingArchive = QString();

	// Prepare the challenge query packet.
	IniSection &config = *ZandronumEnginePlugin::staticInstance()->data()->pConfig;
	const bool useSegmentedQuery = static_cast<bool>(config["SegmentedQueryBETA"]);

	uint32_t standardQuery = SQF_STANDARDQUERY;
	uint32_t extendedQuery = SQF2_STANDARDQUERY;
	if (!gApp->isGameFileIntegrityCheckEnabled())
	{
		extendedQuery &= ~SQF2_PWAD_HASHES;
	}
#define CHALLENGE_SIZE 17
#define ENCODED_CHALLENGE_BUFFER_SIZE (CHALLENGE_SIZE*2)
	const unsigned char challenge[CHALLENGE_SIZE] = {
		SERVER_CHALLENGE,
		WRITEINT32_DIRECT(unsigned char, standardQuery),
		WRITEINT32_DIRECT(unsigned char, millisecondTime()),
		WRITEINT32_DIRECT(unsigned char, extendedQuery),
		WRITEINT8_DIRECT(unsigned char, 1), // ask for the segmented reply if the server supports it
	};
	char challengeOut[ENCODED_CHALLENGE_BUFFER_SIZE];
	const int segmentationOffset = useSegmentedQuery ? 0 : 1;
	int out = ENCODED_CHALLENGE_BUFFER_SIZE;
	HUFFMAN_Encode(challenge, reinterpret_cast<unsigned char *>(challengeOut), CHALLENGE_SIZE - segmentationOffset, &out);
	return QByteArray(challengeOut, out);
#undef CHALLENGE_SIZE
#undef ENCODED_CHALLENGE_BUFFER_SIZE
}

QRgb ZandronumServer::teamColor(unsigned team) const
{
	if (team >= ST_MAX_TEAMS)
		return Server::teamColor(team);

	return teamInfo[team].color().rgb();
}

QString ZandronumServer::teamName(unsigned team) const
{
	if (team == 255)
		return "NO TEAM";

	return team < ST_MAX_TEAMS ? teamInfo[team].name() : "";
}

static void logBytes(const QByteArray &bytes)
{
	fprintf(stderr, "%u bytes (all non-printable characters are replaced with '?'):\n",
		bytes.size());

	static const int BYTES_PER_LINE = 16;

	for (int offset = 0; offset < bytes.size(); offset += BYTES_PER_LINE)
	{
		const int lineSize = qMin(BYTES_PER_LINE, bytes.size() - offset);

		for (int i = 0; i < lineSize; ++i)
		{
			fprintf(stderr, "%02X ", (unsigned char) bytes[offset + i]);
		}
		for (int i = lineSize; i < BYTES_PER_LINE; ++i)
		{
			// Pad the remaining bytes in the last line, if any.
			fprintf(stderr, "   ");
		}
		fprintf(stderr, "| ");
		for (int i = 0; i < lineSize; ++i)
		{
			char c = bytes[offset + i];
			if (c < (char)0x20 || c > (char)0x7e)
			{
				fprintf(stderr, "?");
			}
			else
			{
				fprintf(stderr, "%c", c);
			}
		}
		fprintf(stderr, "\n");
	}
}

void ZandronumServer::updatedSlot(ServerPtr server, int response)
{
	if (response == RESPONSE_BAD)
	{
		// If response is bad we will print the read request to stderr,
		// for debug purposes of course.
		QSharedPointer<ZandronumServer> s = server.staticCast<ZandronumServer>();

		fprintf(stderr, "Bad response from server: %s:%u\n",
			address().toString().toUtf8().constData(), port());
		if (s->currentChallenge.responseType == ChallengeResponse::SEGMENTED)
		{
			for (int segmentIdx = 0;
				 segmentIdx < s->currentChallenge.segments.size();
				++segmentIdx)
			{
				fprintf(stderr, ">> Segment %d:\n", segmentIdx);
				logBytes(s->currentChallenge.segments[segmentIdx]);
			}
			fprintf(stderr, ">> Raw (in arrival order):\n");
			logBytes(s->currentChallenge.raw);
		}
		else
		{
			fprintf(stderr, ">> Single packet:\n");
			logBytes(s->currentChallenge.raw);
		}
		fprintf(stderr, "-- End of response for %s:%u --\n\n",
			address().toString().toUtf8().constData(), port());
		fflush(stderr);
	}

	// Challenge is complete, so drop its tracker.
	this->currentChallenge = Challenge();
}

PathFinder ZandronumServer::wadPathFinder()
{
	PathFinder pathFinder = Server::wadPathFinder();
	if (isTestingServer())
	{
		QScopedPointer<ExeFile> exe(clientExe());
		Message message;
		QString exePath = exe->pathToExe(message);
		if (!exePath.isNull())
		{
			// exePath is path to a .bat/.sh file that resides in
			// directory above the directory of actual deployment of
			// the testing client. Fortunately, name of the .bat/.sh
			// file is the same as the name of the directory that
			// interests us. So, we cut out the file extension and
			// thus we receive a proper path that we can add to
			// PathFinder.
			QFileInfo fileInfo(exePath);
			QString dirPath = gDoomseekerTemplatedPathResolver().resolve(
				Strings::combinePaths(fileInfo.path(),
					fileInfo.completeBaseName()));
			pathFinder.addPrioritySearchDir(dirPath);
		}
	}
	return pathFinder;
}

RConProtocol *ZandronumServer::rcon()
{
	return new ZandronumRConProtocol(self());
}
