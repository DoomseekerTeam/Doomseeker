//------------------------------------------------------------------------------
// tst_odamexgamemode.cpp
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
#include "odamexgamemode.h"

#include <QtTest>

class TestOdamexGameMode : public QObject
{
	Q_OBJECT

private slots:
	void deathmatchClassification_data();
	void deathmatchClassification();
};

void TestOdamexGameMode::deathmatchClassification_data()
{
	QTest::addColumn<uint>("lives");
	QTest::addColumn<uint>("maxPlayers");
	QTest::addColumn<int>("expectedModeIndex");

	QTest::newRow("deathmatch") << 0u << 8u << int(GameMode::SGM_Deathmatch);
	QTest::newRow("duel") << 0u << 2u << int(OdamexGameMode::DERIVED_DUEL);
	QTest::newRow("lives-before-duel")
		<< 1u << 2u << int(OdamexGameMode::DERIVED_LAST_MARINE_STANDING);
}

void TestOdamexGameMode::deathmatchClassification()
{
	// Given: deathmatch server settings with a lives limit and player capacity.
	QFETCH(uint, lives);
	QFETCH(uint, maxPlayers);
	QFETCH(int, expectedModeIndex);
	const QMap<QString, QString> cvars = {
		{"sv_gametype", QString::number(OdamexGameMode::BASE_DEATHMATCH)},
		{"g_lives", QString::number(lives)},
	};

	// When: interpreting the server settings as a game mode.
	const GameMode mode = OdamexGameMode::fromServerSettings(cvars, maxPlayers);

	// Then: lives take precedence over Duel, which takes precedence over Deathmatch.
	QCOMPARE(mode.index(), expectedModeIndex);
}

QTEST_GUILESS_MAIN(TestOdamexGameMode)

#include "tst_odamexgamemode.moc"
