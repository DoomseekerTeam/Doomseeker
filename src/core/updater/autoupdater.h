//------------------------------------------------------------------------------
// autoupdater.h
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
#ifndef DOOMSEEKER_UPDATER_AUTOUPDATER_H
#define DOOMSEEKER_UPDATER_AUTOUPDATER_H

#include "dptr.h"

#include <QDomDocument>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QUrl>

class UpdateChannel;
class UpdatePackage;

/**
 * @brief Deals with program updates/upgrades.
 *
 * AutoUpdater is a non-reusable state machine. This means that if start()
 * was called once then subsequent calls to start() are an error even if
 * the machine has completed its previous job. If updater needs to be
 * launched multiple times then the old object needs to be disposed of
 * and a new object needs to be created.
 *
 * The current implementation is coupled with Doomseeker architecture.
 * The updater will extract information on plugins and Doomseeker version
 * and compare it to the version information included in the updater info
 * file.
 *
 * If versions for selected channels do not match then the actual installation
 * package is downloaded and stored in a space that is persistent between
 * OS reboots. A configuration setting is marked to install the update on next
 * program start.
 *
 * When AutoUpdater finishes its job it emits the finished() signal.
 * errorCode() must be called to check if operation was successful or
 * if the updater stopped on an error. lastNetworkError() may give
 * more insight if updater stopped on a remote error.
 */
class AutoUpdater : public QObject
{
	Q_OBJECT

public:
	enum ErrorCode
	{
		EC_Ok = 0,
		/**
		 * @brief Update was aborted by the user or by the program.
		 */
		EC_Aborted,
		/**
		 * @brief No valid UpdateChannel was specified.
		 */
		EC_NullUpdateChannel,
		/**
		 * @brief Network error when downloading updater info file.
		 */
		EC_UpdaterInfoDownloadProblem,
		/**
		 * @brief Updater info file can't be parsed.
		 */
		EC_UpdaterInfoCannotParse,
		/**
		 * @brief File was parseable but there was no main program
		 *        information inside.
		 */
		EC_UpdaterInfoMissingMainProgramNode,
		/**
		 * @brief One of packages has no revision info.
		 */
		EC_MissingRevisionInfo,
		/**
		 * @brief One of packages has no download URL.
		 */
		EC_MissingDownloadUrl,
		/**
		 * @brief QUrl.isValid() for package download URL returned false
		 *        or QUrl.isRelative() returned true..
		 */
		EC_InvalidDownloadUrl,
		/**
		 * @brief Failed to download update package.
		 */
		EC_PackageDownloadProblem,
		/**
		 * @brief Failed to create directory for updates storage.
		 */
		EC_StorageDirCreateFailure,
		/**
		 * @brief Package file can't be stored on the local filesystem.
		 */
		EC_PackageCantBeSaved,
		/**
		 * @brief Update script can't be merged and stored on the local
		 *        filesystem.
		 */
		EC_ScriptCantBeSaved
	};

	/**
	 * @brief Prefix for all plugins packages names.
	 */
	static const QString PLUGIN_PREFIX;
	/**
	 * @brief Package name for the main program.
	 *
	 * Other packages are plugins.
	 */
	static const QString MAIN_PROGRAM_PACKAGE_NAME;
	static const QString QT_PACKAGE_NAME;
	static const QString WADSEEKER_PACKAGE_NAME;
	/**
	 * @brief Base URL to the directory where "update-info*"
	 *        JSON files are contained.
	 */
	static const QString UPDATER_INFO_URL_BASE;

	static QString errorCodeToString(ErrorCode code);
	/**
	 * @brief Path to updater script XML file.
	 *
	 * This consists of a constant filename which the updater XML script
	 * is always saved under.
	 */
	static QString updaterScriptPath();
	static QString updateStorageDirPath();


	AutoUpdater(QObject *pParent = nullptr);
	~AutoUpdater() override;

