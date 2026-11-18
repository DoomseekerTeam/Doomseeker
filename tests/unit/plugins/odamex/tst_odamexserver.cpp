//------------------------------------------------------------------------------
// tst_odamexserver.cpp
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
// Copyright (C) 2026 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "odamexserver.h"
#include "odamexgamemode.h"

#include "configuration/doomseekerconfig.h"
#include "datapaths.h"
#include "serverapi/playerslist.h"
#include "wadseeker/entities/checksum.h"
#include <QDataStream>
#include <QFile>
#include <QMap>
#include <QRegularExpression>
#include <QScopedValueRollback>
#include <QStandardPaths>
#include <QTextDocumentFragment>
#include <QtTest>
#include <limits>

namespace
{
QByteArray stringCvar(const QByteArray &name, const QString &value)
{
	return name + '\0' + char(6) + value.toUtf8() + '\0';
}

QByteArray booleanCvar(const QByteArray &name)
{
	return name + '\0' + char(1);
}

QByteArray byteCvar(const QByteArray &name, quint8 value)
{
	return name + '\0' + char(2) + char(value);
}

QByteArray wordCvar(const QByteArray &name, quint16 value)
{
	QByteArray data = name + '\0' + char(3);
	QDataStream out(&data, QIODevice::WriteOnly | QIODevice::Append);
	out.setByteOrder(QDataStream::LittleEndian);
	out << value;
	return data;
}

QByteArray integerCvar(const QByteArray &name, qint32 value)
{
	QByteArray data = name + '\0' + char(4);
	QDataStream out(&data, QIODevice::WriteOnly | QIODevice::Append);
	out.setByteOrder(QDataStream::LittleEndian);
	out << value;
	return data;
}

QByteArray floatCvar(const QByteArray &name, const QString &value)
{
	return name + '\0' + char(5) + value.toUtf8() + '\0';
}

struct ReplyOptions
{
	bool hasTimeLeft = false;
	quint16 timeLeft = 0;
	bool populated = false;
	bool teamGame = false;
	QList<int> teamScores;
	quint32 protocolVersion = 7;
};

// Synthetic protocol-5/7 reply; defaults to an empty, unlimited cooperative server.
// Only the supplied CVars/options change; all other fields stay fixed.
QByteArray replyWithCvars(const QList<QByteArray> &cvars,
	const ReplyOptions &options = ReplyOptions())
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	out.setByteOrder(QDataStream::LittleEndian);
	out << quint32(0xAD032003) // Information response.
		<< quint32(12 * 256 + 30) // Engine version 12.3.0.
		<< options.protocolVersion // Negotiated protocol.
		<< quint32(0) // Echoed token.
		<< options.protocolVersion; // Actual server protocol.
	if (options.protocolVersion >= 7)
		out << quint8(0); // Empty revision string.
	else
		out << quint32(0); // Numeric revision in the legacy layout.

	const QList<QByteArray> allCvars = QList<QByteArray>{
		byteCvar("sv_maxclients", 4), byteCvar("sv_maxplayers", 4)
	} + cvars;
	out << quint8(allCvars.size());
	for (const QByteArray &cvar : allCvars)
		out.writeRawData(cvar.constData(), cvar.size());

	out << quint8(0); // No password hash.
	out.writeRawData("MAP01", sizeof("MAP01"));
	if (options.hasTimeLeft || options.protocolVersion < 6)
		out << options.timeLeft;
	if (options.teamGame)
	{
		out << quint8(options.teamScores.size());
		for (int i = 0; i < options.teamScores.size(); ++i)
		{
			const QByteArray name = "Team " + QByteArray::number(i);
			out.writeRawData(name.constData(), name.size() + 1);
			out << quint32(0xFF123456) // Team color must not be mistaken for the score.
				<< qint16(options.teamScores[i]);
		}
	}
	out << quint8(0); // No patches.
	QList<QByteArray> wads{QByteArray("ODAMEX.WAD"), QByteArray("DOOM2.WAD")};
	if (options.populated)
		wads << QByteArray("FRACTIONAL.WAD");
	out << quint8(wads.size());
	const QByteArray hash(16, '\0');
	for (const QByteArray &wad : wads)
	{
		out.writeRawData(wad.constData(), wad.size() + 1);
		out << quint8(hash.size());
		out.writeRawData(hash.constData(), hash.size());
	}
	// An empty team list cannot supply a valid team index for a player.
	const bool hasPlayer = options.populated
		&& (!options.teamGame || !options.teamScores.isEmpty());
	out << quint8(hasPlayer ? 1 : 0);
	if (hasPlayer)
	{
		out.writeRawData("Timer player", sizeof("Timer player"));
		out << quint32(0xFF40CF00); // Player color, BGRA on the wire.
		if (options.teamGame)
			out << quint8(0); // Blue team.
		out << quint16(28) // Ping.
			<< quint16(0) // Minutes in game.
			<< quint8(0) // Not spectating.
			<< qint16(0) // Frags.
			<< quint16(0) // Kills.
			<< quint16(0); // Deaths.
	}
	return data;
}
}

class TestOdamexServer : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();
	void parseOnePlayerReply();
	void parseTeamDeathmatchReply();
	void parseByteScoreLimit_data();
	void parseByteScoreLimit();
	void refreshByteScoreLimit();
	void selectLimitByGameMode_data();
	void selectLimitByGameMode();
	void refreshLimitByGameMode();
	void parseUnsignedLimitDetails_data();
	void parseUnsignedLimitDetails();
	void preserveSignedCvars();
	void parseBooleanCvars();
	void parseTeamScores_data();
	void parseTeamScores();
	void refreshTeamScores_data();
	void refreshTeamScores();
	void rejectTooManyTeams();
	void rejectTruncatedTeamScore_data();
	void rejectTruncatedTeamScore();
	void customDetailsEscapeHtml_data();
	void customDetailsEscapeHtml();
	void refreshReplacesSnapshot_data();
	void refreshReplacesSnapshot();
	void refreshAfterDisablingTimeLimit();
	void refreshAfterClearingSetting_data();
	void refreshAfterClearingSetting();
	void refreshWebSites_data();
	void refreshWebSites();
	void parseFractionalTimeLimit_data();
	void parseFractionalTimeLimit();
	void rejectInvalidTimeLimit_data();
	void rejectInvalidTimeLimit();
};

void TestOdamexServer::initTestCase()
{
	// Configuration defaults require data paths; do not load any user settings.
	QStandardPaths::setTestModeEnabled(true);
	DataPaths::initDefault(false);
}

