//------------------------------------------------------------------------------
// autoupdater.cpp
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
#include "autoupdater.h"

#include "configuration/doomseekerconfig.h"
#include "datapaths.h"
#include "log.h"
#include "strings.hpp"
#include "updater/updatechannel.h"
#include "updater/updatepackagefilter.h"
#include "updater/updaterinfoparser.h"
#include "updater/updaterscriptparser.h"
#include "version.h"
#include <cassert>
#include <QByteArray>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QTemporaryFile>

DClass<AutoUpdater>
{
public:
	/// All scripts are merged to create one big script for all packages.
	QList<QDomDocument> allScripts;
	bool bDownloadAndInstallRequireConfirmation;
	bool bIsRunning;
	/// Used for overall progress.
	bool bPackageDownloadStarted;
	bool bStarted;
	UpdateChannel channel;
	UpdatePackage currentlyDownloadedPackage;
	QStringList downloadedPackagesFilenames;
	AutoUpdater::ErrorCode errorCode;
	QMap<QString, QList<QString> > ignoredPackagesRevisions;
	QList<UpdatePackage> newUpdatePackages;
	QList<UpdatePackage> packagesInDownloadQueue;
	QTemporaryFile *pCurrentPackageFile;
	QNetworkAccessManager *pNam;
	QNetworkReply *pNetworkReply;
};

DPointered(AutoUpdater)

//////////////////////////////////////////////////////////////////////////////

const QString AutoUpdater::PLUGIN_PREFIX = "p-";
const QString AutoUpdater::MAIN_PROGRAM_PACKAGE_NAME = "doomseeker-core";
const QString AutoUpdater::QT_PACKAGE_NAME = "qt";
const QString AutoUpdater::WADSEEKER_PACKAGE_NAME = "wadseeker";
const QString AutoUpdater::UPDATER_INFO_URL_BASE = "https://doomseeker.drdteam.org/updates/";

AutoUpdater::AutoUpdater(QObject *pParent)
	: QObject(pParent)
{
	d->bDownloadAndInstallRequireConfirmation = false;
	d->bPackageDownloadStarted = false;
	d->bIsRunning = false;
	d->bStarted = false;
	d->errorCode = EC_Ok;
	d->pCurrentPackageFile = nullptr;
	d->pNam = new QNetworkAccessManager();
	d->pNetworkReply = nullptr;
}

AutoUpdater::~AutoUpdater()
{
	if (d->pCurrentPackageFile != nullptr)
	{
		delete d->pCurrentPackageFile;
	}
	if (d->pNetworkReply != nullptr)
	{
		d->pNetworkReply->disconnect();
		d->pNetworkReply->abort();
		d->pNetworkReply->deleteLater();
	}
	d->pNam->disconnect();
	d->pNam->deleteLater();
}

void AutoUpdater::abort()
{
	if (d->pNetworkReply != nullptr)
	{
		d->pNetworkReply->disconnect();
		d->pNetworkReply->abort();
		d->pNetworkReply->deleteLater();
		d->pNetworkReply = nullptr;
	}
	emit finishWithError(EC_Aborted);
}

QDomDocument AutoUpdater::adjustUpdaterScriptXml(const QByteArray &xmlSource)
{
	QDomDocument xmlDoc;
	QString xmlError;
	int xmlErrLine = -1;
	int xmlErrCol = -1;
	if (!xmlDoc.setContent(xmlSource, &xmlError, &xmlErrLine, &xmlErrCol))
	{
		gLog << tr("Failed to parse updater XML script: %1, l: %2, c: %3")
			.arg(xmlError).arg(xmlErrLine).arg(xmlErrCol);
		return QDomDocument();
	}
	UpdaterScriptParser scriptParser(xmlDoc);
	QFileInfo currentPackageFileInfo(d->pCurrentPackageFile->fileName());
	QString scriptParserErrMsg = scriptParser.setPackageName(
		currentPackageFileInfo.completeBaseName());
	if (!scriptParserErrMsg.isNull())
	{
		gLog << tr("Failed to modify package name in updater script: %1")
			.arg(scriptParserErrMsg);
		return QDomDocument();
	}
	return xmlDoc;
}

const UpdateChannel &AutoUpdater::channel() const
{
	return d->channel;
}

void AutoUpdater::confirmDownloadAndInstall()
{
	d->packagesInDownloadQueue = d->newUpdatePackages;
	d->bPackageDownloadStarted = true;
	startNextPackageDownload();
}

const QStringList &AutoUpdater::downloadedPackagesFilenames() const
{
	return d->downloadedPackagesFilenames;
}

