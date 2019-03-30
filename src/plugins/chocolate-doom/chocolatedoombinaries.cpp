//------------------------------------------------------------------------------
// chocolatedoombinaries.cpp
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
// Copyright (C) 2019 Pol Marcet Sardà <polmarcetsarda@gmail.com>
//------------------------------------------------------------------------------

#include "chocolatedoombinaries.h"

#include "chocolatedoomengineplugin.h"
#include "chocolatedoomserver.h"
#include "chocolatedoomgameexefactory.h"

#include <applogger.h>
#include <ini/inisection.h>
#include <ini/inivariable.h>
#include <serverapi/gamefile.h>
#include <serverapi/message.h>
#include <serverapi/gameexefactory.h>
#include <gui/internalclientinstaller.h>
#include "strings.hpp"

#include <QSharedPointer>
#include <QTemporaryDir>

#define RELEASE_BINARY_URL "https://www.chocolate-doom.org/downloads/%1/chocolate-%2-%1-win32.zip"
#define CHOCOLATEDOOM_BINARY_NAME "chocolate-doom.exe"
#define CHOCOLATEHERETIC_BINARY_NAME "chocolate-heretic.exe"
#define CHOCOLATEHEXEN_BINARY_NAME "chocolate-hexen.exe"
#define CHOCOLATESTRIFE_BINARY_NAME "chocolate-strife.exe"

ChocolateDoomClientExeFile::ChocolateDoomClientExeFile(const QSharedPointer<const ChocolateDoomServer> &server) : ExeFile(server)
{
	setProgramName(server->plugin()->data()->name);
	setExeTypeName(tr("client"));
	setConfigKey("BinaryPath");
}

const QString ChocolateDoomClientExeFile::pathToExe(Message& message, const InternalClientsInspection internalCliInspection) const
{
	#ifdef Q_OS_WIN32
	if (!config().value("EnableInternalClient").toBool() || internalCliInspection == NoInternalChecking)
	{
	#endif
		return ExeFile::pathToExe(message, internalCliInspection);
	#ifdef Q_OS_WIN32
	}
	// We'll first check if system's version is compatible
	if (internalCliInspection != InternalCheckingWithoutSystemCheck)
	{
		QString systemPath = tryFindCompatibleSystemEngine(message);
		if (!systemPath.isEmpty())
		{
			return systemPath;
		}
	}

	message = Message();
	QString error;

	QString internalClientPathRoot = config()["InternalClientPath"];
	if (internalClientPathRoot.isEmpty())
	{
		error = tr("No internal clients directory specified for Chocolate Doom");
		message = Message::customError(error);
		return QString();
	}

	QString path = versionInstallPath();

	QFileInfo fi(path);
	if (!fi.exists() || isEmptyDir(path))
	{
		error = tr("%1\ndoesn't exist.\nYou need to install new binaries.").arg(path);
		message = Message(Message::Type::GAME_NOT_FOUND_BUT_CAN_BE_INSTALLED, error);
		return QString();
	}

	if (!fi.isDir())
	{
		error = tr("%1\nexists but is NOT a directory.\nCannot proceed.").arg(path);
		message = Message::customError(error);
		return QString();
	}

	QString binPath = path + '/' + getClientExecutableName();
	fi = QFileInfo(binPath);
	if (!fi.exists() || (fi.isDir() && !fi.isBundle()))
	{
		error = tr("%1\nexists but doesn't contain Chocolate Doom executable.\n\n"
			"Doomseeker can still install the game there if you want.").arg(path);
		message = Message(Message::Type::GAME_NOT_FOUND_BUT_CAN_BE_INSTALLED, error);
		return QString();
	}

	// Everything checked out, so proceed to create (if necessary) and return path to the script file.
	QString retPath;
	if (!spawnInternalBatchFile(path, retPath, message))
	{
		// message is already specified inside spawnInternalBatchFile()
		return QString();
	}
	return retPath;
	#endif
}

const QUrl ChocolateDoomClientExeFile::getUrl(Message& message) const
{
	return QUrl(QString(RELEASE_BINARY_URL).arg(version(),
		engineNameForIwad(server()->iwad())));
}

const QString ChocolateDoomClientExeFile::getClientExecutableName() const
{
	return executableNameForIwad(server()->iwad());
}

const QString ChocolateDoomClientExeFile::versionInstallPath() const
{
	return Strings::combinePaths(config().value("InternalClientPath").toString(),
		engineNameForIwad(server()->iwad()) + "-" + version());
}

const QString ChocolateDoomClientExeFile::version() const
{
	if (!server().isNull())
	{
		QString version = server()->gameVersion();
		// Strip out extraneous data in the version number.
		return version.right(version.size()-1 - version.lastIndexOf(' '));
	}
	return QString();
}

const QString ChocolateDoomClientExeFile::tryFindCompatibleSystemEngine(Message &message) const
{
	return QString();
}

const QString ChocolateDoomClientExeFile::executableNameForIwad(const QString &iwad)
{
	QString exeName(CHOCOLATEDOOM_BINARY_NAME);
	if (iwad == "hexen.wad")
	{
		exeName = CHOCOLATEHEXEN_BINARY_NAME;
	}
	else if (iwad == "heretic.wad" || iwad == "heretic1.wad")
	{
		exeName = CHOCOLATEHERETIC_BINARY_NAME;
	}
	else if (iwad == "strife1.wad" || iwad == "strife0.wad")
	{
		exeName = CHOCOLATESTRIFE_BINARY_NAME;
	}
	return exeName;
}

const QString ChocolateDoomClientExeFile::engineNameForIwad(const QString &iwad)
{
	QString name("doom");
	if (iwad == "hexen.wad")
	{
		name = "hexen";
	}
	else if (iwad == "heretic.wad" || iwad == "heretic1.wad")
	{
		name = "heretic";
	}
	else if (iwad == "strife1.wad" || iwad == "strife0.wad")
	{
		name = "strife";
	}
	return name;
}