void TestOdamexServer::parseOnePlayerReply()
{
	// Given: an unmodified protocol-7 reply captured from a local Odamex server.
	QFile dump(":/tests/odamexserver/odamex_p7_one-player.bin");
	QVERIFY2(dump.open(QIODevice::ReadOnly), qPrintable(dump.errorString()));
	const QByteArray data = dump.readAll();
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexServer server(QHostAddress("127.0.0.1"), 10666);

	// When: parsing the datagram through the refresher's public entry point.
	const Server::Response response = server.readRefreshQueryResponse(data);

	// Then: the reply is accepted and its server, file, and player data are decoded.
	QCOMPARE(response, Server::RESPONSE_GOOD);
	QCOMPARE(server.gameVersion(), QString("12.3.0 HEAD, g98b37-12737"));
	QCOMPARE(server.name(), QString("Untitled Odamex Server"));
	QCOMPARE(server.email(), QString("email@domain.com"));
	QCOMPARE(server.maxClients(), quint16(4));
	QCOMPARE(server.maxPlayers(), quint16(4));
	QCOMPARE(server.scoreLimit(), 0u);
	QCOMPARE(server.gameMode().index(), int(GameMode::SGM_Cooperative));
	QVERIFY(!server.isLocked());
	QCOMPARE(server.map(), QString("MAP01"));
	QCOMPARE(server.timeLimit(), quint16(0));
	QCOMPARE(server.iwad(), QString("DOOM2.WAD"));
	QVERIFY(server.wads().isEmpty());
	QVERIFY(server.dehs().isEmpty());
	QCOMPARE(server.players().size(), 1);
	const Player &player = server.player(0);
	QCOMPARE(player.name(), QString("apple [ORANGE]"));
	QCOMPARE(player.ping(), 28ul);
	QCOMPARE(player.score(), 0l);
	QCOMPARE(player.teamNum(), Player::TEAM_NONE);
	QVERIFY(!player.isSpectating());
}

void TestOdamexServer::parseTeamDeathmatchReply()
{
	// Given: a protocol-7 reply with teams, a time limit, and an additional PWAD.
	QFile dump(":/tests/odamexserver/odamex_p7_team-deathmatch.bin");
	QVERIFY2(dump.open(QIODevice::ReadOnly), qPrintable(dump.errorString()));
	const QByteArray data = dump.readAll();
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexServer server(QHostAddress("170.9.239.6"), 10673);

	// When: parsing the datagram through the refresher's public entry point.
	const Server::Response response = server.readRefreshQueryResponse(data);

	// Then: conditional fields are consumed correctly and the PWAD hash is preserved.
	QCOMPARE(response, Server::RESPONSE_GOOD);
	QCOMPARE(server.gameVersion(), QString("12.3.0 stable, g13841-12833"));
	QCOMPARE(server.name(), QString("Tower of Power - Friday Night Fragfest"));
	QCOMPARE(server.maxClients(), quint16(32));
	QCOMPARE(server.maxPlayers(), quint16(16));
	QCOMPARE(server.gameMode().index(), int(GameMode::SGM_TeamDeathmatch));
	QCOMPARE(server.scoreLimit(), 50u);
	QVERIFY(server.gameMode().isTeamGame());
	QVERIFY(!server.isLocked());
	QCOMPARE(server.map(), QString("MAP19"));
	QCOMPARE(server.timeLimit(), quint16(10));
	QCOMPARE(server.timeLeft(), quint16(0));
	QCOMPARE(server.iwad(), QString("DOOM2.WAD"));
	QVERIFY(server.dehs().isEmpty());
	QCOMPARE(server.players().size(), 0);
	QCOMPARE(server.numWads(), 1);
	const PWad &wad = server.wad(0);
	QCOMPARE(wad.name(), QString("TDMAX_B4.WAD"));
	QVERIFY(!wad.isOptional());
	const QList<Checksum> checksums = wad.checksums();
	QCOMPARE(checksums.size(), 1);
	QCOMPARE(checksums.first().algorithm(), QCryptographicHash::Md5);
	QCOMPARE(checksums.first().hash(), QByteArray::fromHex("393d18269fe51ba7f94acd7ed75cd749"));
}

void TestOdamexServer::parseByteScoreLimit_data()
{
	QTest::addColumn<int>("protocolVersion");
	QTest::addColumn<unsigned int>("scoreLimit");

	for (int protocolVersion : {5, 7})
	{
		for (unsigned int scoreLimit : {0u, 1u, 5u, 127u, 128u, 254u, 255u})
			QTest::newRow(qPrintable(QString("protocol-%1-limit-%2")
				.arg(protocolVersion).arg(scoreLimit))) << protocolVersion << scoreLimit;
	}
}

void TestOdamexServer::parseByteScoreLimit()
{
	// Given: Odamex declares sv_scorelimit as a byte in the range 0..255.
	QFETCH(int, protocolVersion);
	QFETCH(unsigned int, scoreLimit);
	QList<QByteArray> cvars{byteCvar("sv_gametype", 3)}; // Capture the Flag.
	if (scoreLimit != 0)
		cvars << byteCvar("sv_scorelimit", quint8(scoreLimit)); // Zero is omitted by the server.
	ReplyOptions options;
	options.protocolVersion = quint32(protocolVersion);
	options.teamGame = true;
	options.teamScores = {0, 0};
	options.populated = true;
	const QByteArray data = replyWithCvars(cvars, options);
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);

	// When: parsing the score limit through the real plugin's refresh entry point.
	const Server::Response response = server.readRefreshQueryResponse(data);

	// Then: the upper byte values remain positive limits, not zero/unlimited.
	QCOMPARE(response, Server::RESPONSE_GOOD);
	QCOMPARE(server.scoreLimit(), scoreLimit);

	// And sanity-check: the byte read preserves subsequent team, file, and player fields.
	QVERIFY(server.gameMode().isTeamGame());
	QCOMPARE(server.scores(), (QList<int>{0, 0, 0, 0}));
	QCOMPARE(server.iwad(), QString("DOOM2.WAD"));
	QCOMPARE(server.numWads(), 1);
	QCOMPARE(server.wad(0).name(), QString("FRACTIONAL.WAD"));
	const QList<Checksum> checksums = server.wad(0).checksums();
	QCOMPARE(checksums.size(), 1);
	QCOMPARE(checksums.first().algorithm(), QCryptographicHash::Md5);
	QCOMPARE(checksums.first().hash(), QByteArray(16, '\0'));
	QCOMPARE(server.players().size(), 1);
	QCOMPARE(server.player(0).teamNum(), Player::TEAM_BLUE);
	QCOMPARE(server.player(0).ping(), 28ul);
}

void TestOdamexServer::refreshByteScoreLimit()
{
	// Given: otherwise identical CTF replies varying only the score-limit CVar.
	ReplyOptions options;
	options.teamGame = true;
	options.teamScores = {0, 0};
	options.populated = true;
	const QList<QByteArray> baseCvars{byteCvar("sv_gametype", 3)};
	const QByteArray initialData = replyWithCvars(
		baseCvars + QList<QByteArray>{byteCvar("sv_scorelimit", 255)}, options);
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);

	// And: the server already holds the maximum byte score limit.
	QCOMPARE(server.readRefreshQueryResponse(initialData), Server::RESPONSE_GOOD);
	QCOMPARE(server.scoreLimit(), 255u);

	// When: changing, repeating, clearing, and restoring the score limit.
	for (unsigned int scoreLimit : {128u, 128u, 0u, 255u, 127u, 0u})
	{
		QList<QByteArray> cvars = baseCvars;
		if (scoreLimit != 0)
			cvars << byteCvar("sv_scorelimit", quint8(scoreLimit));
		const Server::Response response = server.readRefreshQueryResponse(
			replyWithCvars(cvars, options));

		// Then: each snapshot replaces the limit, including omission for unlimited.
		QCOMPARE(response, Server::RESPONSE_GOOD);
		QCOMPARE(server.scoreLimit(), scoreLimit);

		// And sanity-check: the rest of the populated reply remains aligned.
		QCOMPARE(server.iwad(), QString("DOOM2.WAD"));
		QCOMPARE(server.numWads(), 1);
		QCOMPARE(server.players().size(), 1);
		QCOMPARE(server.player(0).teamNum(), Player::TEAM_BLUE);
	}
}

