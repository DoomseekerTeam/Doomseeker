//------------------------------------------------------------------------------
// internalclientinstaller.cpp
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
// Copyright (C) 2019 Pol Marcet Sardà <polmarcetsarda@gmail.com>
//------------------------------------------------------------------------------

#include "internalclientinstaller.h"
#include "ui_internalclientinstaller.h"

#include "datapaths.h"
#include "serverapi/message.h"
#include "serverapi/exefile.h"
#include "version.h"
#include "wadseeker/zip/unarchive.h"

#include <QBuffer>
#include <QNetworkRequest>
#include <QNetworkAccessManager>

DClass<InternalClientInstaller> : public Ui::InternalClientInstaller
{
public:
	InstallationPublicData data;
	ExeFile *engine;
	Message message;

	QByteArray downloadedFileData;
	QString downloadedFilename;
	QNetworkAccessManager networkAccessManager;
	QSharedPointer<QNetworkReply> pNetworkReply;
	QString networkError;
};
DPointeredNoCopy(InternalClientInstaller)

InternalClientInstaller::InternalClientInstaller(QWidget *parent, ExeFile *engine) :
	QDialog(parent)
{
	d->setupUi(this);
	d->data = engine->createInstallationData();
	d->engine = engine;
	d->message = Message::Type::SUCCESSFUL; //If no error is encountered, this won't change

	d->lblVersion->setText(tr("Version: %1").arg(d->data.engineVersion));
	d->lblEngine->setText(tr("Engine: %1").arg(d->data.engineName));
	d->lblInstallationPath->setText(tr("Installation Path: %1").arg(d->data.installationPath.absolutePath()));
	d->lblOverwriteWarning->setVisible(!d->data.installationPath.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty());
	if (!d->data.engineIcon.isNull())
		d->iconEngine->setPixmap(d->data.engineIcon.scaledToWidth(d->iconEngine->width()));
	else
		d->iconEngine->setVisible(false);
	layout()->setSizeConstraint(QLayout::SetFixedSize);
	isDownloading(false);
}

InternalClientInstaller::~InternalClientInstaller()
{
	if (!d->pNetworkReply.isNull())
	{
		d->pNetworkReply->disconnect();
		d->pNetworkReply->abort();
		d->pNetworkReply->deleteLater();
	}
	d->networkAccessManager.disconnect();
	d->networkAccessManager.deleteLater();
}

Message& InternalClientInstaller::getMessage() const
{
	return d->message;
}

void InternalClientInstaller::isDownloading(const bool& downloading)
{
	d->lblDescription->setVisible(!downloading);
	d->lblIntroduction->setVisible(!downloading);
	d->progressBar->setVisible(downloading);
	d->buttonBox->setStandardButtons(downloading ? QDialogButtonBox::Cancel :
		QDialogButtonBox::Yes | QDialogButtonBox::No);
	adjustSize();
}

void InternalClientInstaller::buttonBoxClicked(QAbstractButton* button)
{
	QPushButton* pButton = (QPushButton*)button;
	if (pButton == d->buttonBox->button(QDialogButtonBox::Yes))
	{
		directorySanityCheck();
		d->engine->preInstallationSteps(d->message);
		getUrl(d->engine->getUrl(d->message));
	}
	else if (pButton == d->buttonBox->button(QDialogButtonBox::No))
	{
		reject();
	}
	else if (pButton == d->buttonBox->button(QDialogButtonBox::Cancel))
	{
		reject();
	}
}

void InternalClientInstaller::directorySanityCheck()
{
	if (d->data.installationPath.absolutePath().isEmpty())
		d->message = Message::customError(
			tr("No directory for internal clients specified for engine: %1").arg(d->data.engineName));

	if (!d->data.installationPath.mkpath("."))
		d->message = Message::customError(tr("Unable to create directory:\n%1").arg(d->data.installationPath.absolutePath()));

	if (!DataPaths::checkDirPermissions(d->data.installationPath.absolutePath()))
		d->message = Message::customError(tr("Lack of necessary permissions to the directory:\n%1")
			.arg(d->data.installationPath.absolutePath()));

	if (d->message.isError())
		reject(d->message);
}

