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
#include "wadseeker.h"

#include "seeksession.h"
#include "protocols/idgames/idgames.h"
#include "wadseekermessagetype.h"

#include <QFileInfo>
#include <QNetworkAccessManager>
#include <memory>

#ifndef NDEBUG
#define WADSEEKER_DEBUG_MESSAGES 1
#endif

#ifdef WADSEEKER_DEBUG_MESSAGES
#include <QDebug>
#endif

const QString Wadseeker::defaultSites[] =
{
	QString("https://doom.dogsoft.net/getwad.php?search=%WADNAME%"),
	QString("https://static.allfearthesentinel.com/wads/"),
	// 2022-11-06 - doomshack.org re-added after contacting doomkid on
	// Doomworld via PM
	QString("https://doomshack.org/wadlist.php"),
	QString("https://wads.firestick.games/"), // approved by DrinkyBird on 2022-11-07
	QString("https://euroboros.net/zandronum/wads/"), // approved by Iko on 2022-11-07
	QString("https://static.audrealms.org/wads/"), // approved by DoomJoshuaBoy on 2023-05-01
	QString("https://downloadbox.captainpollutiontv.de/DooM/WADSEEKER/"), // requested by CaptainPollutionTV on 2023-03-11
	QString("") // empty url is treated here like an '\0' in a string
};

const QString Wadseeker::forbiddenWads[] =
{
	"attack", "blacktwr", "bloodsea", "canyon", "catwalk", "combine", "doom",
	"doom2", "fistula", "garrison", "geryon", "heretic", "hexen", "hexdd",
	"manor", "mephisto", "minos", "nessus", "paradox", "plutonia", "subspace",
	"subterra", "teeth", "tnt", "ttrap", "sigil_shreds", "sigil_shreds_compat",
	"strife1", "vesperas", "virgil", "voices", ""
};

DClass<Wadseeker>
{
public:
	QNetworkAccessManager *net;

	/**
	 * The configuration for the next seek Session.
	 */
	SeekParameters seekParameters;

	/**
	 * This object is created when startSeek() method is called. This will
	 * ensure that the parameters won't change during the seek operation.
	 * When seek is not in progress, this is nullptr.
	 */
	std::unique_ptr<SeekSession> session;
};
DPointeredNoCopy(Wadseeker)

Wadseeker::Wadseeker()
{
	d->net = new QNetworkAccessManager(this);

	#ifdef WADSEEKER_DEBUG_MESSAGES
	connect(this, &Wadseeker::message,
		this, [](QString msg, WadseekerLib::MessageType level)
		{
			QString slvl = "    ";
			switch (level)
			{
			case WadseekerLib::CriticalError:
				slvl = "CRIT"; break;
			case WadseekerLib::Error:
				slvl = " ERR"; break;
			case WadseekerLib::Notice:
				slvl = "INFO"; break;
			case WadseekerLib::NoticeImportant:
				slvl = "NOTI"; break;
			case WadseekerLib::Navigation:
				slvl = "NAVI"; break;
			default:
				slvl = "????"; break;
			}
			QDebug debug = qDebug();
			debug.noquote();
			debug << "Wadseeker" << "|" << slvl << "|" << msg;
		}, Qt::DirectConnection);
	#endif
}

Wadseeker::~Wadseeker()
{
}

void Wadseeker::abort()
{
	if (d->session != nullptr)
		d->session->abort();
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

bool Wadseeker::isDownloadingFile(const ModFile &file) const
{
	return d->session != nullptr
		&& d->session->isDownloadingFile(file);
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
	return d->session != nullptr && d->session->isWorking();
}

void Wadseeker::setCustomSite(const QString &url)
{
	d->seekParameters.customSiteUrls = QStringList(url);
}

void Wadseeker::setCustomSites(const QStringList &urls)
{
	d->seekParameters.customSiteUrls = urls;
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
	d->seekParameters.maxConcurrentDownloads = max > 1U ? max : 1U;
}

void Wadseeker::setMaximumConcurrentSeeks(unsigned max)
{
	d->seekParameters.maxConcurrentSeeks = max > 1U ? max : 1U;
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
	d->seekParameters.saveDirectoryPath = dir;
}

void Wadseeker::setWadArchiveEnabled(bool enabled)
{
	// deprecated method that does nothing
}

void Wadseeker::skipFileCurrentUrl(const QString &fileName)
{
	if (d->session != nullptr)
		d->session->skipFileCurrentUrl(fileName);
}

void Wadseeker::skipService(const QString &service)
{
	if (d->session != nullptr)
		d->session->skipService(service);
}

void Wadseeker::skipSiteSeek(const QUrl &url)
{
	if (d->session != nullptr)
		d->session->skipSiteSeek(url);
}

bool Wadseeker::startSeek(const ModSet &wads)
{
	if (d->session != nullptr)
	{
		emit message(tr("Seek already in progress. Please abort the current seek before starting a new one."),
			WadseekerLib::CriticalError);
		return false;
	}
	if (d->net == nullptr)
	{
		emit message(tr("Failed to allocate a network access manager. Wadseeker won't work."),
			WadseekerLib::CriticalError);
		return false;
	}

	d->seekParameters.seekedWads = wads;
	d->session.reset(new SeekSession(d->net, d->seekParameters));
	if (d->session == nullptr)
	{
		emit message(tr("Unable to allocate memory for the seek session."),
			WadseekerLib::CriticalError);
		return false;
	}

	SeekSession *session = d->session.get();

	// A noble idea of reporting 404 links to the site owners...
	connect(session, &SeekSession::badUrlDetected,
		this, [](const QUrl&)
		{
			// TODO this currently does nothing
		});

	// SeekSession is more sophisticated in reporting
	// its result than Wadseeker ;)
	connect(session, &SeekSession::seekFinished,
		this, [this](SeekResult result)
		{
			d->session.reset(nullptr);
			emit allDone(result == SeekResult::Success);
		});

	// Forward the session's signals as Wadseeker's signals.
	connect(session, &SeekSession::fileDownloadFinished,
		this, &Wadseeker::fileDownloadFinished);
	connect(session, &SeekSession::fileDownloadProgress,
		this, &Wadseeker::fileDownloadProgress);
	connect(session, &SeekSession::fileDownloadStarted,
		this, &Wadseeker::fileDownloadStarted);
	connect(session, &SeekSession::fileInstalled,
		this, &Wadseeker::fileInstalled);

	connect(session, &SeekSession::message,
		this, &Wadseeker::message);
	connect(session, &SeekSession::seekStarted,
		this, &Wadseeker::seekStarted);

	connect(session, &SeekSession::serviceFinished,
		this, &Wadseeker::serviceFinished);
	connect(session, &SeekSession::serviceStarted,
		this, &Wadseeker::serviceStarted);

	connect(session, &SeekSession::siteFinished,
		this, &Wadseeker::siteFinished);
	connect(session, &SeekSession::siteProgress,
		this, &Wadseeker::siteProgress);
	connect(session, &SeekSession::siteRedirect,
		this, &Wadseeker::siteRedirect);
	connect(session, &SeekSession::siteStarted,
		this, &Wadseeker::siteStarted);

	d->session->startSeek();

	return true;
}

QString Wadseeker::targetDirectory() const
{
	return d->seekParameters.saveDirectoryPath;
}