void TestOdamexServer::selectLimitByGameMode_data()
{
	QTest::addColumn<int>("baseMode");
	QTest::addColumn<QByteArray>("mutator");
	QTest::addColumn<int>("expectedMode");
	QTest::addColumn<unsigned int>("expectedLimit");

	const QByteArray none;
	const QByteArray lives = integerCvar("g_lives", 3);
	QTest::newRow("cooperative") << 0 << none << int(GameMode::SGM_Cooperative) << 0u;
	QTest::newRow("deathmatch") << 1 << none << int(GameMode::SGM_Deathmatch) << 40000u;
	QTest::newRow("lms") << 1 << lives << int(OdamexGameMode::DERIVED_LAST_MARINE_STANDING)
		<< 40000u;
	QTest::newRow("team-deathmatch") << 2 << none << int(GameMode::SGM_TeamDeathmatch)
		<< 40000u;
	QTest::newRow("team-lms") << 2 << lives
		<< int(OdamexGameMode::DERIVED_TEAM_LAST_MARINE_STANDING) << 40000u;
	QTest::newRow("ctf") << 3 << none << int(GameMode::SGM_CTF) << 5u;
	QTest::newRow("attack-defend-ctf") << 3 << booleanCvar("g_sides")
		<< int(OdamexGameMode::DERIVED_ATTACK_DEFEND_CTF) << 5u;
	QTest::newRow("lms-ctf") << 3 << lives << int(OdamexGameMode::DERIVED_LMS_CTF) << 5u;
	QTest::newRow("horde") << 4 << none << int(OdamexGameMode::DERIVED_HORDE) << 0u;
}

void TestOdamexServer::selectLimitByGameMode()
{
	// Given: a server advertising both limits, regardless of whether it uses them.
	QFETCH(int, baseMode);
	QFETCH(QByteArray, mutator);
	QFETCH(int, expectedMode);
	QFETCH(unsigned int, expectedLimit);
	QList<QByteArray> cvars{wordCvar("sv_fraglimit", 40000), byteCvar("sv_scorelimit", 5)};
	if (baseMode != 0)
		cvars << byteCvar("sv_gametype", quint8(baseMode));
	if (!mutator.isEmpty())
		cvars << mutator;
	ReplyOptions options;
	options.teamGame = baseMode == 2 || baseMode == 3;
	options.teamScores = {0, 0};
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);

	// When: parsing the server's reply.
	const Server::Response response = server.readRefreshQueryResponse(replyWithCvars(cvars, options));

	// Then: the browser's limit comes from the setting that applies to this mode.
	QCOMPARE(response, Server::RESPONSE_GOOD);
	QCOMPARE(server.gameMode().index(), expectedMode);
	QCOMPARE(server.scoreLimit(), expectedLimit);
}

void TestOdamexServer::refreshLimitByGameMode()
{
	// Given: CTF and Deathmatch replies, with zero-valued limits omitted on the wire.
	const auto reply = [](bool ctf, bool hasFragLimit, bool hasScoreLimit)
	{
		QList<QByteArray> cvars{byteCvar("sv_gametype", ctf ? 3 : 1)};
		if (hasFragLimit)
			cvars << wordCvar("sv_fraglimit", 20);
		if (hasScoreLimit)
			cvars << byteCvar("sv_scorelimit", 5);
		ReplyOptions options;
		options.teamGame = ctf;
		options.teamScores = {0, 0};
		return replyWithCvars(cvars, options);
	};
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);

	// And: the browser already shows the CTF score limit.
	QCOMPARE(server.readRefreshQueryResponse(reply(true, true, true)), Server::RESPONSE_GOOD);
	QCOMPARE(server.scoreLimit(), 5u);

	struct Snapshot
	{
		bool ctf;
		bool hasFragLimit;
		bool hasScoreLimit;
		unsigned int expectedLimit;
	};
	const QList<Snapshot> snapshots{
		{false, true,  true,  20}, // Switch to Deathmatch with both settings unchanged.
		{false, false, true,   0}, // Disable the frag limit; do not fall back to score.
		{true,  true,  false,  0}, // Switch to CTF with no score limit; ignore frags.
		{true,  true,  true,   5}, // Restore the score limit.
	};
	for (const Snapshot &snapshot : snapshots)
	{
		// When: refreshing after a mode or limit change.
		const Server::Response response = server.readRefreshQueryResponse(
			reply(snapshot.ctf, snapshot.hasFragLimit, snapshot.hasScoreLimit));

		// Then: only the current mode's limit applies, even if it is omitted.
		QCOMPARE(response, Server::RESPONSE_GOOD);
		QCOMPARE(server.gameMode().index(),
			int(snapshot.ctf ? GameMode::SGM_CTF : GameMode::SGM_Deathmatch));
		QCOMPARE(server.scoreLimit(), snapshot.expectedLimit);
	}
}

void TestOdamexServer::parseUnsignedLimitDetails_data()
{
	QTest::addColumn<QString>("name");
	QTest::addColumn<QByteArray>("cvar");
	QTest::addColumn<int>("value");

	for (int value : {1, 127, 128, 255})
	{
		QTest::newRow(qPrintable(QString("team-capacity-%1").arg(value)))
			<< QString("sv_maxplayersperteam")
			<< byteCvar("sv_maxplayersperteam", quint8(value)) << value;
	}
	for (int value : {1, 32767, 32768, 40000, 65535})
	{
		QTest::newRow(qPrintable(QString("frag-limit-%1").arg(value)))
			<< QString("sv_fraglimit") << wordCvar("sv_fraglimit", quint16(value)) << value;
	}
}

void TestOdamexServer::parseUnsignedLimitDetails()
{
	// Given: a populated team game advertising one limit with its server wire type.
	QFETCH(QString, name);
	QFETCH(QByteArray, cvar);
	QFETCH(int, value);
	ReplyOptions options;
	options.teamGame = true;
	options.teamScores = {0, 0};
	options.populated = true;
	const QByteArray data = replyWithCvars({byteCvar("sv_gametype", 2), cvar}, options);
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);

	// When: parsing the reply and rendering its custom details.
	const Server::Response response = server.readRefreshQueryResponse(data);
	const QString text = QTextDocumentFragment::fromHtml(server.customDetails())
		.toPlainText().simplified();

	// Then: the displayed limit retains its value even when the high bit is set.
	QCOMPARE(response, Server::RESPONSE_GOOD);
	const QRegularExpression pattern(QString("\\b%1\\s+(-?\\d+)\\b")
		.arg(QRegularExpression::escape(name)));
	const QRegularExpressionMatch match = pattern.match(text);
	QVERIFY2(match.hasMatch(), qPrintable(text));
	QCOMPARE(match.captured(1).toInt(), value);

	// And sanity-check: files and player fields following the CVars still align.
	QCOMPARE(server.iwad(), QString("DOOM2.WAD"));
	QCOMPARE(server.numWads(), 1);
	QCOMPARE(server.wad(0).name(), QString("FRACTIONAL.WAD"));
	QCOMPARE(server.players().size(), 1);
	QCOMPARE(server.player(0).teamNum(), Player::TEAM_BLUE);
	QCOMPARE(server.player(0).ping(), 28ul);
}

