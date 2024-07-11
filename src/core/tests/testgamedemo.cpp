//------------------------------------------------------------------------------
// testgamedemo.cpp
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
#include "testgamedemo.h"

#include "serverapi/serverstructs.h"
#include "tests/asserts.h"

#include <QDate>
#include <QDateTime>
#include <QTime>
#include <QTimeZone>
#include <QString>

static GameDemo mkdemometa(QString author = "Player", QString game = "Game",
	QDateTime time = QDateTime::fromMSecsSinceEpoch(0, Qt::UTC),
	QString demopath = "demo.lmp",
	QString iwad = "freedoom.wad", QList<PWad> wads = {})
{
	GameDemo demo;
	demo.demopath = demopath;
	demo.author = author;
	demo.game = game;
	demo.time = time;
	demo.iwad = iwad;
	demo.wads = wads;
	return demo;
}

bool TestGameDemoExportedName::executeTest()
{
	auto withAuthor = [](QString name)
	{
		return QString("%1_Game_1970-01-01T000000Z.lmp").arg(name);
	};

	auto withGame = [](QString name)
	{
		return QString("Player_%1_1970-01-01T000000Z.lmp").arg(name);
	};

	// The plainest check.
	// Time is saved in the basic format without the ':' to make the filename more /kosher/.
	T_ASSERT_EQUAL("Player_Game_1970-01-01T000000Z.lmp",
		mkdemometa("Player", "Game").exportedName());

	// Demo recorded during the period of Roman Empire should be prefixed with leading zeros.
	T_ASSERT_EQUAL("Player_Game_0055-06-11T133700Z.lmp",
		mkdemometa("Player", "Game",
			QDateTime(QDate(55, 6, 11), QTime(13, 37), Qt::UTC)).exportedName());

	// To avoid making filenames more complex, all dates should be saved in UTC.
	T_ASSERT_EQUAL("Player_Doom_2024-02-01T113721Z.lmp",
		mkdemometa("Player", "Doom",
			QDateTime(QDate(2024, 2, 1), QTime(13, 37, 21), Qt::OffsetFromUTC, 2 * 60 * 60))
		.exportedName());

	// Empty author and game still produce some kind of path.
	T_ASSERT_EQUAL("unknownplayer_unknowngame_1970-01-01T000000Z.lmp",
		mkdemometa("", "").exportedName());

	// Invalid datetime, empty author and empty game also produce some kind of path.
	T_ASSERT_EQUAL("unknownplayer_unknowngame_unknowntime.lmp",
		mkdemometa("", "", QDateTime()).exportedName());

	// Author name not /kosher/ for filepaths is mangled.
	// Allowed characters: [A-Za-z0-9]_-
	T_ASSERT_EQUAL(withAuthor(""), mkdemometa("/").exportedName());
	T_ASSERT_EQUAL(withAuthor(""), mkdemometa("\\").exportedName());
	T_ASSERT_EQUAL(withAuthor(""), mkdemometa(".").exportedName());
	T_ASSERT_EQUAL(withAuthor(""), mkdemometa("/./...").exportedName());
	T_ASSERT_EQUAL(withAuthor("mynameisleethx"), mkdemometa("$my:name+is!leet#h.x").exportedName());
	T_ASSERT_EQUAL(withAuthor("CLANValid-1337"), mkdemometa("_CLAN_Valid-1337").exportedName());
	T_ASSERT_EQUAL(withAuthor("CanIhazspace"), mkdemometa("Can I haz space").exportedName());

	// Author name longer than some limit of characters is trimmed.
	QString longname = "IconOfDoomseeker";
	while (longname.size() < GameDemo::MAX_AUTHOR_FILENAME_LEN + 1)
		longname += longname;  // this is exponential, but w/e
	QString shortenedName = longname.left(GameDemo::MAX_AUTHOR_FILENAME_LEN);
	T_ASSERT_EQUAL(withAuthor(shortenedName), mkdemometa(longname).exportedName());

	// Games in normal operation don't have non-/kosher/ characters (except maybe space),
	// but let's test them anyway.
	T_ASSERT_EQUAL(withGame(""), mkdemometa("Player", "/").exportedName());
	T_ASSERT_EQUAL(withGame(""), mkdemometa("Player", "\\").exportedName());
	T_ASSERT_EQUAL(withGame(""), mkdemometa("Player", ".").exportedName());
	T_ASSERT_EQUAL(withGame(""), mkdemometa("Player", "/./...").exportedName());
	T_ASSERT_EQUAL(withGame(""), mkdemometa("Player", " ").exportedName());
	T_ASSERT_EQUAL(withGame("mynameisleethx"), mkdemometa("Player", "$my:name+is!leet#h.x").exportedName());
	T_ASSERT_EQUAL(withGame("CLANValid-1337"), mkdemometa("Player", "_CLAN_Valid-1337").exportedName());
	T_ASSERT_EQUAL(withGame("CanIhazspace"), mkdemometa("Player", "Can I haz space").exportedName());

	return true;
}

