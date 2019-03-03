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

static QList<DataPaths::DirErrno> uniqueErrnosByDir(const QList<DataPaths::DirErrno> &errnos)
{
	QSet<QString> uniqueDirs;
	QList<DataPaths::DirErrno> uniqueErrnos;
	for (const DataPaths::DirErrno &dirErrno : errnos)
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

DataPaths *DataPaths::staticDefaultInstance = nullptr;

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
		failedList.append(dataDirectory);

	return failedList;
}

QList<DataPaths::DirErrno> DataPaths::createDirectories()
{
	QList<DirErrno> failedDirs;
	const QDir appDataDir(systemAppDataDirectory());

	// No need to bother with migrating plugin master caches
	DirErrno cacheDirError = tryCreateDirectory(d->cacheDirectory, ".");
	if (cacheDirError.isError())
		failedDirs << cacheDirError;

	// The existential question here is needed for migration purposes,
	// but on Windows the >=1.2 configDirectory can already exist because
	// Doomseeker <1.2 already stored IRC chat logs there.
	// It is necessary to ask about the .ini file.
	if (!d->configDirectory.exists(DoomseekerFilePaths::INI_FILENAME))
	{
		DirErrno configDirError = tryCreateDirectory(d->configDirectory, ".");
		if (configDirError.isError())
			failedDirs << configDirError;

		#if !defined(Q_OS_MAC)
		else if (appDataDir.exists(".doomseeker"))
		{
			// Migrate config from old versions of Doomseeker (pre 1.2)
			const QDir oldConfigDir(appDataDir.absolutePath() + QDir::separator() + ".doomseeker");
			gLog << QString("Migrating configuration data from '%1'\n\tto '%2'.")
				.arg(oldConfigDir.absolutePath())
				.arg(d->configDirectory.absolutePath());

			for (QFileInfo fileinfo : oldConfigDir.entryInfoList(QStringList("*.ini"), QDir::Files))
			{
				QFile(fileinfo.absoluteFilePath()).copy(d->configDirectory.absoluteFilePath(fileinfo.fileName()));
			}
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
		DirErrno dataDirError = tryCreateDirectory(d->dataDirectory, ".");
		if (dataDirError.isError())
			failedDirs << dataDirError;
		else if (appDataDir.exists(legacyPrefDirectory))
		{
			// Migrate data from old versions of Doomseeker (specifically demos) (pre 1.2)
			const QDir oldConfigDir(appDataDir.absolutePath() + QDir::separator() + legacyPrefDirectory);
			gLog << QString("Migrating user data from '%1'\n\tto '%2'.")
				.arg(oldConfigDir.absolutePath())
				.arg(d->dataDirectory.absolutePath());

			for (QFileInfo fileinfo : oldConfigDir.entryInfoList(QDir::Dirs))
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

	DirErrno demosDirError = tryCreateDirectory(d->dataDirectory, DEMOS_DIR_NAME);
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
	filePaths << programsDataDirectoryPath();

	// The directory which contains the Doomseeker executable may be a good
	// default choice, but on unix systems the bin directory is not worth
	// searching.  Similarly for Mac application bundle.
	const QString progBinDirName = QDir(workingDirectory()).dirName();
	if (progBinDirName != "bin" && progBinDirName != "MacOS")
		filePaths << workingDirectory();

	return filePaths;
}

QString DataPaths::demosDirectoryPath() const
{
	return d->dataDirectory.absoluteFilePath(DEMOS_DIR_NAME);
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
		rootPath = systemAppDataDirectory("storage");
	return Strings::combinePaths(rootPath, subpath);
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
	paths.append(workingDirectory());
	paths.append("./");

	#if !defined(Q_OS_MAC) && !defined(Q_OS_WIN32)
	// On systems where we install to a fixed location, if we see that we are
	// running an installed binary, then we should only load plugins from the
	// expected location.  Otherwise use it only as a last resort.
	const QString installDir = INSTALL_PREFIX "/" INSTALL_LIBDIR "/doomseeker/";
	if (workingDirectory() == INSTALL_PREFIX "/bin")
		paths = QStringList(installDir);
	else
		paths.append(installDir);
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

void DataPaths::setWorkingDirectory(const QString &workingDirectory)
{
	d->workingDirectory = workingDirectory;
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
			dir = envVar;
	}
	#endif

	if (dir.isEmpty())
	{
		dir = QDir::homePath();
		if (!validateDir(dir))
			return QString();
	}

	Strings::trimr(dir, "/\\");

	dir += QDir::separator() + append;

	return QDir(dir).absolutePath();
}

DataPaths::DirErrno DataPaths::tryCreateDirectory(const QDir &rootDir, const QString &dirToCreate) const
{
	QString fullDirPath = Strings::combinePaths(rootDir.path(), dirToCreate);

	// We need to reset errno to prevent false positives
	errno = 0;
	if (!rootDir.mkpath(dirToCreate))
	{
		int errnoval = errno;
		if (errnoval != 0)
			return DirErrno(fullDirPath, errnoval, strerror(errnoval));
		else
		{
			// Try to decipher if we have permissions.
			// First we must find the bottom-most directory that does actually exist.
			QString pathToBottomMostExisting = FileUtils::cdUpUntilExists(
				Strings::combinePaths(rootDir.path(), dirToCreate));
			// If we've found a bottom-most directory that exists, we can check its permissions.
			if (!pathToBottomMostExisting.isEmpty())
			{
				QFileInfo parentDir(pathToBottomMostExisting);
				if (parentDir.exists() && !parentDir.isDir())
				{
					return DirErrno(fullDirPath, DirErrno::CUSTOM_ERROR,
						QObject::tr("parent node is not a directory: %1")
							.arg(parentDir.filePath()));
				}
				if (!checkDirPermissions(parentDir.filePath()))
				{
					return DirErrno(fullDirPath, DirErrno::CUSTOM_ERROR,
						QObject::tr("lack of necessary permissions to the parent directory: %1")
							.arg(parentDir.filePath()));
				}
			}
			// Just give up trying to deduce a correct error.
			return DirErrno(fullDirPath, DirErrno::CUSTOM_ERROR, QObject::tr("cannot create directory"));
		}
	}
	return DirErrno();
}

bool DataPaths::validateAppDataDirectory()
{
	return validateDir(systemAppDataDirectory());
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
	return d->workingDirectory;
}