void TestOdamexServer::preserveSignedCvars()
{
	// Given: a legitimate negative byte sentinel and unknown integers using signed decoding.
	const QByteArray data = replyWithCvars({
		byteCvar("sv_scorelimit", 255),
		byteCvar("g_thingfilter", 255), // Odamex permits -1, serialized as byte 0xFF.
		byteCvar("unknown_byte", 128),
		wordCvar("unknown_word", 32768),
		integerCvar("unknown_integer_min", std::numeric_limits<qint32>::min()),
		integerCvar("unknown_integer_negative", -1),
		integerCvar("unknown_integer_max", std::numeric_limits<qint32>::max())
	});
	const QMap<QString, int> expected{
		{"g_thingfilter", -1}, {"unknown_byte", -128}, {"unknown_word", -32768},
		{"unknown_integer_min", std::numeric_limits<qint32>::min()},
		{"unknown_integer_negative", -1},
		{"unknown_integer_max", std::numeric_limits<qint32>::max()}
	};
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);

	// When: parsing these CVars alongside the score-limit exception.
	const Server::Response response = server.readRefreshQueryResponse(data);
	const QString text = QTextDocumentFragment::fromHtml(server.customDetails())
		.toPlainText().simplified();

	// Then: the exception does not change the generic signed interpretation.
	QCOMPARE(response, Server::RESPONSE_GOOD);
	for (auto iter = expected.constBegin(); iter != expected.constEnd(); ++iter)
	{
		const QRegularExpression pattern(QString("\\b%1\\s+(-?\\d+)\\b")
			.arg(QRegularExpression::escape(iter.key())));
		const QRegularExpressionMatch match = pattern.match(text);
		QVERIFY2(match.hasMatch(), qPrintable(text));
		QCOMPARE(match.captured(1).toInt(), iter.value());
	}
}

void TestOdamexServer::parseBooleanCvars()
{
	// Given: Attack & Defend CTF with two true boolean CVars carrying no value bytes.
	ReplyOptions options;
	options.teamGame = true;
	options.teamScores = {0, 0};
	options.populated = true;
	const QByteArray data = replyWithCvars({byteCvar("sv_gametype", 3),
		booleanCvar("g_sides"), booleanCvar("sv_friendlyfire")}, options);
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);

	// When: parsing the reply and producing server details.
	const Server::Response response = server.readRefreshQueryResponse(data);
	const QString text = QTextDocumentFragment::fromHtml(server.customDetails())
		.toPlainText().simplified();

	// Then: g_sides enables Attack & Defend and friendly fire is shown as true.
	QCOMPARE(response, Server::RESPONSE_GOOD);
	QCOMPARE(server.gameMode().index(), int(OdamexGameMode::DERIVED_ATTACK_DEFEND_CTF));
	const QRegularExpression friendlyFire("\\bsv_friendlyfire\\s+true\\b");
	QVERIFY2(friendlyFire.match(text).hasMatch(), qPrintable(text));

	// And sanity-check: the following populated payload remains aligned.
	QCOMPARE(server.iwad(), QString("DOOM2.WAD"));
	QCOMPARE(server.numWads(), 1);
	QCOMPARE(server.players().size(), 1);
	QCOMPARE(server.player(0).teamNum(), Player::TEAM_BLUE);
}

void TestOdamexServer::parseTeamScores_data()
{
	QTest::addColumn<int>("gameType");
	QTest::addColumn<bool>("sides");
	QTest::addColumn<int>("protocolVersion");
	QTest::addColumn<QList<int>>("teamScores");

	QTest::newRow("team-deathmatch") << 2 << false << 7 << QList<int>{7, 12};
	QTest::newRow("negative-scores") << 2 << false << 7 << QList<int>{-1, -12};
	QTest::newRow("zero-scores") << 2 << false << 7 << QList<int>{0, 0};
	QTest::newRow("capture-the-flag") << 3 << false << 7 << QList<int>{3, 9};
	QTest::newRow("attack-defend-ctf") << 3 << true << 7 << QList<int>{4, -1};
	QTest::newRow("three-teams") << 2 << false << 7 << QList<int>{7, -1, 12};
	QTest::newRow("signed-wire-bounds") << 2 << false << 7 << QList<int>{-32768, 32767};
	// Current Odamex has three teams; this synthetic control fills all API slots.
	QTest::newRow("all-score-slots") << 2 << false << 7 << QList<int>{1, 2, 3, 4};
	QTest::newRow("empty-team-list") << 2 << false << 7 << QList<int>{};
	// Synthetic compatibility control, not a capture from a historical release.
	QTest::newRow("legacy-protocol-5") << 2 << false << 5 << QList<int>{7, -1};
}

void TestOdamexServer::parseTeamScores()
{
	// Given: a populated team-game reply with signed 16-bit points after each color.
	QFETCH(int, gameType);
	QFETCH(bool, sides);
	QFETCH(int, protocolVersion);
	QFETCH(QList<int>, teamScores);
	QList<QByteArray> cvars{byteCvar("sv_gametype", quint8(gameType))};
	if (sides)
		cvars << (QByteArray("g_sides") + '\0' + char(1)); // True boolean, no value bytes.
	ReplyOptions options;
	options.teamGame = true;
	options.teamScores = teamScores;
	options.protocolVersion = quint32(protocolVersion);
	options.populated = true;
	const QByteArray data = replyWithCvars(cvars, options);
	QList<int> expectedScores = teamScores;
	while (expectedScores.size() < MAX_TEAMS)
		expectedScores << 0;
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);

	// When: parsing the complete reply through the public refresh entry point.
	const Server::Response response = server.readRefreshQueryResponse(data);

	// Then: all team points retain their sign and unused score slots stay zero.
	QCOMPARE(response, Server::RESPONSE_GOOD);
	QVERIFY(server.gameMode().isTeamGame());
	QCOMPARE(server.scores(), expectedScores);

	// And sanity-check: team fields do not misalign the subsequent files or player.
	QCOMPARE(server.iwad(), QString("DOOM2.WAD"));
	QCOMPARE(server.numWads(), 1);
	QCOMPARE(server.wad(0).name(), QString("FRACTIONAL.WAD"));
	const QList<Checksum> checksums = server.wad(0).checksums();
	QCOMPARE(checksums.size(), 1);
	QCOMPARE(checksums.first().algorithm(), QCryptographicHash::Md5);
	QCOMPARE(checksums.first().hash(), QByteArray(16, '\0'));
	QCOMPARE(server.players().size(), teamScores.isEmpty() ? 0 : 1);
	if (!teamScores.isEmpty())
	{
		QCOMPARE(server.player(0).name(), QString("Timer player"));
		QCOMPARE(server.player(0).teamNum(), Player::TEAM_BLUE);
		QCOMPARE(server.player(0).ping(), 28ul);
	}
}

