//------------------------------------------------------------------------------
// idgames.cpp
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
#include "idgames.h"

#include "entities/waddownloadinfo.h"
#include "protocols/idgames/idgamesclient.h"
#include "protocols/idgames/idgamesfile.h"
#include "protocols/idgames/idgamesfilematcher.h"
#include "protocols/idgames/idgamesreply.h"
#include "protocols/idgames/idgamesresult.h"
#include "wwwseeker/htmlparser.h"
#include "wwwseeker/urlparser.h"

#include <QDebug>


class Idgames::PrivData
{
public:
	bool bIsAborting;
	QNetworkReply *currentRequest;
	QString idgamesBaseUrl;
	IdgamesClient *idgamesClient;
	IdgamesReply *idgamesReply;
	QNetworkAccessManager *nam;
	WadDownloadInfo seekedFile;
	QString userAgent;
};


Idgames::Idgames(const QString &idgamesPage)
{
	d = new PrivData();
	d->nam = new QNetworkAccessManager();
	d->idgamesBaseUrl = idgamesPage;
	d->idgamesClient = new IdgamesClient(d->nam, idgamesPage);
	d->idgamesReply = nullptr;
	d->bIsAborting = false;
	d->currentRequest = nullptr;
}

Idgames::~Idgames()
{
	if (d->idgamesReply != nullptr)
	{
		delete d->idgamesReply;
	}
	delete d->idgamesClient;
	if (d->currentRequest != nullptr)
	{
		d->currentRequest->deleteLater();
	}
	d->nam->deleteLater();
	delete d;
}

void Idgames::abort()
{
	if (!d->bIsAborting)
	{
		d->bIsAborting = true;
		// If one is occuring, others cannot.
		if (d->idgamesReply != nullptr)
		{
			d->idgamesReply->abort();
		}
		else if (d->currentRequest != nullptr)
		{
			d->currentRequest->abort();
		}
		else
		{
			emit finished(this);
		}
	}
}

QString Idgames::defaultIdgamesUrl()
{
	return IdgamesClient::DEFAULT_URL.toString();
}

void Idgames::extractAndEmitLinks(QByteArray &pageData, const QUrl &pageUrl)
{
	// Get all <A HREFs> from HTML.
	HtmlParser html(pageData);
	QList<Link> links = html.linksFromHtml();

	// Extrat URLs of interest from <A HREFs>
	UrlParser urlParser(links);

	QStringList possibleFilenames;
	possibleFilenames << zipName();
	QList<Link> directLinks = urlParser.directLinks(possibleFilenames, pageUrl);

	if (!directLinks.isEmpty())
	{
		QList<QUrl> directUrls;
		for (const Link &link : directLinks)
		{
			directUrls << link.url;
		}

		emit fileLinksFound(d->seekedFile.name(), directUrls);
	}

	emit finished(this);
}

const WadDownloadInfo &Idgames::file() const
{
	return d->seekedFile;
}

void Idgames::onIdgamesApiQueryFinished()
{
	emit siteFinished(d->idgamesClient->baseUrl());
	IdgamesResult result = d->idgamesReply->result();
	delete d->idgamesReply;
	d->idgamesReply = nullptr;
	if (d->bIsAborting)
	{
		emit finished(this);
		return;
	}
	if (result.isOk())
	{
		QList<IdgamesFile> files = IdgamesFile::parseSearchResult(result.content());
		IdgamesFile bestMatch = IdgamesFileMatcher::match(d->seekedFile, files);
		if (!bestMatch.isNull())
		{
			startNetworkQuery(bestMatch.url());
		}
		else
		{
			QString msg = tr("Idgames - no file found");
			emit message(msg, WadseekerLib::Error);
			emit finished(this);
		}
	}
	else
	{
		QString msg = tr("Idgames - %1: %2").arg(result.errorType(), result.errorMessage());
		emit message(msg, WadseekerLib::Error);
		emit finished(this);
	}
}

void Idgames::queryIdgamesApi()
{
	d->idgamesClient->setBaseUrl(d->idgamesBaseUrl);
	emit siteStarted(d->idgamesClient->baseUrl());
	d->idgamesReply = d->idgamesClient->search(zipName());
	this->connect(d->idgamesReply, SIGNAL(finished()), SLOT(onIdgamesApiQueryFinished()));
}

void Idgames::setFile(const WadDownloadInfo &wad)
{
	d->seekedFile = wad;
}

void Idgames::startSearch()
{
	if (!QUrl(d->idgamesBaseUrl).isValid())
	{
		emit message(tr("Idgames error: invalid base URL:\n%1").arg(d->idgamesBaseUrl),
			WadseekerLib::Error);
		emit finished(this);
		return;
	}

	if (d->seekedFile.name().isEmpty())
	{
		emit message(tr("Idgames error: Specified search filename is empty or invalid."),
			WadseekerLib::Error);
		emit finished(this);
		return;
	}

	emit message(tr("Searching Idgames archive for file: %1").arg(zipName()),
		WadseekerLib::NoticeImportant);
	queryIdgamesApi();
}

void Idgames::networkRequestFinished()
{
	if (d->currentRequest == nullptr)
	{
		return;
	}

	QByteArray pageData = d->currentRequest->readAll();
	QUrl pageUrl = d->currentRequest->url();
	QUrl redirectUrl = d->currentRequest->attribute(
		QNetworkRequest::RedirectionTargetAttribute).toUrl();

	emit siteFinished(pageUrl);

	// Clean up to accept new requests.
	d->currentRequest->deleteLater();
	d->currentRequest = nullptr;

	if (d->bIsAborting)
	{
		emit finished(this);
	}
	else
	{
		if (!redirectUrl.isEmpty() && redirectUrl != pageUrl)
		{
			if (redirectUrl.isRelative())
			{
				redirectUrl = pageUrl.resolved(redirectUrl);
			}
			startNetworkQuery(redirectUrl);
		}
		else
		{
			extractAndEmitLinks(pageData, pageUrl);
		}
	}
}

void Idgames::networkRequestProgress(qint64 done, qint64 total)
{
	if (d->currentRequest != nullptr)
	{
		emit siteProgress(d->currentRequest->url(), done, total);
	}
}

void Idgames::setPage(const QString &url)
{
	d->idgamesBaseUrl = url;
}

void Idgames::setUserAgent(const QString &userAgent)
{
	d->userAgent = userAgent;
}

void Idgames::startNetworkQuery(const QUrl &url)
{
	QNetworkRequest request;
	request.setUrl(url);
	request.setRawHeader("User-Agent", d->userAgent.toUtf8());

	QNetworkReply *pReply = d->nam->get(request);
	d->currentRequest = pReply;
	this->connect(pReply, SIGNAL(downloadProgress(qint64,qint64)),
		SLOT(networkRequestProgress(qint64,qint64)));
	this->connect(pReply, SIGNAL(finished()),
		SLOT(networkRequestFinished()));

	emit siteStarted(url);
}

QString Idgames::zipName() const
{
	return d->seekedFile.archiveName("zip");
}