void AutoUpdater::dumpUpdatePackagesToLog(const QList<UpdatePackage> &packages)
{
	for (const UpdatePackage &pkg : packages)
	{
		gLog << tr(R"(Detected update for package "%1" from version "%2" to version "%3".)")
			.arg(pkg.displayName, pkg.currentlyInstalledDisplayVersion, pkg.displayVersion);
	}
}

void AutoUpdater::emitOverallProgress(const QString &message)
{
	int total = 1;
	int current = 0;
	if (!d->newUpdatePackages.isEmpty() && d->bPackageDownloadStarted)
	{
		total = d->newUpdatePackages.size();
		// Add 1 because currently downloaded package has already been
		// removed from the queue.
		current = total - (d->packagesInDownloadQueue.size() + 1);
		assert(current >= 0 && "AutoUpdater::emitOverallProgress()");
	}
	emit overallProgress(current, total, message);
}

void AutoUpdater::emitStatusMessage(const QString &message)
{
	emit statusMessage(message);
}

AutoUpdater::ErrorCode AutoUpdater::errorCode() const
{
	return d->errorCode;
}

QString AutoUpdater::errorCodeToString(ErrorCode code)
{
	switch (code)
	{
	case EC_Ok:
		return tr("Ok");
	case EC_Aborted:
		return tr("Update was aborted.");
	case EC_NullUpdateChannel:
		return tr("Update channel is not configured. Please check your configuration.");
	case EC_UpdaterInfoDownloadProblem:
		return tr("Failed to download updater info file.");
	case EC_UpdaterInfoCannotParse:
		return tr("Cannot parse updater info file.");
	case EC_UpdaterInfoMissingMainProgramNode:
		return tr("Main program node is missing from updater info file.");
	case EC_MissingRevisionInfo:
		return tr("Revision info on one of the packages is missing from the "
			"updater info file. Check the log for details.");
	case EC_MissingDownloadUrl:
		return tr("Download URL for one of the packages is missing from the "
			"updater info file. Check the log for details.");
	case EC_InvalidDownloadUrl:
		return tr("Download URL for one of the packages is invalid. "
			"Check the log for details.");
	case EC_PackageDownloadProblem:
		return tr("Update package download failed. Check the log for details.");
	case EC_StorageDirCreateFailure:
		return tr("Failed to create directory for updates packages storage.");
	case EC_PackageCantBeSaved:
		return tr("Failed to save update package.");
	case EC_ScriptCantBeSaved:
		return tr("Failed to save update script.");
	default:
		return tr("Unknown error.");
	}
}

QString AutoUpdater::errorString() const
{
	return errorCodeToString(errorCode());
}

void AutoUpdater::finishWithError(ErrorCode code)
{
	d->bIsRunning = false;
	d->errorCode = code;
	emit finished();
}

bool AutoUpdater::isRunning() const
{
	return d->bIsRunning;
}

QNetworkReply::NetworkError AutoUpdater::lastNetworkError() const
{
	if (errorCode() != EC_Ok && d->pNetworkReply != nullptr)
	{
		return d->pNetworkReply->error();
	}
	return QNetworkReply::NoError;
}

QUrl AutoUpdater::mkVersionDataFileUrl()
{
	return QUrl(UPDATER_INFO_URL_BASE + d->channel.versionDataFileName());
}

const QList<UpdatePackage> &AutoUpdater::newUpdatePackages() const
{
	return d->newUpdatePackages;
}

void AutoUpdater::onPackageDownloadFinish()
{
	if (d->pNetworkReply->error() == QNetworkReply::NoError)
	{
		emitStatusMessage(tr("Finished downloading package \"%1\".")
			.arg(d->currentlyDownloadedPackage.displayName));
		startPackageScriptDownload(d->currentlyDownloadedPackage);
	}
	else
	{
		emitStatusMessage(tr("Network error when downloading package \"%1\": [%2] %3")
			.arg(d->currentlyDownloadedPackage.displayName)
			.arg(d->pNetworkReply->error())
			.arg(d->pNetworkReply->errorString()));
		finishWithError(EC_PackageDownloadProblem);
	}
}

void AutoUpdater::onPackageDownloadReadyRead()
{
	const int MAX_CHUNK_SIZE = 2 * 1024 * 1024; // 2MB
	QByteArray data = d->pNetworkReply->read(MAX_CHUNK_SIZE);
	while (!data.isEmpty())
	{
		d->pCurrentPackageFile->write(data);
		data = d->pNetworkReply->read(MAX_CHUNK_SIZE);
	}
}

