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
#include "wadretriever/wadretriever.h"
#include "wadseekerversioninfo.h"
#include <QDir>
#include <QNetworkReply>
#include <QScopedPointer>

DClass<ModInstall>
{
public:
	QString error;
	QScopedPointer<WadRetriever> wadRetriever;
};
DPointeredNoCopy(ModInstall)

ModInstall::ModInstall(QObject *parent)
	: QObject(parent)
{
	d->wadRetriever.reset(new WadRetriever());
}

ModInstall::~ModInstall()
{
}

void ModInstall::install(const QString &targetDir, const ModSet &modSet)
{
	d->error = QString();
	if (!QDir(targetDir).exists())
	{
		errorOut(tr("Target directory doesn't exist or isn't really a directory."));
		return;
	}
	d->wadRetriever.reset(new WadRetriever());
	d->wadRetriever->setUserAgent(WadseekerVersionInfo::userAgent());
	d->wadRetriever->setTargetSavePath(targetDir);
	d->wadRetriever->setMaxConcurrentWadDownloads(1);
	this->connect(d->wadRetriever.data(), SIGNAL(finished()),
		SIGNAL(finished()));
	this->connect(d->wadRetriever.data(), SIGNAL(pendingUrls()),
		SLOT(failInstallation()));
	this->connect(d->wadRetriever.data(),
		SIGNAL(wadDownloadProgress(WadDownloadInfo,qint64,qint64)),
		SLOT(onWadDownloadProgress(WadDownloadInfo,qint64,qint64)));
	QList<WadDownloadInfo> downloadInfo;
	for (const ModFile &file : modSet.modFiles())
	{
		downloadInfo << file.fileName();
	}
	d->wadRetriever->setWads(downloadInfo);
	for (const ModFile &file : modSet.modFiles())
	{
		d->wadRetriever->addUrl(file.fileName(), file.url());
	}
}

void ModInstall::failInstallation()
{
	d->error = tr("Files couldn't be downloaded.");
	d->wadRetriever->abort();
	emit finished();
}

void ModInstall::onWadDownloadProgress(const WadDownloadInfo &wadDownloadInfo,
	qint64 current, qint64 total)
{
	emit fileDownloadProgress(wadDownloadInfo.name(), current, total);
}

void ModInstall::errorOut(const QString &msg)
{
	d->error = msg;
	emit finished();
}

const QString &ModInstall::error() const
{
	return d->error;
}

bool ModInstall::isError() const
{
	return !error().isEmpty();
}
