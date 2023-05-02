//------------------------------------------------------------------------------
// datapaths.cpp
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

#include "application.h"
#include "doomseekerfilepaths.h"
#include "fileutils.h"
#include "log.h"
#include "ntfsperm.h"
#include "plugins/engineplugin.h"
#include "strings.hpp"

#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QSet>
#include <QStandardPaths>

// Sanity check for INSTALL_PREFIX and INSTALL_LIBDIR
#if !defined(INSTALL_PREFIX) || !defined(INSTALL_LIBDIR)
#error Build system should provide definition for INSTALL_PREFIX and INSTALL_LIBDIR
#endif

static QList<DirErrno> uniqueErrnosByDir(const QList<DirErrno> &errnos)
{
	QSet<QString> uniqueDirs;
	QList<DirErrno> uniqueErrnos;
	for (const DirErrno &dirErrno : errnos)
	{
		if (!uniqueDirs.contains(dirErrno.directory.path()))
		{
			uniqueDirs.insert(dirErrno.directory.path());
			uniqueErrnos << dirErrno;
		}
	}
	return uniqueErrnos;
}

static QStringList uniquePaths(const QStringList &paths)
{
	QList<QFileInfo> uniqueMarkers;
	QStringList result;
	for (const QString &path : paths)
	{
		QFileInfo fileInfo(path);
		if (!uniqueMarkers.contains(fileInfo))
		{
			uniqueMarkers << fileInfo;
			result << path;
		}
	}
	return result;
}

DClass<DataPaths>
{
public:
	static const QString PLUGINS_DIR_NAME;

	QDir cacheDirectory;
	QDir configDirectory;
	QDir dataDirectory;

	bool bIsPortableModeOn;

	QDir portableRoot() const
	{
		return QDir::current();
	}

	QString legacyAppDataBaseDir() const
	{
		if (bIsPortableModeOn)
		{
			return QCoreApplication::applicationDirPath();
		}

		// For non-portable model this continues here:
		QString dir;

#ifdef Q_OS_WIN32
		// Let's open new block to prevent variable "bleeding".
		{
			QString envVar = QProcessEnvironment::systemEnvironment().value("APPDATA");
			if (validateDir(envVar))
				dir = envVar;
		}
#endif

		if (dir.isEmpty())
		{
			dir = QDir::homePath();
			if (!validateDir(dir))
				return QString();
		}
		return dir;
	}

	bool validateDir(const QString &path) const
	{
		QFileInfo fileInfo(path);
		return !path.isEmpty() && fileInfo.exists() && fileInfo.isDir();
	}
};

DPointered(DataPaths)

DataPaths *DataPaths::staticDefaultInstance = nullptr;

static const QString CACHE_DIR_NAME = ".cache";
static const QString CONFIG_DIR_NAME = ".doomseeker";
static const QString DATA_DIR_NAME = ".static";
static const QString DEMOS_DIR_NAME = "demos";

const QString DataPaths::CHATLOGS_DIR_NAME = "chatlogs";
const QString PrivData<DataPaths>::PLUGINS_DIR_NAME = "plugins";
const QString DataPaths::TRANSLATIONS_DIR_NAME = "translations";
const QString DataPaths::UPDATE_PACKAGES_DIR_NAME = "updates";
const QString DataPaths::UPDATE_PACKAGE_FILENAME_PREFIX = "doomseeker-update-pkg-";

DataPaths::DataPaths(bool bPortableModeOn)
{
	d->bIsPortableModeOn = bPortableModeOn;

	if (bPortableModeOn)
	{
		setBaseDir(QDir("."));
	}
	else
	{
		d->cacheDirectory.setPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
		#if QT_VERSION >= 0x050500
		// QStandardPaths::AppConfigLocation was added in Qt 5.5.
		d->configDirectory.setPath(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation));
		#else
		// In older 5.x versions we need to construct the config path ourselves.
		d->configDirectory.setPath(Strings::combinePaths(
			QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation),
			Application::NAME));
		#endif
		d->dataDirectory.setPath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
	}
}

DataPaths::~DataPaths()
{
}