void TestOdamexServer::refreshTeamScores_data()
{
	QTest::addColumn<QList<int>>("previousScores");
	QTest::addColumn<QList<int>>("currentScores");
	QTest::addColumn<int>("currentGameType");

	const QList<int> previous{7, -2, 12};
	QTest::newRow("changed-scores") << previous << QList<int>{-1, 5, 4} << 2;
	QTest::newRow("reset-to-zero") << previous << QList<int>{0, 0, 0} << 2;
	QTest::newRow("fewer-teams") << previous << QList<int>{4, -1} << 2;
	QTest::newRow("empty-team-list") << previous << QList<int>{} << 2;
	QTest::newRow("cooperative") << previous << QList<int>{} << 0;
	QTest::newRow("horde") << previous << QList<int>{} << 4;
	QTest::newRow("clear-fourth-slot") << QList<int>{7, -2, 12, 5} << QList<int>{1, 2} << 2;
}

void TestOdamexServer::refreshTeamScores()
{
	// Given: replies changing only team scores/count or switching out of a team game.
	QFETCH(QList<int>, previousScores);
	QFETCH(QList<int>, currentScores);
	QFETCH(int, currentGameType);
	ReplyOptions previous;
	previous.teamGame = true;
	previous.teamScores = previousScores;
	previous.populated = true;
	ReplyOptions current;
	current.teamGame = currentGameType == 2;
	current.teamScores = currentScores;
	current.populated = true;
	const QByteArray previousData = replyWithCvars({byteCvar("sv_gametype", 2)}, previous);
	const QByteArray currentData = replyWithCvars(
		{byteCvar("sv_gametype", quint8(currentGameType))}, current);
	QList<int> expectedPrevious = previousScores;
	while (expectedPrevious.size() < MAX_TEAMS)
		expectedPrevious << 0;
	QList<int> expectedCurrent = currentScores;
	while (expectedCurrent.size() < MAX_TEAMS)
		expectedCurrent << 0;
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);

	// And: the server already holds nonzero points, including later team slots.
	QCOMPARE(server.readRefreshQueryResponse(previousData), Server::RESPONSE_GOOD);
	QCOMPARE(server.scores(), expectedPrevious);

	// When: replacing the snapshot, repeating it, and restoring the original teams.
	for (bool restore : {false, false, true})
	{
		const Server::Response response = server.readRefreshQueryResponse(
			restore ? previousData : currentData);

		// Then: no scores survive from absent teams or an earlier game mode.
		QCOMPARE(response, Server::RESPONSE_GOOD);
		QCOMPARE(server.scores(), restore ? expectedPrevious : expectedCurrent);
		QCOMPARE(server.gameMode().isTeamGame(), restore || current.teamGame);

		// And sanity-check: files and player-team fields remain aligned after refresh.
		QCOMPARE(server.iwad(), QString("DOOM2.WAD"));
		QCOMPARE(server.numWads(), 1);
		QCOMPARE(server.wad(0).name(), QString("FRACTIONAL.WAD"));
		const bool hasPlayer = restore || !current.teamGame || !currentScores.isEmpty();
		QCOMPARE(server.players().size(), hasPlayer ? 1 : 0);
		if (hasPlayer)
			QCOMPARE(server.player(0).teamNum(), restore || current.teamGame
				? Player::TEAM_BLUE : Player::TEAM_NONE);
	}
}

void TestOdamexServer::rejectTooManyTeams()
{
	// Given: a complete reply with more teams than the existing API can store.
	ReplyOptions options;
	options.teamGame = true;
	for (int i = 0; i <= MAX_TEAMS; ++i)
		options.teamScores << i + 1;
	const QByteArray data = replyWithCvars({byteCvar("sv_gametype", 2)}, options);
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);

	// When: parsing the unsupported team count.
	const Server::Response response = server.readRefreshQueryResponse(data);

	// Then: the reply is rejected instead of indexing beyond the score list.
	QCOMPARE(response, Server::RESPONSE_BAD);
}

void TestOdamexServer::rejectTruncatedTeamScore_data()
{
	QTest::addColumn<int>("remainingTeamBytes");
	for (int bytes = 0; bytes < 6; ++bytes)
		QTest::newRow(qPrintable(QString("team-bytes-%1").arg(bytes))) << bytes;
}

void TestOdamexServer::rejectTruncatedTeamScore()
{
	// Given: the last team's four-byte color and two-byte score are incomplete.
	QFETCH(int, remainingTeamBytes);
	ReplyOptions options;
	options.teamGame = true;
	options.teamScores = {7, -1};
	const QByteArray fullData = replyWithCvars({byteCvar("sv_gametype", 2)}, options);
	const QByteArray lastTeamName("Team 1\0", 7);
	const int nameOffset = fullData.indexOf(lastTeamName);
	QVERIFY(nameOffset >= 0);
	const QByteArray data = fullData.left(nameOffset + lastTeamName.size() + remainingTeamBytes);
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);

	// When: parsing the truncated team record.
	const Server::Response response = server.readRefreshQueryResponse(data);

	// Then: a partial color/score must not be accepted as a complete snapshot.
	QCOMPARE(response, Server::RESPONSE_BAD);
}

void TestOdamexServer::customDetailsEscapeHtml_data()
{
	QTest::addColumn<QByteArray>("name");
	QTest::addColumn<QString>("value");
	QTest::addColumn<QString>("expectedName");
	QTest::addColumn<QString>("expectedValue");

	QTest::newRow("plain-text")
		<< QByteArray("sv_motd") << QString("A plain message")
		<< QString("sv_motd") << QString("A plain message");
	QTest::newRow("markup-in-value")
		<< QByteArray("sv_motd")
		<< QString("<b>Message</b><img src=\"https://example.invalid/image?a=1&b=2\">")
		<< QString("sv_motd")
		<< QString("&lt;b&gt;Message&lt;/b&gt;&lt;img src=&quot;"
			"https://example.invalid/image?a=1&amp;b=2&quot;&gt;");
	QTest::newRow("markup-in-name")
		<< QByteArray("<b>name</b>&\"quoted\"") << QString("A plain message")
		<< QString("&lt;b&gt;name&lt;/b&gt;&amp;&quot;quoted&quot;")
		<< QString("A plain message");
	QTest::newRow("literal-entities")
		<< QByteArray("sv_motd") << QString("&lt;b&gt; &amp; &#60; \"quoted\" 'text'")
		<< QString("sv_motd")
		<< QString("&amp;lt;b&amp;gt; &amp;amp; &amp;#60; &quot;quoted&quot; 'text'");
	QTest::newRow("markup-in-name-and-value")
		<< QByteArray("<name>") << QString("<value>&")
		<< QString("&lt;name&gt;") << QString("&lt;value&gt;&amp;");
}

void TestOdamexServer::customDetailsEscapeHtml()
{
	// Given: a full reply with a server-controlled CVar name and value.
	QFETCH(QByteArray, name);
	QFETCH(QString, value);
	QFETCH(QString, expectedName);
	QFETCH(QString, expectedValue);
	const QByteArray data = replyWithCvars({stringCvar(name, value)});
	const QString expectedHtml = "<ul><li>" + expectedName + ' ' + expectedValue + "</li></ul>";
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);

	// When: parsing the reply and formatting its custom details.
	const Server::Response response = server.readRefreshQueryResponse(data);
	const QString details = server.customDetails();

	// Then: server text is escaped, while Doomseeker's list markup remains intact.
	QCOMPARE(response, Server::RESPONSE_GOOD);
	QCOMPARE(details, expectedHtml);

	// And sanity-check: repeated formatting does not mutate or double-escape the data.
	QCOMPARE(server.customDetails(), expectedHtml);
}

