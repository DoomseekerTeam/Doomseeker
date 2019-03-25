//------------------------------------------------------------------------------
// exefile.cpp
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
// Copyright (C) 2013 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "exefile.h"

#include "configuration/doomseekerconfig.h"
#include "gui/internalclientinstaller.h"
#include "ini/inisection.h"
#include "ini/inivariable.h"
#include "message.h"
#include "plugins/engineplugin.h"
#include "serverapi/server.h"
#include "strings.hpp"
#include <QFileInfo>
#include <QString>

#ifdef Q_OS_WIN32
#define SCRIPT_FILE_EXTENSION ".bat"
#else
#define SCRIPT_FILE_EXTENSION ".sh"
#endif

DClass<ExeFile>
{
public:
	QString configKey;
	QString exeTypeName;
	QString programName;
		QSharedPointer<const Server> server;
};

DPointeredNoCopy(ExeFile)

ExeFile::ExeFile(const QSharedPointer<const Server> &server)
{
	d->server = server;
}

ExeFile::~ExeFile()
{
}

const QString &ExeFile::configKey() const
{
	return d->configKey;
}

const QString &ExeFile::exeTypeName() const
{
	return d->exeTypeName;
}

const QString ExeFile::pathToExe(Message &message, const InternalClientsInspection internalCliInspection) const
{
	IniVariable setting = config().retrieveSetting(configKey());

	message = Message();
	QString path = setting.value().toString();
	QString error = "";
	if (path.trimmed().isEmpty())
	{
		error = tr("No %1 executable specified for %2").arg(exeTypeName()).arg(programName());
		message = Message::customError(error);
		return QString();
	}

	QFileInfo fi(path);

	if (!fi.exists() || (fi.isDir() && !fi.isBundle()))
	{
		// Remember: if the file doesn't exist
		// canonicalFilePath() will return an empty string!
		error = tr("Executable for %1 %2:\n%3\nis a directory or doesn't exist.")
			.arg(programName()).arg(exeTypeName())
			.arg(fi.absoluteFilePath());
		message = Message::customError(error);
		return QString();
	}

	return fi.absoluteFilePath();
}

const QString &ExeFile::programName() const
{
	return d->programName;
}

void ExeFile::setConfigKey(const QString &keyName)
{
	d->configKey = keyName;
}

void ExeFile::setExeTypeName(const QString &name)
{
	d->exeTypeName = name;
}

void ExeFile::setProgramName(const QString &name)
{
	d->programName = name;
}

QString ExeFile::workingDirectory(Message &message)
{
	QFileInfo fi(pathToExe(message));
	return fi.absolutePath();
}

const InstallationPublicData ExeFile::createInstallationData() const
{
	return InstallationPublicData();
}

void ExeFile::preInstallationSteps(Message& message) const
{
}

const QUrl ExeFile::getUrl(Message& message) const
{
	return QUrl();
}

void ExeFile::postInstallationSteps(Message& message) const
{
}

const QStringList ExeFile::getExecutableNames() const
{
	return QStringList();
}

const QSharedPointer<const Server> &ExeFile::server() const
{
	return d->server;
}

const IniSection ExeFile::config() const
{
	return gConfig.iniSectionForPlugin(programName());
}


const QString ExeFile::version() const
{
	if (!d->server.isNull())
	{
		QString version = d->server->gameVersion();
		// Strip out extraneous data in the version number.
		return version.left(version.indexOf(' '));
	}
	return QString();
}

const QString ExeFile::versionInstallPath() const
{
	QString path = config().value("InternalClientPath").toString();
	return Strings::combinePaths(path, version());
}

const QString ExeFile::configBinaryPath() const
{
	return QFileInfo((QString) config()["BinaryPath"]).absolutePath();
}

bool ExeFile::spawnInternalBatchFile(const QString& versionDir, QString& fullPathToFile, Message& message) const
{
	QString binaryPath = versionDir + '/' + getExecutableNames().first();
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
	QString workDir = configBinaryPath();
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

	QString cdDir = QString(configBinaryPath()).replace('/', '\\');
	QString exePath = binaryPath.replace('/', '\\');

	content += "cd \"" + cdDir + "\"\r\n";
	content += "\"" + exePath + "\" %*"; // %* deals with all the parameters
	#else
	QString cdDir = configBinaryPath();

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

const bool ExeFile::isEmptyDir(const QString &path)
{
	QDir dir(path);
	return dir.exists() && dir.entryList(
		QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
}

const bool ExeFile::isNonEmptyDir(const QString &path)
{
	QDir dir(path);
	return dir.exists() && !dir.entryList(
		QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty();
}

const QString ExeFile::scriptFileExtension()
{
	return SCRIPT_FILE_EXTENSION;
}

///////////////////////////////////////////////////////////////////////////

DClass<ExeFilePath>
{
public:
	QString path;
	QString workingDir;
};

DPointered(ExeFilePath);

ExeFilePath::ExeFilePath()
{
}

ExeFilePath::ExeFilePath(const QString &path)
{
	d->path = path;
	d->workingDir = QFileInfo(path).path();
}

ExeFilePath::ExeFilePath(const ExeFilePath &other)
{
	this->d = other.d;
}

ExeFilePath::~ExeFilePath()
{
}

ExeFilePath &ExeFilePath::operator=(const ExeFilePath &other)
{
	if (this != &other)
		this->d = other.d;
	return *this;
}

QString ExeFilePath::path() const
{
	return d->path;
}

ExeFilePath &ExeFilePath::setPath(const QString &path)
{
	d->path = path;
	return *this;
}

QString ExeFilePath::workingDir() const
{
	return d->workingDir;
}

ExeFilePath &ExeFilePath::setWorkingDir(const QString &path)
{
	d->workingDir = path;
	return *this;
}
