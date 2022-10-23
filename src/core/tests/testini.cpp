//------------------------------------------------------------------------------
// testini.cpp
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
#include "testini.h"
#include <QByteArray>
#include <QTemporaryFile>


const QByteArray EXAMPLE_INI_FILE = \
	"\
\n\
\n\
[Section.SectionOne] \n\
Key1 = 10 \n\
Key2 = Value \n\
Key3 = \"A long text value\" \n\
";

TestIniFixture::TestIniFixture()
{
	settingsFile.setAutoRemove(true);
	settingsFile.open();
	settingsFile.write(EXAMPLE_INI_FILE);
	settingsFile.flush();
	settingsQt.reset(new QSettings(settingsFile.fileName(), QSettings::IniFormat));
	settings.reset(new SettingsProviderQt(settingsQt.data()));
}

////////////////////////////////////////////////////////////////////////////////

TestReadINI::TestReadINI()
	: TestUnitBase("Read INI")
{
}

bool TestReadINI::executeTest()
{
	TestIniFixture fixture;
	Ini ini(fixture.settings.data());

	// This should disregard characters case.
	IniSection section = ini.section("section.sectionone");
	if (section.isNull())
	{
		testLog << "Section.SectionOne was not read correctly from the INI file.";
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool TestReadINIVariable::executeTest()
{
	TestIniFixture fixture;
	Ini ini(fixture.settings.data());

	IniVariable variable = ini.retrieveSetting("section.sectionone", "key1");

	if (variable.isNull())
	{
		gLog << "Failed to obtain key.";
		return false;
	}

	int varValue = variable;

	if (variable.key().compare("Key1", Qt::CaseInsensitive) != 0)
	{
		gLog << QString("Key name incorrect, expected 'Key1', got '%1'").arg(variable.key());
		return false;
	}

	if (varValue != 10)
	{
		gLog << QString("Value incorrect, expected '10', got '%1'").arg(varValue);
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool TestDeleteINIVariable::executeTest()
{
	TestIniFixture fixture;
	Ini ini(fixture.settings.data());

	if (!ini.hasSetting("section.sectionone", "key1"))
	{
		gLog << "Variable doesn't exist already!";
		return false;
	}

	// Another way of removing a variable is to delete it directly from the
	// Ini file through Ini::deleteSetting(). Here we remove it from the
	// section. This works the same and is provided for convenience.
	IniSection section = ini.section("section.sectionone");
	section.deleteSetting("key1");

	if (ini.hasSetting("section.sectionone", "key1"))
	{
		gLog << "Failed to delete the variable.";
		return false;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////

bool TestDeleteINISection::executeTest()
{
	TestIniFixture fixture;
	Ini ini(fixture.settings.data());

	if (!ini.hasSetting("section.sectionone", "key1"))
	{
		gLog << "Section doesn't exist already!";
		return false;
	}

	ini.deleteSection("section.sectionone");

	if (ini.hasSetting("section.sectionone", "key1") ||
		ini.hasSetting("section.sectionone", "key3"))
	{
		gLog << "Failed to delete the section.";
		return false;
	}

	return true;
}
