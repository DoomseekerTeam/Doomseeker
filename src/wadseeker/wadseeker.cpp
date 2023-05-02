//------------------------------------------------------------------------------
// wadseeker.cpp
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
#include "entities/modfile.h"
#include "entities/modset.h"
#include "entities/waddownloadinfo.h"
#include "protocols/wadarchive/wadarchiveclient.h"
#include "wadretriever/wadretriever.h"
#include "wadseeker.h"
#include "wadseekerversioninfo.h"
#include "wwwseeker/entities/fileseekinfo.h"
#include "wwwseeker/idgames.h"
#include "wwwseeker/urlparser.h"
#include "wwwseeker/wwwseeker.h"
#include "zip/un7zip.h"
#include "zip/unzip.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStack>

const QString Wadseeker::defaultSites[] =
{
	// 2017-12-08 - doomshack.org removed at request of owner. Prefers clients to go through wad-archive.
	QString("https://doom.dogsoft.net/getwad.php?search=%WADNAME%"),
	QString("https://static.allfearthesentinel.net/wads/"),
	QString("") // empty url is treated here like an '\0' in a string
};

const QString Wadseeker::forbiddenWads[] =
{
	"attack", "blacktwr", "bloodsea", "canyon", "catwalk", "combine", "doom",
	"doom2", "fistula", "garrison", "geryon", "heretic", "hexen", "hexdd",
	"manor", "mephisto", "minos", "nessus", "paradox", "plutonia", "subspace",
	"subterra", "teeth", "tnt", "ttrap", "strife1", "vesperas", "virgil",
	"voices", ""
};
///////////////////////////////////////////////////////////////////////
DClass<Wadseeker>
{
public:
	class SeekParameters
	{
	public:
		bool bIdgamesEnabled;
		bool bWadArchiveEnabled;
		QString customSiteUrl;
		QString idgamesUrl;
		unsigned maxConcurrentDownloads;
		unsigned maxConcurrentSeeks;
		QString saveDirectoryPath;
		ModSet seekedWads;
		QStringList sitesUrls;

		SeekParameters()
		{
			this->bIdgamesEnabled = true;
			this->bWadArchiveEnabled = true;
			this->idgamesUrl = Wadseeker::defaultIdgamesUrl();
			this->maxConcurrentDownloads = 3;
			this->maxConcurrentSeeks = 3;
			this->sitesUrls = Wadseeker::defaultSitesListEncoded();
		}
	};

	bool bIsAborting;

	/**
	 * @brief Idgames objects that will handle each file
	 *        separately.
	 *
	 * Each file is probed one-at-a-time on the IDGames page. This
	 * was designed to prevent creating too many request to the
	 * IDGames page.
	 *
	 * As long as there are Idgames clients on the
	 * list the Wadseeker will return true on isWorking() as there
	 * is a potential chance that new links to files will be found.
	 */
	QList<Idgames *> idgamesClients;

	SeekParameters seekParameters;

	/**
	 * This object is created when startSeek() method is called. This will
	 * ensure that the parameters won't change during the seek operation.
	 * When seek is not in progress this is nullptr.
	 */
	SeekParameters *seekParametersForCurrentSeek;

	WadArchiveClient *wadArchiveClient;
	WadRetriever *wadRetriever;
	WWWSeeker *wwwSeeker;
};
DPointeredNoCopy(Wadseeker)
///////////////////////////////////////////////////////////////////////////
Wadseeker::Wadseeker()
{
	d->bIsAborting = false;
	d->seekParametersForCurrentSeek = nullptr;
	d->wadArchiveClient = nullptr;
	d->wadRetriever = nullptr;
	d->wwwSeeker = nullptr;
}

Wadseeker::~Wadseeker()
{
	if (d->wadArchiveClient != nullptr)
	{
		delete d->wadArchiveClient;
	}

	while (!d->idgamesClients.isEmpty())
	{
		delete d->idgamesClients.takeFirst();
	}

	if (d->wwwSeeker != nullptr)
	{
		delete d->wwwSeeker;
	}

	if (d->wadRetriever != nullptr)
	{
		delete d->wadRetriever;
	}

	if (d->seekParametersForCurrentSeek != nullptr)
	{
		delete d->seekParametersForCurrentSeek;
	}
}

