//------------------------------------------------------------------------------
// seeksession.cpp
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
#include "seeksession.h"

#include "wwwseeker/entities/fileseekinfo.h"
#include "wwwseeker/idgames.h"
#include "wwwseeker/urlparser.h"
#include "wwwseeker/wwwseeker.h"
#include "wadretriever/wadretriever.h"
#include "wadseeker.h"
#include "wadseekerversioninfo.h"

#include <QScopedPointer>
#include <QSharedPointer>

/*
  SeekParameters
*/

SeekParameters::SeekParameters()
{
	this->bIdgamesEnabled = true;
	this->idgamesUrl = Idgames::defaultIdgamesUrl();
	this->maxConcurrentDownloads = 3;
	this->maxConcurrentSeeks = 3;
	this->sitesUrls = Wadseeker::defaultSitesListEncoded();
}

/*
  SeekSession
*/
DClass<SeekSession>
{
public:
	SeekParameters params;
	bool aborting;
	/**
	 * @brief Idgames objects that will handle each file
	 *        separately.
	 *
	 * Each file is probed one-at-a-time on the IDGames page. This
	 * was designed to prevent creating too many request to the
	 * IDGames page.
	 */
	QList<QSharedPointer<Idgames>> idgamesClients;
	QScopedPointer<WadRetriever> wadRetriever;
	QScopedPointer<WWWSeeker> wwwSeeker;
};
DPointeredNoCopy(SeekSession);

SeekSession::SeekSession(SeekParameters params)
{
	d->params = params;
	d->aborting = false;

	// WWWSeeker
	d->wwwSeeker.reset(new WWWSeeker);
	d->wwwSeeker->setUserAgent(WadseekerVersionInfo::userAgent());

	auto *wwwSeeker = d->wwwSeeker.get();

	// Connect signals to slots.
	connect(wwwSeeker, &WWWSeeker::finished,
		this, &SeekSession::wwwSeekerFinished, Qt::QueuedConnection);
	connect(wwwSeeker, &WWWSeeker::linkFound,
		this, &SeekSession::fileLinkFound, Qt::QueuedConnection);

	// Forward the signals.
	connect(wwwSeeker, &WWWSeeker::message,
		this, &SeekSession::message);
	connect(wwwSeeker, &WWWSeeker::siteFinished,
		this, &SeekSession::siteFinished, Qt::QueuedConnection);
	connect(wwwSeeker, &WWWSeeker::siteProgress,
		this, &SeekSession::siteProgress, Qt::QueuedConnection);
	connect(wwwSeeker, &WWWSeeker::siteRedirect,
		this, &SeekSession::siteRedirect, Qt::QueuedConnection);
	connect(wwwSeeker, &WWWSeeker::siteStarted,
		this, &SeekSession::siteStarted, Qt::QueuedConnection);

	// WadRetriever
	d->wadRetriever.reset(new WadRetriever);
	d->wadRetriever->setUserAgent(WadseekerVersionInfo::userAgent());

	auto *wadRetriever = d->wadRetriever.get();
	connect(wadRetriever, &WadRetriever::badUrlDetected,
		this, &SeekSession::badUrlDetected);
	// The wadRetrieverFinished() slot will handle difference between finished()
	// and pendingUrls() signals.
	connect(wadRetriever, &WadRetriever::finished,
		this, &SeekSession::wadRetrieverFinished, Qt::QueuedConnection);
	connect(wadRetriever, &WadRetriever::pendingUrls,
		this, &SeekSession::wadRetrieverFinished, Qt::QueuedConnection);

	connect(wadRetriever, &WadRetriever::message,
		this, &SeekSession::wadRetrieverMessage);
	connect(wadRetriever, &WadRetriever::wadDownloadFinished,
		this, &SeekSession::wadRetrieverDownloadFinished);
	connect(wadRetriever, &WadRetriever::wadDownloadStarted,
		this, &SeekSession::wadRetrieverDownloadStarted);
	connect(wadRetriever, &WadRetriever::wadDownloadProgress,
		this, &SeekSession::wadRetrieverDownloadProgress);
	connect(wadRetriever, &WadRetriever::wadInstalled,
		this, &SeekSession::wadRetrieverWadInstalled);
}

