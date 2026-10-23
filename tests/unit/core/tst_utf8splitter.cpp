//------------------------------------------------------------------------------
// tst_utf8splitter.cpp
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "utf8splitter.h"

#include <QtTest>

class TestUtf8Splitter : public QObject
{
	Q_OBJECT

private slots:
	void splitPreservesCharacters();
};

void TestUtf8Splitter::splitPreservesCharacters()
{
	// Given: 250 two-byte characters and a limit falling inside a character.
	const QString input(250, QChar(0x0105)); // a with 'ogonek'
	const QByteArray encoded = input.toUtf8();
	const int maxChunkLength = 385;

	// When: splitting the UTF-8 data into size-limited chunks.
	const QList<QByteArray> result = Utf8Splitter().split(encoded, maxChunkLength);

	// Then: both chunks end on character boundaries and preserve the input.
	QCOMPARE(result.size(), 2);
	QCOMPARE(result[0].size(), 384);
	QCOMPARE(result[1].size(), 116);
	QCOMPARE(static_cast<int>(static_cast<unsigned char>(result[0][382])), 0xc4);
	QCOMPARE(static_cast<int>(static_cast<unsigned char>(result[0][383])), 0x85);
	QCOMPARE(static_cast<int>(static_cast<unsigned char>(result[1][0])), 0xc4);
	QCOMPARE(static_cast<int>(static_cast<unsigned char>(result[1][1])), 0x85);
	QCOMPARE(result[0] + result[1], encoded);
}

QTEST_GUILESS_MAIN(TestUtf8Splitter)

#include "tst_utf8splitter.moc"
