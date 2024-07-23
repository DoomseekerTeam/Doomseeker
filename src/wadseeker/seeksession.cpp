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

#include "protocols/idgames/idgames.h"
#include "wadinstaller.h"
#include "wadseeker.h"
#include "wadseekermessagetype.h"
#include "wadseekerversioninfo.h"
#include "www/urlmirror.h"
#include "www/urlparser.h"
#include "www/urlset.h"
#include "www/webcrawl.h"
#include "www/webquery.h"
#include "www/www.h"
#include "wwwseeker/fileseekinfo.h"
#include "wwwseeker/linkseeker.h"
#include "wwwseeker/linkresourcematcher.h"

#include <QList>
#include <QMap>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QStringList>
#include <QString>
#include <QTimer>
#include <QUrl>

#include <algorithm>
#include <cassert>
#include <functional>

static const int MAX_CRAWL_LENGTH = 10;

static const int PRIORITY_CUSTOM_URL = 0;
static const int PRIORITY_REDIRECT_URL = 1;
static const int PRIORITY_DOWNLOAD_URL = 2;
static const int PRIORITY_CRAWL_URL = 3;
static const int PRIORITY_SITE_URL = 4;

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
struct Inflight
{
	CrawlGoal goal;
	WebCrawl crawl;
	QSharedPointer<WebQuery> query;

	const QString &resource() const { return crawl.resource(); }
	int priority() const { return crawl.priority(); }
};

namespace
{
static const QString SITE_RESOURCE = "";

class FileSeekInfoList : public QList<FileSeekInfo>
{
public:
	FileSeekInfoList::iterator findByName(const QString &name)
	{
		auto it = this->begin();
		for (; it != this->end() && it->file() != name; ++it);
		return it;
	}
};

class InflightList : public QList<Inflight>
{
public:
	unsigned countByGoal(CrawlGoal goal) const
	{
		unsigned total = 0;
		for (const auto &item : *this)
		{
			if (item.goal == goal)
				++total;
		}
		return total;
	}

	InflightList::iterator findByResource(const QString &resource)
	{
		auto it = this->begin();
		for (; it != this->end() && it->resource() != resource; ++it);
		return it;
	}

	InflightList::iterator findByQuery(WebQuery *query)
	{
		auto it = this->begin();
		for (; it != this->end() && it->query.data() != query; ++it);
		return it;
	}

	InflightList::iterator findByUrl(const QUrl &url)
	{
		auto it = this->begin();
		for (; it != this->end() && it->query->url() != url; ++it);
		return it;
	}

	void removeQuery(WebQuery *query)
	{
		for (auto it = this->begin(); it != this->end(); ++it)
		{
			if (it->query.data() == query)
			{
				this->erase(it);
				return;
			}
		}
	}
};

class WadDownloadInfoList : public QList<WadDownloadInfo>
{
public:
	WadDownloadInfoList::iterator findByName(const QString &name)
	{
		auto it = this->begin();
		for (; it != this->end() && it->name() != name; ++it);
		return it;
	}
};

enum class SeekStatus
{
	Pending, Running, Finished
};
}

DClass<SeekSession>
{
public:
	QNetworkAccessManager *net;
	SeekParameters params;
	bool aborting;
	SeekStatus status;
	/**
	 * @brief Idgames objects that will handle each file
	 *        separately.
	 *
	 * Each file is probed one-at-a-time on the IDGames page. This
	 * was designed to prevent creating too many request to the
	 * IDGames page.
	 */
	QList<QSharedPointer<Idgames>> idgamesClients;

	// The seekers.
	QScopedPointer<WWW> www;
	QScopedPointer<LinkSeeker> linkSeeker;

	/**
	 * A crawler timer is needed to delay call to crawl() to the next main loop
	 * iteration.
	 *
	 * - Sometimes crawl() may be called several times in succession, so
	 *   using timer limits the crawl() to run only once.
	 *
	 * - crawl() may lead to finished() emission and object destruction,
	 *   so it's best to defer this, lest we wish to be caught with a
	 *   destroyed object in the middle of some method..
	 */
	QTimer crawler;

	// The seeked items.
	FileSeekInfoList seekedFiles;
	WadDownloadInfoList wads;

	// The seeks in motion.
	UrlSet urlHistory;
	UrlMirrorList mirrors;
	WebCrawlList crawls;
	InflightList inflight;
};
DPointeredNoCopy(SeekSession);