SeekSession::~SeekSession()
{
}

void SeekSession::abort()
{
	if (!d->aborting)
	{
		d->aborting = true;
		abortSeekers();
		d->wadRetriever->abort();
	}
}

void SeekSession::abortSeekers()
{
	stopIdgames();
	d->wwwSeeker->abort();
}

void SeekSession::cleanUpIfAllFinished()
{
	if (isAllFinished())
	{
		cleanUpAfterFinish();
	}
}

void SeekSession::cleanUpAfterFinish()
{
	// If there are no more WAD downloads pending for URLs when finish is
	// announced, then the SeekSession procedure is a success.
	SeekResult result = SeekResult::Success;
	if (d->aborting)
		result = SeekResult::Aborted;
	else if (d->wadRetriever->isAnyWadPendingUrl())
		result = SeekResult::Failed;
	emit seekFinished(result);
}

void SeekSession::fileLinkFound(const QString &filename, const QUrl &url)
{
	emit message(tr("Found link to file \"%1\": %2").arg(filename)
		.arg(url.toEncoded().constData()), WadseekerLib::Notice);

	d->wadRetriever->addUrl(d->params.seekedWads.findFileName(filename), url);
}

void SeekSession::fileMirrorLinksFound(const QString &filename, const QList<QUrl> &urls)
{
	emit message(tr("Found mirror links to file \"%1\":").arg(filename), WadseekerLib::Notice);
	for (const QUrl &url : urls)
	{
		QString strUrl = url.toEncoded().constData();
		emit message(tr("    %2").arg(strUrl), WadseekerLib::Notice);
	}

	d->wadRetriever->addMirrorUrls(filename, urls);
}

void SeekSession::idgamesClientFinished(Idgames *pEmitter)
{
	emit message(tr("IDGames client for file \"%1\" has finished->").arg(pEmitter->file().name()),
		WadseekerLib::NoticeImportant);

	for (auto idgames : d->idgamesClients)
	{
		if (idgames.get() == pEmitter)
		{
			d->idgamesClients.removeOne(idgames);
			break;
		}
	}

	if (!d->aborting)
	{
		startNextIdgamesClient();
	}

	if (d->idgamesClients.isEmpty())
	{
		emit serviceFinished("idgames");
	}
	cleanUpIfAllFinished();
}

bool SeekSession::isAllFinished() const
{
	return !isWorking();
}

bool SeekSession::isDownloadingFile(const ModFile &file) const
{
	return d->wadRetriever->isDownloadingWad(file.fileName());
}

bool SeekSession::isWorking() const
{
	return d->wwwSeeker->isWorking()
		|| d->wadRetriever->isAnyDownloadWorking()
		|| !d->idgamesClients.isEmpty();
}

void SeekSession::setupIdgamesClients(const QList<WadDownloadInfo> &wadDownloadInfoList)
{
	for (const WadDownloadInfo &wad : wadDownloadInfoList)
	{
		QSharedPointer<Idgames> idgames(new Idgames(d->params.idgamesUrl));

		idgames->setUserAgent(WadseekerVersionInfo::userAgent());
		idgames->setFile(wad);

		auto *pIdgames = idgames.get();
		connect(pIdgames, &Idgames::finished,
			this, &SeekSession::idgamesClientFinished);

		connect(pIdgames, &Idgames::message,
			this, &SeekSession::message);

		connect(pIdgames, &Idgames::fileLinksFound,
			this, &SeekSession::fileMirrorLinksFound);

		// Forward signals as with WWWSeeker
		connect(pIdgames, &Idgames::siteFinished,
			this, &SeekSession::siteFinished, Qt::QueuedConnection);
		connect(pIdgames, &Idgames::siteProgress,
			this, &SeekSession::siteProgress, Qt::QueuedConnection);
		connect(pIdgames, &Idgames::siteStarted,
			this, &SeekSession::siteStarted, Qt::QueuedConnection);

		d->idgamesClients << idgames;
	}
}