void TestOdamexServer::refreshReplacesSnapshot_data()
{
	QTest::addColumn<QString>("previousFixture");
	QTest::addColumn<QString>("currentFixture");

	const QString limited = "odamex_p7_deathmatch-time-limited.bin";
	const QString unlimited = "odamex_p7_deathmatch-unlimited.bin";
	const QString cooperative = "odamex_p7_one-player.bin";
	const QString teamDeathmatch = "odamex_p7_team-deathmatch.bin";
	QTest::newRow("limited-to-unlimited") << limited << unlimited;
	QTest::newRow("unlimited-to-limited") << unlimited << limited;
	QTest::newRow("repeat-limited") << limited << limited;
	QTest::newRow("repeat-unlimited") << unlimited << unlimited;
	QTest::newRow("cooperative-to-team-deathmatch") << cooperative << teamDeathmatch;
	QTest::newRow("team-deathmatch-to-cooperative") << teamDeathmatch << cooperative;
}

void TestOdamexServer::refreshReplacesSnapshot()
{
	// Given: captured snapshots representing a reconfiguration at the same endpoint.
	QFETCH(QString, previousFixture);
	QFETCH(QString, currentFixture);
	QFile previousDump(QString(":/tests/odamexserver/%1").arg(previousFixture));
	QVERIFY2(previousDump.open(QIODevice::ReadOnly), qPrintable(previousDump.errorString()));
	const QByteArray previousData = previousDump.readAll();
	QFile currentDump(QString(":/tests/odamexserver/%1").arg(currentFixture));
	QVERIFY2(currentDump.open(QIODevice::ReadOnly), qPrintable(currentDump.errorString()));
	const QByteArray currentData = currentDump.readAll();
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);

	// And: the server already holds the previous snapshot.
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);
	QCOMPARE(server.readRefreshQueryResponse(previousData), Server::RESPONSE_GOOD);

	// And: a fresh server provides the reference state for the current snapshot.
	OdamexServer freshServer(QHostAddress("192.0.2.1"), 10666);
	QCOMPARE(freshServer.readRefreshQueryResponse(currentData), Server::RESPONSE_GOOD);

	// When: replacing the previous snapshot with another complete reply.
	const Server::Response response = server.readRefreshQueryResponse(currentData);

	// Then: parsed state matches a fresh parse, regardless of the previous snapshot.
	// This tests state replacement, not correctness of the initial decoding itself.
	QCOMPARE(response, Server::RESPONSE_GOOD);
	QCOMPARE(server.gameVersion(), freshServer.gameVersion());
	QCOMPARE(server.name(), freshServer.name());
	QCOMPARE(server.email(), freshServer.email());
	QCOMPARE(server.maxClients(), freshServer.maxClients());
	QCOMPARE(server.maxPlayers(), freshServer.maxPlayers());
	QCOMPARE(server.gameMode().index(), freshServer.gameMode().index());
	QCOMPARE(server.gameMode().isTeamGame(), freshServer.gameMode().isTeamGame());
	QCOMPARE(server.isLocked(), freshServer.isLocked());
	QCOMPARE(server.map(), freshServer.map());
	QCOMPARE(server.scoreLimit(), freshServer.scoreLimit());
	QCOMPARE(server.timeLimit(), freshServer.timeLimit());
	QCOMPARE(server.timeLeft(), freshServer.timeLeft());
	QCOMPARE(server.webSite(), freshServer.webSite());
	QCOMPARE(server.additionalWebSites(), freshServer.additionalWebSites());
	QCOMPARE(server.customDetails(), freshServer.customDetails());
	QCOMPARE(server.iwad(), freshServer.iwad());
	QCOMPARE(server.dehs(), freshServer.dehs());
	QCOMPARE(server.numWads(), freshServer.numWads());
	for (int i = 0; i < freshServer.numWads(); ++i)
	{
		const PWad &wad = server.wad(i);
		const PWad &expectedWad = freshServer.wad(i);
		QCOMPARE(wad.name(), expectedWad.name());
		QCOMPARE(wad.isOptional(), expectedWad.isOptional());
		const QList<Checksum> checksums = wad.checksums();
		const QList<Checksum> expectedChecksums = expectedWad.checksums();
		QCOMPARE(checksums.size(), expectedChecksums.size());
		for (int j = 0; j < expectedChecksums.size(); ++j)
		{
			QCOMPARE(checksums[j].algorithm(), expectedChecksums[j].algorithm());
			QCOMPARE(checksums[j].hash(), expectedChecksums[j].hash());
		}
	}
	QCOMPARE(server.players().size(), freshServer.players().size());
	for (int i = 0; i < freshServer.players().size(); ++i)
	{
		const Player &player = server.player(i);
		const Player &expectedPlayer = freshServer.player(i);
		QCOMPARE(player.name(), expectedPlayer.name());
		QCOMPARE(player.ping(), expectedPlayer.ping());
		QCOMPARE(player.score(), expectedPlayer.score());
		QCOMPARE(player.teamNum(), expectedPlayer.teamNum());
		QCOMPARE(player.isSpectating(), expectedPlayer.isSpectating());
		QCOMPARE(player.isBot(), expectedPlayer.isBot());
	}
}

void TestOdamexServer::refreshAfterDisablingTimeLimit()
{
	// Given: a captured Deathmatch reply and a variant with only the time limit removed.
	// The CVar count is adjusted, and the conditional time-left field is omitted.
	// Regression for https://zandronum.com/tracker/view.php?id=4584.
	QFile limitedDump(":/tests/odamexserver/odamex_p7_deathmatch-time-limited.bin");
	QVERIFY2(limitedDump.open(QIODevice::ReadOnly), qPrintable(limitedDump.errorString()));
	const QByteArray limitedData = limitedDump.readAll();
	QFile unlimitedDump(":/tests/odamexserver/odamex_p7_deathmatch-time-limit-disabled.bin");
	QVERIFY2(unlimitedDump.open(QIODevice::ReadOnly), qPrintable(unlimitedDump.errorString()));
	const QByteArray unlimitedData = unlimitedDump.readAll();
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);

	// And sanity-check: the unlimited reply parses successfully on a fresh server.
	OdamexServer freshServer(QHostAddress("192.0.2.1"), 10666);
	QCOMPARE(freshServer.readRefreshQueryResponse(unlimitedData), Server::RESPONSE_GOOD);
	QCOMPARE(freshServer.gameMode().index(), int(GameMode::SGM_Deathmatch));
	QCOMPARE(freshServer.timeLimit(), quint16(0));
	QCOMPARE(freshServer.map(), QString("MAP00"));
	QCOMPARE(freshServer.iwad(), QString("DOOM2.WAD"));
	QCOMPARE(freshServer.numWads(), 1);
	QCOMPARE(freshServer.wad(0).name(), QString("DUEL2020OA.WAD"));

	// And: the server under test has already parsed a reply with a 10-minute limit.
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);
	QCOMPARE(server.readRefreshQueryResponse(limitedData), Server::RESPONSE_GOOD);
	QCOMPARE(server.gameMode().index(), int(GameMode::SGM_Deathmatch));
	QCOMPARE(server.timeLimit(), quint16(10));

	// When: disabling the time limit, refreshing again, and restoring the limit.
	for (bool timeLimited : {false, false, true})
	{
		const QByteArray &data = timeLimited ? limitedData : unlimitedData;
		const Server::Response response = server.readRefreshQueryResponse(data);

		// Then: every refresh succeeds and reflects the current reply, not old state.
		QCOMPARE(response, Server::RESPONSE_GOOD);
		QCOMPARE(server.gameMode().index(), int(GameMode::SGM_Deathmatch));
		QCOMPARE(server.timeLimit(), quint16(timeLimited ? 10 : 0));
		QCOMPARE(server.timeLeft(), quint16(0));
		QCOMPARE(server.map(), QString("MAP00"));
		QCOMPARE(server.iwad(), QString("DOOM2.WAD"));
		QCOMPARE(server.numWads(), 1);
		const PWad &wad = server.wad(0);
		QCOMPARE(wad.name(), QString("DUEL2020OA.WAD"));
		const QList<Checksum> checksums = wad.checksums();
		QCOMPARE(checksums.size(), 1);
		QCOMPARE(checksums.first().algorithm(), QCryptographicHash::Md5);
		QCOMPARE(checksums.first().hash(), QByteArray::fromHex("212deb430323759b26ac6f8dda8a2ff1"));
		QCOMPARE(server.players().size(), 0);
	}
}

