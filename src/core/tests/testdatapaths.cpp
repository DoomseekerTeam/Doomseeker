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

#ifdef Q_OS_UNIX
	// Home of someone else
	const struct passwd *pwent = getpwnam("root");
	if (pwent == nullptr)
	{
		testLog << "Unable to retrieve home dir for root";
		return false;
	}
	const QString rootHome = pwent->pw_dir;
	if (dataPaths.resolveTemplated("~root") != rootHome)
	{
		testLog << QString("Home resolution of root failed: %1 != %2")
			.arg(dataPaths.resolveTemplated("~root"), rootHome);
		return false;
	}
	if (dataPaths.resolveTemplated("~root/") != rootHome + "/")
	{
		testLog << QString("Home resolution of root/ failed: %1 != %2")
			.arg(dataPaths.resolveTemplated("~root/"), rootHome + "/");
		return false;
	}
	if (dataPaths.resolveTemplated("~root/extra/more") != rootHome + "/extra/more")
	{
		testLog << QString("Home resolution of root/extra failed: %1 != %2")
			.arg(dataPaths.resolveTemplated("~root/extra/more"), rootHome + "/extra/more");
		return false;
	}

	// Home of user who doesn't exist
	QString noUser = "~noUser";
	while (getpwnam(noUser.toLatin1().constData()) != nullptr)
	{
		noUser += "1";
	}
	if (dataPaths.resolveTemplated(noUser) != noUser)
	{
		testLog << QString("Home resolution of user who doesn't exist failed: %1 != %2")
			.arg(dataPaths.resolveTemplated(noUser), noUser);
		return false;
	}
	noUser += "/";
	if (dataPaths.resolveTemplated(noUser) != noUser)
	{
		testLog << QString("Home resolution of user who doesn't exist with / failed: %1 != %2")
			.arg(dataPaths.resolveTemplated(noUser), noUser);
		return false;
	}
	noUser += "extra";
	if (dataPaths.resolveTemplated(noUser) != noUser)
	{
		testLog << QString("Home resolution of user who doesn't exist with /extra failed: %1 != %2")
			.arg(dataPaths.resolveTemplated(noUser), noUser);
		return false;
	}
#endif
#ifdef Q_OS_WIN
	const QString noUser = "~guest\\stuff";
	if (dataPaths.resolveTemplated(noUser) != noUser)
	{
		testLog << QString("Home resolution of a named user failed: %1 != %2")
			.arg(dataPaths.resolveTemplated(noUser), noUser);
		return false;
	}
#endif

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

	// Variable which name starts with PROGDIR
	qputenv("PROGDIRECTORY", "some/folder");
	if (dataPaths.resolveTemplated("$PROGDIRECTORY/logs") != "some/folder/logs")
	{
		testLog << QString("$PROGDIRECTORY resolution failure: %1 != %2")
			.arg(dataPaths.resolveTemplated("$PROGDIRECTORY/logs"), "some/folder/logs");
		return false;
	}

	return true;
}