void Wadseeker::abort()
{
	if (isWorking() && !d->bIsAborting)
	{
		d->bIsAborting = true;
		abortSeekers();
		if (d->wadRetriever != nullptr)
		{
			d->wadRetriever->abort();
		}
	}
}

void Wadseeker::abortSeekers()
{
	stopWadArchiveClient();
	stopIdgames();
	abortWwwSeeker();
}

void Wadseeker::abortWwwSeeker()
{
	if (d->wwwSeeker != nullptr)
	{
		d->wwwSeeker->abort();
	}
}

void Wadseeker::cleanUpIfAllFinished()
{
	if (isAllFinished())
	{
		cleanUpAfterFinish();
	}
}

void Wadseeker::cleanUpAfterFinish()
{
	if (d->seekParametersForCurrentSeek == nullptr)
	{
		return;
	}

	// If there are no more WAD downloads pending for URLs when finish is
	// announced, then the Wadseeker procedure is a success.
	bool bSuccess = !d->wadRetriever->isAnyWadPendingUrl() && !d->bIsAborting;
	d->bIsAborting = false;

	delete d->seekParametersForCurrentSeek;
	d->seekParametersForCurrentSeek = nullptr;

	delete d->wadArchiveClient;
	d->wadArchiveClient = nullptr;

	d->wadRetriever->deleteLater();
	d->wadRetriever = nullptr;

	delete d->wwwSeeker;
	d->wwwSeeker = nullptr;

	emit allDone(bSuccess);
}

const QString Wadseeker::defaultIdgamesUrl()
{
	return Idgames::defaultIdgamesUrl();
}

QStringList Wadseeker::defaultSitesListEncoded()
{
	QStringList list;
	for (int i = 0; !defaultSites[i].isEmpty(); ++i)
	{
		list << QUrl::toPercentEncoding(defaultSites[i]);
	}
	return list;
}

void Wadseeker::fileLinkFound(const QString &filename, const QUrl &url)
{
	emit message(tr("Found link to file \"%1\": %2").arg(filename)
		.arg(url.toEncoded().constData()), WadseekerLib::Notice);

	d->wadRetriever->addUrl(d->seekParameters.seekedWads.findFileName(filename), url);
}

void Wadseeker::fileMirrorLinksFound(const QString &filename, const QList<QUrl> &urls)
{
	emit message(tr("Found mirror links to file \"%1\":").arg(filename), WadseekerLib::Notice);
	for (const QUrl &url : urls)
	{
		QString strUrl = url.toEncoded().constData();
		emit message(tr("    %2").arg(strUrl), WadseekerLib::Notice);
	}

	d->wadRetriever->addMirrorUrls(filename, urls);
}

ModSet Wadseeker::filterAllowedOnlyWads(const ModSet &wads)
{
	ModSet result;
	for (const ModFile &wad : wads.modFiles())
	{
		if (!isForbiddenWad(wad))
		{
			result.addModFile(wad);
		}
	}
	return result;
}

ModSet Wadseeker::filterForbiddenOnlyWads(const ModSet &wads)
{
	ModSet result;
	for (const ModFile &wad : wads.modFiles())
	{
		if (isForbiddenWad(wad))
		{
			result.addModFile(wad);
		}
	}
	return result;
}

void Wadseeker::idgamesClientFinished(Idgames *pEmitter)
{
	emit message(tr("IDGames client for file \"%1\" has finished->").arg(pEmitter->file().name()),
		WadseekerLib::NoticeImportant);

	d->idgamesClients.removeOne(pEmitter);
	delete pEmitter;

	if (!d->bIsAborting)
	{
		startNextIdgamesClient();
	}

	if (d->idgamesClients.isEmpty())
	{
		emit serviceFinished("idgames");
	}
	if (isAllFinished())
	{
		cleanUpAfterFinish();
	}
}

bool Wadseeker::isAllFinished() const
{
	return !isWorking();
}

bool Wadseeker::isDownloadingFile(const ModFile &file) const
{
	if (d->wadRetriever != nullptr)
	{
		return d->wadRetriever->isDownloadingWad(file.fileName());
	}

	return false;
}

