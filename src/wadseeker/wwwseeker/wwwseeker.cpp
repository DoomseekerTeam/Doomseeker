//------------------------------------------------------------------------------
// wwwseeker.cpp
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
#include "wwwseeker.h"

#include "htmlparser.h"
#include "protocols/http.h"
#include "protocols/networkreply.h"
#include "protocols/networkreplysignalwrapper.h"
#include "urlparser.h"
#include "wadseekerversioninfo.h"
#include "wwwseeker/entities/fileseekinfo.h"

#include <QDebug>
#include <QFileInfo>

WWWSeeker::WWWSeeker()
{
	d.bIsAborting = false;
	d.bIsWorking = false;
	d.maxConcurrentSiteDownloads = 3;
	d.pNetworkAccessManager = new QNetworkAccessManager();
}

WWWSeeker::~WWWSeeker()
{
	for (NetworkReply *pInfo : d.networkQueries)
	{
		pInfo->deleteMembersLater();
		delete pInfo;
	}

	// Produces crashes if simply deleted.
	d.pNetworkAccessManager->deleteLater();
}

void WWWSeeker::abort()
{
	d.bIsAborting = true;

	d.fileSiteUrls.clear();
	d.sitesUrls.clear();
	d.seekedFiles.clear();

	if (d.networkQueries.isEmpty())
	{
		d.bIsWorking = false;
		emit finished();
	}
	else
	{
		d.sitesUrls.clear();
		d.seekedFiles.clear();

		for (NetworkReply *pInfo : d.networkQueries)
		{
			pInfo->reply->abort();
		}
	}
}

void WWWSeeker::addFileSiteUrl(const QString &filename, const QUrl &url)
{
	const int DEFAULT_PRIORITY = 0;
	addFileSiteUrlWithPriority(filename, url, DEFAULT_PRIORITY);
}

void WWWSeeker::addFileSiteUrlWithPriority(const QString &filename, const QUrl &url, int priority)
{
	if (!wasUrlUsed(url))
	{
		QString lowerCaseFilename = filename.toLower();

		if (d.fileSiteUrls.contains(lowerCaseFilename))
		{
			// Add to existing URL list.
			QMultiMap<int, QUrl> &urls = d.fileSiteUrls[lowerCaseFilename];
			urls.insert(priority, url);
		}
		else
		{
			// Create a new URL list.
			QMultiMap<int, QUrl> urls;
			urls.insert(priority, url);
			d.fileSiteUrls.insert(lowerCaseFilename, urls);

			// Don't forget to update the key list with a new key.
			d.fileSiteKeyRotationList << lowerCaseFilename;
		}
	}
}

void WWWSeeker::addNetworkReply(const QNetworkRequest &request, QNetworkReply *pReply)
{
	auto pQueryInfo = new NetworkReply(request, pReply);
	pQueryInfo->setProgressTimeout(NetworkReply::SUGGESTED_PROGRESS_TIMEOUT_MSECS);
	pQueryInfo->startConnectionTimeoutTimer();

	this->connect(pQueryInfo->signalWrapper, SIGNAL(downloadProgress(NetworkReply*,qint64,qint64)),
		SLOT(networkQueryDownloadProgress(NetworkReply*,qint64,qint64)));
	this->connect(pQueryInfo->signalWrapper, SIGNAL(error(NetworkReply*,QNetworkReply::NetworkError)),
		SLOT(networkQueryError(NetworkReply*,QNetworkReply::NetworkError)));
	this->connect(pQueryInfo->signalWrapper, SIGNAL(finished(NetworkReply*)),
		SLOT(networkQueryFinished(NetworkReply*)));
	this->connect(pQueryInfo->signalWrapper, SIGNAL(metaDataChanged(NetworkReply*)),
		SLOT(networkQueryMetaDataChanged(NetworkReply*)));

	d.networkQueries << pQueryInfo;
}

void WWWSeeker::addSiteUrl(const QUrl &url)
{
	if (!wasUrlUsed(url))
	{
		d.sitesUrls << url;
	}
}

void WWWSeeker::addSitesUrls(const QList<QUrl> &urlsList)
{
	for (const QUrl &url : urlsList)
	{
		addSiteUrl(url);
	}
}

void WWWSeeker::deleteNetworkReply(NetworkReply *pReply)
{
	pReply->deleteMembersLater();
	d.networkQueries.removeOne(pReply);
}

