//------------------------------------------------------------------------------
// wadarchiveclient.cpp
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
// Copyright (C) 2015 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "wadarchiveclient.h"

#include "entities/checksum.h"
#include "entities/waddownloadinfo.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

class WadArchiveClient::PrivData
{
public:
	WadDownloadInfo currentWad;
	QNetworkAccessManager *nam;
	QNetworkReply *replyName;
	QNetworkReply *replyChecksum;
	QString userAgent;
	QList<WadDownloadInfo> queue;
	QList<QUrl> urls;

	QUrl buildUrlName(const WadDownloadInfo &wad)
	{
		if (!wad.isValid() || wad.name().contains("/"))
		{
			return QUrl();
		}
		return QUrl(QString("https://www.wad-archive.com/wadseeker/%1").arg(wad.name()));
	}

	QUrl buildUrlChecksum(const WadDownloadInfo &wad)
	{
		if (wad.isValid() || !wad.name().contains("/"))
		{
			for (Checksum checksum : wad.checksums())
			{
				if (checksum.algorithm() == QCryptographicHash::Md5 || checksum.algorithm() == QCryptographicHash::Sha1)
					return QUrl(QString("https://www.wad-archive.com/api/latest/%1").arg(QString(checksum.hash().toHex())));
			}
		}
		return QUrl();
	}

	QUrl buildBadUrlReporterUrl(const QUrl &url)
	{
		return QUrl(QString("https://www.wad-archive.com/wadseeker/missing/%1")
			.arg(QString(url.toString().toUtf8().toBase64())));
	}
};


WadArchiveClient::WadArchiveClient()
{
	d = new PrivData();
	d->nam = new QNetworkAccessManager();
	d->replyName = nullptr;
	d->replyChecksum = nullptr;
}

WadArchiveClient::~WadArchiveClient()
{
	abort();
	if (d->replyName != nullptr)
	{
		d->replyName->deleteLater();
	}
	if (d->replyChecksum != nullptr)
	{
		d->replyChecksum->deleteLater();
	}
	d->nam->deleteLater();
	delete d;
}

void WadArchiveClient::abort()
{
	d->queue.clear();
	if (d->replyName != nullptr)
	{
		d->replyName->abort();
	}
	if (d->replyChecksum != nullptr)
	{
		d->replyChecksum->abort();
	}
	else
	{
		emit finished();
	}
}

void WadArchiveClient::emitFinished()
{
	emit message(tr("Wad Archive finished."), WadseekerLib::NoticeImportant);
	emit finished();
}

void WadArchiveClient::enqueue(const WadDownloadInfo &wad)
{
	if (d->buildUrlName(wad).isValid())
	{
		d->queue << wad;
	}
}

bool WadArchiveClient::isWorking() const
{
	return !d->queue.isEmpty() || d->replyName != nullptr || d->replyChecksum != nullptr;
}

void WadArchiveClient::setUserAgent(const QString &userAgent)
{
	d->userAgent = userAgent;
}

void WadArchiveClient::start()
{
	#ifndef NDEBUG
	qDebug() << "WadArchiveClient::start()";
	#endif
	emit message(tr("Wad Archive started."), WadseekerLib::Notice);
	startNextInQueue();
}

void WadArchiveClient::startNextInQueue()
{
	if (d->queue.isEmpty())
	{
		emitFinished();
		return;
	}
	d->currentWad = d->queue.takeFirst();
	QUrl urlName = d->buildUrlName(d->currentWad);
	QUrl urlChecksum = d->buildUrlChecksum(d->currentWad);
	#ifndef NDEBUG
	qDebug() << "wad archive search:" << urlName;
	qDebug() << "wad archive checksum:" << urlChecksum;
	#endif
	emit message(tr("Querying Wad Archive for %1").arg(d->currentWad.name()), WadseekerLib::Notice);

	d->replyName = startQNetworkReply(urlName);
	d->replyChecksum = startQNetworkReply(urlChecksum);
	this->connect(d->replyName, SIGNAL(finished()), SLOT(onQueryFinished()));
	this->connect(d->replyChecksum, SIGNAL(finished()), SLOT(onQueryFinished()));
}

void WadArchiveClient::onQueryFinished()
{
	if (d->replyName->isFinished() && d->replyChecksum->isFinished())
	{
		emit message(tr("Wad Archive query finished."), WadseekerLib::Notice);
		QVariantList elementsName = QJsonDocument::fromJson(d->replyName->readAll()).toVariant().toList();
		QVariantMap elementsChecksum = QJsonDocument::fromJson(d->replyChecksum->readAll()).toVariant().toMap();
		if (elementsName.size() > 0)
		{
			parseWadArchiveStructure(elementsName[0].toMap(), elementsChecksum);
		}

		d->replyName->deleteLater();
		d->replyName = nullptr;
		d->replyChecksum->deleteLater();
		d->replyChecksum = nullptr;
		startNextInQueue();
	}
}

void WadArchiveClient::parseWadArchiveStructure(const QVariantMap &mapName, const QVariantMap &mapChecksum)
{
	QVariantList linksName = mapName["links"].toList();
	QVariantList linksChecksum = mapChecksum["links"].toList();
	for (QVariant link : linksName)
	{
		if (linksChecksum.isEmpty() || linksChecksum.contains(link))
		{
			d->urls << QUrl(link.toString());
			emit urlFound(d->currentWad.name(), QUrl(link.toString()));
		}
	}
}

void WadArchiveClient::reportBadUrlIfOriginatingFromHere(const QUrl &url)
{
	if (d->urls.contains(url))
	{
		emit message(tr("Reporting bad URL to Wad Archive: %1").arg(url.toString()),
			WadseekerLib::Notice);
		QUrl reportUrl = d->buildBadUrlReporterUrl(url);
		#ifndef NDEBUG
		qDebug() << "Wad Archive report url:" << reportUrl;
		#endif
		QNetworkReply *reply = startQNetworkReply(reportUrl);
		connect(reply, SIGNAL(finished()), reply, SLOT(deleteLater()));
	}
}

QNetworkReply *WadArchiveClient::startQNetworkReply(const QUrl &url)
{
	QNetworkRequest request;
	request.setUrl(url);
	//MUST BE CHANGED. For the moment this skips the bot filter in the API.
	request.setRawHeader("User-Agent", d->userAgent.toUtf8());
	return d->nam->get(request);
}
