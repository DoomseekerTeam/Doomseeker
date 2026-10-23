//------------------------------------------------------------------------------
// tst_filefind.cpp
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
#include "filefind.h"

#include <QStringList>
#include <QtTest>

class TestFileFind : public QObject
{
	Q_OBJECT

private slots:
	void rootFilePreferredCaseInsensitive();
};

void TestFileFind::rootFilePreferredCaseInsensitive()
{
	// Given: a nested match appearing before a root-level match in the listing.
	const QStringList files = {"subdir/Example.WAD", "other.wad", "example.wad"};
	FileFind finder(files);

	// When: searching by filename with different letter case.
	const int index = finder.findFilename("EXAMPLE.WAD");

	// Then: the root-level match wins despite appearing later in the listing.
	QCOMPARE(index, 2);
}

QTEST_GUILESS_MAIN(TestFileFind)

#include "tst_filefind.moc"
