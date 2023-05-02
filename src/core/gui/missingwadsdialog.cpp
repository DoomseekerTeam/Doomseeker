//------------------------------------------------------------------------------
// missingwadsdialog.cpp
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
#include "missingwadsdialog.h"

#include "application.h"
#include "gui/mainwindow.h"
#include "gui/wadseekerinterface.h"
#include "serverapi/serverstructs.h"
#include "ui_missingwadsdialog.h"
#include <QListWidgetItem>
#include <wadseeker/entities/modfile.h>
#include <wadseeker/freedoom.h>
#include <wadseeker/wadseeker.h>

DClass<MissingWadsDialog> : public Ui::MissingWadsDialog
{
public:
	::MissingWadsDialog::MissingWadsProceed decision;
	QList<PWad> missingWads;
	QList<PWad> incompatibleWads;
};
DPointeredNoCopy(MissingWadsDialog)

MissingWadsDialog::MissingWadsDialog(const QList<PWad> &missingWads, const QList<PWad> &incompatibleWads, QWidget *parent)
	: QDialog(parent)
{
	d->setupUi(this);
	d->decision = Cancel;
	d->missingWads = missingWads;
	d->incompatibleWads = incompatibleWads;

	setup();
	adjustSize();
}

MissingWadsDialog::~MissingWadsDialog()
{
}

void MissingWadsDialog::setAllowIgnore(bool allow)
{
	d->btnIgnore->setVisible(allow);
	d->lblUseIgnore->setVisible(allow);
	d->lblUseIgnoreCantRun->setVisible(allow);
}

void MissingWadsDialog::setup()
{
	d->btnInstallFreedoom->hide();
	if (WadseekerInterface::isInstantiated())
		setupWadseekerIsRunning();
	else
		setupWadseekerNotRunning();
}

void MissingWadsDialog::setupWadseekerIsRunning()
{
	d->areaWadseekerAlreadyRunning->show();
	d->areaWadseekerCanBeRun->hide();
	d->btnInstall->hide();
}

void MissingWadsDialog::setupWadseekerNotRunning()
{
	d->areaWadseekerAlreadyRunning->hide();
	d->areaWadseekerCanBeRun->show();

	setupForbiddenFilesArea();
	setupDownloadableFilesArea();
	setupOptionalFilesArea();
	setupIncompatibleFilesArea();

	d->btnInstall->setVisible(hasAnyAllowedFile());
}

void MissingWadsDialog::setupForbiddenFilesArea()
{
	QList<PWad> files = forbiddenFiles();
	if (!files.isEmpty())
	{
		d->areaCantBeDownloaded->show();
		QStringList names;
		for (PWad file : files)
		{
			names << file.name();
		}
		d->lblCantBeDownloadedFiles->setText(names.join(", "));

		bool installFreedoom = isFreedoomReplaceableOnList(names);
		d->lblInstallFreedoom->setVisible(installFreedoom);
		d->btnInstallFreedoom->setVisible(installFreedoom);
	}
	else
		d->areaCantBeDownloaded->hide();
}

void MissingWadsDialog::setupDownloadableFilesArea()
{
	QList<PWad> files = downloadableFiles();
	if (!files.isEmpty())
	{
		d->areaCanBeDownloadedFiles->show();
		QStringList names;
		for (PWad file : files)
		{
			names << file.name();
		}
		d->lblCanBeDownloadedFiles->setText(names.join(", "));
	}
	else
		d->areaCanBeDownloadedFiles->hide();
}

void MissingWadsDialog::setupOptionalFilesArea()
{
	QList<PWad> files = optionalFiles();
	if (!files.isEmpty())
	{
		d->areaOptionalFiles->show();
		for (const PWad &file : files)
		{
			auto item = new QListWidgetItem(file.name(), d->optionalFilesList);
			item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
			item->setCheckState(Qt::Checked);
		}
	}
	else
		d->areaOptionalFiles->hide();
}

