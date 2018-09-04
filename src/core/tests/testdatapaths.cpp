//------------------------------------------------------------------------------
// testdatapaths.cpp
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
#include "testdatapaths.h"
#include "datapaths.h"
#include "strings.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QProcessEnvironment>

TestDataPathsAppDataDirectoryAccess::TestDataPathsAppDataDirectoryAccess(bool bPortable)
: TestUnitBase("Data Paths - App Data directory access")
{
	if (bPortable)
	{
		_testName += " - portable";
	}

	this->bPortable = bPortable;
}

bool TestDataPathsAppDataDirectoryAccess::executeTest()
{
	DataPaths dataPaths(bPortable);

	QString appDataDir = dataPaths.systemAppDataDirectory();

	testLog << QString("App Data directory: %1").arg(appDataDir);

	return dataPaths.validateAppDataDirectory();
}

////////////////////////////////////////////////////////////////////////////////

TestDataPathsAppDataDirectoryWrite::TestDataPathsAppDataDirectoryWrite(bool bPortable)
: TestUnitBase("Data Paths - App Data directory write")
{
	if (bPortable)
	{
		_testName += " - portable";
	}

	this->bPortable = bPortable;
}

bool TestDataPathsAppDataDirectoryWrite::executeTest()
{
	DataPaths dataPaths(bPortable);

	QString appDataDirPath = dataPaths.systemAppDataDirectory();
	QDir appDataDir(appDataDirPath);

	QString randomDirName;
	bool bEntryDoesExist = false;
	do
	{
		// Continue generating random directory names as long as we don't get
		// a directory name that doesn't exist. With 32 characters this should
		// be quite easy.
		randomDirName = Strings::createRandomAlphaNumericString(32);
		bEntryDoesExist = appDataDir.exists(randomDirName);
	}
	while (bEntryDoesExist == true);

	QString randomDirPath = appDataDirPath + "/" + randomDirName;

	testLog << "Attempting to create directory:";
	testLog << randomDirPath;

	if (!appDataDir.mkdir(randomDirName))
	{
		testLog << "Failed.";
		return false;
	}

	QString testFilePath = randomDirPath + "/" + "tmpfile.tmp";
	QFile testFile(testFilePath);

	testLog << "Attempting to open following file for writing:";
	testLog << testFilePath;

	bool bReturnValue = true;

	if (!testFile.open(QIODevice::WriteOnly))
	{
		testLog << "Failed.";
		bReturnValue = false;
	}
	else
	{
		testFile.close();
		testFile.remove();
	}

	// Clean up!
	appDataDir.rmdir(randomDirName);

	return bReturnValue;
}

////////////////////////////////////////////////////////////////////////////////

TestDataPathsResolveTemplated::TestDataPathsResolveTemplated(bool bPortable)
	: TestUnitBase("Data Paths - Resolve Templated")
{
	if (bPortable)
	{
		_testName += " - portable";
	}

	this->bPortable = bPortable;
}

bool TestDataPathsResolveTemplated::executeTest()
{
	DataPaths dataPaths(bPortable);

	// PROGDIR
	qputenv("PROGDIR", "this env. var should be ignored");
	QString exeLocation = QCoreApplication::applicationDirPath();
	if (dataPaths.resolveTemplated("$PROGDIR") != exeLocation)
	{
		testLog << QString("simple $PROGDIR resolution failure: %1 != %2")
			.arg(dataPaths.resolveTemplated("$PROGDIR"), exeLocation);
		return false;
	}

	// PROGDIR suffixed
	if (dataPaths.resolveTemplated("$PROGDIR/logs") != Strings::combinePaths(exeLocation, "logs"))
	{
		testLog << QString("suffixed $PROGDIR resolution failure: %1 != %2")
			.arg(dataPaths.resolveTemplated("$PROGDIR/logs"), exeLocation + "logs");
		return false;
	}

	// Just home
	if (dataPaths.resolveTemplated("~") != QDir::homePath())
	{
		testLog << QString("Home resolution failed: %1 != %2")
			.arg(dataPaths.resolveTemplated("~"), QDir::homePath());
		return false;
	}

	// Complex resolution
	qputenv("G00dV4r", "/data/is/mine");
	qputenv("an0ther", "ok");
	{
		QString templated = "~/logs/$an0ther/$G00dV4r/$an0ther/$badvar44/done";
		QString expected = QDir::homePath() + "/logs/ok//data/is/mine/ok//done";
		if (dataPaths.resolveTemplated(templated) != expected)
		{
			testLog << QString("complex env resolution failure: %1 != %2")
				.arg(dataPaths.resolveTemplated(templated), expected);
			return false;
		}
	}

	return true;
}
