//------------------------------------------------------------------------------
// tst_strings.cpp
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
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "strings.hpp"
#include "plugins/pluginloader.h"

#include <QtTest>

class TestStrings : public QObject
{
	Q_OBJECT

private slots:
	void initTestCase();
	void cleanupTestCase();
	void wrapUrlsWithHtmlATags();
};

void TestStrings::initTestCase()
{
	// URL wrapping also recognizes plugin schemes. Use an empty registry so
	// this unit test never searches for or loads installed plugins.
	PluginLoader::init({}, {});
}

void TestStrings::cleanupTestCase()
{
	PluginLoader::deinit();
}

void TestStrings::wrapUrlsWithHtmlATags()
{
	const QString input = "http://www.example.com www.example.com "
		"ftp://ftp.example.com www.\nwww http:// www.link.with.tag.com< "
		"http://x http://www.example.com?var1=1&var2=2#anchor www.no-spacebar.com";
	const QString expected =
		"<a href=\"http://www.example.com\">http://www.example.com</a> "
		"<a href=\"http://www.example.com\">www.example.com</a> "
		"<a href=\"ftp://ftp.example.com\">ftp://ftp.example.com</a> "
		"www.\nwww http:// "
		"<a href=\"http://www.link.with.tag.com\">www.link.with.tag.com</a>< "
		"<a href=\"http://x\">http://x</a> "
		"<a href=\"http://www.example.com?var1=1&var2=2#anchor\">"
		"http://www.example.com?var1=1&var2=2#anchor</a> "
		"<a href=\"http://www.no-spacebar.com\">www.no-spacebar.com</a>";

	const QString result = Strings::wrapUrlsWithHtmlATags(input);
	QCOMPARE(result.count("<a href="), 7);
	QCOMPARE(result, expected);
}

QTEST_GUILESS_MAIN(TestStrings)

#include "tst_strings.moc"
