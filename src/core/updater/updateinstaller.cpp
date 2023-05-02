//------------------------------------------------------------------------------
// updateinstaller.cpp
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
#include "updateinstaller.h"

#include "configuration/doomseekerconfig.h"
#include "datapaths.h"
#include "log.h"
#include "main.h"
#include "strings.hpp"
#include "updater/autoupdater.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTemporaryFile>

#ifdef Q_OS_WIN32
const QString UPDATER_EXECUTABLE_FILENAME = "updater.exe";
#else
const QString UPDATER_EXECUTABLE_FILENAME = "updater";
#endif

UpdateInstaller::UpdateInstaller(QObject *pParent)
	: QObject(pParent)
{
}

UpdateInstaller::~UpdateInstaller()
{
}

QString UpdateInstaller::copyUpdaterExecutableToTemporarySpace()
{
	// Windows will complain if we try to overwrite an executable
	// of a running process. To be able to update the updater itself,
	// we need to copy it out to somewhere else and then launch this clone.
	QString updaterProgramPath = Strings::combinePaths(
		QCoreApplication::applicationDirPath(),
		UPDATER_EXECUTABLE_FILENAME);

	// Copying the file to the same directory as the packages, and prefixing
	// its filename with the same prefix as packages, will ensure
	// that the main program will remove the cloned updater once
	// update is finished.
	QString updaterCloneFilename = QString("%1-%2").arg(
		DataPaths::UPDATE_PACKAGE_FILENAME_PREFIX).arg(
		UPDATER_EXECUTABLE_FILENAME);
	QString clonePath = Strings::combinePaths(
		AutoUpdater::updateStorageDirPath(), updaterCloneFilename);

	if (QFile::copy(updaterProgramPath, clonePath))
	{
		bool bPermissionsSet = QFile::setPermissions(clonePath,
			QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner
			| QFile::ExeGroup | QFile::ReadGroup | QFile::ReadOther);
		if (bPermissionsSet)
		{
			return clonePath;
		}
	}
	gLog << tr("Failed to copy the updater executable to a temporary"
			" space: \"%1\" -> \"%2\".").arg(updaterProgramPath, clonePath);
	return QString();
}

QString UpdateInstaller::errorCodeToStr(ErrorCode code)
{
	switch (code)
	{
	case EC_Ok:
		return tr("Ok");
	case EC_NothingToUpdate:
		return tr("Nothing to update.");
	case EC_UpdatePackageMissing:
		return tr("Update package or script are not found. Check log for details.");
	case EC_ProcessStartFailure:
		return tr("Failed to start updater process.");
	default:
		return tr("Unknown error: %1.").arg(code);
	}
}

QString UpdateInstaller::getPercentEncodedCurrentProcessArgs()
{
	QStringList argsEncoded;
	if (gDefaultDataPaths->isPortableModeOn())
	{
		argsEncoded << QUrl::toPercentEncoding("--portable");
	}
	if (!Main::argDataDir.isEmpty())
	{
		argsEncoded << QUrl::toPercentEncoding("--datadir");
		argsEncoded << QUrl::toPercentEncoding(Main::argDataDir);
	}
	return argsEncoded.join(" ");
}

QString UpdateInstaller::processErrorCodeToStr(ProcessErrorCode code)
{
	switch (code)
	{
	case PEC_Ok:
		return tr("Ok");
	case PEC_UnableToReadUpdateScript:
		return tr("Unable to read update script.");
	case PEC_NoInstallationDirectorySpecified:
		return tr("No installation directory specified.");
	case PEC_UnableToDeterminePathOfUpdater:
		return tr("Unable to determine path of updater.");
	case PEC_GeneralFailure:
		return tr("General failure.");
	default:
		return tr("Unknown process error code: %1.").arg(code);
	}
}

UpdateInstaller::ErrorCode UpdateInstaller::startInstallation()
{
	QString scriptPath = AutoUpdater::updaterScriptPath();
	QFile fileScript(scriptPath);
	if (fileScript.exists())
	{
		gLog << tr("Installing update.");
		QString packagesDirPath = AutoUpdater::updateStorageDirPath();
		QDir packagesDir(packagesDirPath);
		// Do some "does file exist" validation. This isn't 100% reliable but
		// should work correctly in most cases.
		bool isPackageOk = packagesDir.exists();
		if (isPackageOk)
		{
			if (!startUpdaterProcess(packagesDirPath, scriptPath))
			{
				return EC_ProcessStartFailure;
			}
		}
		else
		{
			gLog << tr("Package directory \"%1\" doesn't exist.").arg(packagesDirPath);
			return EC_UpdatePackageMissing;
		}
	}
	else
	{
		gLog << tr("Update was about to be installed but "
				"update script \"%1\" is missing.").arg(scriptPath);
		return EC_UpdatePackageMissing;
	}
	return EC_Ok;
}

bool UpdateInstaller::startUpdaterProcess(const QString &packagesDir,
	const QString &scriptFilePath)
{
	QString updaterProgramPath = copyUpdaterExecutableToTemporarySpace();
	if (updaterProgramPath.isEmpty())
	{
		return false;
	}
	qDebug() << "Updater program is located at path: " << updaterProgramPath;
	QFile updaterProgramFile(updaterProgramPath);
	QFileInfo programFileInfo(QCoreApplication::applicationFilePath());
	QStringList args;
	#ifdef Q_OS_DARWIN
	// On Mac we're updating the bundle, but we get the location of the binary (<stuff>/Contents/MacOS/)
	args << "--install-dir" << (QCoreApplication::applicationDirPath() + "/../..");
	#else
	args << "--install-dir" << QCoreApplication::applicationDirPath();
	#endif
	args << "--package-dir" << packagesDir;
	args << "--script" << scriptFilePath;
	args << "--exec" << QDir::toNativeSeparators(programFileInfo.absoluteFilePath());
	QString currentProcessArgs = getPercentEncodedCurrentProcessArgs();
	if (!currentProcessArgs.isEmpty())
	{
		args << "--args" << currentProcessArgs;
	}
	bool bStarted = QProcess::startDetached(updaterProgramPath, args);
	if (!bStarted)
	{
		gLog << tr("Failed to start updater process: %1 %2")
			.arg(updaterProgramPath).arg(args.join(" "));
	}
	return bStarted;
}