void SeekSession::setupSitesUrls()
{
	d->wwwSeeker->clearVisitedUrlsList();

	auto incorporateUrl = [&] (bool isCustom, const QString &strSiteUrl)
		{
			if (UrlParser::isWadnameTemplateUrl(strSiteUrl))
			{
				for (const ModFile &wad : d->params.seekedWads.modFiles())
				{
					QUrl url = UrlParser::resolveWadnameTemplateUrl(strSiteUrl, wad.fileName());
					if (url.isValid())
					{
						const int priority = isCustom ?
							WWWSeeker::PRIORITY_HIGH : WWWSeeker::PRIORITY_NORMAL;
						d->wwwSeeker->addFileSiteUrlWithPriority(wad.fileName(), url, priority);
					}
				}
			}
			else
			{
				QUrl url(strSiteUrl);
				if (url.isValid())
				{
					if (isCustom)
						d->wwwSeeker->addCustomSiteUrl(url);
					else
						d->wwwSeeker->addSiteUrl(url);
				}
			}
		};

	for (const auto &urlStr : d->params.customSiteUrls)
		incorporateUrl(true, urlStr);

	for (const QString &strSiteUrl : d->params.sitesUrls)
		incorporateUrl(false, strSiteUrl);
}

void SeekSession::skipFileCurrentUrl(const QString &fileName)
{
	d->wadRetriever->skipCurrentUrl(fileName);
}

void SeekSession::skipService(const QString &service)
{
	if (service == "idgames")
	{
		stopIdgames();
	}
	else
	{
		emit message(tr("Attempted to abort unknown service '%1'").arg(service),
			WadseekerLib::Error);
	}
}

void SeekSession::skipSiteSeek(const QUrl &url)
{
	d->wwwSeeker->skipSite(url);
}

void SeekSession::startIdgames()
{
	emit serviceStarted("idgames");
	startNextIdgamesClient();
}

void SeekSession::startNextIdgamesClient()
{
	if (!d->aborting && !d->idgamesClients.isEmpty())
	{
		QSharedPointer<Idgames> idgames = d->idgamesClients.first();
		idgames->startSearch();
	}
	cleanUpIfAllFinished();
}

void SeekSession::stopIdgames()
{
	if (d->idgamesClients.isEmpty())
	{
		return;
	}
	QSharedPointer<Idgames> current = d->idgamesClients.takeFirst();
	d->idgamesClients.clear();
	current->abort();
}

