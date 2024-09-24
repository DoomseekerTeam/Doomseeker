//------------------------------------------------------------------------------
// seeksession.h
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
// Copyright (C) 2024 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef WADSEEKER_SEEKSESSION_H
#define WADSEEKER_SEEKSESSION_H

#include "dptr.h"
#include "entities/modset.h"
#include "wadseekermessagetype.h"
#include "www/webquery.h"

#include <QList>
#include <QObject>
#include <QSharedPointer>
#include <QStringList>
#include <QString>

class QNetworkAccessManager;

class Link;
class Idgames;
class NetworkReply;
class WebCrawl;
class WadDownloadInfo;

struct Inflight;

/**
 * Result of a SeekSession's work.
 */
enum class SeekResult
{
	/// Session completed and all seeked files were found and installed.
	Success,
	/// Session completed but at least one file was not found.
	Failed,
	/// A program or user error prevented the session from
	/// running (or even starting) correctly.
	Error,
	/// Session was aborted on request.
	Aborted,
};

struct SeekParameters
{
	bool bIdgamesEnabled;
	QStringList customSiteUrls;
	QString idgamesUrl;
	unsigned maxConcurrentDownloads;
	unsigned maxConcurrentSeeks;
	QString saveDirectoryPath;
	ModSet seekedWads;
	QStringList sitesUrls;

	SeekParameters();
};

/**
 * What is the expected outcome of getting a given URL?
 */
enum class CrawlGoal
{
	Seek, Download,
};

/**
 * Wadseeker's seek session that crawls the Web looking for files.
 *
 * The SeekSession is a long-living, asynchronous object. It's used like this:
 *
 * 1. Prepare session's configuration with SeekParameters.
 * 2. Create it and connect its signals.
 * 3. Run startSeek() to kick off the session.
 * 4. SeekSession notifies of its progress with various signals.
 *
 * When the session emits the seekFinished() signal, it means it's done
 * everything it was asked to do, and can be disposed of. There are no
 * more running jobs at this point. If you need to abort the session
 * early, call abort() first, but you still need to await seekFinished().
 */
class SeekSession : public QObject
{
	Q_OBJECT;
	Q_DISABLE_COPY(SeekSession);

public:
	/**
	 * @param net QNAM that will be used to send the outgoing queries.
	 * @param params Session's configuration; immutable.
	 */
	SeekSession(QNetworkAccessManager *net, SeekParameters params);
	~SeekSession();

	// Flow control.

	/**
	 * Abort the seek completely, leaving the job uncompleted.
	 *
	 * The abort is asynchronous and the session may continue to work
	 * for an unspecified amount of time, even if it will take care to
	 * make this time as short as possible. The seekFinished() signal
	 * is emitted once the abort is complete.
	 */
	void abort();

	/**
	 * SeekSession's entry-point; call this when you're ready to go.
	 */
	void startSeek();

	// Seek state mutators / retrievers.

	bool isDownloadingFile(const ModFile &file) const;
	bool isWorking() const;
	void skipFileCurrentUrl(const QString &fileName);
	void skipService(const QString &service);
	void skipSiteSeek(const QUrl &url);

signals:
	void badUrlDetected(QUrl);

	void fileDownloadFinished(const ModFile &filename);
	void fileDownloadProgress(const ModFile &filename, qint64 done, qint64 total);
	void fileDownloadStarted(const ModFile &filename, const QUrl &url);
	void fileInstalled(const ModFile &filename);

	void message(const QString &msg, WadseekerLib::MessageType type);

	void seekFinished(SeekResult);
	void seekStarted(const ModSet &filenames);

	void serviceFinished(const QString &name);
	void serviceStarted(const QString &name);

	void siteFinished(const QUrl &site);
	void siteProgress(const QUrl &site, qint64 bytes, qint64 total);
	void siteRedirect(const QUrl &oldUrl, const QUrl &newUrl);
	void siteStarted(const QUrl &site);

private:
	DPtr<SeekSession> d;

	void conclude();
	void concludeIfAllFinished();
	void finish(SeekResult result);
	bool isAllFinished() const;
	bool isEverythingFound() const;
	void setupIdgamesClients(const QList<WadDownloadInfo> &wadDownloadInfoList);
	void startFlight(const Inflight &flight);
	void startNextIdgamesClient();
	void startNextCrawl(CrawlGoal goal);
	void startIdgames();
	void stopAll();
	void stopIdgames();

private slots:
	void crawl();

	void fileMirrorLinksFound(const QString &filename, QList<QUrl> urls);
	void idgamesClientFinished();

	void onLinkSeekFinished();
	void onLinkSeekFound(const WebCrawl &crawl, const Link &link);

	void onMessage(const QString &msg, WadseekerLib::MessageType type);

	void onWwwError(WebQuery *query);
	void onWwwFinished(WebQuery *query);
	void onWwwFoundType(WebQuery *query, WebResourceType);
	void onWwwProgress(WebQuery *query, qint64 bytesReceived, qint64 bytesTotal);
};

#endif