bool WWWSeeker::isMoreToSearch() const
{
	if (d.seekedFiles.isEmpty())
	{
		return false;
	}

	bool hasSiteUrls = !d.sitesUrls.isEmpty();
	bool hasFileSiteUrls = !d.fileSiteUrls.isEmpty();
	bool hasCustomUrl = d.customSiteUrl.isValid();

	return hasSiteUrls
		|| hasFileSiteUrls
		|| hasCustomUrl;
}

FileSeekInfo *WWWSeeker::findFileSeekInfo(const QString &seekedName)
{
	QList<FileSeekInfo>::iterator it;
	for (it = d.seekedFiles.begin(); it != d.seekedFiles.end(); ++it)
	{
		FileSeekInfo &info = *it;
		QStringList possibleFilenames = info.possibleFilenames();
		for (const QString &possibleFilename : possibleFilenames)
		{
			if (possibleFilename.compare(seekedName, Qt::CaseInsensitive) == 0)
			{
				return &info;
			}
		}
	}

	return nullptr;
}

NetworkReply *WWWSeeker::findNetworkReply(QNetworkReply *pReply)
{
	for (NetworkReply *info : d.networkQueries)
	{
		if (*info == pReply)
		{
			return info;
		}
	}

	return nullptr;
}

NetworkReply *WWWSeeker::findNetworkReply(const QUrl &url)
{
	for (NetworkReply *info : d.networkQueries)
	{
		if (info->request().url() == url)
		{
			return info;
		}
	}

	return nullptr;
}

bool WWWSeeker::isDirectUrl(const QUrl &url, QString &outFileName) const
{
	// Utilize what UrlParser gives us for this.
	QList<Link> links;
	links << Link(url, "");

	UrlParser urlParser(links);
	for (const FileSeekInfo &fileSeekInfo : d.seekedFiles)
	{
		const QString &file = fileSeekInfo.file();
		const QStringList &possibleFilenames = fileSeekInfo.possibleFilenames();

		QList<Link> directLinks = urlParser.directLinks(possibleFilenames, url);
		if (!directLinks.isEmpty())
		{
			outFileName = file;
			return true;
		}
	}

	return false;
}

void WWWSeeker::networkQueryDownloadProgress(NetworkReply *pReply, qint64 current, qint64 total)
{
	emit siteProgress(pReply->request().url(), current, total);
}

void WWWSeeker::networkQueryError(NetworkReply *pReply, QNetworkReply::NetworkError code)
{
	// We shall ignore OperationCanceledError because this error is caused
	// by a call to QNetworkReply::abort() and it may confuse users.
	// "Why am I getting this error? Is it a bug? Yeah, it is a bug!"
	if (code != QNetworkReply::NoError && code != QNetworkReply::OperationCanceledError)
	{
		emit message(tr("Site \"%1\": network error occurred: %2")
			.arg(pReply->request().url().toString(), pReply->errorString()), WadseekerLib::Error);
	}
	qDebug() << "WWWSeeker::networkQueryError() " << code;
}

void WWWSeeker::logHeaders(NetworkReply *reply)
{
	qDebug() << "HEADERS";
	qDebug() << "URL " << reply->url();
	QList<QByteArray> headers = reply->rawHeaderList();
	for (const QByteArray &headerName : headers)
	{
		QByteArray headerData = reply->rawHeader(headerName);
		qDebug() << QString("%1: %2").arg(QString::fromUtf8(headerName)).arg(
				QString::fromUtf8(headerData));
	}
	printf("END OF HEADERS\n");
}