void TestOdamexServer::refreshAfterClearingSetting_data()
{
	QTest::addColumn<QByteArray>("omittedCvar");
	QTest::addColumn<QString>("expectedName");
	QTest::addColumn<QString>("expectedEmail");
	QTest::addColumn<uint>("expectedScoreLimit");

	const QString name = "Refresh test server";
	const QString email = "admin@example.invalid";
	QTest::newRow("clear-hostname")
		<< stringCvar("sv_hostname", name) << QString() << email << 5u;
	QTest::newRow("clear-email")
		<< stringCvar("sv_email", email) << name << QString() << 5u;
	QTest::newRow("disable-score-limit")
		<< byteCvar("sv_scorelimit", 5) << name << email << 0u;
}

void TestOdamexServer::refreshAfterClearingSetting()
{
	// Given: full replies differing only in one omitted empty/zero-valued CVar.
	QFETCH(QByteArray, omittedCvar);
	QFETCH(QString, expectedName);
	QFETCH(QString, expectedEmail);
	QFETCH(uint, expectedScoreLimit);
	const QString name = "Refresh test server";
	const QString email = "admin@example.invalid";
	const QList<QByteArray> cvars{
		stringCvar("sv_hostname", name), stringCvar("sv_email", email),
		byteCvar("sv_gametype", 3), byteCvar("sv_scorelimit", 5)
	};
	QList<QByteArray> clearedCvars = cvars;
	QVERIFY(clearedCvars.removeOne(omittedCvar));
	ReplyOptions options;
	options.teamGame = true;
	options.teamScores = {0, 0};
	const QByteArray originalData = replyWithCvars(cvars, options);
	const QByteArray clearedData = replyWithCvars(clearedCvars, options);
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);

	// And: the server already holds the original nonempty settings.
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);
	QCOMPARE(server.readRefreshQueryResponse(originalData), Server::RESPONSE_GOOD);
	QCOMPARE(server.name(), name);
	QCOMPARE(server.email(), email);
	QCOMPARE(server.scoreLimit(), 5u);

	// When: clearing the setting, refreshing again, and restoring its original value.
	for (bool restored : {false, false, true})
	{
		const Server::Response response = server.readRefreshQueryResponse(
			restored ? originalData : clearedData);

		// Then: the omitted setting is cleared, unrelated settings are unchanged,
		// and restoring the setting works on the same server object.
		QCOMPARE(response, Server::RESPONSE_GOOD);
		QCOMPARE(server.name(), restored ? name : expectedName);
		QCOMPARE(server.email(), restored ? email : expectedEmail);
		QCOMPARE(server.scoreLimit(), restored ? 5u : expectedScoreLimit);
		QCOMPARE(server.maxClients(), quint16(4));
		QCOMPARE(server.maxPlayers(), quint16(4));
		QCOMPARE(server.gameMode().index(), int(GameMode::SGM_CTF));
		QCOMPARE(server.map(), QString("MAP01"));
		QCOMPARE(server.iwad(), QString("DOOM2.WAD"));
	}
}

void TestOdamexServer::refreshWebSites_data()
{
	QTest::addColumn<QString>("previousWebsite");
	QTest::addColumn<QString>("previousDownloads");
	QTest::addColumn<QString>("currentWebsite");
	QTest::addColumn<QString>("currentDownloads");
	QTest::addColumn<QString>("expectedWebsite");
	QTest::addColumn<QStringList>("expectedAdditional");

	const QString none;
	const QString first = "https://first.example.invalid/wads";
	const QString second = "https://second.example.invalid/wads";
	const QString third = "https://third.example.invalid/wads";
	const QString fourth = "https://fourth.example.invalid/wads";
	const QString website = "https://server.example.invalid/";
	const QString newWebsite = "https://new.example.invalid/";
	const QString downloads = first + ' ' + second;
	const QString newDownloads = third + ' ' + fourth;

	QTest::newRow("repeat-single-download-site")
		<< none << first << none << first << first << QStringList();
	QTest::newRow("repeat-multiple-download-sites")
		<< none << downloads << none << downloads << first << QStringList{second};
	QTest::newRow("replace-single-download-site")
		<< none << first << none << third << third << QStringList();
	QTest::newRow("replace-multiple-download-sites")
		<< none << downloads << none << newDownloads << third << QStringList{fourth};
	QTest::newRow("remove-secondary-download-site")
		<< none << downloads << none << first << first << QStringList();
	QTest::newRow("clear-download-sites")
		<< none << downloads << none << none << none << QStringList();
	QTest::newRow("add-download-sites")
		<< none << none << none << downloads << first << QStringList{second};
	QTest::newRow("repeat-website-and-download-sites")
		<< website << downloads << website << downloads << website << QStringList{first, second};
	QTest::newRow("replace-website")
		<< website << downloads << newWebsite << downloads << newWebsite
		<< QStringList{first, second};
	QTest::newRow("clear-website")
		<< website << none << none << none << none << QStringList();
	QTest::newRow("website-to-download-fallback")
		<< website << downloads << none << downloads << first << QStringList{second};
	QTest::newRow("download-fallback-to-website")
		<< none << downloads << website << downloads << website << QStringList{first, second};
	QTest::newRow("clear-download-sites-with-website")
		<< website << downloads << website << none << website << QStringList();
}