bool Wadseeker::isForbiddenWad(const ModFile &wad)
{
	QFileInfo fiWad(wad.fileName());
	// Check the basename, ignore extension.
	// This will block names like "doom2.zip" but also "doom2.pk3" and
	// "doom2.whatever".
	QString basename = fiWad.completeBaseName();
	for (int i = 0; !forbiddenWads[i].isEmpty(); ++i)
	{
		if (basename.compare(forbiddenWads[i], Qt::CaseInsensitive) == 0)
		{
			return true;
		}
	}

	return false;
}

bool Wadseeker::isWorking() const
{
	if (d->seekParametersForCurrentSeek == nullptr)
	{
		// We're in a "waiting for orders" state, ie. seek is not in progress.
		return false;
	}

	return d->wwwSeeker->isWorking()
		|| d->wadRetriever->isAnyDownloadWorking()
		|| (d->wadArchiveClient != nullptr && d->wadArchiveClient->isWorking())
		|| !d->idgamesClients.isEmpty();
}

void Wadseeker::prepareSeekObjects()
{
	// WWWSeeker
	d->wwwSeeker = new WWWSeeker();
	d->wwwSeeker->setUserAgent(WadseekerVersionInfo::userAgent());

	// Connect signals to slots.
	this->connect(d->wwwSeeker, SIGNAL(finished()),
		SLOT(wwwSeekerFinished()), Qt::QueuedConnection);
	this->connect(d->wwwSeeker, SIGNAL(linkFound(const QString&,const QUrl&)),
		SLOT(fileLinkFound(const QString&,const QUrl&)), Qt::QueuedConnection);

	// Forward signals up to outside of the library.
	this->connect(d->wwwSeeker, SIGNAL(message(const QString&,WadseekerLib::MessageType)),
		SIGNAL(message(const QString&,WadseekerLib::MessageType)));
	this->connect(d->wwwSeeker, SIGNAL(siteFinished(const QUrl&)),
		SIGNAL(siteFinished(const QUrl&)), Qt::QueuedConnection);
	this->connect(d->wwwSeeker, SIGNAL(siteProgress(const QUrl&,qint64,qint64)),
		SIGNAL(siteProgress(const QUrl&,qint64,qint64)), Qt::QueuedConnection);
	this->connect(d->wwwSeeker, SIGNAL(siteRedirect(const QUrl&,const QUrl&)),
		SIGNAL(siteRedirect(const QUrl&,const QUrl&)), Qt::QueuedConnection);
	this->connect(d->wwwSeeker, SIGNAL(siteStarted(const QUrl&)),
		SIGNAL(siteStarted(const QUrl&)), Qt::QueuedConnection);

	// WadRetriever
	d->wadRetriever = new WadRetriever();
	d->wadRetriever->setUserAgent(WadseekerVersionInfo::userAgent());
	connect(d->wadRetriever, SIGNAL(badUrlDetected(QUrl)), SLOT(reportBadUrl(QUrl)));

	// Connect signals to slots.

	// The wadRetrieverFinished() slot will handle difference between finished()
	// and pendingUrls() signals.
	this->connect(d->wadRetriever, SIGNAL(finished()),
		SLOT(wadRetrieverFinished()), Qt::QueuedConnection);
	this->connect(d->wadRetriever, SIGNAL(pendingUrls()),
		SLOT(wadRetrieverFinished()), Qt::QueuedConnection);

	this->connect(d->wadRetriever, SIGNAL(message(const QString&,WadseekerLib::MessageType)),
		SLOT(wadRetrieverMessage(const QString&,WadseekerLib::MessageType)));

	this->connect(d->wadRetriever, SIGNAL(wadDownloadFinished(WadDownloadInfo)),
		SLOT(wadRetrieverDownloadFinished(WadDownloadInfo)));

	this->connect(d->wadRetriever, SIGNAL(wadDownloadStarted(WadDownloadInfo,const QUrl&)),
		SLOT(wadRetrieverDownloadStarted(WadDownloadInfo,const QUrl&)));

	this->connect(d->wadRetriever, SIGNAL(wadDownloadProgress(WadDownloadInfo,qint64,qint64)),
		SLOT(wadRetrieverDownloadProgress(WadDownloadInfo,qint64,qint64)));

	this->connect(d->wadRetriever, SIGNAL(wadInstalled(WadDownloadInfo)),
		SLOT(wadRetrieverWadInstalled(WadDownloadInfo)));
}

void Wadseeker::reportBadUrl(const QUrl &url)
{
	if (d->wadArchiveClient != nullptr)
	{
		d->wadArchiveClient->reportBadUrlIfOriginatingFromHere(url);
	}
}

