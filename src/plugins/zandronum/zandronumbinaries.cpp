//------------------------------------------------------------------------------
// zandronumbinaries.cpp
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "zandronumbinaries.h"


#include "application.h"
#include "serverapi/message.h"
#include "zandronumengineplugin.h"
#include "zandronumserver.h"

#include <ini/inisection.h>
#include <ini/inivariable.h>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QScopedPointer>

#include <applogger.h>
#include <gui/internalclientinstaller.h>
#include <plugins/engineplugin.h>
#include <serverapi/message.h>
#include <strings.hpp>

#define TESTING_BINARY_URL_BASE "https://zandronum.com/downloads/testing/%1/ZandroDev%1-%2"
#define RELEASE_BINARY_URL_BASE "https://zandronum.com/downloads/zandronum%1-"

#ifdef Q_OS_WIN32
#define TESTING_BINARY_URL TESTING_BINARY_URL_BASE "windows.zip"
#define RELEASE_BINARY_URL RELEASE_BINARY_URL_BASE"win32-base.zip"
#define ZANDRONUM_BINARY_NAME "zandronum.exe"
#else

#ifndef __x86_64__
#define TESTING_BINARY_URL TESTING_BINARY_URL_BASE "linux-x86.tar.bz2"
#define RELEASE_BINARY_URL RELEASE_BINARY_URL_BASE"linux-x86.tar.bz2"
#else
#define TESTING_BINARY_URL TESTING_BINARY_URL_BASE "linux-x86_64.tar.bz2"
#define RELEASE_BINARY_URL RELEASE_BINARY_URL_BASE"linux-x86_64.tar.bz2"
#endif

#ifndef Q_OS_DARWIN
#define ZANDRONUM_BINARY_NAME "zandronum"
#else
#define ZANDRONUM_BINARY_NAME "Zandronum.app"
#define ZANDRONUM_APP_BUNDLE_BIN "/Contents/MacOS/zandronum"
#endif
#endif

ZandronumClientExeFile::ZandronumClientExeFile(const QSharedPointer<const ZandronumServer> &server) : ExeFile(server)
{
	setProgramName(server->plugin()->data()->name);
	setExeTypeName(tr("client"));
	setConfigKey("BinaryPath");
}

const InstallationPublicData ZandronumClientExeFile::createInstallationData() const
{
	InstallationPublicData data;
	data.engineIcon = server()->icon();
	data.engineName = programName();
	data.engineVersion = version();
	data.installationPath = versionInstallPath();
	return data;
}

const QString ZandronumClientExeFile::pathToExe(Message& message, const InternalClientsInspection internalCliInspection) const
{
	#ifndef Q_OS_DARWIN

	if (!config().value("EnableInternalClient").toBool() || internalCliInspection == NoInternalChecking)
	{
	#endif
		return ExeFile::pathToExe(message, internalCliInspection);
	#ifndef Q_OS_DARWIN
	}
	else
	{
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
			error = tr("No internal clients directory specified for Zandronum");
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

		QString binPath = path + '/' + ZANDRONUM_BINARY_NAME;
		fi = QFileInfo(binPath);
		if (!fi.exists() || (fi.isDir() && !fi.isBundle()))
		{
			error = tr("%1\nexists but doesn't contain Zandronum executable.\n\n"
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
	}
	#endif
}

void ZandronumClientExeFile::preInstallationSteps(Message& message) const
{
	// copy all .ini's. On Linux .ini's are kept in ~/.zandronum so this will
	// do nothing, but on Windows this should work like magic.
	QDir baseBinaryDir(configBinaryPath());
	QStringList nameFilters;
	nameFilters << "*.ini";
	QStringList iniFiles = baseBinaryDir.entryList(nameFilters, QDir::Files);
	foreach(QString str, iniFiles)
	{
		QString sourcePath = baseBinaryDir.absolutePath() + '/' + str;
		QString targetPath = versionInstallPath() + '/' + str;
		QFile file(sourcePath);
		file.copy(targetPath);
	}
}

const QUrl ZandronumClientExeFile::getUrl(Message& message) const
{
	// Find the hg revision string
	ZandronumVersion version(server()->gameVersion());
	QString hgVersion;
	if (version.hgVersionDate() < 4000) // Arbitrary number here to determine between old and new format
		hgVersion = QString("%1").arg(version.hgVersionDate());
	else
		hgVersion = QString("%1-%2").arg(version.hgVersionDate()).arg(version.hgVersionTime(), 4, 10, QChar('0'));

	// Get URL
	QString versionPrefix;
	if (version.revisionVersion() != 0)
	{
		versionPrefix = QString("%1.%2.%3").arg(version.majorVersion())
			.arg(version.minorVersion()).arg(version.revisionVersion());
	}
	else
	{
		versionPrefix = QString("%1.%2").arg(version.majorVersion())
			.arg(version.minorVersion());
	}

	QUrl url = server()->isTestingServer() ?
		QString(TESTING_BINARY_URL).arg(versionPrefix).arg(hgVersion) :
		QString(RELEASE_BINARY_URL).arg(versionPrefix);

	return url;
}

const QStringList ZandronumClientExeFile::getExecutableNames() const
{
	return QStringList(QString(ZANDRONUM_BINARY_NAME));
}

const QString ZandronumClientExeFile::tryFindCompatibleSystemEngine(Message &message) const
{
	CommandLineInfo cmdInfo;
	cmdInfo.executable = QFileInfo(ExeFile::pathToExe(message));
	cmdInfo.args << "--version";
	if (!cmdInfo.executable.absoluteFilePath().isEmpty())
	{
		AppLogger app(cmdInfo);
		OutputAppLogger output = app.runAndLogExecutableUntilTime(1000);

		foreach (const QString &entry, output.stdoutput)
		{
			if(entry.startsWith(version()))
				return cmdInfo.executable.absoluteFilePath();
		}
	}
	return QString();
}
