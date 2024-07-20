//------------------------------------------------------------------------------
// modinstall.cpp
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
#include "modinstall.h"

#include "entities/modset.h"
#include "entities/waddownloadinfo.h"
#include "wadinstaller.h"
#include "wadseekerversioninfo.h"
#include "www/urlparser.h"
#include "www/webquery.h"
#include "www/www.h"
#include <QDir>
#include <QNetworkReply>
#include <QScopedPointer>

#include <cassert>
#include <functional>

DClass<ModInstall>
{
public:
	bool aborting;
	QString error;
	QNetworkAccessManager *net;
	QScopedPointer<WWW> www;
	QMap<QString, QSharedPointer<WebQuery>> inflight;
	QList<WadDownloadInfo> wads;
	QString targetDir;
};
DPointeredNoCopy(ModInstall)

ModInstall::ModInstall(QObject *parent)
	: QObject(parent)
{
	d->aborting = false;
	d->net = new QNetworkAccessManager();
	d->www.reset(new WWW(d->net));
	d->www->setUserAgent(WadseekerVersionInfo::userAgent());
}

ModInstall::~ModInstall()
{
	d->net->deleteLater();
}

void ModInstall::abort()
{
	if (d->aborting)
		return;

	d->aborting = true;
	if (!d->inflight.isEmpty())
	{
		for (QSharedPointer<WebQuery> inflight : d->inflight)
			inflight->abort();
	}
	else
	{
		emit finished();
	}
}

void ModInstall::install(const QString &targetDir, const ModSet &modSet)
{
	d->error = QString();
	d->targetDir = targetDir;
	if (!QDir(targetDir).exists())
	{
		errorOut(tr("Target directory doesn't exist or isn't really a directory."));
		return;
	}

	for (const ModFile &file : modSet.modFiles())
	{
		d->wads << WadDownloadInfo(file);
	}
	if (d->wads.isEmpty())
	{
		errorOut(tr("Nothing to install."));
		return;
	}

	for (const ModFile &file : modSet.modFiles())
	{
		// getFile() can errorOut(), in which case further files should
		// not be get. The ones that have already been scheduled for
		// getting will be aborted individually.
		if (d->aborting)
			break;
		this->getFile(file, file.url());
	}
}

void ModInstall::getFile(const ModFile &modFile, const QUrl &url)
{
	// TODO consider just using SeekSession here, but configured
	// so that it doesn't look for extra links.
	//
	// TODO What if multiple files have the same URL because they sit in
	// the same archive?
	//
	// TODO What if multiple URLs come from the same host and this host
	// blocks parallel downloads?
	using namespace std::placeholders;

	assert(!d->aborting);

	// Check if the URL is /kosher/ and don't even try if it's not.
	if (!UrlParser::isWwwUrl(url))
	{
		this->errorOut(tr("File \"%1\" has URL \"%2\"; Wadseeker is not handling this kind of URLs.")
			.arg(modFile.fileName())
			.arg(url.toString()));
		return;
	}

	QSharedPointer<WebQuery> query = d->www->get(url);
	QWeakPointer<WebQuery> queryWeak = query;
	this->connect(query.data(), &WebQuery::finished,
		this, [this, queryWeak, modFile]()
		{
			// TODO this needs to be reviewed because I bet it's bugged to heck
			// and doesn't cover all the edge-cases.
			QSharedPointer<WebQuery> query = queryWeak.toStrongRef();
			d->inflight.remove(modFile.name());

			if (query != nullptr)
			{
				if (query->isRedirected())
				{
					this->getFile(modFile, query->redirectUrl());
					return;
				}

				if (query->error() == QNetworkReply::NoError)
				{
					WadInstaller installer(d->targetDir, d->wads);
					WadInstallerResult result = installer.install(WadDownloadInfo(modFile), query->reply());
					for (QFileInfo file : result.installedWads)
					{
						// Stop looking for the installed WADs.
						for (auto it = d->wads.begin(); it != d->wads.end(); ++it)
						{
							if (it->name() == file.fileName())
							{
								d->wads.erase(it);
								break;
							}
						}
						// Abort the query for the just installed WAD, in case if
						// it had a separate one and this WAD was just installed
						// by an archive.
						QSharedPointer<WebQuery> filesQuery = d->inflight.value(file.fileName());
						d->inflight.remove(file.fileName());
						if (filesQuery != nullptr)
						{
							// We just installed that so no point in continuing the download.
							filesQuery->abort();
						}
					}
				}
				else if (query->error() != QNetworkReply::OperationCanceledError)
				{
					// TODO Who cares about individual file's errors, right?
					this->errorOut(tr("Web query for file %1 failed (%2): %3")
						.arg(modFile.name())
						.arg(query->url().toString())
						.arg(query->errorString()));
				}
				if (d->inflight.isEmpty())
					emit finished();
			}
			else
			{
				this->errorOut(tr("Lost track of a Web query for file %1.").arg(modFile.name()));
			}
		});
	this->connect(query.data(), &WebQuery::progress,
		this, std::bind(&ModInstall::fileDownloadProgress, this, modFile.fileName(), _1, _2));
}

void ModInstall::errorOut(const QString &msg)
{
	d->error = msg;
	abort();
}

const QString &ModInstall::error() const
{
	return d->error;
}

bool ModInstall::isError() const
{
	return !error().isEmpty();
}
