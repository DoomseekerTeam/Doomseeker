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
#include "entities/waddownloadinfo.h"
#include "wadseekermessagetype.h"

#include <QList>
#include <QObject>
#include <QStringList>
#include <QString>

class Idgames;

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
 * An active seek session.
 */
class SeekSession : public QObject
{
	Q_OBJECT;
	Q_DISABLE_COPY(SeekSession);

public:
	SeekSession(SeekParameters params);
	~SeekSession();

	void abort();
	bool isDownloadingFile(const ModFile &file) const;
	bool isWorking() const;
	void skipFileCurrentUrl(const QString &fileName);
	void skipService(const QString &service);
	void skipSiteSeek(const QUrl &url);
	void startSeek();

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

	void abortSeekers();
	void cleanUpAfterFinish();
	void cleanUpIfAllFinished();
	bool isAllFinished() const;
	void setupIdgamesClients(const QList<WadDownloadInfo> &wadDownloadInfoList);
	void setupSitesUrls();
	void startNextIdgamesClient();
	void startIdgames();
	void stopIdgames();

private slots:
	void fileLinkFound(const QString &filename, const QUrl &url);
	void fileMirrorLinksFound(const QString &filename, const QList<QUrl> &urls);
	void idgamesClientFinished(Idgames *pEmitter);
	void wadRetrieverDownloadFinished(WadDownloadInfo wadDownloadInfo);
	void wadRetrieverDownloadProgress(WadDownloadInfo wadDownloadInfo, qint64 current, qint64 total);
	void wadRetrieverDownloadStarted(WadDownloadInfo wadDownloadInfo, const QUrl &url);
	void wadRetrieverFinished();
	void wadRetrieverMessage(const QString &message, WadseekerLib::MessageType type);
	void wadRetrieverWadInstalled(WadDownloadInfo wadDownloadInfo);
	void wwwSeekerFinished();
};

#endif