	void abort();
	/**
	 * @brief setChannel() .
	 */
	const UpdateChannel &channel() const;
	/**
	 * @brief Filenames for packages which are ready to install.
	 *
	 * These are names only. These files should reside in a known location,
	 * pointed to by DataPaths::localDataLocationPath() in the
	 * DataPaths::UPDATE_PACKAGES_DIR_NAME subdir.
	 * This ensures that if portable mode is copied to a different
	 * directory, it will still know where to get downloaded packages.
	 *
	 * Script files are not listed here because their names are the same
	 * as for packages but with ".xml" appended.
	 *
	 * Contents of the list are only valid if updater finishes with
	 * EC_Ok status. Before that, or if updater finishes with error status,
	 * the contents of the list are undefined.
	 */
	const QStringList &downloadedPackagesFilenames() const;
	ErrorCode errorCode() const;
	QString errorString() const;

	bool isRunning() const;

	/**
	 * @brief The network error that caused the updater to fail.
	 *
	 * This returns a valid value only if errorCode() is not equal
	 * to EC_Ok and if the failure was in fact caused by the network
	 * error.
	 */
	QNetworkReply::NetworkError lastNetworkError() const;
	/**
	 * @brief List of new update packages to install.
	 *
	 * When downloadAndInstallConfirmationRequested() signal is emitted
	 * this accessor can be used to obtain the list of updates
	 * that will be installed if user confirms the install.
	 */
	const QList<UpdatePackage> &newUpdatePackages() const;
	/**
	 * @brief Update channel name.
	 */
	void setChannel(const UpdateChannel &updateChannel);
	/**
	 * @brief Revisions set in this map will not be treated as updates
	 *        even if they differ from the currently installed one.
	 *
	 * However, if update for at least one unignored package is detected,
	 * then the entire list is treated as if it wasn't set.
	 *
	 * @param packagesRevisions
	 *     Key - package name. Value - list of revisions.
	 */
	void setIgnoreRevisions(const QMap<QString, QList<QString> > &packagesRevisions);
	/**
	 * @brief Controls if the download&installation process is automated.
	 *
	 * If set to true then downloadAndInstallConfirmationRequested()
	 * signal will be emitted and the updater will suspend until
	 * confirmDownloadAndInstall() slot is called. If false then
	 * download&installation will proceed automatically as soon
	 * as new packages are detected.
	 *
	 * Default: false.
	 */
	void setRequireDownloadAndInstallConfirmation(bool b);
	void start();

public slots:
	void confirmDownloadAndInstall();

signals:
	/**
	 * @brief Information on update packages has been received
	 *        and install confirmation is requested.
	 */
	void downloadAndInstallConfirmationRequested();
	/**
	 * @brief AutoUpdater has finished its job.
	 *
	 * This signal is emitted if either job completed normally
	 * or ended due to an error.
	 */
	void finished();

	void overallProgress(int current, int total, const QString &msg);
	void packageDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void statusMessage(const QString &message);

private:
	DPtr<AutoUpdater> d;

	/**
	 * @brief Updates package name to fit the current package filename.
	 */
	QDomDocument adjustUpdaterScriptXml(const QByteArray &xmlSource);
	/**
	 * @brief Writes a log message for every entry on the list.
	 *
	 * The log is always dumped, but the main purpose is to
	 * notify user that update was detected in case
	 * if user enabled automatic updates.
	 */
	void dumpUpdatePackagesToLog(const QList<UpdatePackage> &packages);
	void emitOverallProgress(const QString &message);
	void emitStatusMessage(const QString &message);
	void finishWithError(ErrorCode code);
	QUrl mkVersionDataFileUrl();

	ErrorCode saveUpdaterScript();
	void startPackageDownload(const UpdatePackage &pkg);
	void startPackageScriptDownload(const UpdatePackage &pkg);
	void startNextPackageDownload();

private slots:
	void onPackageDownloadFinish();
	void onPackageDownloadReadyRead();
	void onPackageScriptDownloadFinish();
	void onUpdaterInfoDownloadFinish();
};

#endif