void WWWSeeker::networkQueryFinished(NetworkReply *pReply)
{
	QUrl url = pReply->request().url();

	#ifndef NDEBUG
	printf("WWWSeeker::networkQueryFinished()\n");
	logHeaders(pReply);
	#endif

	QUrl possibleRedirectUrl = pReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
	if (!possibleRedirectUrl.isEmpty()
		&& possibleRedirectUrl != url)
	{
		// Redirect URL cannot be simply added to the site URLs as those URLs
		// will throw away all visited URLs. In at least one case a redirect
		// leads to the same page but with different attachments.
		// In this case let's start a new query omitting the entire limitation
		// system here.
		if (possibleRedirectUrl.isRelative())
		{
			possibleRedirectUrl = url.resolved(possibleRedirectUrl);
		}

		deleteNetworkReply(pReply);

		if (!d.bIsAborting)
		{
			emit siteRedirect(url, possibleRedirectUrl);
			startNetworkQuery(possibleRedirectUrl);
		}

		printf("Finished URL %s - redirect to %s\n", url.toEncoded().constData(), possibleRedirectUrl.toEncoded().constData());
	}
	else
	{
		parseAsHtml(pReply);

		deleteNetworkReply(pReply);
		emit siteFinished(url);
	}

	bool bAreNetworkQueriesEmpty = d.networkQueries.isEmpty();

	if (bAreNetworkQueriesEmpty && !isMoreToSearch())
	{
		// Work is finished if there are no more site URLs to find.
		d.bIsWorking = false;
		emit finished();
	}
	else
	{
		if (!d.bIsAborting)
		{
			startNextSites();
		}
	}
}

void WWWSeeker::networkQueryMetaDataChanged(NetworkReply *pReply)
{
	QUrl url = pReply->request().url();

	#ifndef NDEBUG
	printf("WWWSeeker::networkQueryMetaDataChanged()\n");
	logHeaders(pReply);
	#endif

	Http http(pReply);
	if (!http.isHtmlContentType())
	{
		bool bAttachmentDetected = false;
		if (http.isApplicationContentType())
		{
			// In this block we recognize and handle file downloads for data types
			// different than HTML.

			QString attachmentName = http.attachmentName();
			// If attachment name is empty at this point we should try to attempt
			// to extract it from URL. It is important to have a proper attachment
			// name.
			if (attachmentName.isEmpty())
			{
				attachmentName = http.urlFilename();
			}

			// See if we just stumbled upon a download link for a file we seek.
			FileSeekInfo *attachmentSeekInfo = findFileSeekInfo(attachmentName);

			#ifndef NDEBUG
			printf("Attachment detected on URL %s\n", url.toString().toUtf8().constData());
			if (attachmentSeekInfo != nullptr)
			{
				printf("Forwarding the detected attachment to \"%s\" download queue\n",
					attachmentSeekInfo->file().toUtf8().constData());
			}
			#endif
			// Abort further download here.
			// This should execute networkQueryFinished() request.

			if (attachmentSeekInfo != nullptr)
			{
				bAttachmentDetected = true;

				// Forward the current download URL to the WAD download queue.
				emit message(tr("Attachment %1 detected on URL: %2. Forwarding to WAD download queue.").arg(attachmentName, url.toString()),
					WadseekerLib::Notice);
				emit linkFound(attachmentSeekInfo->file(), url);
			}
		}

		pReply->abort();

		if (!bAttachmentDetected)
		{
			emit message(tr("Non-HTML data on URL: %1. Aborting.").arg(url.toString()),
				WadseekerLib::Notice);
		}
	}
}

void WWWSeeker::parseAsHtml(NetworkReply *pReply)
{
	QByteArray downloadedData = pReply->readAll();
	QUrl url = pReply->request().url();

	if (downloadedData.isEmpty())
	{
		// Nothing to parse.
		return;
	}

	// Get all <A HREFs> from HTML.
	HtmlParser html(downloadedData);
	QList<Link> links = html.linksFromHtml();

	#ifndef NDEBUG
	qDebug() << "Finished URL" << url << "; content type"
		<< pReply->header(QNetworkRequest::ContentTypeHeader).toString()
		<< ". Data size:" << downloadedData.size();
	qDebug() << "Links:" << links.size();
	#endif

	// Extract URLs of interest from <A HREFs>
	UrlParser urlParser(links);

	for (const FileSeekInfo &fileSeekInfo : d.seekedFiles)
	{
		const QString &file = fileSeekInfo.file();
		const QStringList &possibleFilenames = fileSeekInfo.possibleFilenames();

		QList<Link> siteLinks = urlParser.siteLinks(possibleFilenames, url);
		QList<Link> directLinks = urlParser.directLinks(possibleFilenames, url);

		#ifndef NDEBUG
		printf("File %s\n", file.toUtf8().constData());
		printf("Site links: %d\n", siteLinks.size());
		printf("Direct links: %d\n", directLinks.size());
		#endif

		for (const Link &link : siteLinks)
		{
			#ifndef NDEBUG
			printf("Adding url %s\n", link.url.toEncoded().constData());
			#endif
			addFileSiteUrl(file, link.url);
		}

		for (const Link &link : directLinks)
		{
			emit linkFound(file, link.url);
		}
	}
}

