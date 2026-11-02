//------------------------------------------------------------------------------
// tst_gameinfotip.cpp
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
#include "configuration/doomseekerconfig.h"
#include "datapaths.h"
#include "serverapi/player.h"
#include "serverapi/server.h"
#include "serverapi/tooltips/gameinfotip.h"
#include <QMap>
#include <QRegularExpression>
#include <QScopedValueRollback>
#include <QStandardPaths>
#include <QTextDocumentFragment>
#include <QtTest>
#include <limits>

namespace
{
using TeamScores = QMap<QString, int>;

class TooltipServer : public Server
{
public:
	TooltipServer() : Server(QHostAddress("192.0.2.1"), 10666) {}

	EnginePlugin *plugin() const override { return nullptr; }
	using Server::scoresMutable;
};
}

class TestGameInfoTip : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();
	void teamScores_data();
	void teamScores();
};

void TestGameInfoTip::initTestCase()
{
	QStandardPaths::setTestModeEnabled(true);
	DataPaths::initDefault(false);
}

void TestGameInfoTip::teamScores_data()
{
	QTest::addColumn<QList<int>>("scores");
	QTest::addColumn<QList<int>>("populatedTeams");
	QTest::addColumn<bool>("teamGame");
	QTest::addColumn<TeamScores>("expectedScores");

	QTest::newRow("negative-and-positive")
		<< QList<int>{-1, 7, 0, 0} << QList<int>{0, 1} << true
		<< TeamScores{{"Blue", -1}, {"Red", 7}};
	QTest::newRow("all-teams-negative")
		<< QList<int>{-1, -2, -3, -4} << QList<int>{0, 1, 2, 3} << true
		<< TeamScores{{"Blue", -1}, {"Red", -2}, {"Green", -3}, {"Gold", -4}};
	QTest::newRow("signed-score-bounds")
		<< QList<int>{std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), 0, 0}
		<< QList<int>{0, 1} << true
		<< TeamScores{{"Blue", std::numeric_limits<int>::min()},
			{"Red", std::numeric_limits<int>::max()}};
	QTest::newRow("zero-and-positive")
		<< QList<int>{0, 12, 0, 0} << QList<int>{0, 1} << true
		<< TeamScores{{"Blue", 0}, {"Red", 12}};
	QTest::newRow("unpopulated-team-hidden")
		<< QList<int>{5, -1, 0, 0} << QList<int>{0} << true
		<< TeamScores{{"Blue", 5}};
	QTest::newRow("non-team-game")
		<< QList<int>{-1, 7, 0, 0} << QList<int>{0, 1} << false << TeamScores{};
}

void TestGameInfoTip::teamScores()
{
	// Given: the real server model holding signed scores and players on selected teams.
	QFETCH(QList<int>, scores);
	QFETCH(QList<int>, populatedTeams);
	QFETCH(bool, teamGame);
	QFETCH(TeamScores, expectedScores);
	const QScopedValueRollback<bool> disableLookup(gConfig.doomseeker.bLookupHosts, false);
	const auto server = QSharedPointer<TooltipServer>::create();
	server->setGameMode(teamGame ? GameMode::mkTeamDeathmatch() : GameMode::mkCooperative());
	server->setMaxClients(4);
	server->setMaxPlayers(4);
	server->scoresMutable() = scores;
	for (int team : populatedTeams)
		server->addPlayer(Player(QString("Player %1").arg(team), 0, 0,
			static_cast<Player::PlayerTeam>(team)));
	GameInfoTip tip(server);

	// When: generating the public HTML.
	const QString html = tip.generateHTML();
	const QString text = QTextDocumentFragment::fromHtml(html).toPlainText().simplified();

	// Then: each displayed team has the correct signed score.
	for (int team = 0; team < MAX_TEAMS; ++team)
	{
		const QString name = server->teamName(team);
		const QString namePattern = "\\b" + QRegularExpression::escape(name) + "\\b";
		if (expectedScores.contains(name))
		{
			const QRegularExpression scorePattern(namePattern + "\\s*:?\\s*([+-]?\\d+)\\b");
			const auto match = scorePattern.match(text);
			QVERIFY2(match.hasMatch(), qPrintable(text));
			bool ok;
			const qint64 score = match.captured(1).toLongLong(&ok);
			QVERIFY2(ok, qPrintable(text));
			QCOMPARE(score, static_cast<qint64>(expectedScores.value(name)));
		}
		else
			QVERIFY2(!text.contains(QRegularExpression(namePattern)), qPrintable(text));
	}

	// And sanity-check: generating the tooltip does not alter the stored scores.
	QCOMPARE(server->scores(), scores);
}

QTEST_GUILESS_MAIN(TestGameInfoTip)

#include "tst_gameinfotip.moc"