void Wadseeker::setCustomSite(const QString &url)
{
	d->seekParameters.customSiteUrl = url;
}

void Wadseeker::setIdgamesEnabled(bool bEnabled)
{
	d->seekParameters.bIdgamesEnabled = bEnabled;
}

void Wadseeker::setIdgamesUrl(QString archiveUrl)
{
	d->seekParameters.idgamesUrl = archiveUrl;
}

void Wadseeker::setMaximumConcurrentDownloads(unsigned max)
{
	if (max < 1)
	{
		max = 1;
	}

	d->seekParameters.maxConcurrentDownloads = max;
}

void Wadseeker::setMaximumConcurrentSeeks(unsigned max)
{
	if (max < 1)
	{
		max = 1;
	}

	d->seekParameters.maxConcurrentSeeks = max;
}

void Wadseeker::setPrimarySites(const QStringList &urlList)
{
	d->seekParameters.sitesUrls = urlList;
}

void Wadseeker::setPrimarySitesToDefault()
{
	QList<QString> list;
	for (int i = 0; !defaultSites[i].isEmpty(); ++i)
	{
		list << defaultSites[i];
	}
	setPrimarySites(list);
}

void Wadseeker::setTargetDirectory(const QString &dir)
{
	QString targetDir = dir;
	if (!dir.isEmpty())
	{
		if (dir[dir.length() - 1] != QDir::separator())
		{
			targetDir += '/';
		}
	}

	d->seekParameters.saveDirectoryPath = targetDir;
}

void Wadseeker::setWadArchiveEnabled(bool enabled)
{
	d->seekParameters.bWadArchiveEnabled = enabled;
}

void Wadseeker::setupIdgamesClients(const QList<WadDownloadInfo> &wadDownloadInfoList)
{
	for (const WadDownloadInfo &wad : wadDownloadInfoList)
	{
		auto pIdgames = new Idgames(d->seekParametersForCurrentSeek->idgamesUrl);

		pIdgames->setUserAgent(WadseekerVersionInfo::userAgent());
		pIdgames->setFile(wad);

		this->connect(pIdgames, SIGNAL(finished(Idgames*)),
			SLOT(idgamesClientFinished(Idgames*)));

		this->connect(pIdgames, SIGNAL(message(const QString&,WadseekerLib::MessageType)),
			SIGNAL(message(const QString&,WadseekerLib::MessageType)));

		this->connect(pIdgames, SIGNAL(fileLinksFound(const QString&,const QList<QUrl>&)),
			SLOT(fileMirrorLinksFound(const QString&,const QList<QUrl>&)));

		// Forward signals as with WWWSeeker
		this->connect(pIdgames, SIGNAL(siteFinished(const QUrl&)),
			SIGNAL(siteFinished(const QUrl&)), Qt::QueuedConnection);
		this->connect(pIdgames, SIGNAL(siteProgress(const QUrl&,qint64,qint64)),
			SIGNAL(siteProgress(const QUrl&,qint64,qint64)), Qt::QueuedConnection);
		this->connect(pIdgames, SIGNAL(siteStarted(const QUrl&)),
			SIGNAL(siteStarted(const QUrl&)), Qt::QueuedConnection);

		d->idgamesClients << pIdgames;
	}
}

void Wadseeker::setupSitesUrls()
{
	const int PRIORITY_CUSTOM = 100;
	const int PRIORITY_NORMAL = 0;
	d->wwwSeeker->clearVisitedUrlsList();

	if (UrlParser::isWadnameTemplateUrl(d->seekParametersForCurrentSeek->customSiteUrl))
	{
		// If URL containts wadname placeholder we need to create a unique
		// URL for each searched wad.
		for (const ModFile &wad : d->seekParametersForCurrentSeek->seekedWads.modFiles())
		{
			QUrl url = UrlParser::resolveWadnameTemplateUrl(
				d->seekParametersForCurrentSeek->customSiteUrl, wad.fileName());
			if (url.isValid())
			{
				d->wwwSeeker->addFileSiteUrlWithPriority(wad.fileName(), url, PRIORITY_CUSTOM);
			}
		}
	}
	else
	{
		d->wwwSeeker->setCustomSiteUrl(d->seekParametersForCurrentSeek->customSiteUrl);
	}

	for (const QString &strSiteUrl : d->seekParametersForCurrentSeek->sitesUrls)
	{
		if (UrlParser::isWadnameTemplateUrl(strSiteUrl))
		{
			for (const ModFile &wad : d->seekParametersForCurrentSeek->seekedWads.modFiles())
			{
				QUrl url = UrlParser::resolveWadnameTemplateUrl(strSiteUrl, wad.fileName());
				if (url.isValid())
				{
					d->wwwSeeker->addFileSiteUrlWithPriority(wad.fileName(), url, PRIORITY_NORMAL);
				}
			}
		}
		else
		{
			QUrl url(strSiteUrl);
			if (url.isValid())
			{
				d->wwwSeeker->addSiteUrl(url);
			}
		}
	}
}

