//------------------------------------------------------------------------------
// odamexbinaries.cpp
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

#include "odamexbinaries.h"
#include "odamexengineplugin.h"
#include "odamexserver.h"

#include <applogger.h>
#include <ini/inisection.h>
#include <ini/inivariable.h>
#include <serverapi/gamefile.h>
#include <serverapi/message.h>
#include <serverapi/gameexefactory.h>
#include <gui/internalclientinstaller.h>
#include <QSharedPointer>
#include <QTemporaryDir>

#define RELEASE_BINARY_URL_BASE "https://downloads.sourceforge.net/odamex/odamex-"
#define ODAMEX_BINARY_NAME "odamex.exe"

#ifdef Q_OS_WIN32
#define RELEASE_BINARY_URL RELEASE_BINARY_URL_BASE"win32-%1.zip"
#else
#define RELEASE_BINARY_URL ""
#endif

OdamexClientExeFile::OdamexClientExeFile(const QSharedPointer<const OdamexServer> &server) : ExeFile(server)
{
	setProgramName(server->plugin()->data()->name);
	setExeTypeName(tr("client"));
	setConfigKey("BinaryPath");
}

const QString OdamexClientExeFile::pathToExe(Message& message, const InternalClientsInspection internalCliInspection) const
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
			return systemPath;
	}

	message = Message();
	QString error;

	QString internalClientPathRoot = config()["InternalClientPath"];
	if (internalClientPathRoot.isEmpty())
	{
		error = tr("No internal clients directory specified for Odamex");
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

	QString binPath = path + '/' + ODAMEX_BINARY_NAME;
	fi = QFileInfo(binPath);
	if (!fi.exists() || (fi.isDir() && !fi.isBundle()))
	{
		error = tr("%1\nexists but doesn't contain Odamex executable.\n\n"
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

const QUrl OdamexClientExeFile::getUrl(Message& message) const
{
	return QUrl(QString(RELEASE_BINARY_URL).arg(version()));
}

void OdamexClientExeFile::postInstallationSteps(Message& message) const
{
	QDir baseDir(versionInstallPath());
	foreach (QString baseEntry, baseDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
	{
		if (QFileInfo(baseDir.absoluteFilePath(baseEntry)).isDir())
		{
			QDir subDir(baseDir.absoluteFilePath(baseEntry));
			foreach (QString subEntry, subDir.entryList(QDir::Files))
			{
				if (!subDir.rename(subDir.absoluteFilePath(subEntry),
					baseDir.absoluteFilePath(subEntry)))
					message = Message::customError(tr("Could not move file: %1\n into: %2")
						.arg(subEntry, baseDir.absolutePath()));
			}
		}
	}
}

const QString OdamexClientExeFile::getClientExecutableName() const
{
	return QString(ODAMEX_BINARY_NAME);
}

const QString OdamexClientExeFile::tryFindCompatibleSystemEngine(Message &message) const
{
	CommandLineInfo cmdInfo;
	GameFileList files = OdamexEnginePlugin::staticInstance()->gameExe()->gameFiles();
	QString clientPath;
	foreach (GameFile file, files.asQList())
	{
		if (file.executable() & GameFile::Server)
			cmdInfo.executable = config().value(file.configName()).toString();
		else if (file.executable() & GameFile::Client)
			clientPath = config().value(file.configName()).toString();
	}
	if (!cmdInfo.executable.absoluteFilePath().isEmpty() && !clientPath.isEmpty())
	{
		AppLogger app(cmdInfo);
		QTemporaryDir tmpDir;
		QString captured;
		if (tmpDir.isValid())
		{
			OutputAppLogger output = app.runAndLogExecutableUntilTime(1000);
			QRegExp regExp("Odamex v[0-9]*.[0-9]*.[0-9]* \\([a-z0-9]+\\)");
			regExp.indexIn(output.stdoutput.first());
			QString captured = regExp.capturedTexts().first();
		}
		if (!captured.isEmpty())
		{
			int beginVersion = captured.indexOf('v')+1;
			QString capturedVersion = captured.mid(beginVersion,
				captured.indexOf(' ', beginVersion) - beginVersion);
			if (capturedVersion == version())
				return clientPath;
		}
	}
	return QString();
}
