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

#include "log.h"

#include "../wadseeker/zip/unarchive.h"
#include "application.h"
#include "plugins/engineplugin.h"
#include "serverapi/message.h"
#include "version.h"
#include "gui/internalclientinstaller.h"
#include "zandronumengineplugin.h"
#include "zandronumserver.h"

#include <ini/inisection.h>
#include <ini/inivariable.h>
#include <QBuffer>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QScopedPointer>
#include <strings.hpp>

#define TESTING_BINARY_URL_BASE "https://zandronum.com/downloads/testing/%1/ZandroDev%1-%2"
#define RELEASE_BINARY_URL_BASE "https://zandronum.com/downloads/zandronum%1-"

#ifdef Q_OS_WIN32
#define TESTING_BINARY_URL TESTING_BINARY_URL_BASE "windows.zip"
#define RELEASE_BINARY_URL RELEASE_BINARY_URL_BASE"windows.zip"
#define ZANDRONUM_BINARY_NAME "zandronum.exe"
#define SCRIPT_FILE_EXTENSION ".bat"
#else

#ifndef __x86_64__
#define TESTING_BINARY_URL TESTING_BINARY_URL_BASE "linux-x86.tar.bz2"
#define RELEASE_BINARY_URL RELEASE_BINARY_URL_BASE"linux-x86.tar.bz2"
#else
#define TESTING_BINARY_URL TESTING_BINARY_URL_BASE "linux-x86_64.tar.bz2"
#define RELEASE_BINARY_URL RELEASE_BINARY_URL_BASE"linux-x86_64.tar.bz2"
#endif

#ifndef Q_OS_MAC
#define ZANDRONUM_BINARY_NAME "zandronum"
#else
#define ZANDRONUM_BINARY_NAME "Zandronum.app"
#define ZANDRONUM_APP_BUNDLE_BIN "/Contents/MacOS/zandronum"
#endif
#define SCRIPT_FILE_EXTENSION ".sh"
#endif

class ZandronumClientExeFile::PrivData
{
public:
	QSharedPointer<const ZandronumServer> server;