QString DataPaths::cacheLocationPath() const
{
	return d->cacheDirectory.path();
}

QStringList DataPaths::canWrite() const
{
	QStringList failedList;

	QString dataDirectory = programsDataDirectoryPath();
	if (!d->validateDir(dataDirectory))
		failedList.append(dataDirectory);

	return failedList;
}

QList<DirErrno> DataPaths::createDirectories()
{
	QList<DirErrno> failedDirs;

	DirErrno cacheDirError = FileUtils::mkpath(d->cacheDirectory);
	if (cacheDirError.isError())
		failedDirs << cacheDirError;

	DirErrno configDirError = FileUtils::mkpath(d->configDirectory);
	if (configDirError.isError())
		failedDirs << configDirError;

	DirErrno dataDirError = FileUtils::mkpath(d->dataDirectory);
	if (dataDirError.isError())
		failedDirs << dataDirError;

	DirErrno demosDirError = FileUtils::mkpath(d->dataDirectory.filePath(DEMOS_DIR_NAME));
	if (demosDirError.isError())
		failedDirs << demosDirError;

	return uniqueErrnosByDir(failedDirs);
}

DataPaths *DataPaths::defaultInstance()
{
	return staticDefaultInstance;
}


QStringList DataPaths::defaultWadPaths() const
{
	QStringList filePaths;

	if (d->bIsPortableModeOn)
	{
		filePaths << localDataLocationPath();
		filePaths << programsDataDirectoryPath();
		filePaths << ".";
		for (int i = 0; i < filePaths.size(); ++i)
			filePaths[i] = portablizePath(filePaths[i]);
	}
	else
	{
		filePaths << localDataLocationPath();
		filePaths << programsDataDirectoryPath();
		// The directory which contains the Doomseeker executable may be a good
		// default choice, but on unix systems the bin directory is not worth
		// searching.  Similarly for Mac application bundle.
		const QDir appDir(QCoreApplication::applicationDirPath());
		const QString progBinDirName = appDir.dirName();
		if (progBinDirName != "bin" && progBinDirName != "MacOS")
			filePaths << appDir.path();
	}

	return filePaths;
}

QString DataPaths::demosDirectoryPath() const
{
	return d->dataDirectory.filePath(DEMOS_DIR_NAME);
}

QString DataPaths::documentsLocationPath(const QString &subpath) const
{
	QString rootPath;
	if (!isPortableModeOn())
	{
		rootPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first();
		rootPath = Strings::combinePaths(rootPath, QCoreApplication::applicationName());
	}
	else
	{
		rootPath = QCoreApplication::applicationDirPath();
		rootPath = Strings::combinePaths(rootPath, "storage");
	}
	return QDir(rootPath).filePath(subpath);
}

QString DataPaths::env(const QString &key)
{
	return QProcessEnvironment::systemEnvironment().value(key);
}

void DataPaths::initDefault(bool bPortableModeOn)
{
	assert(staticDefaultInstance == nullptr && "DataPaths can have only one default.");
	if (staticDefaultInstance == nullptr)
		staticDefaultInstance = new DataPaths(bPortableModeOn);
}

bool DataPaths::isPortableModeOn() const
{
	return d->bIsPortableModeOn;
}

QString DataPaths::localDataLocationPath(const QString &subpath) const
{
	return Strings::combinePaths(d->dataDirectory.path(), subpath);
}

QString DataPaths::pluginLocalDataLocationPath(const EnginePlugin &plugin) const
{
	return localDataLocationPath(QString("%1/%2").arg(
		PrivData<DataPaths>::PLUGINS_DIR_NAME, plugin.nameCanonical()));
}

QString DataPaths::pluginDocumentsLocationPath(const EnginePlugin &plugin) const
{
	return documentsLocationPath(QString("%1/%2").arg(
		PrivData<DataPaths>::PLUGINS_DIR_NAME, plugin.nameCanonical()));
}

