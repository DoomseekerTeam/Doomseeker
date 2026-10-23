//------------------------------------------------------------------------------
// tst_ini.cpp
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
#include "ini/ini.h"
#include "ini/settingsproviderqt.h"

#include <QtTest>
#include <QScopedPointer>
#include <QSettings>
#include <QTemporaryFile>

class TestIni : public QObject
{
	Q_OBJECT

private slots:
	void init();
	void cleanup();
	void readSection();
	void readVariable();
	void deleteVariable();
	void deleteSection();

private:
	QScopedPointer<QTemporaryFile> file;
	QScopedPointer<QSettings> settings;
	QScopedPointer<SettingsProviderQt> provider;
	QScopedPointer<Ini> ini;
};

void TestIni::init()
{
	const QByteArray contents = \
		"\n\n" \
		"[Section.SectionOne] \n" \
		"Key1 = 10 \n" \
		"Key2 = Value \n" \
		"Key3 = \"A long text value\" \n" \
		;
	file.reset(new QTemporaryFile);
	QVERIFY2(file->open(), qPrintable(file->errorString()));
	QCOMPARE(file->write(contents), qint64(contents.size()));
	QVERIFY(file->flush());
	file->close();
	settings.reset(new QSettings(file->fileName(), QSettings::IniFormat));
	provider.reset(new SettingsProviderQt(settings.data()));
	ini.reset(new Ini(provider.data()));
}

void TestIni::cleanup()
{
	ini.reset();
	provider.reset();
	settings.reset();
	file.reset();
}

void TestIni::readSection()
{
	// Given: a fresh INI file whose section name contains uppercase letters.

	// When: looking up the section with a lowercase name.
	const IniSection section = ini->section("section.sectionone");

	// Then: the lookup ignores case.
	QVERIFY(!section.isNull());
}

void TestIni::readVariable()
{
	// Given: a fresh INI file containing Key1 = 10.

	// When: looking up the variable with lowercase section and key names.
	IniVariable variable = ini->retrieveSetting("section.sectionone", "key1");

	// Then: both the key and its integer value are retrieved.
	QVERIFY(!variable.isNull());
	QCOMPARE(variable.key().compare("Key1", Qt::CaseInsensitive), 0);
	QCOMPARE(int(variable), 10);
}

void TestIni::deleteVariable()
{
	// Given: an existing variable accessed through its section.
	QVERIFY(ini->hasSetting("section.sectionone", "key1"));
	IniSection section = ini->section("section.sectionone");

	// When: deleting the variable through the section accessor.
	section.deleteSetting("key1");

	// Then: the INI no longer contains the variable.
	QVERIFY(!ini->hasSetting("section.sectionone", "key1"));
}

void TestIni::deleteSection()
{
	// Given: a section containing several variables.
	QVERIFY(ini->hasSetting("section.sectionone", "key1"));
	QVERIFY(ini->hasSetting("section.sectionone", "key3"));

	// When: deleting the section.
	ini->deleteSection("section.sectionone");

	// Then: the section's variables are removed.
	QVERIFY(!ini->hasSetting("section.sectionone", "key1"));
	QVERIFY(!ini->hasSetting("section.sectionone", "key3"));
}

QTEST_GUILESS_MAIN(TestIni)

#include "tst_ini.moc"