void Wadseeker::setupWadArchiveClient(const QList<WadDownloadInfo> &wadDownloadInfos)
{
	d->wadArchiveClient = new WadArchiveClient();
	d->wadArchiveClient->setUserAgent(WadseekerVersionInfo::userAgent());
	for (const WadDownloadInfo &wad : wadDownloadInfos)
	{
		d->wadArchiveClient->enqueue(wad);
	}

	this->connect(d->wadArchiveClient, SIGNAL(message(QString,WadseekerLib::MessageType)),
		SIGNAL(message(QString,WadseekerLib::MessageType)));
	this->connect(d->wadArchiveClient, SIGNAL(finished()),
		SLOT(wadArchiveFinished()));
	this->connect(d->wadArchiveClient, SIGNAL(urlFound(QString,QUrl)),
		SLOT(fileLinkFound(QString,QUrl)));
}

void Wadseeker::skipFileCurrentUrl(const QString &fileName)
{
	if (d->wadRetriever != nullptr)
	{
		d->wadRetriever->skipCurrentUrl(fileName);
	}
}

void Wadseeker::skipService(const QString &service)
{
	if (service == "WadArchive")
	{
		stopWadArchiveClient();
	}
	else if (service == "idgames")
	{
		stopIdgames();
	}
	else
	{
		qDebug() << "Wadseeker: attempted to abort unknown service " << service;
		emit message(tr("Attempted to abort unknown service '%1'").arg(service),
			WadseekerLib::Error);
	}
}

void Wadseeker::skipSiteSeek(const QUrl &url)
{
	if (d->wwwSeeker != nullptr)
	{
		d->wwwSeeker->skipSite(url);
	}
}

void Wadseeker::startIdgames()
{
	emit serviceStarted("idgames");
	startNextIdgamesClient();
}

void Wadseeker::startNextIdgamesClient()
{
	if (!d->bIsAborting && !d->idgamesClients.isEmpty())
	{
		Idgames *pIdgames = d->idgamesClients.first();

		pIdgames->startSearch();
	}

	if (isAllFinished())
	{
		cleanUpAfterFinish();
	}
}

void Wadseeker::stopIdgames()
{
	if (d->idgamesClients.isEmpty())
	{
		return;
	}
	Idgames *current = d->idgamesClients.takeFirst();
	qDeleteAll(d->idgamesClients);
	d->idgamesClients.clear();

	current->abort();
}

void Wadseeker::startWadArchiveClient()
{
	d->wadArchiveClient->start();
	if (d->wadArchiveClient->isWorking())
	{
		emit serviceStarted("WadArchive");
	}
	if (isAllFinished())
	{
		cleanUpAfterFinish();
	}
}

void Wadseeker::stopWadArchiveClient()
{
	if (d->wadArchiveClient != nullptr)
	{
		d->wadArchiveClient->abort();
	}
}

