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
	void ctfSidesClassification_data();
	void ctfSidesClassification();
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
	const QVariantMap cvars = {
		{"sv_gametype", QString::number(OdamexGameMode::BASE_DEATHMATCH)},
		{"g_lives", QString::number(lives)},
	};

	// When: interpreting the server settings as a game mode.
	const GameMode mode = OdamexGameMode::fromServerSettings(cvars, maxPlayers);

	// Then: lives take precedence over Duel, which takes precedence over Deathmatch.
	QCOMPARE(mode.index(), expectedModeIndex);
}

void TestOdamexGameMode::ctfSidesClassification_data()
{
	QTest::addColumn<QVariant>("sides");
	QTest::addColumn<bool>("attackDefend");

	QTest::newRow("boolean-true") << QVariant(true) << true;
	QTest::newRow("boolean-false") << QVariant(false) << false;
	QTest::newRow("omitted") << QVariant() << false;
	QTest::newRow("legacy-enabled") << QVariant(QString("1")) << true;
	QTest::newRow("legacy-disabled") << QVariant(QString("0")) << false;
	QTest::newRow("invalid-text") << QVariant(QString("invalid")) << false;
	QTest::newRow("negative-integer") << QVariant(-1) << false;
}

void TestOdamexGameMode::ctfSidesClassification()
{
	// Given: a CTF server whose sides setting is typed, legacy text, or absent.
	QFETCH(QVariant, sides);
	QFETCH(bool, attackDefend);
	QVariantMap cvars{{"sv_gametype", int(OdamexGameMode::BASE_CTF)}};
	if (sides.isValid())
		cvars.insert("g_sides", sides);

	// When: interpreting the server settings as a game mode.
	const GameMode mode = OdamexGameMode::fromServerSettings(cvars, 8);

	// Then: only enabled sides select Attack & Defend rather than ordinary CTF.
	const int expected = attackDefend ? int(OdamexGameMode::DERIVED_ATTACK_DEFEND_CTF)
		: int(GameMode::SGM_CTF);
	QCOMPARE(mode.index(), expected);
}

QTEST_GUILESS_MAIN(TestOdamexGameMode)

#include "tst_odamexgamemode.moc"
