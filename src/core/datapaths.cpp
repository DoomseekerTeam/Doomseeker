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
#include "log.h"
#include "plugins/engineplugin.h"
#include "strings.hpp"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <cassert>
#include <cstdlib>

#if QT_VERSION >= 0x050000
#include <QStandardPaths>
#endif

// Sanity check for INSTALL_PREFIX and INSTALL_LIBDIR
#if !defined(INSTALL_PREFIX) || !defined(INSTALL_LIBDIR)
#error Build system should provide definition for INSTALL_PREFIX and INSTALL_LIBDIR
#endif

// On NTFS file systems, ownership and permissions checking is disabled by
// default for performance reasons. The following int toggles it by
// incrementation and decrementation of its value.
// See: http://doc.qt.io/qt-5/qfileinfo.html#ntfs-permissions
#ifdef Q_OS_WIN32
extern Q_CORE_EXPORT int qt_ntfs_permission_lookup;
#else
// We'll need to declare an int with the same name to compile successfully in other platforms.
int qt_ntfs_permission_lookup;
#endif

static QStringList uniquePaths(const QStringList &paths)
{
	QList<QFileInfo> uniqueMarkers;
	QStringList result;
	foreach (const QString &path, paths)
	{
		if (!uniqueMarkers.contains(path))
		{
			uniqueMarkers << path;
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

		QString workingDirectory;

		bool bIsPortableModeOn;
};

DPointered(DataPaths)

DataPaths *DataPaths::staticDefaultInstance = NULL;

static const QString LEGACY_APPDATA_DIR_NAME = ".doomseeker";
static const QString DEMOS_DIR_NAME = "demos";

const QString DataPaths::CHATLOGS_DIR_NAME = "chatlogs";
const QString PrivData<DataPaths>::PLUGINS_DIR_NAME = "plugins";
const QString DataPaths::TRANSLATIONS_DIR_NAME = "translations";
const QString DataPaths::UPDATE_PACKAGES_DIR_NAME = "updates";
const QString DataPaths::UPDATE_PACKAGE_FILENAME_PREFIX = "doomseeker-update-pkg-";

DataPaths::DataPaths(bool bPortableModeOn)
{
	d->bIsPortableModeOn = bPortableModeOn;

	// Logically this would be "./" but our only use of this class as of
	// Doomseeker 1.1 would use setWorkingDirectory to applicationDirPath()
	d->workingDirectory = QCoreApplication::applicationDirPath();

	if (bPortableModeOn)
	{
		d->cacheDirectory = systemAppDataDirectory(".cache");
		d->configDirectory = systemAppDataDirectory(LEGACY_APPDATA_DIR_NAME);
		d->dataDirectory = systemAppDataDirectory(".static");
	}
	else
	{
#if QT_VERSION >= 0x050000
		d->cacheDirectory = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
		#if QT_VERSION >= 0x050500
		// QStandardPaths::AppConfigLocation was added in Qt 5.5.
		d->configDirectory = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
		#else
		// In older 5.x versions we need to construct the config path ourselves.
		d->configDirectory = Strings::combinePaths(
			QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation),
			Application::NAME);
		#endif
		d->dataDirectory = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
#else
		d->cacheDirectory = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
		d->dataDirectory = QDesktopServices::storageLocation(QDesktopServices::DataLocation);

#ifdef Q_OS_MAC
		d->configDirectory = systemAppDataDirectory("Library/Preferences/Doomseeker");
#else
		d->configDirectory = systemAppDataDirectory(LEGACY_APPDATA_DIR_NAME);
#endif

#endif
	}

	gLog << QString("Cache directory: %1").arg(d->cacheDirectory.absolutePath());
	gLog << QString("Config directory: %1").arg(d->configDirectory.absolutePath());
	gLog << QString("Data directory: %1").arg(d->dataDirectory.absolutePath());
}

DataPaths::~DataPaths()
{
}

QString DataPaths::cacheLocationPath() const
{
	return d->cacheDirectory.absolutePath();
}

QStringList DataPaths::canWrite() const
{
	QStringList failedList;

	QString dataDirectory = programsDataDirectoryPath();
	if (!validateDir(dataDirectory))
	{
		failedList.append(dataDirectory);
	}

	return failedList;
}

bool DataPaths::createDirectories()
{
	// This variable should only be changed to false and only if something
	// fails.
	bool bAllSuccessful = true;
	const QDir appDataDir(systemAppDataDirectory());

	// No need to bother with migrating plugin master caches
	if (!tryCreateDirectory(d->cacheDirectory, "."))
	{
		bAllSuccessful = false;
	}

	// The existential question here is needed for migration purposes,
	// but on Windows the >=1.2 configDirectory can already exist because
	// Doomseeker <1.2 already stored IRC chat logs there.
	// It is necessary to ask about the .ini file.
	if (!d->configDirectory.exists(DoomseekerFilePaths::INI_FILENAME))
	{
		if (!tryCreateDirectory(d->configDirectory, "."))
		{
			bAllSuccessful = false;
		}
#if !defined(Q_OS_MAC)
		else if (appDataDir.exists(".doomseeker"))
		{
			// Migrate config from old versions of Doomseeker (pre 1.2)
			const QDir oldConfigDir(appDataDir.absolutePath() + QDir::separator() + ".doomseeker");
			gLog << QString("Migrating configuration data from '%1'\n\tto '%2'.")
				.arg(oldConfigDir.absolutePath())
				.arg(d->configDirectory.absolutePath());

			foreach (QFileInfo fileinfo, oldConfigDir.entryInfoList(QStringList("*.ini"), QDir::Files))
				QFile(fileinfo.absoluteFilePath()).copy(d->configDirectory.absoluteFilePath(fileinfo.fileName()));
		}
#endif
	}

	// In >=1.2 and on Windows platform, dataDirectory can be the same
	// as configDirectory. To do the migration properly, we need to
	// ask about existence of a subdirectory inside the data directory.
	if (!d->dataDirectory.exists(DEMOS_DIR_NAME))
	{
#ifdef Q_OS_MAC
		const QString legacyPrefDirectory = "Library/Preferences/Doomseeker";
#else
		const QString legacyPrefDirectory = ".doomseeker";
#endif
		if (!tryCreateDirectory(d->dataDirectory, "."))
		{
			bAllSuccessful = false;
		}
		else if (appDataDir.exists(legacyPrefDirectory))
		{
			// Migrate data from old versions of Doomseeker (specifically demos) (pre 1.2)
			const QDir oldConfigDir(appDataDir.absolutePath() + QDir::separator() + legacyPrefDirectory);
			gLog << QString("Migrating user data from '%1'\n\tto '%2'.")
				.arg(oldConfigDir.absolutePath())
				.arg(d->dataDirectory.absolutePath());

			foreach (QFileInfo fileinfo, oldConfigDir.entryInfoList(QDir::Dirs))
			{
				const QString origPath = fileinfo.absoluteFilePath();
				QFile file(origPath);
				if (file.rename(d->dataDirectory.absoluteFilePath(fileinfo.fileName())))
				{
					// On Windows this will create an useless .lnk shortcut
					// without the .lnk extension, so don't bother.
#if !defined(Q_OS_WIN32)
					file.link(origPath);
#endif
				}
			}
		}
	}

	if (!tryCreateDirectory(d->dataDirectory, DEMOS_DIR_NAME))
	{
		bAllSuccessful = false;
	}

	return bAllSuccessful;
}

DataPaths *DataPaths::defaultInstance()
{
	return staticDefaultInstance;
}


QStringList DataPaths::defaultWadPaths() const
{
	QStringList filePaths;
	filePaths << programsDataDirectoryPath();

	// The directory which contains the Doomseeker executable may be a good
	// default choice, but on unix systems the bin directory is not worth
	// searching.  Similarly for Mac application bundle.
	const QString progBinDirName = QDir(workingDirectory()).dirName();
	if(progBinDirName != "bin" && progBinDirName != "MacOS")
		filePaths << workingDirectory();

	return filePaths;
}

QString DataPaths::demosDirectoryPath() const
{
	return d->dataDirectory.absoluteFilePath(DEMOS_DIR_NAME);
}

QStringList DataPaths::directoriesExist() const
{
	QStringList failedList;
	QStringList checkedList;
	QList<QDir> checkList;

	checkList << d->cacheDirectory << d->configDirectory << d->dataDirectory;

	foreach(const QDir &dataDirectory, checkList)
	{
		if (checkedList.contains(dataDirectory.absolutePath()))
		{
			continue;
		}
		checkedList << dataDirectory.absolutePath();
		if (!dataDirectory.exists())
		{
			failedList.append(dataDirectory.absolutePath());
		}
	}

	return failedList;
}

QStringList DataPaths::directoriesWithoutPermissions() const
{
	QStringList failedList;
	QStringList checkedList;
	QList<QDir> checkList;

	checkList << d->cacheDirectory << d->configDirectory << d->dataDirectory;

	foreach(const QDir &dataDirectory, checkList)
	{
		if (checkedList.contains(dataDirectory.absolutePath()))
		{
			continue;
		}
		checkedList << dataDirectory.absolutePath();
		if (!validatePermissions(dataDirectory.absolutePath()))
		{
			failedList.append(dataDirectory.absolutePath());
		}
	}

	return failedList;
}

QString DataPaths::documentsLocationPath(const QString &subpath) const
{
	QString rootPath;
	if (!isPortableModeOn())
	{
#if QT_VERSION >= 0x050000
		rootPath = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first();
#else
		rootPath = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation);
#endif
		rootPath = Strings::combinePaths(rootPath, QCoreApplication::applicationName());
	}
	else
	{
		rootPath = systemAppDataDirectory("storage");
	}
	return Strings::combinePaths(rootPath, subpath);
}