void MissingWadsDialog::setupIncompatibleFilesArea()
{
	QList<PWad> files = incompatibleFiles();
	if (!files.isEmpty())
	{
		d->areaIncompatibleFiles->show();
		for (const PWad &file : files)
		{
			auto item = new QListWidgetItem(file.name(), d->incompatibleFilesList);
			item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
			item->setCheckState(Qt::Checked);
		}
	}
	else
		d->areaIncompatibleFiles->hide();
}

bool MissingWadsDialog::isFreedoomReplaceableOnList(const QStringList &files) const
{
	for (const QString &file : files)
	{
		if (Freedoom::hasFreedoomReplacement(file))
			return true;
	}
	return false;
}

void MissingWadsDialog::updateStateAccordingToFileSelection()
{
	d->btnInstall->setEnabled(!filesToDownload().isEmpty());
}

void MissingWadsDialog::ignoreMissingFiles()
{
	d->decision = Ignore;
	accept();
}

void MissingWadsDialog::installFreedoom()
{
	QTimer::singleShot(0, gApp->mainWindow(), SLOT(showInstallFreedoomDialog()));
	accept();
}

void MissingWadsDialog::installMissingFiles()
{
	d->decision = Install;
	accept();
}

QList<PWad> MissingWadsDialog::downloadableFiles() const
{
	QList<PWad> result;
	for (PWad file : d->missingWads)
	{
		if (!Wadseeker::isForbiddenWad(file) && !file.isOptional())
			result << file;
	}
	return result;
}

QList<PWad> MissingWadsDialog::forbiddenFiles() const
{
	QList<PWad> result;
	QList<PWad> wads;
	wads << d->missingWads << d->incompatibleWads;
	for (const PWad &file : wads)
	{
		if (Wadseeker::isForbiddenWad(file.name()))
			result << file;
	}
	return result;
}

QList<PWad> MissingWadsDialog::optionalFiles() const
{
	QList<PWad> result;
	for (const PWad &file : d->missingWads)
	{
		if (!Wadseeker::isForbiddenWad(file.name()) && file.isOptional())
			result << file;
	}
	return result;
}

QList<PWad> MissingWadsDialog::incompatibleFiles() const
{
	QList<PWad> result;
	for (const PWad &file : d->incompatibleWads)
	{
		if (!Wadseeker::isForbiddenWad(file.name()))
			result << file;
	}
	return result;
}

QList<PWad> MissingWadsDialog::filesToDownload() const
{
	QList<PWad> result = downloadableFiles();
	result << selectedOptionalFiles();
	result << selectedIncompatibleFiles();
	return result;
}

QList<PWad> MissingWadsDialog::selectedOptionalFiles() const
{
	QStringList result;
	for (int i = 0; i < d->optionalFilesList->count(); ++i)
	{
		QListWidgetItem *item = d->optionalFilesList->item(i);
		if (item->checkState() == Qt::Checked)
			result << item->text();
	}
	return filenamesToPwads(result, optionalFiles());
}

QList<PWad> MissingWadsDialog::selectedIncompatibleFiles() const
{
	QStringList result;
	for (int i = 0; i < d->incompatibleFilesList->count(); ++i)
	{
		QListWidgetItem *item = d->incompatibleFilesList->item(i);
		if (item->checkState() == Qt::Checked)
			result << item->text();
	}
	return filenamesToPwads(result, incompatibleFiles());
}

QList<PWad> MissingWadsDialog::filenamesToPwads(const QStringList &names, QList<PWad> files) const
{
	QList<PWad> result;
	for (QString name : names)
	{
		for (int i = 0; i < files.size(); ++i)
		{
			if (files[i].name() == name)
			{
				result << files[i];
				files.removeAt(i);
				break;
			}
		}
	}
	return result;
}

bool MissingWadsDialog::hasAnyAllowedFile() const
{
	return !downloadableFiles().isEmpty() ||
		!optionalFiles().isEmpty() ||
		!incompatibleFiles().isEmpty();
}

MissingWadsDialog::MissingWadsProceed MissingWadsDialog::decision() const
{
	return d->decision;
}