SeekSession::SeekSession(QNetworkAccessManager *net, SeekParameters params)
{
	assert(net != nullptr);
	d->net = net;
	d->params = params;
	d->aborting = false;
	d->status = SeekStatus::Pending;

	// WWW
	d->www.reset(new WWW(net));
	d->www->setUserAgent(WadseekerVersionInfo::userAgent());

	// LinkSeeker
	d->linkSeeker.reset(new LinkSeeker);
	connect(d->linkSeeker.data(), &LinkSeeker::finished,
		this, &SeekSession::onLinkSeekFinished);
	connect(d->linkSeeker.data(), &LinkSeeker::linkFound,
		this, &SeekSession::onLinkSeekFound);

	// Crawler
	d->crawler.setSingleShot(true);
	connect(&d->crawler, &QTimer::timeout,
		this, &SeekSession::crawl);

	// Connect self to emit navigation messages.
	connect(this, &SeekSession::siteStarted,
		this, [this](const QUrl &url)
		{ emit message(tr("Site started: %1").arg(url.toString()), WadseekerLib::Navigation); });
	connect(this, &SeekSession::siteFinished,
		this, [this](const QUrl &url)
		{ emit message(tr("Site finished: %1").arg(url.toString()), WadseekerLib::Navigation); });
	connect(this, &SeekSession::siteRedirect,
		this, [this](const QUrl &oldUrl, const QUrl &newUrl)
		{ emit message(tr("Site redirect: %1 -> %2")
			.arg(oldUrl.toString())
			.arg(newUrl.toString()), WadseekerLib::Navigation); });
	connect(this, &SeekSession::fileDownloadStarted,
		this, [this](const ModFile &file, const QUrl &url)
		{ emit message(tr("File \"%1\" download started: %2")
			.arg(file.fileName())
			.arg(url.toString()), WadseekerLib::Navigation); });
	connect(this, &SeekSession::fileDownloadFinished,
		this, [this](const ModFile &file)
		{ emit message(tr("File \"%1\" download finished")
			.arg(file.fileName()), WadseekerLib::Navigation); });

}

SeekSession::~SeekSession()
{
}

void SeekSession::abort()
{
	if (!d->aborting)
	{
		emit message(tr("Aborting ..."), WadseekerLib::NoticeImportant);
		d->aborting = true;
		this->stopAll();
		d->crawler.start(0);
	}
}

void SeekSession::concludeIfAllFinished()
{
	if (isAllFinished())
		conclude();
}

void SeekSession::conclude()
{
	// If there are no more WAD downloads pending for URLs when finish is
	// announced, then the SeekSession procedure is a success.
	SeekResult result = SeekResult::Success;
	if (d->aborting)
		result = SeekResult::Aborted;
	else if (!isEverythingFound())
		result = SeekResult::Failed;
	this->finish(result);
}

void SeekSession::crawl()
{
	if (d->status != SeekStatus::Running)
		return;

	if (isAllFinished())
	{
		conclude();
		return;
	}

	if (d->aborting)
		return;

	// Everything was found, abort all running and pending ops.
	if (isEverythingFound())
	{
		d->crawls.clear();
		stopAll();
	}

	if (!isEverythingFound())
	{
		if (d->inflight.countByGoal(CrawlGoal::Download) < d->params.maxConcurrentDownloads)
		{
			startNextCrawl(CrawlGoal::Download);
		}
		if (d->inflight.countByGoal(CrawlGoal::Seek) < d->params.maxConcurrentSeeks)
		{
			startNextCrawl(CrawlGoal::Seek);
		}
	}

	// It's possible that all crawling attempts have failed and all
	// queues for this session are now spent without any query being
	// in flight. We should check for that and finish the session.
	concludeIfAllFinished();
}

