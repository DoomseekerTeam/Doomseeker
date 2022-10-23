//------------------------------------------------------------------------------
// ip2cupdater.h
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
// Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------
#ifndef __IP2CUPDATER_H__
#define __IP2CUPDATER_H__

#include <QByteArray>
#include <QNetworkReply>
#include <QObject>

/**
 * @brief IP2CUpdater is responsible for downloading a new version of database
 * from the site.
 *
 * As IP2C class remains the core of the whole system for more
 * information refer to there.
 */
class IP2CUpdater : public QObject
{
	Q_OBJECT

public:
	enum UpdateStatus
	{
		UpToDate,
		UpdateNeeded,
		UpdateCheckError
	};

	IP2CUpdater(QObject *parent = nullptr);
	~IP2CUpdater() override;

	void downloadDatabase(const QString &savePath);
	const QByteArray &downloadedData();

	/**
	 * @brief Obtains rollback data from specified file.
	 *
	 * This will always clear previous rollback data. You can keep only
	 * one instance of rollback file at a time.
	 *
	 * @return True if rollback data was obtained, false otherwise.
	 * Old rollback data will be cleared anyway.
	 *
	 * @see rollback()
	 */
	bool getRollbackData(const QString &databasePath);

	bool hasDownloadedData() const
	{
		return !retrievedData.isEmpty();
	}
	bool hasRollbackData() const
	{
		return !rollbackData.isEmpty();
	}

	bool isWorking() const;

	/**
	 * @brief Checks if IP2C file must be updated.
	 *
	 * The call is asynchronous as the locally stored database is compared
	 * against the one hosted online on Doomseeker's web page. Once it
	 * completes, an updateNeeded() signal is emitted.
	 *
	 * @param filePath - Path to the file. It is assumed that this is the
	 *     IP2Country database file.
	 */
	void needsUpdate(const QString &filePath);


	/**
	 * @brief Saves rollback data to the specified file. This data must be
	 * first obtained through the getRollbackData() method.
	 *
	 * If there is nothing to rollback this will do nothing. This will also
	 * clear previously obtained rollback data.
	 *
	 * @return True if rollback succeeded, false if there was nothing to
	 * rollback to or the save failed.
	 *
	 * @see getRollbackData()
	 */
	bool rollback(const QString &savePath);

	/**
	 * @brief Saves recently downloaded data to the specified file.
	 *
	 * This will do nothing if there is no downloaded data.
	 *
	 * @return True if save succeeded, false if there was nothing to save
	 * or the save failed.
	 */
	bool saveDownloadedData(const QString &savePath);

signals:
	/**
	 * @brief In case of failure the downloadedData array will be empty.
	 */
	void databaseDownloadFinished(const QByteArray &downloadedData);
	void downloadProgress(qint64 value, qint64 max);
	/**
	 * @brief Emitted status is one of UpdateStatus enum values.
	 */
	void updateNeeded(int status);

private:
	static const QUrl dbChecksumUrl();
	static const QUrl dbDownloadUrl();

	QNetworkAccessManager *pNetworkAccessManager;
	QNetworkReply *pCurrentNetworkReply;

	QString lastAsyncCallPath;
	QByteArray retrievedData;
	QByteArray rollbackData;

	void abort();
	void get(const QUrl &url, const char *finishedSlot);
	bool handleRedirect(QNetworkReply &reply, const char *finishedSlot);
	bool save(const QByteArray &saveWhat, const QString &savePath);

private slots:
	void checksumDownloadFinished();
	void downloadFinished();
};

#endif