void SeekSession::startSeek()
{
	// Validate the parameters; mutate them if necessary.
	if (d->params.saveDirectoryPath.isEmpty())
	{
		emit message(tr("No target directory specified! Aborting."), WadseekerLib::CriticalError);
		emit seekFinished(SeekResult::Error);
		return;
	}

	QDir targetDir(d->params.saveDirectoryPath);
	if (!targetDir.exists())
	{
		emit message(tr("The destination directory \"%1\" doesn't exist or is a file itself. "
				"Aborting.").arg(d->params.saveDirectoryPath),
			WadseekerLib::CriticalError);
		emit seekFinished(SeekResult::Error);
		return;
	}

	ModSet filteredWadsList;
	for (ModFile wad : d->params.seekedWads.modFiles())
	{
		if (wad.fileName().isEmpty())
			continue;

		if (Wadseeker::isForbiddenWad(wad))
		{
			emit message(tr("WAD \"%1\" is on the forbidden list. "
					"Wadseeker will not download this WAD.").arg(wad.fileName()),
				WadseekerLib::Error);
			continue;
		}

		filteredWadsList.addModFile(wad);
	}

	if (filteredWadsList.isEmpty())
	{
		emit message(tr("Cannot start search. No WADs to seek."), WadseekerLib::CriticalError);
		emit seekFinished(SeekResult::Error);
		return;
	}
	d->params.seekedWads = filteredWadsList;

	// Setup the session's objects.
	setupSitesUrls();

	QList<FileSeekInfo> fileSeekInfosList;
	QList<WadDownloadInfo> wadDownloadInfoList;

	for (const ModFile &wad : filteredWadsList.modFiles())
	{
		// Create download info object for this WAD.
		WadDownloadInfo wadDownloadInfo(wad);
		wadDownloadInfoList << wadDownloadInfo;

		// Generate all possible filenames.
		QStringList possibleFilenames;
		possibleFilenames << wadDownloadInfo.possibleArchiveNames();
		possibleFilenames << wadDownloadInfo.name();

		fileSeekInfosList << FileSeekInfo(wad.fileName(), possibleFilenames);
		emit message(tr("WAD %1: %2").arg(wad.fileName(), fileSeekInfosList.last().possibleFilenames().join(", ")), WadseekerLib::Notice);
	}

	if (d->params.bIdgamesEnabled)
	{
		setupIdgamesClients(wadDownloadInfoList);
	}

	d->wadRetriever->setMaxConcurrentWadDownloads(d->params.maxConcurrentDownloads);
	d->wadRetriever->setTargetSavePath(d->params.saveDirectoryPath);
	d->wadRetriever->setWads(wadDownloadInfoList);

	d->wwwSeeker->setMaxConcurrectSiteDownloads(d->params.maxConcurrentSeeks);

	// Start doing the job.
	emit seekStarted(filteredWadsList);
	d->wwwSeeker->startSearch(fileSeekInfosList);
	if (d->params.bIdgamesEnabled)
	{
		startIdgames();
	}
}

void SeekSession::wadRetrieverDownloadFinished(WadDownloadInfo wadDownloadInfo)
{
	emit fileDownloadFinished(wadDownloadInfo.name());
}

void SeekSession::wadRetrieverDownloadProgress(WadDownloadInfo wadDownloadInfo, qint64 current, qint64 total)
{
	emit fileDownloadProgress(wadDownloadInfo.name(), current, total);
}

void SeekSession::wadRetrieverDownloadStarted(WadDownloadInfo wadDownloadInfo, const QUrl &url)
{
	emit message(tr(R"(Started downloading file "%1" from URL "%2")")
		.arg(wadDownloadInfo.name(), url.toString()), WadseekerLib::Notice);
	emit fileDownloadStarted(wadDownloadInfo.name(), url);
}

void SeekSession::wadRetrieverFinished()
{
	if (!isAllFinished())
	{
		if (d->wadRetriever->areAllWadsPendingUrls())
		{
			emit message(tr("WadRetriever is pending for download URLs."), WadseekerLib::Notice);
		}
		else
		{
			abortSeekers();
			emit message(tr("WadRetriever is finished."), WadseekerLib::NoticeImportant);
		}
	}
	cleanUpIfAllFinished();
}

void SeekSession::wadRetrieverMessage(const QString &message, WadseekerLib::MessageType type)
{
	emit this->message(message, type);

	if (type == WadseekerLib::CriticalError)
	{
		abort();
	}
}

void SeekSession::wadRetrieverWadInstalled(WadDownloadInfo wadDownloadInfo)
{
	if (wadDownloadInfo.size() > 0)
	{
		// If size of the file is known it is a good idea to emit it now.
		emit fileDownloadProgress(wadDownloadInfo.name(), wadDownloadInfo.size(), wadDownloadInfo.size());
	}
	emit fileInstalled(wadDownloadInfo.name());

	d->wwwSeeker->removeSeekedFile(wadDownloadInfo.name());
}

void SeekSession::wwwSeekerFinished()
{
	emit message("WWWSeeker is finished.", WadseekerLib::NoticeImportant);
	cleanUpIfAllFinished();
}