	bool isEmptyDir(const QString &path) const
	{
		QDir dir(path);
		return dir.exists() && dir.entryList(
			QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
	}

	bool isNonEmptyDir(const QString &path) const
	{
		QDir dir(path);
		return dir.exists() && !dir.entryList(
			QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
	}
};

ZandronumClientExeFile::ZandronumClientExeFile(const QSharedPointer<const ZandronumServer> &server)
{
	d = new PrivData();
	d->server = server;
	setProgramName(server->plugin()->data()->name);
	setExeTypeName(tr("client"));
	setConfigKey("BinaryPath");
}

ZandronumClientExeFile::~ZandronumClientExeFile()
{
	delete d;
}

IniSection &ZandronumClientExeFile::config()
{
	return *ZandronumEnginePlugin::staticInstance()->data()->pConfig;
}

const InstallationPublicData ZandronumClientExeFile::createInstallationData() const
{
	const // clear warnings
	#include "zandronum.xpm"

	InstallationPublicData data;
	data.engineIcon = QPixmap(zandronum_xpm);
	data.engineName = programName();
	data.engineVersion = version();
	data.installationPath = versionInstallPath();
	return data;
}

QString ZandronumClientExeFile::pathToExe(Message &message)
{
	IniSection &config = *ZandronumEnginePlugin::staticInstance()->data()->pConfig;

	if (!config.value("EnableInternalClient").toBool())
	{
		return ExeFile::pathToExe(message);
	}
	else
	{
		message = Message();
		QString error;

		QString internalClientPathRoot = config["InternalClientPath"];
		if (internalClientPathRoot.isEmpty())
		{
			error = tr("No internal clients directory specified for Zandronum");
			message = Message::customError(error);
			return QString();
		}

		QString path = versionInstallPath();

		QFileInfo fi(path);
		if (!fi.exists() || d->isEmptyDir(path))
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
}

QString ZandronumClientExeFile::version() const
{
	QString version = d->server->gameVersion();
	// Strip out extraneous data in the version number.
	return version.left(version.indexOf(' '));
}

QString ZandronumClientExeFile::versionInstallPath() const
{
	IniSection &config = *ZandronumEnginePlugin::staticInstance()->data()->pConfig;
	QString path =  config["InternalClientPath"];
	return Strings::combinePaths(path, version());
}

QString ZandronumClientExeFile::workingDirectory()
{
	IniSection &config = *ZandronumEnginePlugin::staticInstance()->data()->pConfig;

	QFileInfo fi((QString)config["BinaryPath"]);
	return fi.absolutePath();
}

void ZandronumClientExeFile::preInstallationSteps(Message& message)
{
	// copy all .ini's. On Linux .ini's are kept in ~/.zandronum so this will
	// do nothing, but on Windows this should work like magic.
	QDir baseBinaryDir(workingDirectory());
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
	ZandronumVersion version(d->server->gameVersion());
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

	QUrl url = d->server->isTestingServer() ?
		QString(TESTING_BINARY_URL).arg(versionPrefix).arg(hgVersion) :
		QString(RELEASE_BINARY_URL).arg(versionPrefix);

	return url;
}

void ZandronumClientExeFile::postInstallationSteps(Message& message)
{
	// Nothing needed
}

const QStringList ZandronumClientExeFile::getExecutableNames() const
{
	QStringList list;
	list << QString(ZANDRONUM_BINARY_NAME);
	return list;
}

QString ZandronumClientExeFile::scriptFileExtension()
{
	return SCRIPT_FILE_EXTENSION;
}

bool ZandronumClientExeFile::spawnInternalBatchFile(const QString &versionDir, QString &fullPathToFile, Message &message)
{
	QString binaryPath = versionDir + '/' + ZANDRONUM_BINARY_NAME;
	// This will create an actual path to file, because there is no '/' at the end
	// of scriptFilepath.
	fullPathToFile = versionDir + SCRIPT_FILE_EXTENSION;
	QFileInfo fi(fullPathToFile);
	QFile file(fullPathToFile);
	if (fi.isDir())
	{
		QString error = tr("%1\n should be a script file but is a directory!").arg(fullPathToFile);
		message = Message::customError(error);
		return false;
	}

	if (fi.exists())
	{
		if ((file.permissions() & QFile::ExeUser) == 0)
		{
			QString error = tr("You don't have permissions to execute file: %1\n").arg(fullPathToFile);
			message = Message::customError(error);
			return false;
		}
		return true;
	}

	QString content;
	#ifdef Q_OS_WIN32
	// Create Windows batch file
	// Extract drive letter:
	QString driveLetter;
	QString workDir = workingDirectory();
	for (int i = 0; i < workDir.length(); ++i)
	{
		if (workDir[i] == ':')
		{
			driveLetter = workDir.left(i);
		}
	}

	if (!driveLetter.isEmpty())
	{
		content += driveLetter + ":\r\n";
	}

	QString cdDir = workingDirectory().replace('/', '\\');
	QString exePath = binaryPath.replace('/', '\\');

	content += "cd \"" + cdDir + "\"\r\n";
	content += "\"" + exePath + "\" %*"; // %* deals with all the parameters
	#else
	QString cdDir = workingDirectory();

	// Create Unix script file
	content  = "#!/bin/sh\n";
	content += "cd \"" + cdDir + "\" \n";
	content += "export LANG=C\n";
	#ifdef Q_OS_MAC
	content += "\"" + binaryPath + ZANDRONUM_APP_BUNDLE_BIN "\" \"$@\"\n";
	#else
	content += "\"" + binaryPath + "\" \"$@\"\n"; // "$@" deals with all the parameters
	#endif
	#endif

	if (!file.open(QIODevice::WriteOnly))
	{
		QString error = tr("Couldn't open batch file \"%1\" for writing").arg(fullPathToFile);
		message = Message::customError(error);
		return false;
	}

	if (file.write(content.toUtf8()) < 0)
	{
		QString error = tr("Error while writing batch file \"%1\"").arg(fullPathToFile);
		message = Message::customError(error);
		file.close();
		return false;
	}

	file.close();

	if (!file.setPermissions(file.permissions() | QFile::ExeUser))
	{
		QString error = tr("Cannot set permissions for file:\n%1").arg(fullPathToFile);
		message = Message::customError(error);
		return false;
	}
	return true;
}
