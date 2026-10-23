//------------------------------------------------------------------------------
// tst_modreader.cpp
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
// Copyright (C) 2019 Pol Marcet Sardà <polmarcetsarda@gmail.com>
//------------------------------------------------------------------------------
#include "modreader.h"

#include <QtTest>
#include <QResource>

class TestModReader : public QObject
{
	Q_OBJECT

private slots:
	void mapsFromSupportedFormats();
};

void TestModReader::mapsFromSupportedFormats()
{
	// Given: an archive containing PK3, Doom, Hexen, and UDMF maps.
	const QResource archive(":/tests/modreader/7zedWadsTest.7z");
	QVERIFY(archive.isValid());
	const auto reader = ModReader::create(archive.absoluteFilePath());
	QVERIFY(!reader.isNull());

	// When: loading the archive and enumerating maps.
	QVERIFY(reader->load());
	const QStringList maps = reader->getAllMaps();

	// Then: maps from every supported format, including nested WADs, are found.
	const QStringList expected = {"pk3map", "BASEPK3", "HEXENMAP", "DOOMMAP", "UDMFMAP"};
	for (const QString &map : expected)
		QVERIFY2(maps.contains(map), qPrintable(QString("Missing map: %1").arg(map)));
}

QTEST_GUILESS_MAIN(TestModReader)

#include "tst_modreader.moc"