void InternalClientInstaller::getUrl(const QUrl& url)
{
	d->buttonBox->setEnabled(false);
	isDownloading(true);
	QNetworkRequest request;
	request.setUrl(url);
	request.setRawHeader("User-Agent", Version::userAgent().toUtf8());

	d->pNetworkReply.reset(d->networkAccessManager.get(request));

	connect(d->pNetworkReply.data(), SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
	connect(d->pNetworkReply.data(), SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(errorReceived(QNetworkReply::NetworkError)));
	connect(d->pNetworkReply.data(), SIGNAL(finished()), this, SLOT(downloadFinished()));
	d->buttonBox->setEnabled(true);
}

void InternalClientInstaller::errorReceived(QNetworkReply::NetworkError code)
{
	d->networkError = d->pNetworkReply->errorString();
	d->message = Message::customError(tr("Failed to download testing binary.\n\n%1").arg(d->networkError));
	reject(d->message);
}

void InternalClientInstaller::downloadFinished()
{
	QUrl possibleRedirectUrl = d->pNetworkReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
	QUrl url = d->pNetworkReply->request().url();
	if (!possibleRedirectUrl.isEmpty()
		&& possibleRedirectUrl != url)
	{
		// Redirect.
		if (possibleRedirectUrl.isRelative())
		{
			possibleRedirectUrl = url.resolved(possibleRedirectUrl);
		}

		d->pNetworkReply->deleteLater();
		getUrl(possibleRedirectUrl);
	}
	else
	{
		QUrl url = d->pNetworkReply->request().url();
		QFileInfo fi(url.path());
		QString filename = fi.fileName();

		d->downloadedFilename = filename;
		d->downloadedFileData = d->pNetworkReply->readAll();

		unpackDownload();
		if (d->message.isError())
			reject(d->message);

		// Allow engine to de it's final steps.
		d->engine->postInstallationSteps(d->message);
		if (d->message.isError())
			reject(d->message);
		accept();
	}
}

void InternalClientInstaller::downloadProgress(qint64 value, qint64 max)
{
	d->progressBar->setValue(value);
	d->progressBar->setMaximum(max);
}

void InternalClientInstaller::unpackDownload()
{
	// Extract the needed files.
	QFileInfo fi(d->downloadedFilename);
	QByteArray data = d->downloadedFileData;
	QBuffer dataBuffer(&data);
	QString executable = d->engine->getClientExecutableName();
	QScopedPointer<UnArchive> archive(UnArchive::openArchive(fi, &dataBuffer));
	if(!archive.isNull())
	{
		for(int i = 0;;++i)
		{
			QString filename = archive->fileNameFromIndex(i);

			if (filename.isNull())
			{
				break;
			}

			QString fileTargetPath = d->data.installationPath.path() + QDir::separator() + filename;
			if (fileTargetPath.contains('/'))
			{
				QFileInfo targetPathInfo(fileTargetPath);
				QString path = targetPathInfo.path();
				QDir dir;
				if (!dir.exists(path))
					dir.mkdir(path);
			}
			if(archive->extract(i, fileTargetPath))
			{
				// Make sure we can execute the binary.
				if(executable.compare(QFileInfo(filename).fileName(), Qt::CaseInsensitive) == 0)
				{
					QFile binaryFile(fileTargetPath);
					binaryFile.setPermissions(binaryFile.permissions() | QFile::ExeUser);
				}
			}
			else
			{
				d->message = Message::customError(tr("Extrating client failed. Responsible file: %1").arg(filename));
			}
		}
	}
	else
	{
		d->message = Message::customError(tr("Could not extract the packed client. Responsible file: %1").arg(fi.absolutePath()));
	}
}

void InternalClientInstaller::reject()
{
	reject(Message::Type::CANCELLED);
}

void InternalClientInstaller::reject(Message message)
{
		if (!d->pNetworkReply.isNull())
	{
		d->pNetworkReply->disconnect();
		d->pNetworkReply->abort();
		d->pNetworkReply->deleteLater();
		d->pNetworkReply.reset(NULL);
	}
	d->message = message;
	done(Rejected);
}