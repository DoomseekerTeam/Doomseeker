//------------------------------------------------------------------------------
// tst_player.cpp
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
// Copyright (C) 2018 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "serverapi/player.h"

#include <QtTest>

class TestPlayer : public QObject
{
	Q_OBJECT

private slots:
	void nameColorTagsStripped_data();
	void nameColorTagsStripped();
};

void TestPlayer::nameColorTagsStripped_data()
{
	QTest::addColumn<QString>("name");
	QTest::addColumn<QString>("expected");

	QTest::newRow("plain") << "Simple Joe '89" << "Simple Joe '89";
	QTest::newRow("predefined-colors")
		<< "[PRO] \034gHunter \034fKiller [PRO]" << "[PRO] Hunter Killer [PRO]";
	QTest::newRow("incomplete-predefined-color") << "Haxxor\034" << "Haxxor";
	QTest::newRow("pwad-colors")
		<< "\034[e]Emmet \034[Brown]McFly\034[aqua]" << "Emmet McFly";
	QTest::newRow("unclosed-pwad-color") << "\034[eEmmet" << "eEmmet";
	QTest::newRow("unclosed-pwad-color-at-end") << "McFly \034[" << "McFly ";
	QTest::newRow("empty-pwad-color") << "Mar\034[]ty" << "Marty";
}

void TestPlayer::nameColorTagsStripped()
{
	// Given: a player name containing plain text or possibly malformed color tags.
	QFETCH(QString, name);
	QFETCH(QString, expected);
	const Player player(name, 0, 0);

	// When: stripping color tags from the name.
	const QString result = player.nameColorTagsStripped();

	// Then: only the tags are removed and the remaining text is preserved.
	QCOMPARE(result, expected);
}

QTEST_GUILESS_MAIN(TestPlayer)

#include "tst_player.moc"