bool TestGameDemoImprintPath::executeTest()
{
	auto imprint = [](QString path)
	{
		GameDemo demo;
		demo.imprintPath(path);
		return demo;
	};

	{ // V1, plain
		GameDemo demo = imprint("Emag_15.12.1997_15.16.17.lmp");
		T_ASSERT_EQUAL("Emag", demo.game);
		T_ASSERT_DATETIME_EQUAL(QDateTime(QDate(1997, 12, 15), QTime(15, 16, 17)), demo.time);
		T_ASSERT_EQUAL("", demo.author);
		T_ASSERT_EQUAL("", demo.iwad);
		T_ASSERT_ISEMPTY(demo.wads);
	}

	{ // V1, with wads
		GameDemo demo = imprint("Emag_15.12.1997_15.16.17_freedoom.wad_av.wad_zvox.wad.lmp");
		T_ASSERT_EQUAL("Emag", demo.game);
		T_ASSERT_DATETIME_EQUAL(QDateTime(QDate(1997, 12, 15), QTime(15, 16, 17)), demo.time);
		T_ASSERT_EQUAL("", demo.author);
		T_ASSERT_EQUAL("freedoom.wad", demo.iwad);
		T_ASSERT_SIZE(2, demo.wads);
		T_ASSERT_EQUAL("av.wad", demo.wads[0].name());
		T_ASSERT_FALSE(demo.wads[0].isOptional());
		T_ASSERT_EQUAL("zvox.wad", demo.wads[1].name());
		T_ASSERT_FALSE(demo.wads[1].isOptional());
	}

	{ // V1, wad with underscore
		GameDemo demo = imprint("Emag_15.12.1997_15.16.17_free__doom.wad.lmp");
		T_ASSERT_EQUAL("free_doom.wad", demo.iwad);
	}

	{ // V2 (the year part is deliberately mismatching)
		GameDemo demo = imprint("Lepray/2023/Emag_1997-12-15T151617Z.lmp");
		T_ASSERT_EQUAL("Emag", demo.game);
		T_ASSERT_DATETIME_EQUAL(QDateTime(QDate(1997, 12, 15), QTime(15, 16, 17), Qt::UTC), demo.time);

		T_ASSERT_EQUAL("Lepray", demo.author);
		T_ASSERT_EQUAL("", demo.iwad);
		T_ASSERT_ISEMPTY(demo.wads);
	}

	{ // Exported
		GameDemo demo = imprint("Lepray_Emag_1997-12-15T151617Z.lmp");
		T_ASSERT_EQUAL("Emag", demo.game);
		T_ASSERT_DATETIME_EQUAL(QDateTime(QDate(1997, 12, 15), QTime(15, 16, 17), Qt::UTC), demo.time);

		T_ASSERT_EQUAL("Lepray", demo.author);
		T_ASSERT_EQUAL("", demo.iwad);
		T_ASSERT_ISEMPTY(demo.wads);
	}

	// Now try some mismatching names.

	{ // no player or game info
		GameDemo demo = imprint("_1997-12-15T151617Z.lmp");
		T_ASSERT_EQUAL("", demo.game);
		T_ASSERT_DATETIME_EQUAL(QDateTime(QDate(1997, 12, 15), QTime(15, 16, 17), Qt::UTC), demo.time);

		T_ASSERT_EQUAL("", demo.author);
		T_ASSERT_EQUAL("", demo.iwad);
		T_ASSERT_ISEMPTY(demo.wads);
	}

	{ // It's V2, but no player name
		GameDemo demo = imprint("1997/Emag_1997-12-15T151617Z.lmp");
		T_ASSERT_EQUAL("Emag", demo.game);
		T_ASSERT_DATETIME_EQUAL(QDateTime(QDate(1997, 12, 15), QTime(15, 16, 17), Qt::UTC), demo.time);

		T_ASSERT_EQUAL("", demo.author);
		T_ASSERT_EQUAL("", demo.iwad);
		T_ASSERT_ISEMPTY(demo.wads);
	}

	{ // It's V2, but no player name and no year
		GameDemo demo = imprint("Emga_1997-12-15T151617Z.lmp");
		T_ASSERT_EQUAL("Emga", demo.game);
		T_ASSERT_DATETIME_EQUAL(QDateTime(QDate(1997, 12, 15), QTime(15, 16, 17), Qt::UTC), demo.time);

		T_ASSERT_EQUAL("", demo.author);
		T_ASSERT_EQUAL("", demo.iwad);
		T_ASSERT_ISEMPTY(demo.wads);
	}


	{ // Not a supported format
		GameDemo demo = imprint("watermelon.lmp");
		T_ASSERT_EQUAL("", demo.game);
		T_ASSERT_FALSE(demo.time.isValid());
		T_ASSERT_EQUAL("", demo.author);
		T_ASSERT_EQUAL("", demo.iwad);
		T_ASSERT_ISEMPTY(demo.wads);
	}

	return true;
}

