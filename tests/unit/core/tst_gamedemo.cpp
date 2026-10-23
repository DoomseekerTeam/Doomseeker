//------------------------------------------------------------------------------
// tst_gamedemo.cpp
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
// Copyright (C) 2024 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "gamedemo.h"
#include "serverapi/serverstructs.h"

#include <QtTest>
#include <QTimeZone>

class TestGameDemo : public QObject
{
	Q_OBJECT

private slots:
	void names_data();
	void names();
	void imprintPath_data();
	void imprintPath();
};

void TestGameDemo::names_data()
{
	QTest::addColumn<QString>("author");
	QTest::addColumn<QString>("game");
	QTest::addColumn<QDateTime>("time");
	QTest::addColumn<QString>("exported");
	QTest::addColumn<QString>("managed");

	const QDateTime epoch = QDateTime::fromMSecsSinceEpoch(0, QTimeZone(0));
	QTest::newRow("plain") << "Player" << "Game" << epoch
		<< "Player_Game_1970-01-01T000000Z.lmp"
		<< "Player/1970/Game_1970-01-01T000000Z.lmp";
	QTest::newRow("leading-year-zeros") << "Player" << "Game"
		<< QDateTime(QDate(55, 6, 11), QTime(13, 37), QTimeZone(0))
		<< "Player_Game_0055-06-11T133700Z.lmp"
		<< "Player/0055/Game_0055-06-11T133700Z.lmp";
	QTest::newRow("utc-conversion") << "Player" << "Doom"
		<< QDateTime(QDate(2024, 2, 1), QTime(13, 37, 21), QTimeZone(7200))
		<< "Player_Doom_2024-02-01T113721Z.lmp"
		<< "Player/2024/Doom_2024-02-01T113721Z.lmp";
	QTest::newRow("empty-author-and-game") << "" << "" << epoch
		<< "unknownplayer_unknowngame_1970-01-01T000000Z.lmp"
		<< "_/1970/_1970-01-01T000000Z.lmp";
	QTest::newRow("invalid-time") << "" << "" << QDateTime()
		<< "unknownplayer_unknowngame_unknowntime.lmp" << "_/_/_unknown.lmp";

	const QList<QPair<QString, QString>> names = {
		{"/", ""}, {"\\", ""}, {".", ""}, {"/./...", ""}, {" ", ""},
		{"$my:name+is!leet#h.x", "mynameisleethx"},
		{"_CLAN_Valid-1337", "CLANValid-1337"},
		{"Can I haz space", "CanIhazspace"}
	};
	for (int i = 0; i < names.size(); ++i)
	{
		const QString raw = names[i].first;
		const QString clean = names[i].second;
		const QString managedAuthor = clean.isEmpty() ? "_" : clean;
		QTest::newRow(qPrintable(QString("author-%1").arg(i)))
			<< raw << "Game" << epoch
			<< QString("%1_Game_1970-01-01T000000Z.lmp").arg(clean)
			<< QString("%1/1970/Game_1970-01-01T000000Z.lmp").arg(managedAuthor);
		QTest::newRow(qPrintable(QString("game-%1").arg(i)))
			<< "Player" << raw << epoch
			<< QString("Player_%1_1970-01-01T000000Z.lmp").arg(clean)
			<< QString("Player/1970/%1_1970-01-01T000000Z.lmp").arg(clean);
	}

	QString longName = "IconOfDoomseeker";
	while (longName.size() <= int(GameDemo::MAX_AUTHOR_FILENAME_LEN))
		longName += longName;
	const QString shortened = longName.left(GameDemo::MAX_AUTHOR_FILENAME_LEN);
	QTest::newRow("long-author") << longName << "Game" << epoch
		<< QString("%1_Game_1970-01-01T000000Z.lmp").arg(shortened)
		<< QString("%1/1970/Game_1970-01-01T000000Z.lmp").arg(shortened);
}