bool Wadseeker::startSeek(const ModSet &wads)
{
	if (d->seekParametersForCurrentSeek != nullptr)
	{
		emit message(tr("Seek already in progress. Please abort the current seek before starting a new one."),
			WadseekerLib::CriticalError);
		return false;
	}

	if (d->seekParameters.saveDirectoryPath.isEmpty())
	{
		QString err = tr("No target directory specified! Aborting.");
		emit message(err, WadseekerLib::CriticalError);
		return false;
	}

	QDir targetDir(d->seekParameters.saveDirectoryPath);
	if (!targetDir.exists())
	{
		emit message(tr("File save directory \"%1\" doesn't exist or is a file. Aborting.").arg(d->seekParameters.saveDirectoryPath),
			WadseekerLib::CriticalError);
		return false;
	}

	ModSet filteredWadsList;
	for (ModFile wad : wads.modFiles())
	{
		if (wad.fileName().isEmpty())
		{
			continue;
		}

		if (isForbiddenWad(wad))
		{
			emit message(tr("WAD \"%1\" is on the forbidden list. Wadseeker will not download this WAD.").arg(wad.fileName()),
				WadseekerLib::Error);
			continue;
		}

		filteredWadsList.addModFile(wad);
	}

	if (filteredWadsList.isEmpty())
	{
		emit message(tr("Cannot start search. No WADs to seek."),
			WadseekerLib::CriticalError);
		return false;
	}

	d->bIsAborting = false;
	d->seekParameters.seekedWads = filteredWadsList;
	d->seekParametersForCurrentSeek = new PrivData<Wadseeker>::SeekParameters(d->seekParameters);

	// Important call - creates new objects. These objects will be deleted
	// either in destructor or in cleanUpAfterFinish() method.
	prepareSeekObjects();

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

	if (d->seekParametersForCurrentSeek->bIdgamesEnabled)
	{
		setupIdgamesClients(wadDownloadInfoList);
	}
	if (d->seekParametersForCurrentSeek->bWadArchiveEnabled)
	{
		setupWadArchiveClient(wadDownloadInfoList);
	}

	d->wadRetriever->setMaxConcurrentWadDownloads(d->seekParametersForCurrentSeek->maxConcurrentDownloads);
	d->wadRetriever->setTargetSavePath(d->seekParametersForCurrentSeek->saveDirectoryPath);
	d->wadRetriever->setWads(wadDownloadInfoList);

	d->wwwSeeker->setMaxConcurrectSiteDownloads(d->seekParametersForCurrentSeek->maxConcurrentSeeks);

	emit seekStarted(filteredWadsList);
	d->wwwSeeker->startSearch(fileSeekInfosList);
	if (d->seekParametersForCurrentSeek->bIdgamesEnabled)
	{
		startIdgames();
	}
	if (d->seekParametersForCurrentSeek->bWadArchiveEnabled)
	{
		startWadArchiveClient();
	}

	return true;
}

QString Wadseeker::targetDirectory() const
{
	return d->seekParameters.saveDirectoryPath;
}

void Wadseeker::wadArchiveFinished()
{
	emit serviceFinished("WadArchive");
	cleanUpIfAllFinished();
}

void Wadseeker::wadRetrieverDownloadFinished(WadDownloadInfo wadDownloadInfo)
{
	emit fileDownloadFinished(wadDownloadInfo.name());
}

void Wadseeker::wadRetrieverDownloadProgress(WadDownloadInfo wadDownloadInfo, qint64 current, qint64 total)
{
	emit fileDownloadProgress(wadDownloadInfo.name(), current, total);
}

void Wadseeker::wadRetrieverDownloadStarted(WadDownloadInfo wadDownloadInfo, const QUrl &url)
{
	emit message(tr(R"(Started downloading file "%1" from URL "%2")")
		.arg(wadDownloadInfo.name(), url.toString()), WadseekerLib::Notice);
	emit fileDownloadStarted(wadDownloadInfo.name(), url);
}

void Wadseeker::wadRetrieverFinished()
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

	if (isAllFinished())
	{
		cleanUpAfterFinish();
	}
}

void Wadseeker::wadRetrieverMessage(const QString &message, WadseekerLib::MessageType type)
{
	emit this->message(message, type);

	if (type == WadseekerLib::CriticalError)
	{
		abort();
	}
}

void Wadseeker::wadRetrieverWadInstalled(WadDownloadInfo wadDownloadInfo)
{
	if (wadDownloadInfo.size() > 0)
	{
		// If size of the file is known it is a good idea to emit it now.
		emit fileDownloadProgress(wadDownloadInfo.name(), wadDownloadInfo.size(), wadDownloadInfo.size());
	}
	emit fileInstalled(wadDownloadInfo.name());

	d->wwwSeeker->removeSeekedFile(wadDownloadInfo.name());
}

void Wadseeker::wwwSeekerFinished()
{
	emit message("WWWSeeker is finished.", WadseekerLib::NoticeImportant);
	if (isAllFinished())
	{
		cleanUpAfterFinish();
	}
}
