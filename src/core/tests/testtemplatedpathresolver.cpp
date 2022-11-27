//------------------------------------------------------------------------------
// testtemplatedpathresolver.cpp
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
#include "testtemplatedpathresolver.h"

#include "templatedpathresolver.h"
#include "strings.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QProcessEnvironment>

#ifdef Q_OS_UNIX
#include <sys/types.h>
#include <pwd.h>
#endif

TestTemplatedPathResolver::TestTemplatedPathResolver()
	: TestUnitBase("Templated Path Resolver")
{
}

bool TestTemplatedPathResolver::executeTest()
{
	TemplatedPathResolver resolver;
	resolver.setEnvVarsEnabled(true);
	resolver.setProgdirEnabled(true);
	resolver.setUserHomeEnabled(true);

	// PROGDIR
	qputenv("PROGDIR", "this env. var should be ignored");
	QString exeLocation = QCoreApplication::applicationDirPath();
	if (resolver.resolve("$PROGDIR") != exeLocation)
	{
		testLog << QString("simple $PROGDIR resolution failure: %1 != %2")
			.arg(resolver.resolve("$PROGDIR"), exeLocation);
		return false;
	}

	// PROGDIR suffixed
	if (resolver.resolve("$PROGDIR/logs") != Strings::combinePaths(exeLocation, "logs"))
	{
		testLog << QString("suffixed $PROGDIR resolution failure: %1 != %2")
			.arg(resolver.resolve("$PROGDIR/logs"), exeLocation + "logs");
		return false;
	}

	// Just home
	if (resolver.resolve("~") != QDir::homePath())
	{
		testLog << QString("Home resolution failed: %1 != %2")
			.arg(resolver.resolve("~"), QDir::homePath());
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
	if (resolver.resolve("~root") != rootHome)
	{
		testLog << QString("Home resolution of root failed: %1 != %2")
			.arg(resolver.resolve("~root"), rootHome);
		return false;
	}
	if (resolver.resolve("~root/") != rootHome + "/")
	{
		testLog << QString("Home resolution of root/ failed: %1 != %2")
			.arg(resolver.resolve("~root/"), rootHome + "/");
		return false;
	}
	if (resolver.resolve("~root/extra/more") != rootHome + "/extra/more")
	{
		testLog << QString("Home resolution of root/extra failed: %1 != %2")
			.arg(resolver.resolve("~root/extra/more"), rootHome + "/extra/more");
		return false;
	}

	// Home of user who doesn't exist
	QString noUser = "~noUser";
	while (getpwnam(noUser.toLatin1().constData()) != nullptr)
	{
		noUser += "1";
	}
	if (resolver.resolve(noUser) != noUser)
	{
		testLog << QString("Home resolution of user who doesn't exist failed: %1 != %2")
			.arg(resolver.resolve(noUser), noUser);
		return false;
	}
	noUser += "/";
	if (resolver.resolve(noUser) != noUser)
	{
		testLog << QString("Home resolution of user who doesn't exist with / failed: %1 != %2")
			.arg(resolver.resolve(noUser), noUser);
		return false;
	}
	noUser += "extra";
	if (resolver.resolve(noUser) != noUser)
	{
		testLog << QString("Home resolution of user who doesn't exist with /extra failed: %1 != %2")
			.arg(resolver.resolve(noUser), noUser);
		return false;
	}
#endif
#ifdef Q_OS_WIN
	const QString noUser = "~guest\\stuff";
	if (resolver.resolve(noUser) != noUser)
	{
		testLog << QString("Home resolution of a named user failed: %1 != %2")
			.arg(resolver.resolve(noUser), noUser);
		return false;
	}
#endif

	// Complex resolution
	qputenv("G00dV4r", "/data/is/mine");
	qputenv("an0ther", "ok");
	{
		QString templated = "~/logs/$an0ther/$G00dV4r/$an0ther/$badvar44/done";
		QString expected = QDir::homePath() + "/logs/ok//data/is/mine/ok//done";
		if (resolver.resolve(templated) != expected)
		{
			testLog << QString("complex env resolution failure: %1 != %2")
				.arg(resolver.resolve(templated), expected);
			return false;
		}
	}

	// Variable which name starts with PROGDIR
	qputenv("PROGDIRECTORY", "some/folder");
	if (resolver.resolve("$PROGDIRECTORY/logs") != "some/folder/logs")
	{
		testLog << QString("$PROGDIRECTORY resolution failure: %1 != %2")
			.arg(resolver.resolve("$PROGDIRECTORY/logs"), "some/folder/logs");
		return false;
	}

	return true;
}