void AutoUpdater::onPackageScriptDownloadFinish()
{
	if (d->pNetworkReply->error() == QNetworkReply::NoError)
	{
		emitStatusMessage(tr("Finished downloading package script \"%1\".")
			.arg(d->currentlyDownloadedPackage.displayName));
		QByteArray xmlData = d->pNetworkReply->readAll();
		QDomDocument xmlDoc = adjustUpdaterScriptXml(xmlData);
		if (xmlDoc.isNull())
		{
			finishWithError(EC_PackageDownloadProblem);
			return;
		}
		d->allScripts.append(xmlDoc);

		if (!d->packagesInDownloadQueue.isEmpty())
		{
			startNextPackageDownload();
		}
		else
		{
			emitStatusMessage(tr("All packages downloaded. Building updater script."));
			ErrorCode result = saveUpdaterScript();
			finishWithError(result);
		}
	}
	else
	{
		emitStatusMessage(tr("Network error when downloading package script \"%1\": [%2] %3")
			.arg(d->currentlyDownloadedPackage.displayName)
			.arg(d->pNetworkReply->error())
			.arg(d->pNetworkReply->errorString()));
		finishWithError(EC_PackageDownloadProblem);
	}
}

void AutoUpdater::onUpdaterInfoDownloadFinish()
{
	if (d->pNetworkReply->error() != QNetworkReply::NoError)
	{
		finishWithError(EC_UpdaterInfoDownloadProblem);
		return;
	}
	QByteArray json = d->pNetworkReply->readAll();
	UpdaterInfoParser parser;
	auto parseResult = (ErrorCode) parser.parse(json);
	if (parseResult == EC_Ok)
	{
		UpdatePackageFilter filter;
		filter.setIgnoreRevisions(d->ignoredPackagesRevisions);
		QList<UpdatePackage> packagesList = filter.filter(parser.packages());
		if (!packagesList.isEmpty())
		{
			dumpUpdatePackagesToLog(packagesList);
			d->newUpdatePackages = packagesList;
			if (d->bDownloadAndInstallRequireConfirmation)
			{
				emitStatusMessage(tr("Requesting update confirmation."));
				emitOverallProgress(tr("Confirm"));
				emit downloadAndInstallConfirmationRequested();
			}
			else
			{
				confirmDownloadAndInstall();
			}
		}
		else
		{
			// Nothing to update.
			emitStatusMessage(tr("No new program updates detected."));
			if (filter.wasAnyUpdatePackageIgnored())
			{
				emitStatusMessage(tr("Some update packages were ignored. To install them "
					"select \"Check for updates\" option from \"Help\" menu."));
			}
			finishWithError(EC_Ok);
		}
	}
	else
	{
		finishWithError(parseResult);
	}
}

AutoUpdater::ErrorCode AutoUpdater::saveUpdaterScript()
{
	QDomDocument xmlDocAllScripts;
	UpdaterScriptParser scriptParser(xmlDocAllScripts);
	for (const QDomDocument &doc : d->allScripts)
	{
		scriptParser.merge(doc);
	}
	QFile f(updaterScriptPath());
	if (!f.open(QIODevice::WriteOnly))
	{
		return EC_ScriptCantBeSaved;
	}
	f.write(xmlDocAllScripts.toByteArray());
	f.close();
	return EC_Ok;
}

void AutoUpdater::setChannel(const UpdateChannel &updateChannel)
{
	d->channel = updateChannel;
}

void AutoUpdater::setIgnoreRevisions(const QMap<QString, QList<QString> > &packagesRevisions)
{
	d->ignoredPackagesRevisions = packagesRevisions;
}

void AutoUpdater::setRequireDownloadAndInstallConfirmation(bool b)
{
	d->bDownloadAndInstallRequireConfirmation = b;
}

void AutoUpdater::start()
{
	if (d->bStarted)
	{
		qDebug() << "Cannot start AutoUpdater more than once.";
		// Always cause assertion failure. Program shouldn't
		// go into this state.
		assert(false && "Cannot start AutoUpdater more than once.");
		return;
	}
	d->bStarted = true;
	d->bPackageDownloadStarted = false;
	if (d->channel.isNull())
	{
		finishWithError(EC_NullUpdateChannel);
		return;
	}
	QDir storageDir(updateStorageDirPath());
	if (!storageDir.mkpath("."))
	{
		gLog << tr("Failed to create directory for updates storage: %1")
			.arg(storageDir.path());
		finishWithError(EC_StorageDirCreateFailure);
	}
	d->bIsRunning = true;
	QNetworkRequest request;
	request.setRawHeader("User-Agent", Version::userAgent().toUtf8());
	request.setUrl(mkVersionDataFileUrl());
	QNetworkReply *pReply = d->pNam->get(request);
	// The updater info file should always be very small and
	// we can safely store it all in memory.
	this->connect(pReply,
		SIGNAL(finished()),
		SLOT(onUpdaterInfoDownloadFinish()));
	this->connect(pReply,
		SIGNAL(downloadProgress(qint64,qint64)),
		SIGNAL(packageDownloadProgress(qint64,qint64)));
	d->pNetworkReply = pReply;
	emitOverallProgress(tr("Update info"));
}