QString DataPaths::env(const QString &key)
{
	return QProcessEnvironment::systemEnvironment().value(key);
}

void DataPaths::initDefault(bool bPortableModeOn)
{
	assert(staticDefaultInstance == NULL && "DataPaths can have only one default.");
	if (staticDefaultInstance == NULL)
	{
		staticDefaultInstance = new DataPaths(bPortableModeOn);
	}
}

bool DataPaths::isPortableModeOn() const
{
	return d->bIsPortableModeOn;
}

QString DataPaths::localDataLocationPath(const QString& subpath) const
{
	return Strings::combinePaths(d->dataDirectory.absolutePath(), subpath);
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
	paths.append(localDataLocationPath());
	paths.append(workingDirectory());
	paths.append("./");
#ifndef Q_OS_WIN32
	paths.append(INSTALL_PREFIX "/" INSTALL_LIBDIR "/doomseeker/");
#endif
	paths = uniquePaths(paths);
	return Strings::combineManyPaths(paths, "engines/");
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
		return QString();
	#endif
}

QString DataPaths::programsDataDirectoryPath() const
{
	return d->configDirectory.absolutePath();
}

QString DataPaths::resolveTemplated(const QString &templatedPath) const
{
	QString resolved = templatedPath;

	// $PROGDIR is a special placeholder that always resolves to the directory
	// where executable is stored, even if an env. var with the same name exists.
	resolved.replace("$PROGDIR", QCoreApplication::applicationDirPath());

	// Match ~ to home dir.
	QStringList homePatterns;
	homePatterns << "^~[/\\\\]" << "^~$";
	QStringList homeReplaces;
	homeReplaces << "~/" << "~";
	for (int idx = 0; idx < homePatterns.size(); ++idx)
	{
		QRegExp regex(homePatterns[idx]);
		if (resolved.contains(regex))
		{
			QString replacer = homeReplaces[idx].replace("~", QDir::homePath());
			resolved = resolved.replace(regex, replacer);
		}
	}

	// Match arbitrary env. vars.
	QRegExp envVarRegex("\\$([a-zA-Z_][a-zA-Z0-9_]*)");
	int offset = 0;
	while (true)
	{
		int varIdx = envVarRegex.indexIn(resolved, offset);
		if (varIdx < 0)
			break;
		QString name = envVarRegex.cap(1);
		QString varValue = "";
		if (QProcessEnvironment::systemEnvironment().contains(name))
		{
			varValue = qgetenv(name.toUtf8());
		}
		resolved = resolved.left(varIdx) + varValue + resolved.mid(varIdx + envVarRegex.matchedLength());
		offset = varIdx + varValue.length();
	}
	return resolved;
}

