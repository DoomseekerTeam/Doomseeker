//------------------------------------------------------------------------------
// testini.h
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
#ifndef __TESTINI_H__
#define __TESTINI_H__

#include "tests/testbase.h"
#include <QScopedPointer>
#include <QSettings>
#include <QTemporaryFile>

class SettingsProvider;

class TestIniFixture
{
public:
	TestIniFixture();

	QScopedPointer<SettingsProvider> settings;
private:
	QScopedPointer<QSettings> settingsQt;
	QTemporaryFile settingsFile;
};

class TestReadINI : public TestUnitBase
{
public:
	TestReadINI();

	bool executeTest() override;
};

class TestReadINIVariable : public TestUnitBase
{
public:
	TestReadINIVariable()
		: TestUnitBase("Read INI Variable") {}

	bool executeTest() override;
};

class TestDeleteINIVariable : public TestUnitBase
{
public:
	TestDeleteINIVariable()
		: TestUnitBase("Delete INI Variable") {}

	bool executeTest() override;
};

class TestDeleteINISection : public TestUnitBase
{
public:
	TestDeleteINISection()
		: TestUnitBase("Delete INI Section") {}

	bool executeTest() override;
};

#endif