void AutoUpdater::startNextPackageDownload()
{
	assert(!d->packagesInDownloadQueue.isEmpty() && "AutoUpdater::startNextPackageDownload()");
	UpdatePackage pkg = d->packagesInDownloadQueue.takeFirst();
	startPackageDownload(pkg);
}

void AutoUpdater::startPackageDownload(const UpdatePackage &pkg)
{
	QUrl url = pkg.downloadUrl;
	if (!url.isValid() || url.isRelative())
	{
		// Parser already performs a check for this but let's do this
		// again to make sure nothing got lost on the way.
		gLog << tr("Invalid download URL for package \"%1\": %2")
			.arg(pkg.displayName, pkg.downloadUrl.toString());
		finishWithError(EC_InvalidDownloadUrl);
		return;
	}
	emitOverallProgress(tr("Package: %1").arg(pkg.displayName));
	gLog << tr("Downloading package \"%1\" from URL: %2.").arg(pkg.displayName,
		pkg.downloadUrl.toString());

	QString fileNameTemplate = QString("%1%2-XXXXXX.zip")
		.arg(DataPaths::UPDATE_PACKAGE_FILENAME_PREFIX).arg(pkg.name);
	QString filePathTemplate = Strings::combinePaths(updateStorageDirPath(), fileNameTemplate);
	qDebug() << "filePathTemplate: " << filePathTemplate;
	if (d->pCurrentPackageFile != nullptr)
	{
		delete d->pCurrentPackageFile;
	}
	d->pCurrentPackageFile = new QTemporaryFile(filePathTemplate);
	d->pCurrentPackageFile->setAutoRemove(false);
	if (!d->pCurrentPackageFile->open())
	{
		gLog << tr("Couldn't save file in path: %1").arg(updateStorageDirPath());
		delete d->pCurrentPackageFile;
		d->pCurrentPackageFile = nullptr;
		finishWithError(EC_PackageCantBeSaved);
		return;
	}
	QFileInfo fileInfo(d->pCurrentPackageFile->fileName());
	d->downloadedPackagesFilenames << fileInfo.fileName();

	QNetworkRequest request;
	request.setRawHeader("User-Agent", Version::userAgent().toUtf8());
	request.setUrl(url);
	QNetworkReply *pReply = d->pNam->get(request);
	d->currentlyDownloadedPackage = pkg;
	d->pNetworkReply = pReply;
	this->connect(pReply, SIGNAL(readyRead()),
		SLOT(onPackageDownloadReadyRead()));
	this->connect(pReply, SIGNAL(finished()),
		SLOT(onPackageDownloadFinish()));
	this->connect(pReply, SIGNAL(downloadProgress(qint64,qint64)),
		SIGNAL(packageDownloadProgress(qint64,qint64)));
}

void AutoUpdater::startPackageScriptDownload(const UpdatePackage &pkg)
{
	QUrl url = pkg.downloadScriptUrl;
	if (!url.isValid() || url.isRelative())
	{
		// Parser already performs a check for this but let's do this
		// again to make sure nothing got lost on the way.
		gLog << tr("Invalid download URL for package script \"%1\": %2")
			.arg(pkg.displayName, pkg.downloadScriptUrl.toString());
		finishWithError(EC_InvalidDownloadUrl);
		return;
	}
	gLog << tr("Downloading package script \"%1\" from URL: %2.").arg(pkg.displayName,
		pkg.downloadScriptUrl.toString());

	QNetworkRequest request;
	request.setRawHeader("User-Agent", Version::userAgent().toUtf8());
	request.setUrl(url);
	QNetworkReply *pReply = d->pNam->get(request);
	d->currentlyDownloadedPackage = pkg;
	d->pNetworkReply = pReply;
	// Scripts are small enough that they can be downloaded "in one take",
	// without saving them continuously to a file.
	this->connect(pReply, SIGNAL(finished()),
		SLOT(onPackageScriptDownloadFinish()));
	this->connect(pReply,
		SIGNAL(downloadProgress(qint64,qint64)),
		SIGNAL(packageDownloadProgress(qint64,qint64)));
}

QString AutoUpdater::updaterScriptPath()
{
	QString dirPath = gDefaultDataPaths->localDataLocationPath(DataPaths::UPDATE_PACKAGES_DIR_NAME);
	QString name = DataPaths::UPDATE_PACKAGE_FILENAME_PREFIX + "-updater-script.xml";
	return Strings::combinePaths(dirPath, name);
}

QString AutoUpdater::updateStorageDirPath()
{
	return gDefaultDataPaths->localDataLocationPath(DataPaths::UPDATE_PACKAGES_DIR_NAME);
}
