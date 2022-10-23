//------------------------------------------------------------------------------
// idgames.h
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
// Copyright (C) 2009 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __IDGAMES_H_
#define __IDGAMES_H_

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QUrl>

#include "wadseekermessagetype.h"

class WadDownloadInfo;

class Idgames : public QObject
{
	Q_OBJECT

public:
	static QString defaultIdgamesUrl();

	/**
	 * Creates object to handle Idgames archive.
	 * @param idgamesPage - base address of idgames archive.
	 */
	Idgames(const QString &idgamesPage);
	~Idgames() override;

	void abort();

	const WadDownloadInfo &file() const;

	void setFile(const WadDownloadInfo &wad);
	void setPage(const QString &url);
	void setUserAgent(const QString &userAgent);

	/**
	 * Entry method.
	 */
	void startSearch();


signals:
	/**
	 * @brief Emitted once Idgames client finishes its search.
	 *
	 * @param pThis - Pointer to the emitter object.
	 */
	void finished(Idgames *pThis);

	/**
	 * @brief Emitted when a download URL for a file is found.
	 *
	 * @param wadName
	 *      The name of the WAD passed to setFile()
	 * @param mirrorUrls
	 *      Download URLs for the file. These are all mirrors of the same
	 *      file on different servers.
	 */
	void fileLinksFound(const QString &wadName, const QList<QUrl> &mirrorUrls);

	void message(const QString &msg, WadseekerLib::MessageType type);

	/**
	 * @brief Emitted when a WWW site finishes download.
	 */
	void siteFinished(const QUrl &site);

	/**
	 * @brief Emitted when a WWW site is being downloaded.
	 */
	void siteProgress(const QUrl &site, qint64 bytes, qint64 total);

	/**
	 * @brief Emitted when a download of a WWW site starts.
	 */
	void siteStarted(const QUrl &site);

private:
	enum PageProcessResults
	{
		NotIdgames = -2,
		StringTooShort = -1,
		NoPositions = 0,
		Ok = 1,
	};

	class PrivData;
	PrivData *d;

	/**
	 * @brief Extracts WAD download links from specified page.
	 *
	 * Each link is emitted through fileUrlFound() signal.
	 *
	 * ATM only one link will be emitted - the link Germany mirror FTP
	 * server (as it comes first on the page). There's nothing worst than
	 * have multiple links to the same invalid file.
	 *
	 * @param pageData - HTML code of the page.
	 * @param pageUrl - URL to the page.
	 */
	void extractAndEmitLinks(QByteArray &pageData, const QUrl &pageUrl);

	void queryIdgamesApi();

	/**
	 * @brief Starts network query using specified URL.
	 */
	void startNetworkQuery(const QUrl &url);

	QString zipName() const;

private slots:
	void onIdgamesApiQueryFinished();
	void networkRequestFinished();
	void networkRequestProgress(qint64 done, qint64 total);
};

#endif