void DataPaths::setWorkingDirectory(const QString &workingDirectory)
{
	d->workingDirectory = workingDirectory;
}

QStringList DataPaths::staticDataSearchDirs(const QString& subdir)
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
		{
			paths[i] = Strings::combinePaths(paths[i], subdirFiltered);
		}
	}
	return paths;
}

QString DataPaths::systemAppDataDirectory(QString append) const
{
	Strings::triml(append, "/\\");

	if (isPortableModeOn())
	{
		QString path = d->workingDirectory + "/" + append;
		return QDir(path).absolutePath();
	}

	// For non-portable model this continues here:
	QString dir;

	#ifdef Q_OS_WIN32
	// Let's open new block to prevent variable "bleeding".
	{
		QString envVar = env("APPDATA");
		if (validateDir(envVar))
		{
			dir = envVar;
		}
	}
	#endif

	if (dir.isEmpty())
	{
		dir = QDir::homePath();
		if (!validateDir(dir))
		{
			return QString();
		}
	}

	Strings::trimr(dir, "/\\");

	dir += QDir::separator() + append;

	return QDir(dir).absolutePath();
}

bool DataPaths::tryCreateDirectory(const QDir& rootDir, const QString& dirToCreate) const
{
	return rootDir.mkpath(dirToCreate);
}

bool DataPaths::validateAppDataDirectory()
{
	return validateDir(systemAppDataDirectory());
}

bool DataPaths::validateDir(const QString& path)
{
	QFileInfo fileInfo(path);

	bool bCondition1 = !path.isEmpty();
	bool bCondition2 = fileInfo.exists();
	bool bCondition3 = fileInfo.isDir();

	return bCondition1 && bCondition2 && bCondition3;
}

bool DataPaths::validatePermissions(const QString& path)
{
	++qt_ntfs_permission_lookup;
	QFileInfo fileInfo(path);

	bool bCondition1 = fileInfo.isExecutable();
	bool bCondition2 = fileInfo.isWritable();
	bool bCondition3 = fileInfo.isReadable();
	--qt_ntfs_permission_lookup;

	return bCondition1 && bCondition2 && bCondition3;
}

const QString &DataPaths::workingDirectory() const
{
	return d->workingDirectory;
}
