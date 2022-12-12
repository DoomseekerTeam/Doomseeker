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
#include "datapaths.h"
#include "strings.hpp"
#include "testdatapaths.h"

#include <QCoreApplication>
#include <QDir>
#include <QProcessEnvironment>

#ifdef Q_OS_UNIX
#include <sys/types.h>
#include <pwd.h>
#endif

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
	} while (bEntryDoesExist == true);

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
