//------------------------------------------------------------------------------
// tst_checksum.cpp
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
#include "entities/checksum.h"

#include <QtTest>

class TestChecksum : public QObject
{
	Q_OBJECT

private slots:
	void constructorStoresHashAndAlgorithm();
};

void TestChecksum::constructorStoresHashAndAlgorithm()
{
	// Given: fixed hash bytes and their algorithm.
	const QByteArray hash = QByteArray::fromHex("900150983cd24fb0d6963f7d28e17f72");
	const QCryptographicHash::Algorithm algorithm = QCryptographicHash::Md5;

	// When: constructing a checksum.
	const Checksum checksum(hash, algorithm);

	// Then: both values are preserved unchanged.
	QCOMPARE(checksum.hash(), hash);
	QCOMPARE(checksum.algorithm(), algorithm);
}

QTEST_GUILESS_MAIN(TestChecksum)

#include "tst_checksum.moc"
