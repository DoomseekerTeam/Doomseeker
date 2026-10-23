//------------------------------------------------------------------------------
// tst_filesearchpath.cpp
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
// Copyright (C) 2023 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "pathfinder/filesearchpath.h"

#include <QtTest>

class TestFileSearchPath : public QObject
{
	Q_OBJECT

private slots:
	void contains_data();
	void contains();
	void merge();
};

void TestFileSearchPath::contains_data()
{
	QTest::addColumn<QString>("candidate");
	QTest::addColumn<bool>("recursive");
	QTest::addColumn<bool>("expected");

	auto addCase = [](const char *name, const QString &path, bool plain, bool recursive)
	{
		QTest::newRow(qPrintable(QString("plain-%1").arg(name))) << path << false << plain;
		QTest::newRow(qPrintable(QString("recursive-%1").arg(name))) << path << true << recursive;
	};
	addCase("exact", "avo/ca", true, true);
	addCase("trailing-separator", "avo/ca/", true, true);
	addCase("repeated-separators", "avo//ca//", true, true);
	addCase("shorter-name", "avo/c", false, false);
	addCase("longer-name", "avo/cado", false, false);
	addCase("subdirectory", "avo/ca/do", false, true);
	addCase("parent", "avo", false, false);
#ifdef Q_OS_WIN
	addCase("native-separators", "avo\\ca", true, true);
	addCase("native-trailing-separator", "avo\\ca\\", true, true);
	addCase("native-subdirectory", "avo\\ca\\do", false, true);
	addCase("native-longer-name", "avo\\cado", false, false);
#endif
}

void TestFileSearchPath::contains()
{
	// Given: a search path with or without recursive searching.
	QFETCH(QString, candidate);
	QFETCH(bool, recursive);
	QFETCH(bool, expected);
	FileSearchPath path("avo/ca");
	path.setRecursive(recursive);

	// When: checking a candidate directory.
	const bool result = path.contains(candidate);

	// Then: only the directory and permitted descendants are contained.
	QCOMPARE(result, expected);
}

void TestFileSearchPath::merge()
{
	// Given: duplicate paths and paths covered by recursive parents.
	auto recursive = [](const QString &name)
	{
		FileSearchPath path(name);
		path.setRecursive(true);
		return path;
	};
	QList<FileSearchPath> paths;
	paths << FileSearchPath("avo//ca") << FileSearchPath("avo/ca/")
		<< FileSearchPath("avo/ca/do") << FileSearchPath("ba/na/na")
		<< recursive("ba/na") << recursive("ap") << recursive("ap/ple")
		<< FileSearchPath("recuA") << recursive("recuA")
		<< recursive("recuB") << FileSearchPath("recuB");

	// When: merging redundant entries.
	FileSearchPath::merge(paths);

	// Then: normalized paths retain their order and recursive coverage.
	QCOMPARE(paths.size(), 6);
	QCOMPARE(paths[0].path(), QString("avo/ca/"));
	QVERIFY(!paths[0].isRecursive());
	QCOMPARE(paths[1].path(), QString("avo/ca/do"));
	QVERIFY(!paths[1].isRecursive());
	QCOMPARE(paths[2].path(), QString("ba/na"));
	QVERIFY(paths[2].isRecursive());
	QCOMPARE(paths[3].path(), QString("ap"));
	QVERIFY(paths[3].isRecursive());
	QCOMPARE(paths[4].path(), QString("recuA"));
	QVERIFY(paths[4].isRecursive());
	QCOMPARE(paths[5].path(), QString("recuB"));
	QVERIFY(paths[5].isRecursive());
}

QTEST_GUILESS_MAIN(TestFileSearchPath)

#include "tst_filesearchpath.moc"