bool TestGameDemoManagedName::executeTest()
{
	auto withAuthor = [](QString name)
	{
		return QString("%1/1970/Game_1970-01-01T000000Z.lmp").arg(name);
	};

	auto withGame = [](QString name)
	{
		return QString("Player/1970/%1_1970-01-01T000000Z.lmp").arg(name);
	};

	// The plainest check.
	// Time is saved in the basic format without the ':' to make the filename more /kosher/.
	T_ASSERT_EQUAL("Player/1970/Game_1970-01-01T000000Z.lmp",
		mkdemometa("Player", "Game").managedName());

	// Demo recorded during the period of Roman Empire should be prefixed with leading zeros.
	T_ASSERT_EQUAL("Player/0055/Game_0055-06-11T133700Z.lmp",
		mkdemometa("Player", "Game",
			QDateTime(QDate(55, 6, 11), QTime(13, 37), Qt::UTC)).managedName());

	// To avoid making filenames more complex, all dates should be saved in UTC.
	T_ASSERT_EQUAL("Player/2024/Doom_2024-02-01T113721Z.lmp",
		mkdemometa("Player", "Doom",
			QDateTime(QDate(2024, 2, 1), QTime(13, 37, 21), Qt::OffsetFromUTC, 2 * 60 * 60))
		.managedName());

	// Empty author and game still produce some kind of path.
	T_ASSERT_EQUAL("_/1970/_1970-01-01T000000Z.lmp",
		mkdemometa("", "").managedName());

	// Invalid datetime, empty author and empty game also produce some kind of path.
	T_ASSERT_EQUAL("_/_/_unknown.lmp",
		mkdemometa("", "", QDateTime()).managedName());

	// Author name not /kosher/ for filepaths is mangled.
	// Allowed characters: [A-Za-z0-9]_-
	T_ASSERT_EQUAL(withAuthor("_"), mkdemometa("/").managedName());
	T_ASSERT_EQUAL(withAuthor("_"), mkdemometa("\\").managedName());
	T_ASSERT_EQUAL(withAuthor("_"), mkdemometa(".").managedName());
	T_ASSERT_EQUAL(withAuthor("_"), mkdemometa("/./...").managedName());
	T_ASSERT_EQUAL(withAuthor("_"), mkdemometa(" ").managedName());
	T_ASSERT_EQUAL(withAuthor("mynameisleethx"), mkdemometa("$my:name+is!leet#h.x").managedName());
	T_ASSERT_EQUAL(withAuthor("CLANValid-1337"), mkdemometa("_CLAN_Valid-1337").managedName());
	T_ASSERT_EQUAL(withAuthor("CanIhazspace"), mkdemometa("Can I haz space").managedName());

	// Author name longer than some limit of characters is trimmed.
	QString longname = "IconOfDoomseeker";
	while (longname.size() < GameDemo::MAX_AUTHOR_FILENAME_LEN + 1)
		longname += longname;  // this is exponential, but w/e
	QString shortenedName = longname.left(GameDemo::MAX_AUTHOR_FILENAME_LEN);
	T_ASSERT_EQUAL(withAuthor(shortenedName), mkdemometa(longname).managedName());

	// Games in normal operation don't have non-/kosher/ characters (except maybe space),
	// but let's test them anyway.
	T_ASSERT_EQUAL(withGame(""), mkdemometa("Player", "/").managedName());
	T_ASSERT_EQUAL(withGame(""), mkdemometa("Player", "\\").managedName());
	T_ASSERT_EQUAL(withGame(""), mkdemometa("Player", ".").managedName());
	T_ASSERT_EQUAL(withGame(""), mkdemometa("Player", "/./...").managedName());
	T_ASSERT_EQUAL(withGame(""), mkdemometa("Player", " ").managedName());
	T_ASSERT_EQUAL(withGame("mynameisleethx"), mkdemometa("Player", "$my:name+is!leet#h.x").managedName());
	T_ASSERT_EQUAL(withGame("CLANValid-1337"), mkdemometa("Player", "_CLAN_Valid-1337").managedName());

	return true;
}