QStringList DataPaths::pluginSearchLocationPaths() const
{
	QStringList paths;
	paths.append(QCoreApplication::applicationDirPath());
	paths.append("./");

	#if !defined(Q_OS_DARWIN) && !defined(Q_OS_WIN32)
	// On systems where we install to a fixed location, if we see that we are
	// running an installed binary, then we should only load plugins from the
	// expected location.  Otherwise use it only as a last resort.
	const QString installDir = INSTALL_PREFIX "/" INSTALL_LIBDIR "/doomseeker/";
	if (QCoreApplication::applicationDirPath() == INSTALL_PREFIX "/bin")
		paths = QStringList(installDir);
	else
		paths.append(installDir);
	#endif

	paths = uniquePaths(paths);
	return Strings::combineManyPaths(paths, "engines/");
}

QString DataPaths::portablizePath(const QString &path) const
{
	if (isPortableModeOn())
	{
		QFileInfo pathInfo(path);
		if (pathInfo.isAbsolute())
		{
			return d->portableRoot().relativeFilePath(path);
		}
	}
	return path;
}

QString DataPaths::programFilesDirectory(MachineType machineType)
{
	#ifdef Q_OS_WIN32
	QString envVarName = "";

	switch (machineType)
	{
	case x86:
		envVarName = "ProgramFiles(x86)";
		break;

	case x64:
		envVarName = "ProgramW6432";
		break;

	case Preferred:
		envVarName = "ProgramFiles";
		break;

	default:
		return QString();
	}

	QString path = env(envVarName);
	if (path.isEmpty() && machineType != Preferred)
	{
		// Empty outcome may happen on 32-bit systems where variables
		// like "ProgramFiles(x86)" may not exist.
		//
		// If "ProgramFiles" variable is empty then something is seriously
		// wrong with the system.
		path = programFilesDirectory(Preferred);
	}

	return path;

	#else
	Q_UNUSED(machineType);
	return QString();
	#endif
}

QString DataPaths::programsDataDirectoryPath() const
{
	return d->configDirectory.path();
}

void DataPaths::setBaseDir(const QDir &baseDir)
{
	d->cacheDirectory.setPath(baseDir.filePath(CACHE_DIR_NAME));
	d->configDirectory.setPath(baseDir.filePath(CONFIG_DIR_NAME));
	d->dataDirectory.setPath(baseDir.filePath(DATA_DIR_NAME));
}

void DataPaths::setWorkingDirectory(const QString &workingDirectory)
{
	// no-op; this method is deprecated
}

QStringList DataPaths::staticDataSearchDirs(const QString &subdir)
{
	QStringList paths;
	paths.append(QDir::currentPath()); // current working dir
	paths.append(QCoreApplication::applicationDirPath()); // where exe is located
	#ifndef Q_OS_WIN32
	paths.append(INSTALL_PREFIX "/share/doomseeker"); // standard arch independent linux path
	#endif
	paths = uniquePaths(paths);
	QString subdirFiltered = subdir.trimmed();
	if (!subdirFiltered.isEmpty())
	{
		for (int i = 0; i < paths.size(); ++i)
			paths[i] = Strings::combinePaths(paths[i], subdirFiltered);
	}
	return paths;
}

QString DataPaths::systemAppDataDirectory(QString append) const
{
	return QDir(d->legacyAppDataBaseDir()).filePath(append);
}

bool DataPaths::validateAppDataDirectory()
{
	return true;
}

bool DataPaths::validateDir(const QString &path)
{
	QFileInfo fileInfo(path);

	bool bCondition1 = !path.isEmpty();
	bool bCondition2 = fileInfo.exists();
	bool bCondition3 = fileInfo.isDir();

	return bCondition1 && bCondition2 && bCondition3;
}

bool DataPaths::checkDirPermissions(const QString &path)
{
	// BLOCK - keep this to absolute mininum
	++qt_ntfs_permission_lookup;
	QFileInfo dir(path);
	bool permissions = dir.isReadable()
		&& dir.isWritable()
		&& dir.isExecutable();
	--qt_ntfs_permission_lookup;
	// END OF BLOCK
	return permissions;
}

const QString &DataPaths::workingDirectory() const
{
	// Need to use a static variable because the method returns a reference.
	static QString dirPath = QCoreApplication::applicationDirPath();
	return dirPath;
}
