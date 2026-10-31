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

#include "configuration/doomseekerconfig.h"
#include "datapaths.h"
#include "serverapi/playerslist.h"
#include "wadseeker/entities/checksum.h"
#include <QDataStream>
#include <QFile>
#include <QScopedValueRollback>
#include <QStandardPaths>
#include <QtTest>

namespace
{
QByteArray stringCvar(const QByteArray &name, const QString &value)
{
	return name + '\0' + char(6) + value.toUtf8() + '\0';
}

QByteArray byteCvar(const QByteArray &name, quint8 value)
{
	return name + '\0' + char(2) + char(value);
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
};

// Synthetic protocol-7 cooperative reply; defaults to an empty, unlimited server.
// Only the supplied CVars/options change; all other fields stay fixed.
QByteArray replyWithCvars(const QList<QByteArray> &cvars,
	const ReplyOptions &options = ReplyOptions())
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	out.setByteOrder(QDataStream::LittleEndian);
	out << quint32(0xAD032003) // Information response.
		<< quint32(12 * 256 + 30) // Engine version 12.3.0.
		<< quint32(7) // Negotiated protocol.
		<< quint32(0) // Echoed token.
		<< quint32(7) // Actual server protocol.
		<< quint8(0); // Empty revision string.

	const QList<QByteArray> allCvars = QList<QByteArray>{
		byteCvar("sv_maxclients", 4), byteCvar("sv_maxplayers", 4)
	} + cvars;
	out << quint8(allCvars.size());
	for (const QByteArray &cvar : allCvars)
		out.writeRawData(cvar.constData(), cvar.size());

	out << quint8(0); // No password hash.
	out.writeRawData("MAP01", sizeof("MAP01"));
	if (options.hasTimeLeft)
		out << options.timeLeft;
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
	out << quint8(options.populated ? 1 : 0);
	if (options.populated)
	{
		out.writeRawData("Timer player", sizeof("Timer player"));
		out << quint32(0xFF40CF00) // Player color, BGRA on the wire.
			<< quint16(28) // Ping.
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
	QCOMPARE(server.maxClients(), 4);
	QCOMPARE(server.maxPlayers(), 4);
	QCOMPARE(server.scoreLimit(), 5u);
	QCOMPARE(server.gameMode().index(), int(GameMode::SGM_Cooperative));
	QVERIFY(!server.isLocked());
	QCOMPARE(server.map(), QString("MAP01"));
	QCOMPARE(server.timeLimit(), 0);
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
	QCOMPARE(server.maxClients(), 32);
	QCOMPARE(server.maxPlayers(), 16);
	QCOMPARE(server.gameMode().index(), int(GameMode::SGM_TeamDeathmatch));
	QVERIFY(server.gameMode().isTeamGame());
	QVERIFY(!server.isLocked());
	QCOMPARE(server.map(), QString("MAP19"));
	QCOMPARE(server.timeLimit(), 10);
	QCOMPARE(server.timeLeft(), 0);
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
	QCOMPARE(freshServer.timeLimit(), 0);
	QCOMPARE(freshServer.map(), QString("MAP00"));
	QCOMPARE(freshServer.iwad(), QString("DOOM2.WAD"));
	QCOMPARE(freshServer.numWads(), 1);
	QCOMPARE(freshServer.wad(0).name(), QString("DUEL2020OA.WAD"));

	// And: the server under test has already parsed a reply with a 10-minute limit.
	OdamexServer server(QHostAddress("192.0.2.1"), 10666);
	QCOMPARE(server.readRefreshQueryResponse(limitedData), Server::RESPONSE_GOOD);
	QCOMPARE(server.gameMode().index(), int(GameMode::SGM_Deathmatch));
	QCOMPARE(server.timeLimit(), 10);

	// When: disabling the time limit, refreshing again, and restoring the limit.
	for (bool timeLimited : {false, false, true})
	{
		const QByteArray &data = timeLimited ? limitedData : unlimitedData;
		const Server::Response response = server.readRefreshQueryResponse(data);

		// Then: every refresh succeeds and reflects the current reply, not old state.
		QCOMPARE(response, Server::RESPONSE_GOOD);
		QCOMPARE(server.gameMode().index(), int(GameMode::SGM_Deathmatch));
		QCOMPARE(server.timeLimit(), timeLimited ? 10 : 0);
		QCOMPARE(server.timeLeft(), 0);
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
		byteCvar("sv_scorelimit", 5)
	};
	QList<QByteArray> clearedCvars = cvars;
	QVERIFY(clearedCvars.removeOne(omittedCvar));
	const QByteArray originalData = replyWithCvars(cvars);
	const QByteArray clearedData = replyWithCvars(clearedCvars);
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
		QCOMPARE(server.maxClients(), 4);
		QCOMPARE(server.maxPlayers(), 4);
		QCOMPARE(server.gameMode().index(), int(GameMode::SGM_Cooperative));
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
	QCOMPARE(server.timeLimit(), expectedTimeLimit);
	QCOMPARE(server.timeLeft(), timeLeft);
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