void TestOdamexServer::refreshWebSites()
{
	// Given: full replies whose only changes are website/download-site CVars.
	// Empty strings are omitted, as on current servers. The parser also supports
	// the legacy sv_website CVar, which takes precedence over the download fallback.
	QFETCH(QString, previousWebsite);
	QFETCH(QString, previousDownloads);
	QFETCH(QString, currentWebsite);
	QFETCH(QString, currentDownloads);
	QFETCH(QString, expectedWebsite);
	QFETCH(QStringList, expectedAdditional);
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);

	for (bool downloadsFirst : {false, true})
	{
		const auto makeReply = [downloadsFirst](const QString &website, const QString &downloads)
		{
			QList<QByteArray> cvars;
			if (!website.isEmpty())
				cvars << stringCvar("sv_website", website);
			if (!downloads.isEmpty())
			{
				const QByteArray cvar = stringCvar("sv_downloadsites", downloads);
				if (downloadsFirst)
					cvars.prepend(cvar);
				else
					cvars.append(cvar);
			}
			return replyWithCvars(cvars);
		};
		const QByteArray previousData = makeReply(previousWebsite, previousDownloads);
		const QByteArray currentData = makeReply(currentWebsite, currentDownloads);

		// And: the server already holds the previous website/download-site settings.
		OdamexServer server(QHostAddress("192.0.2.1"), 10666);
		QCOMPARE(server.readRefreshQueryResponse(previousData), Server::RESPONSE_GOOD);
		QCOMPARE(server.webSite(), previousWebsite.isEmpty()
			? previousDownloads.section(' ', 0, 0) : previousWebsite);

		// When: applying the new settings and repeating the same reply.
		for (int refresh = 0; refresh < 2; ++refresh)
		{
			const Server::Response response = server.readRefreshQueryResponse(currentData);

			// Then: only the current URLs remain, with deterministic fallback and no
			// duplicates introduced by refreshing, regardless of CVar order.
			QCOMPARE(response, Server::RESPONSE_GOOD);
			QCOMPARE(server.webSite(), expectedWebsite);
			QCOMPARE(server.additionalWebSites(), expectedAdditional);
			QStringList expectedAll = expectedAdditional;
			expectedAll.prepend(expectedWebsite);
			QCOMPARE(server.allWebSites(), expectedAll);
			QCOMPARE(server.map(), QString("MAP01"));
			QCOMPARE(server.iwad(), QString("DOOM2.WAD"));
		}
	}
}

void TestOdamexServer::parseFractionalTimeLimit_data()
{
	QTest::addColumn<QString>("timeLimit");
	QTest::addColumn<bool>("hasTimeLeft");
	QTest::addColumn<int>("timeLeft");
	QTest::addColumn<int>("expectedTimeLimit");

	// Field presence follows the server's round-to-nearest rule. The display
	// API stores whole minutes, rounded up so a positive limit is not unlimited.
	QTest::newRow("omitted-limit") << QString() << false << 0 << 0;
	QTest::newRow("zero") << QString("0") << false << 0 << 0;
	QTest::newRow("integer") << QString("10") << true << 7 << 10;
	QTest::newRow("decimal-integer") << QString("10.0") << true << 7 << 10;
	QTest::newRow("below-half-minute") << QString("0.1") << false << 0 << 1;
	QTest::newRow("just-below-half-minute") << QString("0.49999997") << false << 0 << 1;
	QTest::newRow("half-minute") << QString("0.5") << true << 0 << 1;
	QTest::newRow("above-half-minute") << QString("0.500001") << true << 0 << 1;
	QTest::newRow("below-one-minute") << QString("0.999") << true << 0 << 1;
	QTest::newRow("above-one-minute") << QString("1.1") << true << 1 << 2;
	QTest::newRow("fractional-minutes") << QString("10.5") << true << 7 << 11;
	QTest::newRow("below-server-maximum") << QString("1439.9") << true << 1439 << 1440;
	QTest::newRow("server-maximum") << QString("1440") << true << 1440 << 1440;
	QTest::newRow("scientific-notation") << QString("1e-05") << false << 0 << 1;

	// Current servers cap limits at 1440 minutes. These synthetic oversized
	// values check that the displayed limit stays at the API's maximum value.
	QTest::newRow("display-maximum") << QString("65535") << true << 65535 << 65535;
	QTest::newRow("above-display-maximum") << QString("65536") << true << 0 << 65535;
	QTest::newRow("large-finite-limit") << QString("3.4e38") << true << 0 << 65535;
}

void TestOdamexServer::parseFractionalTimeLimit()
{
	// Given: a populated reply with an explicitly specified time-left layout.
	// The fixtures do not derive field presence using the parser's conversion.
	QFETCH(QString, timeLimit);
	QFETCH(bool, hasTimeLeft);
	QFETCH(int, timeLeft);
	QFETCH(int, expectedTimeLimit);
	QList<QByteArray> cvars;
	if (!timeLimit.isEmpty())
		cvars << floatCvar("sv_timelimit", timeLimit);
	ReplyOptions options;
	options.hasTimeLeft = hasTimeLeft;
	options.timeLeft = quint16(timeLeft);
	options.populated = true;
	const QByteArray data = replyWithCvars(cvars, options);
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);

	// When: parsing the reply on a fresh server, without any previous refresh state.
	const Server::Response response = server.readRefreshQueryResponse(data);

	// Then: the reply stays aligned through all file and player fields, even
	// when a zero time-left field could otherwise be mistaken for empty lists.
	QCOMPARE(response, Server::RESPONSE_GOOD);
	QCOMPARE(server.map(), QString("MAP01"));
	QCOMPARE(server.iwad(), QString("DOOM2.WAD"));
	QCOMPARE(server.numWads(), 1);
	QCOMPARE(server.wad(0).name(), QString("FRACTIONAL.WAD"));
	const QList<Checksum> checksums = server.wad(0).checksums();
	QCOMPARE(checksums.size(), 1);
	QCOMPARE(checksums.first().algorithm(), QCryptographicHash::Md5);
	QCOMPARE(checksums.first().hash(), QByteArray(16, '\0'));
	QVERIFY(server.dehs().isEmpty());
	QCOMPARE(server.players().size(), 1);
	const Player &player = server.player(0);
	QCOMPARE(player.name(), QString("Timer player"));
	QCOMPARE(player.ping(), 28ul);
	QCOMPARE(player.score(), 0l);
	QCOMPARE(player.teamNum(), Player::TEAM_NONE);
	QVERIFY(!player.isSpectating());
	QCOMPARE(int(server.timeLimit()), expectedTimeLimit);
	QCOMPARE(int(server.timeLeft()), timeLeft);
}

void TestOdamexServer::rejectInvalidTimeLimit_data()
{
	QTest::addColumn<QString>("timeLimit");
	QTest::newRow("empty") << QString();
	QTest::newRow("not-a-number") << QString("invalid");
	QTest::newRow("negative") << QString("-1");
	QTest::newRow("nan") << QString("nan");
	QTest::newRow("infinity") << QString("inf");
	QTest::newRow("float-overflow") << QString("1e1000");
}

void TestOdamexServer::rejectInvalidTimeLimit()
{
	// Given: a reply containing an invalid float, not an omitted zero limit.
	QFETCH(QString, timeLimit);
	const QByteArray data = replyWithCvars({floatCvar("sv_timelimit", timeLimit)});
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);

	// When: converting the received limit to the model's integer representation.
	const Server::Response response = server.readRefreshQueryResponse(data);

	// Then: invalid or non-finite values are rejected, not converted to unlimited.
	QCOMPARE(response, Server::RESPONSE_BAD);
}

QTEST_GUILESS_MAIN(TestOdamexServer)

#include "tst_odamexserver.moc"