void TestGameDemo::names()
{
	// Given: demo metadata with names and a recording time.
	QFETCH(QString, author);
	QFETCH(QString, game);
	QFETCH(QDateTime, time);
	QFETCH(QString, exported);
	QFETCH(QString, managed);
	GameDemo demo;
	demo.demopath = "demo.lmp";
	demo.author = author;
	demo.game = game;
	demo.time = time;
	demo.iwad = "freedoom.wad";

	// When: deriving the exported and managed names.
	const QString exportedName = demo.exportedName();
	const QString managedName = demo.managedName();

	// Then: names are sanitized and dates are represented in UTC.
	QCOMPARE(exportedName, exported);
	QCOMPARE(managedName, managed);
}

void TestGameDemo::imprintPath_data()
{
	QTest::addColumn<QString>("path");
	QTest::addColumn<QString>("game");
	QTest::addColumn<QString>("author");
	QTest::addColumn<QDateTime>("time");
	QTest::addColumn<QString>("iwad");
	QTest::addColumn<QStringList>("wads");

	const QDateTime local(QDate(1997, 12, 15), QTime(15, 16, 17));
	const QDateTime utc(QDate(1997, 12, 15), QTime(15, 16, 17), QTimeZone(0));
	QTest::newRow("v1-plain") << "Emag_15.12.1997_15.16.17.lmp"
		<< "Emag" << "" << local << "" << QStringList();
	QTest::newRow("v1-wads")
		<< "Emag_15.12.1997_15.16.17_freedoom.wad_av.wad_zvox.wad.lmp"
		<< "Emag" << "" << local << "freedoom.wad" << QStringList{"av.wad", "zvox.wad"};
	QTest::newRow("v1-escaped-underscore")
		<< "Emag_15.12.1997_15.16.17_free__doom.wad.lmp"
		<< "Emag" << "" << local << "free_doom.wad" << QStringList();
	QTest::newRow("v2-mismatched-directory-year")
		<< "Lepray/2023/Emag_1997-12-15T151617Z.lmp"
		<< "Emag" << "Lepray" << utc << "" << QStringList();
	QTest::newRow("exported") << "Lepray_Emag_1997-12-15T151617Z.lmp"
		<< "Emag" << "Lepray" << utc << "" << QStringList();
	QTest::newRow("no-player-or-game") << "_1997-12-15T151617Z.lmp"
		<< "" << "" << utc << "" << QStringList();
	QTest::newRow("v2-no-player") << "1997/Emag_1997-12-15T151617Z.lmp"
		<< "Emag" << "" << utc << "" << QStringList();
	QTest::newRow("v2-no-player-or-year") << "Emga_1997-12-15T151617Z.lmp"
		<< "Emga" << "" << utc << "" << QStringList();
	QTest::newRow("unsupported") << "watermelon.lmp"
		<< "" << "" << QDateTime() << "" << QStringList();
}

void TestGameDemo::imprintPath()
{
	// Given: a demo path using a supported naming convention or an unknown one.
	QFETCH(QString, path);
	QFETCH(QString, game);
	QFETCH(QString, author);
	QFETCH(QDateTime, time);
	QFETCH(QString, iwad);
	QFETCH(QStringList, wads);
	GameDemo demo;

	// When: extracting metadata from the path.
	demo.imprintPath(path);

	// Then: all available metadata, including WAD order, is recovered.
	QCOMPARE(demo.game, game);
	QCOMPARE(demo.author, author);
	QCOMPARE(demo.time.isValid(), time.isValid());
	if (time.isValid())
		QCOMPARE(demo.time, time);
	QCOMPARE(demo.iwad, iwad);
	QCOMPARE(demo.wads.size(), wads.size());
	for (int i = 0; i < wads.size(); ++i)
	{
		QCOMPARE(demo.wads[i].name(), wads[i]);
		QVERIFY(!demo.wads[i].isOptional());
	}
}

QTEST_GUILESS_MAIN(TestGameDemo)

#include "tst_gamedemo.moc"
