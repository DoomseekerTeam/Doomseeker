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
#include <QFile>
#include <QScopedValueRollback>
#include <QStandardPaths>
#include <QtTest>

class TestOdamexServer : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();
	void parseOnePlayerReply();
	void parseTeamDeathmatchReply();
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
	QCOMPARE(data.size(), 1712);
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

QTEST_GUILESS_MAIN(TestOdamexServer)

#include "tst_odamexserver.moc"
