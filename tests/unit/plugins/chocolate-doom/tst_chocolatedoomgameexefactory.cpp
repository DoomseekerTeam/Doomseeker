//------------------------------------------------------------------------------
// tst_chocolatedoomgameexefactory.cpp
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
#include "chocolatedoomgameexefactory.h"
#include "serverapi/gamefile.h"

#include <QtTest>

class TestChocolateDoomGameExeFactory : public QObject
{
	Q_OBJECT

private slots:
	void executableForIwad_data();
	void executableForIwad();
};

void TestChocolateDoomGameExeFactory::executableForIwad_data()
{
	QTest::addColumn<QString>("iwad");
	QTest::addColumn<QString>("expectedConfigName");

	QTest::newRow("doom") << "doom2.wad" << "BinaryPath";
	QTest::newRow("hexen") << "hexen.wad" << "ChocolateHexenBinaryPath";
	QTest::newRow("heretic-shareware") << "heretic1.wad" << "ChocolateHereticBinaryPath";
	QTest::newRow("strife-demo") << "strife0.wad" << "ChocolateStrifeBinaryPath";
	QTest::newRow("unknown") << "unknown.wad" << "BinaryPath";
}

void TestChocolateDoomGameExeFactory::executableForIwad()
{
	// Given: an IWAD name and the executable configuration it should select.
	QFETCH(QString, iwad);
	QFETCH(QString, expectedConfigName);

	// When: selecting an executable for the IWAD.
	const GameFile executable = ChocolateDoomGameExeFactory::executableForIwad(iwad);

	// Then: game-specific IWADs select their executable; other names use Doom.
	QCOMPARE(executable.configName(), expectedConfigName);
}

QTEST_GUILESS_MAIN(TestChocolateDoomGameExeFactory)

#include "tst_chocolatedoomgameexefactory.moc"
