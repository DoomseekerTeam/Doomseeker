//------------------------------------------------------------------------------
// freedoomdialog.cpp
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
#include "freedoomdialog.h"

#include "configuration/doomseekerconfig.h"
#include "fileutils.h"
#include "pathfinder/filesearchpath.h"
#include "pathfinder/pathfinder.h"
#include "pathfinder/wadpathfinder.h"
#include "ui_freedoomdialog.h"
#include <QCheckBox>
#include <QCompleter>
#include <QDirModel>
#include <QMessageBox>
#include <wadseeker/entities/checksum.h>
#include <wadseeker/entities/modset.h>
#include <wadseeker/freedoom.h>
#include <wadseeker/modinstall.h>

DClass<FreedoomDialog> : public Ui::FreedoomDialog
{
public:
	Freedoom *freedoom;
	ModInstall *modInstall;
	QString targetDir;
};
DPointered(FreedoomDialog)

FreedoomDialog::FreedoomDialog(QWidget *parent)
	: QDialog(parent)
{
	d->setupUi(this);

	d->freedoom = new Freedoom(this);
	this->connect(d->freedoom, SIGNAL(finished()), SLOT(applyFreedoomVersionInfo()));

	d->modInstall = new ModInstall(this);
	this->connect(d->modInstall, SIGNAL(finished()), SLOT(onModInstallFinished()));
	this->connect(d->modInstall, SIGNAL(fileDownloadProgress(QString,qint64,qint64)),
		SLOT(showFileDownloadProgress(QString,qint64,qint64)));

	d->btnInstall->setEnabled(false);
	setupInstallPaths();
	setupWadsTable();
	fetchInfo();
}

FreedoomDialog::~FreedoomDialog()
{
}

void FreedoomDialog::accept()
{
	ModSet modSet = selectedModFiles();
	if (modSet.isEmpty())
	{
		QMessageBox::critical(this, tr("Install Freedoom"),
			tr("Select at least one file."));
		return;
	}
	resetProgressBar();
	showStatus(tr("Downloading & installing ..."));
	d->targetDir = d->cboInstallPath->currentText();
	d->modInstall->install(d->targetDir, modSet);
}

void FreedoomDialog::onModInstallFinished()
{
	if (!d->modInstall->isError())
	{
		updateConfig();
		fetchInfo();
	}
	else
		showError(d->modInstall->error());
}

void FreedoomDialog::updateConfig()
{
	gConfig.doomseeker.enableFreedoomInstallation(d->targetDir);
	gConfig.saveToFile();
}

void FreedoomDialog::fetchInfo()
{
	resetProgressBar();
	d->btnRetry->hide();
	showStatus(tr("Downloading Freedoom version info ..."));
	d->freedoom->requestModSet();
}

void FreedoomDialog::resetProgressBar()
{
	d->progressBar->setFormat(tr("Working ..."));
	d->progressBar->show();
	d->progressBar->setMaximum(0);
	d->progressBar->setValue(0);
}

void FreedoomDialog::applyFreedoomVersionInfo()
{
	if (!d->freedoom->isError())
		showModInfo(d->freedoom->modSet());
	else
		showError(tr("Error: %1").arg(d->freedoom->error()));
}

void FreedoomDialog::showModInfo(const ModSet &modSet)
{
	d->wadsArea->show();
	d->workInProgressArea->hide();

	d->wadsTable->setSortingEnabled(false);
	d->wadsTable->clearContents();
	while (d->wadsTable->rowCount() > 0)
	{
		d->wadsTable->removeRow(d->wadsTable->rowCount() - 1);
	}
	for (const ModFile &file : modSet.modFiles())
	{
		insertModFile(file);
	}
	d->wadsTable->setSortingEnabled(true);
	d->btnInstall->setEnabled(true);
}

void FreedoomDialog::insertModFile(const ModFile &file)
{
	WadPathFinder pathFinder = WadPathFinder(PathFinder());
	pathFinder.setAllowAliases(false);
	QString location = pathFinder.find(file.fileName()).path();

	QString status = tr("OK");
	bool needsInstall = false;
	if (location.isEmpty())
	{
		needsInstall = true;
		status = tr("Missing");
	}
	else
	{
		QByteArray md5 = FileUtils::md5(location);
		for (Checksum checksum : file.checksums())
		{
			if (md5 != checksum.hash() && checksum.algorithm() == QCryptographicHash::Md5)
			{
				status = tr("Different");
				needsInstall = true;
				break;
			}
		}
	}
	if (file.fileName().compare("freedm.wad", Qt::CaseInsensitive) == 0)
		needsInstall = false;

	d->wadsTable->insertRow(d->wadsTable->rowCount());
	int row = d->wadsTable->rowCount() - 1;
	d->wadsTable->setItem(row, ColName, new QTableWidgetItem(file.fileName()));
	d->wadsTable->setItem(row, ColStatus, new QTableWidgetItem(status));

	auto checkBox = new QCheckBox();
	checkBox->setChecked(needsInstall);
	d->wadsTable->setCellWidget(row, ColInstall, checkBox);

	QString tooltip = tr("<p>File: %1<br>Version: %2<br>"
		"Description: %3<br>Location: %4</p>")
			.arg(file.name(), file.version(), file.description(), location);

	for (int col = 0; col < d->wadsTable->columnCount(); ++col)
	{
		QTableWidgetItem *item = d->wadsTable->item(row, col);
		if (item != nullptr)
			item->setToolTip(tooltip);
	}
}

void FreedoomDialog::showError(const QString &text)
{
	d->btnRetry->setVisible(true);
	showStatus(text);
	d->progressBar->hide();
}

void FreedoomDialog::showStatus(const QString &text)
{
	d->wadsArea->hide();
	d->workInProgressArea->show();
	d->btnInstall->setEnabled(false);
	d->lblWorkInProgress->setText(text);
}

ModSet FreedoomDialog::selectedModFiles() const
{
	ModSet modSet;
	for (int row = 0; row < d->wadsTable->rowCount(); ++row)
	{
		auto checkbox = static_cast<QCheckBox *>(d->wadsTable->cellWidget(row, ColInstall));
		if (checkbox->isChecked())
		{
			QString fileName = d->wadsTable->item(row, ColName)->text();
			modSet.addModFile(d->freedoom->modSet().findFileName(fileName));
		}
	}
	return modSet;
}

void FreedoomDialog::setupInstallPaths()
{
	auto completer = new QCompleter(this);
	completer->setModel(new QDirModel(completer));
	d->cboInstallPath->setCompleter(completer);

	for (const FileSearchPath &path : gConfig.combinedWadseekPaths())
	{
		d->cboInstallPath->addItem(path.path());
	}
	d->cboInstallPath->setCurrentText(gConfig.wadseeker.targetDirectory);
}

void FreedoomDialog::setupWadsTable()
{
	QHeaderView *header = d->wadsTable->horizontalHeader();
	header->setSectionResizeMode(ColName, QHeaderView::Stretch);
	header->setSectionResizeMode(ColStatus, QHeaderView::ResizeToContents);
	header->setSectionResizeMode(ColInstall, QHeaderView::ResizeToContents);
	d->wadsTable->setColumnWidth(ColName, 150);
}

void FreedoomDialog::showFileDownloadProgress(const QString &file,
	qint64 current, qint64 total)
{
	d->progressBar->setFormat(tr("%1 %p%").arg(file));
	d->progressBar->setMaximum(total);
	d->progressBar->setValue(current);
}
