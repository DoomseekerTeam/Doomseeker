//------------------------------------------------------------------------------
// ip2cupdater.cpp
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
// Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------
#include "ip2cupdater.h"

#include <QCryptographicHash>
#include <QFile>
#include <QNetworkAccessManager>
#include <QTemporaryFile>
#include <zlib.h>

#include "log.h"
#include "version.h"

IP2CUpdater::IP2CUpdater(QObject *parent)
	: QObject(parent)
{
	pCurrentNetworkReply = nullptr;
	pNetworkAccessManager = new QNetworkAccessManager();
}

IP2CUpdater::~IP2CUpdater()
{
	abort();

	if (pNetworkAccessManager != nullptr)
		pNetworkAccessManager->deleteLater();
}

void IP2CUpdater::abort()
{
	if (pCurrentNetworkReply != nullptr)
	{
		pCurrentNetworkReply->disconnect();
		pCurrentNetworkReply->abort();
		pCurrentNetworkReply->deleteLater();
	}
	pCurrentNetworkReply = nullptr;
}

void IP2CUpdater::checksumDownloadFinished()
{
	if (handleRedirect(*pCurrentNetworkReply, SLOT(checksumDownloadFinished())))
		return;

	if (pCurrentNetworkReply->error() != QNetworkReply::NoError)
	{
		gLog << tr("IP2C checksum check network error: %1").arg(pCurrentNetworkReply->errorString());
		abort();
		emit updateCheckFinished(UpdateCheckError);
		return;
	}

	QByteArray remoteMd5 = pCurrentNetworkReply->readAll();
	remoteMd5 = remoteMd5.trimmed();

	pCurrentNetworkReply->deleteLater();
	pCurrentNetworkReply = nullptr;

	QByteArray localMd5;
	QFile file(this->lastAsyncCallPath);
	if (file.open(QIODevice::ReadOnly))
	{
		localMd5 = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5);
		localMd5 = localMd5.toHex().toLower();
	}

	gLog << tr("Comparing IP2C hashes: local = %1, remote = %2").arg(
			QString(localMd5)).arg(QString(remoteMd5));
	bool needed = localMd5 != remoteMd5;
	if (needed)
		gLog << tr("IP2C update needed.");
	emit updateCheckFinished(needed ? UpdateNeeded : UpToDate);
}

const QUrl IP2CUpdater::dbChecksumUrl()
{
	return QUrl("https://doomseeker.drdteam.org/ip2c/IpToCountry_v3.dat.md5");
}

const QUrl IP2CUpdater::dbDownloadUrl()
{
	return QUrl("https://doomseeker.drdteam.org/ip2c/IpToCountry_v3.dat.gz");
}

void IP2CUpdater::downloadDatabase(const QString &savePath)
{
	this->lastAsyncCallPath = savePath;
	get(dbDownloadUrl(), SLOT(downloadFinished()));
}

void IP2CUpdater::downloadFinished()
{
	if (handleRedirect(*pCurrentNetworkReply, SLOT(downloadFinished())))
		return;

	QByteArray data = pCurrentNetworkReply->readAll();

	pCurrentNetworkReply->deleteLater();
	pCurrentNetworkReply = nullptr;

	// First we need to write it to a temporary file
	QTemporaryFile tmpFile;
	if (tmpFile.open())
	{
		tmpFile.write(data);

		QString tmpFilePath = tmpFile.fileName();

		QByteArray uncompressedData;
		gzFile gz = gzopen(tmpFilePath.toUtf8().constData(), "rb");
		if (gz != nullptr)
		{
			static const int CHUNK_SIZE = 128 * 1024;
			char chunk[CHUNK_SIZE];
			int bytesRead = 0;
			while ((bytesRead = gzread(gz, chunk, CHUNK_SIZE)) != 0)
			{
				uncompressedData.append(QByteArray(chunk, bytesRead));
			}
			gzclose(gz);

			retrievedData = uncompressedData;
		}
	}

	emit databaseDownloadFinished(retrievedData);
}

bool IP2CUpdater::handleRedirect(QNetworkReply &reply, const char *finishedSlot)
{
	QUrl possibleRedirectUrl = reply.attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
	QUrl url = reply.request().url();
	if (!possibleRedirectUrl.isEmpty() && possibleRedirectUrl != url)
	{
		// Redirect.
		if (possibleRedirectUrl.isRelative())
			possibleRedirectUrl = url.resolved(possibleRedirectUrl);
		get(possibleRedirectUrl, finishedSlot);
		return true;
	}
	return false;
}

void IP2CUpdater::get(const QUrl &url, const char *finishedSlot)
{
	abort();
	retrievedData.clear();

	QNetworkRequest request;
	request.setUrl(url);
	request.setRawHeader("User-Agent", Version::userAgent().toUtf8());

	pCurrentNetworkReply = pNetworkAccessManager->get(request);
	this->connect(pCurrentNetworkReply, SIGNAL(downloadProgress(qint64,qint64)),
		SIGNAL(downloadProgress(qint64,qint64)));
	this->connect(pCurrentNetworkReply, SIGNAL(finished()), finishedSlot);
}

bool IP2CUpdater::getRollbackData(const QString &databasePath)
{
	rollbackData.clear();
	QFile file(databasePath);
	if (file.open(QIODevice::ReadOnly))
	{
		rollbackData = file.readAll();
		file.close();
		return true;
	}
	return false;
}

bool IP2CUpdater::isWorking() const
{
	return pCurrentNetworkReply != nullptr;
}

void IP2CUpdater::startUpdateCheck(const QString &filePath)
{
	QFile file(filePath);
	if (!file.exists())
	{
		emit updateCheckFinished(UpdateNeeded);
		return;
	}

	this->lastAsyncCallPath = filePath;
	gLog << tr("Checking if IP2C database at '%1' needs updating.").arg(filePath);
	get(dbChecksumUrl(), SLOT(checksumDownloadFinished()));
}

bool IP2CUpdater::rollback(const QString &savePath)
{
	bool bSuccess = save(rollbackData, savePath);
	rollbackData.clear();

	return bSuccess;
}

bool IP2CUpdater::save(const QByteArray &saveWhat, const QString &savePath)
{
	if (saveWhat.isEmpty())
		return false;

	QFile file(savePath);
	if (file.open(QIODevice::WriteOnly))
	{
		file.write(saveWhat);
		file.close();
		return true;
	}
	return false;
}

bool IP2CUpdater::saveDownloadedData(const QString &savePath)
{
	return save(retrievedData, savePath);
}