void WWWSeeker::removeSeekedFile(const QString &file)
{
	// Make sure to remove all occurences of the filename.
	QList<FileSeekInfo> toRemove;

	for (const FileSeekInfo &fileOnList : d.seekedFiles)
	{
		if (fileOnList == file)
		{
			toRemove << fileOnList;
		}
	}

	for (const FileSeekInfo &fileToRemove : toRemove)
	{
		d.seekedFiles.removeAll(fileToRemove);
	}

	if (d.seekedFiles.isEmpty() && isWorking())
	{
		abort();
	}
}

void WWWSeeker::setUserAgent(const QString &userAgent)
{
	d.userAgent = userAgent;
}

void WWWSeeker::skipSite(const QUrl &url)
{
	NetworkReply *pInfo = findNetworkReply(url);
	if (pInfo != nullptr)
	{
		pInfo->reply->abort();
	}
}

void WWWSeeker::startNetworkQuery(const QUrl &url)
{
	QNetworkRequest request;
	request.setUrl(url);
	request.setRawHeader("User-Agent", d.userAgent.toUtf8());

	QNetworkReply *reply = d.pNetworkAccessManager->get(request);
	addNetworkReply(request, reply);
}

void WWWSeeker::startNextSites()
{
	while (d.networkQueries.size() < d.maxConcurrentSiteDownloads
		&& isMoreToSearch())
	{
		QUrl url = takeNextUrl();
		qDebug() << "Took URL:" << url.toString();

		if (url.isValid())
		{
			d.visitedUrls << url;

			// It is possible that a direct URL will be placed here. Detect
			// it and forward it to the WadRetriever.
			QString filename;
			if (isDirectUrl(url, filename))
			{
				emit linkFound(filename, url);
			}
			else
			{
				qDebug() << "Starting site:" << url.toString();

				startNetworkQuery(url);
				emit siteStarted(url);
			}
		}
	}

	if (d.networkQueries.isEmpty() && !isMoreToSearch())
	{
		// No more sites to check, no more queries to wait for a reply.
		d.bIsWorking = false;
		emit finished();
	}
}

void WWWSeeker::startSearch(const QList<FileSeekInfo> &seekedFiles)
{
	if (isWorking())
	{
		return;
	}

	d.bIsAborting = false;
	d.bIsWorking = true;

	d.seekedFiles = seekedFiles;
	d.visitedUrls.clear();

	startNextSites();
}

QUrl WWWSeeker::takeNextUrl()
{
	printf("Taking next URL\n");

	// Custom site has priority above all else.
	if (d.customSiteUrl.isValid())
	{
		printf("Taking custom URL\n");
		QUrl tmp = d.customSiteUrl;
		d.customSiteUrl = QUrl(); // Make it invalid.

		return tmp;
	}

	// Try to take one of the sites that are suspected to contain at least
	// one of the seeked files.
	while (!d.fileSiteKeyRotationList.isEmpty())
	{
		QString lowerCaseFilename = d.fileSiteKeyRotationList.takeFirst().toLower();

		if (d.fileSiteUrls.contains(lowerCaseFilename))
		{
			QMultiMap<int, QUrl> &urls = d.fileSiteUrls[lowerCaseFilename];
			if (!urls.isEmpty())
			{
				int priority = urls.keys().back();
				QUrl url = urls.values(priority).takeLast();
				urls.remove(priority, url);
				if (urls.isEmpty())
				{
					// Remove now empty list from the hash map.
					d.fileSiteUrls.remove(lowerCaseFilename);
				}

				return url;
			}
		}
	}

	// Refill the rotation list
	d.fileSiteKeyRotationList = d.fileSiteUrls.keys();

	// Take one of the generic sites.
	while (!d.sitesUrls.isEmpty())
	{
		QUrl url = d.sitesUrls.takeFirst();
		if (!wasUrlUsed(url))
		{
			return url;
		}
	}

	return QUrl();
}

const QString &WWWSeeker::userAgent() const
{
	return d.userAgent;
}

bool WWWSeeker::wasUrlUsed(const QUrl &url) const
{
	for (const QUrl &usedUrl : d.visitedUrls)
	{
		if (UrlParser::urlEqualsCaseInsensitive(usedUrl, url))
		{
			return true;
		}
	}

	return false;
}