void SeekSession::fileMirrorLinksFound(const QString &filename, QList<QUrl> urls)
{
	urls = d->urlHistory.sieve(urls);

	if (!urls.isEmpty())
	{
		emit message(tr("Found mirror links to file \"%1\":").arg(filename), WadseekerLib::Notice);
		for (const QUrl &url : urls)
		{
			QString strUrl = url.toEncoded().constData();
			emit message(tr("    %2").arg(strUrl), WadseekerLib::Notice);
		}

		d->mirrors.addMirrorUrls(urls, filename);
		for (const QUrl &url : urls)
		{
			// Technically these WebCrawls also have some history, but currently
			// they are only called from the Idgames clients, so I suppose we
			// can treat them as "starting from scratch".
			d->crawls << WebCrawl(url, filename, PRIORITY_DOWNLOAD_URL);
		}
		d->crawler.start(0);
	}
}

void SeekSession::finish(SeekResult result)
{
	d->status = SeekStatus::Finished;
	emit seekFinished(result);
}

void SeekSession::idgamesClientFinished(Idgames *pEmitter)
{
	emit message(tr("IDGames client for file \"%1\" has finished->").arg(pEmitter->file().name()),
		WadseekerLib::NoticeImportant);

	for (auto idgames : d->idgamesClients)
	{
		if (idgames.data() == pEmitter)
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
	d->crawler.start(0);
}

bool SeekSession::isAllFinished() const
{
	return !isWorking();
}

bool SeekSession::isEverythingFound() const
{
	return d->seekedFiles.isEmpty();
}

bool SeekSession::isDownloadingFile(const ModFile &file) const
{
	return d->inflight.findByResource(file.fileName()) != d->inflight.end();
}

bool SeekSession::isWorking() const
{
	if (!d->inflight.isEmpty()
		|| !d->idgamesClients.isEmpty())
	{
		return true;
	}

	if (d->aborting || this->isEverythingFound())
		return false;

	return !d->crawls.isEmpty()
		|| d->linkSeeker->isWorking();
}

void SeekSession::onMessage(const QString &msg, WadseekerLib::MessageType type)
{
	emit message(msg, type);
	if (type == WadseekerLib::CriticalError)
		this->abort();
}

void SeekSession::onLinkSeekFinished()
{
	d->crawler.start(0);
}

void SeekSession::onLinkSeekFound(const WebCrawl &crawl, const Link &link)
{
	QList<LinkResourceMatch> matches = LinkResourceMatcher::matchLink(d->seekedFiles, link);
	for (const LinkResourceMatch &match : matches)
	{
		QUrl linkUrl = crawl.url().resolved(match.link.url);

		// Quietly ignore /non-kosher/ URLs.
		if (!UrlParser::isWwwUrl(linkUrl))
			return;

		emit message(tr("Found possible link to file \"%1\": %2")
			.arg(match.resource)
			.arg(linkUrl.toString()),
			WadseekerLib::Notice);

		d->crawls << WebCrawl(linkUrl, match.resource, PRIORITY_CRAWL_URL, crawl.history());
		d->crawler.start(0);
	}
}

void SeekSession::onWwwError(WebQuery *query)
{
	if (query->error() == QNetworkReply::OperationCanceledError)
	{
		emit message(tr("Site aborted: %1").arg(query->url().toString()),
			WadseekerLib::Notice);
	}
	else
	{
		emit message(tr("Site failed \"%1\": %2")
			.arg(query->url().toString())
			.arg(query->errorString()),
			WadseekerLib::Error);
	}
}

void SeekSession::onWwwFinished(WebQuery *query)
{
	InflightList::iterator inflightIt = d->inflight.findByQuery(query);
	emit siteFinished(query->url());

	bool slurped = false; // has the query been used in any manner
	// TODO refactor the Khufu Pyramid here.
	if (inflightIt != d->inflight.end())
	{
		Inflight inflight = *inflightIt;
		// Get what we want from the inflight and remove it now.
		QString resource = inflight.resource();
		QSharedPointer<WebQuery> query = inflight.query;
		d->inflight.erase(inflightIt);

		// If we're aborting, do nothing about the query.
		if (d->aborting)
		{
			slurped = true;
		}

		// Check if the query finished properly before we process it.
		if (!slurped && query->error() != QNetworkReply::NoError)
		{
			slurped = true;
		}

		// Check for a redirect.
		if (!slurped && query->isRedirected())
		{
			// Redirect URL doesn't go through the queue because the
			// queue never accepts duplicates and there was at least one
			// case where a redirect would lead to the same page but
			// with different attachments.

			// If redirect was from a custom site, it must be on the
			// same priority. If it was from a regular site, we should
			// treat it as slightly more important than the original
			// request.
			int newPriority = inflight.priority() <= PRIORITY_CUSTOM_URL
				? inflight.priority()
				: PRIORITY_REDIRECT_URL;

			// The 'dest' URL is already resolved and absolute.
			d->crawls << WebCrawl(query->redirectUrl(), inflight.resource(),
				newPriority, inflight.crawl.history());
			slurped = true;
		}

		// We got some kind of resource.
		if (!slurped)
		{
			// Handle the query differently depending on the resource type it
			// carries. Does this beg for a Strategy design pattern?
			switch (query->resourceType())
			{
			case WebResourceType::Binary:
				// If the WebQuery is downloading something of binary type, we
				// may be interested in it, but only if we know it leads to one
				// of the resources we seek.
				if (!resource.isEmpty() && d->seekedFiles.contains(resource))
				{
					// Are we still looking for this resource?
					//
					// TODO even if we don't, but the downloaded item is an
					// archive, we could try to look into it and see if other
					// seeked resources are inside.
					WadDownloadInfoList::iterator wad = d->wads.findByName(resource);
					if (wad != d->wads.end())
					{
						emit message(tr("Attempting to install file %1 of size %2 from URL: %3")
							.arg(resource)
							.arg(query->reply().size())
							.arg(query->url().toString()),
							WadseekerLib::Notice);

						// Recollect the list of the resources, but only those
						// that haven't been found, yet.
						WadDownloadInfoList stillSeekedWads;
						for (const WadDownloadInfo &wad : d->wads)
						{
							if (d->seekedFiles.findByName(wad.name()) != d->seekedFiles.end())
							{
								stillSeekedWads << wad;
							}
						}

						// Attempt install.
						WadInstaller installer(d->params.saveDirectoryPath, stillSeekedWads);
						WadInstallerResult result = installer.install(*wad, query->reply());
						if (result.isError())
						{
							// The file was downloaded just fine, but failed
							// to install, so we should assume that all mirrors
							// have the same file with the same exact problem.
							// That's what mirrors are for, after all.
							QList<UrlMirror> mirrors = d->mirrors.mirrorsWithUrl(query->url());
							for (const UrlMirror &mirror : mirrors)
							{
								for (const QUrl &url : mirror.urls)
								{
									d->urlHistory.insert(url);
									auto crawl = d->crawls.findByUrl(url);
									if (crawl != d->crawls.end())
										d->crawls.erase(crawl);
								}
							}

							emit message(tr("Failed to install file \"%1\" from URL %2: %3")
								.arg(resource)
								.arg(query->url().toString())
								.arg(result.error),
								result.isCriticalError() ? WadseekerLib::CriticalError : WadseekerLib::Error);
						}
						else
						{
							// All files that were successfully installed are removed from the seek.
							for (QFileInfo wadInfo : result.installedWads)
							{
								FileSeekInfoList::iterator seekedFile = d->seekedFiles.findByName(wadInfo.fileName());
								if (seekedFile != d->seekedFiles.end())
									d->seekedFiles.erase(seekedFile);
								emit message(tr("Installed WAD: %1 [%2 bytes]")
									.arg(wadInfo.fileName())
									.arg(wadInfo.size()),
									WadseekerLib::Notice);
								emit fileInstalled(wadInfo.fileName());
							}
						}
						slurped = true;
					}
				}
				break;
			case WebResourceType::Html:
				// The slurping began here as soon as the content type became known.
				slurped = true;
				break;
			case WebResourceType::Unknown:
				// Wadseeker deals only with a limited amount of Web resources.
				break;
			}
		}
	}

	if (!slurped)
	{
		// If query is finished and its content has not been recognized,
		// then we're not interested in the result of this query.
		emit message(tr("No interesting content at URL: %1")
			.arg(query->url().toString()), WadseekerLib::Notice);
	}

	d->crawler.start(0);
}

void SeekSession::onWwwFoundType(WebQuery *query, WebResourceType type)
{
	auto flight = d->inflight.findByQuery(query);
	WebCrawl crawl = flight != d->inflight.end() ? flight->crawl : query->url();

	// As soon as we know the resource's type, we can start processing it.
	switch (type)
	{
	case WebResourceType::Binary:
		// Cool, continue.
		break;
	case WebResourceType::Html:
		d->linkSeeker->acceptHtml(crawl, &query->reply());
		connect(&query->reply(), &QNetworkReply::finished,
			d->linkSeeker.data(), std::bind(&LinkSeeker::conclude, d->linkSeeker.data(), &query->reply()));
		break;
	default:
		// No point in downloading the query further.
		emit message(tr("Wadseeker is unable to deal with "
			"the data at %1").arg(query->url().toString()),
			WadseekerLib::Error);
		query->abort();
		break;
	}
}

void SeekSession::onWwwProgress(WebQuery *query, qint64 bytesReceived, qint64 bytesTotal)
{
	InflightList::iterator inflight = d->inflight.findByQuery(query);
	if (inflight != d->inflight.end())
	{
		// Wadseeker makes a hard distinction between getting websites
		// websites and getting files and informs the user by different
		// signals.
		if (inflight->resource().isEmpty())
		{
			emit siteProgress(inflight->query->url(), bytesReceived, bytesTotal);
		}
		else
		{
			emit fileDownloadProgress(inflight->resource(), bytesReceived, bytesTotal);
		}
	}
}

void SeekSession::setupIdgamesClients(const QList<WadDownloadInfo> &wadDownloadInfoList)
{
	for (const WadDownloadInfo &wad : wadDownloadInfoList)
	{
		QSharedPointer<Idgames> idgames(new Idgames(d->params.idgamesUrl));

		idgames->setUserAgent(WadseekerVersionInfo::userAgent());
		idgames->setFile(wad);

		auto *pIdgames = idgames.data();
		connect(pIdgames, &Idgames::finished,
			this, &SeekSession::idgamesClientFinished);

		connect(pIdgames, &Idgames::message,
			this, &SeekSession::onMessage);

		connect(pIdgames, &Idgames::fileLinksFound,
			this, &SeekSession::fileMirrorLinksFound);

		// Forward signals
		connect(pIdgames, &Idgames::siteFinished,
			this, &SeekSession::siteFinished, Qt::QueuedConnection);
		connect(pIdgames, &Idgames::siteProgress,
			this, &SeekSession::siteProgress, Qt::QueuedConnection);
		connect(pIdgames, &Idgames::siteStarted,
			this, &SeekSession::siteStarted, Qt::QueuedConnection);

		d->idgamesClients << idgames;
	}
}

void SeekSession::skipFileCurrentUrl(const QString &fileName)
{
	for (Inflight &inflight : d->inflight)
	{
		if (inflight.resource() == fileName)
		{
			inflight.query->abort();
			break;
		}
	}
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
	d->urlHistory << url;

	auto crawl = d->crawls.findByUrl(url);
	if (crawl != d->crawls.end())
		d->crawls.erase(crawl);

	auto inflight = d->inflight.findByUrl(url);
	if (inflight != d->inflight.end())
		inflight->query->abort();
	else
		d->crawler.start(0);
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
	concludeIfAllFinished();
}

void SeekSession::stopAll()
{
	for (Inflight &inflight : d->inflight)
		inflight.query->abort();
	stopIdgames();
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

void SeekSession::startNextCrawl(CrawlGoal goal)
{
	// Sort the crawls by priority before going through them.
	std::sort(d->crawls.begin(), d->crawls.end(),
		[](const WebCrawl &c1, const WebCrawl &c2)
		{ return c1.priority() < c2.priority(); });
	for (auto it = d->crawls.begin(); it != d->crawls.end();)
	{
		WebCrawl &crawl = *it;

		// Silently skip already visited URLs.
		if (d->urlHistory.contains(crawl.url()))
		{
			it = d->crawls.erase(it);
			continue;
		}

		// Don't let the servers lead us on a goose chase.
		if (crawl.historySize() >= MAX_CRAWL_LENGTH)
		{
			emit message(tr("Too many site jumps in a Web crawl started at \"%1\"; "
				"aborting at \"%2\"; "
				"Did the server lead us on a goose chase?")
				.arg(it->history().first().toString())
				.arg(it->url().toString()), WadseekerLib::Error);
			it = d->crawls.erase(it);
			continue;
		}

		// Check if the URL is /kosher/.
		if (!UrlParser::isWwwUrl(crawl.url()))
		{
			// Take care to produce a precise error message ;)
			QString nonKosherUrlMsg;
			if (crawl.historySize() >= 2)
			{
				nonKosherUrlMsg = tr("Aborting Web crawl started at \"%1\", from \"%2\" to \"%3\"; "
						"Wadseeker is not handling URLs with scheme \"%4\".")
					.arg(crawl.history().first().toString())
					.arg(crawl.history()[crawl.historySize() - 2].toString())
					.arg(crawl.url().toString())
					.arg(crawl.url().scheme());
			}
			else if (crawl.historySize() == 2)
			{
				nonKosherUrlMsg = tr("Aborting Web crawl started at \"%1\" to \"%2\"; "
						"Wadseeker is not handling URLs with scheme \"%3\".")
					.arg(crawl.history().first().toString())
					.arg(crawl.url().toString())
					.arg(crawl.url().scheme());
			}
			else
			{
				nonKosherUrlMsg = tr("Aborting a Web crawl to \"%1\" ; "
						"Wadseeker is not handling URLs with scheme \"%2\".")
					.arg(crawl.url().toString())
					.arg(crawl.url().scheme());
			}
			emit message(nonKosherUrlMsg, WadseekerLib::Error);
			it = d->crawls.erase(it);
			continue;
		}

		// If we're running too many seeks at the moment, pause.
		int maxConcurrent = 1;
		switch (goal)
		{
		case CrawlGoal::Download:
			maxConcurrent = d->params.maxConcurrentDownloads;
			break;
		case CrawlGoal::Seek:
			maxConcurrent = d->params.maxConcurrentSeeks;
			break;
		default:
			maxConcurrent = 1;
			break;
		}
		if (d->inflight.countByGoal(goal) >= maxConcurrent)
			break;

		// Check if the current WebCrawl is appropriate for the current crawling
		// goal. Download crawls operate on resources with names.
		if ((goal == CrawlGoal::Download && crawl.resource().isEmpty())
			|| (goal == CrawlGoal::Seek && !crawl.resource().isEmpty()))
		{
			++it;
			continue;
		}

		// Don't start a query if there's another query running on this host.
		// Some WWW servers limit simultaneous connections from one client.
		if (d->www->isHostInflight(crawl.url()))
		{
			++it;
			continue;
		}

		// If this is a crawl with a resource, don't run it if another query is
		// running with that resource already. We may unnecessarily download the
		// same file more than once.
		if (!crawl.resource().isEmpty())
		{
			auto otherInflight = d->inflight.findByResource(crawl.resource());
			if (otherInflight != d->inflight.end())
			{
				++it;
				continue;
			}
		}

		// Finally, we can start the query.
		QSharedPointer<WebQuery> query = d->www->get(crawl.url());
		startFlight(Inflight {
			goal,
			crawl,
			query
		});
		it = d->crawls.erase(it);
	}
}

void SeekSession::startFlight(const Inflight &flight)
{
	using namespace std::placeholders;

	d->inflight << flight;
	d->urlHistory << flight.query->url();

	WebQuery *query = flight.query.data();
	connect(query, &WebQuery::errorOccurred,
		this, std::bind(&SeekSession::onWwwError, this, query));
	connect(query, &WebQuery::finished,
		this, std::bind(&SeekSession::onWwwFinished, this, query));
	connect(query, &WebQuery::foundType,
		this, std::bind(&SeekSession::onWwwFoundType, this, query, _1));
	connect(query, &WebQuery::progress,
		this, std::bind(&SeekSession::onWwwProgress, this, query, _1, _2));

	if (flight.resource() != WebCrawl::UNNAMED_RESOURCE)
		emit fileDownloadStarted(flight.resource(), flight.query->url());
	emit siteStarted(query->url());

	// Check again if the URL is /kosher/.
	//
	// Nevertheless, if we reached this location, then the query is
	// running already, and that's too late to do this check. The damage
	// has been done. There was supposed to be a check somewhere before
	// here, but it wasn't performed.
	if (!UrlParser::isWwwUrl(flight.query->url()))
	{
		emit message(tr("Abort URL: \"%1\"; "
				"Wadseeker is not handling URLs with scheme \"%2\".")
			.arg(flight.query->url().toString())
			.arg(flight.query->url().scheme()),
			WadseekerLib::Error);
		flight.query->abort();
	}
}

void SeekSession::startSeek()
{
	if (d->status != SeekStatus::Pending)
	{
		emit message(tr("Seek session was run already. Cannot run again. "
			"Create a new session instead."), WadseekerLib::CriticalError);
		this->finish(SeekResult::Error);
		return;
	}

	// Validate the parameters; mutate them if necessary.
	if (d->params.saveDirectoryPath.isEmpty())
	{
		emit message(tr("No target directory specified! Aborting."), WadseekerLib::CriticalError);
		this->finish(SeekResult::Error);
		return;
	}

	QDir targetDir(d->params.saveDirectoryPath);
	if (!targetDir.exists())
	{
		emit message(tr("The destination directory \"%1\" doesn't exist or is a file itself. "
				"Aborting.").arg(d->params.saveDirectoryPath),
			WadseekerLib::CriticalError);
		this->finish(SeekResult::Error);
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
		this->finish(SeekResult::Error);
		return;
	}
	d->params.seekedWads = filteredWadsList;

	// Setup the initial URLs.
	auto addUrl = [&](QString urlStr, int priority)
	{
		if (UrlParser::isWadnameTemplateUrl(urlStr))
		{
			for (const ModFile &wad : d->params.seekedWads.modFiles())
			{
				QUrl url = UrlParser::resolveWadnameTemplateUrl(urlStr, wad.fileName());
				if (url.isValid())
					d->crawls << WebCrawl(url, wad.fileName(), priority);
			}
		}
		else
		{
			QUrl url(urlStr);
			if (url.isValid())
				d->crawls << WebCrawl(url, WebCrawl::UNNAMED_RESOURCE, priority);
		}
	};

	for (QString url : d->params.customSiteUrls)
		addUrl(url, PRIORITY_CUSTOM_URL);
	for (QString url : d->params.sitesUrls)
		addUrl(url, PRIORITY_SITE_URL);

	// TODO see if we need both structures
	FileSeekInfoList fileSeekInfosList;
	WadDownloadInfoList wadDownloadInfoList;
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
	}
	d->seekedFiles = fileSeekInfosList;
	d->wads = wadDownloadInfoList;

	if (d->params.bIdgamesEnabled)
	{
		setupIdgamesClients(wadDownloadInfoList);
	}

	// Start doing the job.
	d->status = SeekStatus::Running;
	emit seekStarted(filteredWadsList);
	for (const ModFile &wad : d->wads)
	{
		auto seeked = d->seekedFiles.findByName(wad.fileName());
		if (seeked != d->seekedFiles.end())
		{
			emit message(tr("WAD %1: %2")
				.arg(wad.fileName())
				.arg(seeked->possibleFilenames().join(", ")), WadseekerLib::Notice);
		}
		else
		{
			emit message(tr("WAD %1").arg(wad.fileName()), WadseekerLib::Notice);
		}
	}

	if (d->params.bIdgamesEnabled)
	{
		startIdgames();
	}
	d->crawler.start(0);
}
