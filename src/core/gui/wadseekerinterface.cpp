//------------------------------------------------------------------------------
// wadseekerinterface.cpp
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
#include "gui/wadseekerinterface.h"
#include "ui_wadseekerinterface.h"

#include "application.h"
#include "configuration/doomseekerconfig.h"
#include "gui/commongui.h"
#include "gui/helpers/taskbarbutton.h"
#include "gui/helpers/taskbarprogress.h"
#include "mainwindow.h"
#include "serverapi/server.h"
#include "serverapi/serverstructs.h"
#include "strings.hpp"
#include "templatedpathresolver.h"
#include "wadseeker/entities/checksum.h"
#include "wadseeker/entities/modfile.h"
#include "wadseeker/entities/modset.h"

#include <QFontDatabase>
#include <QMessageBox>
#include <QPushButton>

#include <algorithm>

namespace
{
struct LogLevel
{
	QString tag;
	QString style;
};

class LogLevelMap
{
public:
	LogLevelMap() {}

	LogLevel operator[](const WadseekerLib::MessageType type) const
	{
		return this->map.value(type, this->unknown);
	}

	void insert(WadseekerLib::MessageType type, LogLevel level)
	{
		this->map.insert(type, level);
	}

	void refit()
	{
		for (LogLevel &lvl : this->map)
			lvl.tag = lvl.tag.trimmed();
		int longest = 0;
		for (const LogLevel &lvl : this->map)
			longest = std::max<int>(lvl.tag.length(), longest);
		for (LogLevel &lvl : this->map)
		{
			while (lvl.tag.length() < longest)
				lvl.tag.prepend(' ');
		}
		this->unknown = LogLevel{ QString(longest, '?'), QString() };
	}

private:
	QMap<WadseekerLib::MessageType, LogLevel> map;
	LogLevel unknown;
};
};

const int WadseekerInterface::UPDATE_INTERVAL_MS = 500;
WadseekerInterface *WadseekerInterface::currentInstance = nullptr;

DClass<WadseekerInterface> : public Ui::WadseekerInterface
{
public:
	QPushButton *btnAbort;
	QPushButton *btnClose;

	QString name;
	LogLevelMap loglevels;

	bool bCompletedSuccessfully;
	bool preventGame;
	TaskbarButton *taskbarButton;
	TaskbarProgress *taskbarProgress;
};

DPointered(WadseekerInterface)

WadseekerInterface::WadseekerInterface(QWidget *parent)
	: QDialog(parent)
{
	construct();
	bAutomatic = false;
}

WadseekerInterface::WadseekerInterface(ServerPtr server, QWidget *parent)
	: QDialog(parent)
{
	construct();
	setupAutomatic();
	d->lblTop->show();
	d->lblTop->setText(tr("Downloading WADs for server \"%1\"").arg(server->name()));
	setCustomSites(server->allWebSites());
}

WadseekerInterface::~WadseekerInterface()
{
	currentInstance = nullptr;
}

void WadseekerInterface::abortService(const QString &service)
{
	appendLog(tr("Aborting service: %1").arg(service), WadseekerLib::Notice, d->name);
	wadseeker.skipService(service);
}

void WadseekerInterface::abortSite(const QUrl &url)
{
	appendLog(tr("Aborting site: %1").arg(url.toString()), WadseekerLib::Notice, d->name);
	wadseeker.skipSiteSeek(url);
}

void WadseekerInterface::accept()
{
	if (isAutomatic())
	{
		if (d->bCompletedSuccessfully)
			done(QDialog::Accepted);
	}
	else
	{
		if (d->leWadName->text().isEmpty())
			return;
		else
		{
			seekedWads.clear();
			QStringList pwadNames = d->leWadName->text().split(',', Qt::SkipEmptyParts);
			for (QString pwadName : pwadNames)
			{
				seekedWads << pwadName.trimmed();
			}
		}
		startSeeking(seekedWads);
	}
}

void WadseekerInterface::appendLog(const QString &message, WadseekerLib::MessageType type, const QString &source)
{
	static const int MAX_SOURCE_LEN = QString("Doomseeker").length();
	static const QString lineStart = "<span style=\"%1; white-space: pre-wrap;\">";
	static const QString lineEnd = "</span><br>\n";
	const LogLevel level = d->loglevels[type];

	QString log;
	for (const QString &line : message.split("\n"))
	{
		log += lineStart.arg(level.style)
			+ QString("%1|%2| %3").arg(source.left(MAX_SOURCE_LEN), -MAX_SOURCE_LEN).arg(level.tag, line)
			+ lineEnd;
	}

	d->teWadseekerOutput->moveCursor(QTextCursor::End);
	d->teWadseekerOutput->insertHtml(log);
}

void WadseekerInterface::allDone(bool bSuccess)
{
	setStateWaiting();
	d->bCompletedSuccessfully = bSuccess;
	QApplication::alert(this);
	if (bSuccess)
	{
		appendLog(tr("All done. Success."), WadseekerLib::NoticeImportant, d->name);

		if (isAutomatic() && !d->preventGame)
		{
			if (isActiveWindow())
				done(QDialog::Accepted);
			else
				d->btnStartGame->show();
		}
	}
	else
	{
		QList<PWad> failures = unsuccessfulWads();

		for (const PWad &failure : failures)
		{
			d->twWads->setFileFailed(failure.name());
		}

		appendLog(tr("All done. Fail."), WadseekerLib::CriticalError, d->name);
	}
}

void WadseekerInterface::connectWadseekerObject()
{
	// Connect Wadseeker to the dialog box.
	connect(&wadseeker, &Wadseeker::allDone,
		this, &WadseekerInterface::allDone);
	connect(&wadseeker, &Wadseeker::message,
		this, [this](const QString &msg, WadseekerLib::MessageType type)
		{ this->appendLog(msg, type, tr("Wadseeker")); });
	connect(&wadseeker, &Wadseeker::seekStarted,
		this, &WadseekerInterface::seekStarted);
	connect(&wadseeker, &Wadseeker::fileInstalled,
		this, &WadseekerInterface::fileDownloadSuccessful);
	connect(&wadseeker, &Wadseeker::siteFinished,
		this, &WadseekerInterface::siteFinished);
	connect(&wadseeker, &Wadseeker::siteProgress,
		this, &WadseekerInterface::siteProgress);
	connect(&wadseeker, &Wadseeker::siteRedirect,
		this, &WadseekerInterface::siteRedirect);
	connect(&wadseeker, &Wadseeker::siteStarted,
		this, &WadseekerInterface::siteStarted);
	connect(&wadseeker, &Wadseeker::serviceStarted,
		this, &WadseekerInterface::serviceStarted);
	connect(&wadseeker, &Wadseeker::serviceFinished,
		this, &WadseekerInterface::serviceFinished);

	// Connect Wadseeker to the WADs table widget.
	connect(&wadseeker, &Wadseeker::fileDownloadFinished,
		d->twWads, &WadseekerWadsTable::setFileDownloadFinished);
	connect(&wadseeker, &Wadseeker::fileDownloadProgress,
		d->twWads, &WadseekerWadsTable::setFileProgress);
	connect(&wadseeker, &Wadseeker::fileDownloadStarted,
		d->twWads, &WadseekerWadsTable::setFileUrl);
}

void WadseekerInterface::construct()
{
	d->setupUi(this);
	CommonGUI::setupDialog(*this);
	d->name = "Doomseeker";

	d->teWadseekerOutput->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	d->preventGame = false;
	d->bCompletedSuccessfully = false;
	d->btnAbort = d->buttonBox->button(QDialogButtonBox::Abort);
	d->btnClose = d->buttonBox->button(QDialogButtonBox::Close);

	d->taskbarButton = new TaskbarButton(this);

	d->taskbarProgress = d->taskbarButton->progress();
	d->taskbarProgress->setMaximum(d->pbOverallProgress->maximum());

	initMessageColors();

	setStateWaiting();

	this->setWindowIcon(QIcon(":/icon.png"));
	d->btnStartGame->hide();
	connect(&this->updateTimer, &QTimer::timeout,
		this, &WadseekerInterface::registerUpdateRequest);

	connectWadseekerObject();

	// Connect tables.
	connect(d->twWads, &QWidget::customContextMenuRequested,
		this, &WadseekerInterface::showWadsTableContextMenu);

	bAutomatic = false;
	bFirstShown = false;

	QStringList urlList = gConfig.wadseeker.searchURLs;
	if (gConfig.wadseeker.bAlwaysUseDefaultSites)
	{
		for (int i = 0; !Wadseeker::defaultSites[i].isEmpty(); ++i)
			urlList << Wadseeker::defaultSites[i];
	}

	wadseeker.setPrimarySites(urlList);

	updateTimer.setSingleShot(false);
	updateTimer.start(UPDATE_INTERVAL_MS);
}

WadseekerInterface *WadseekerInterface::create(QWidget *parent)
{
	if (!isInstantiated())
	{
		currentInstance = new WadseekerInterface(parent);
		return currentInstance;
	}
	return nullptr;
}

WadseekerInterface *WadseekerInterface::create(ServerPtr server, QWidget *parent)
{
	if (!isInstantiated())
	{
		currentInstance = new WadseekerInterface(server, parent);
		return currentInstance;
	}
	return nullptr;
}

WadseekerInterface *WadseekerInterface::createAutoNoGame(QWidget *parent)
{
	WadseekerInterface *interface = create(parent);
	if (interface != nullptr)
	{
		interface->setupAutomatic();
		interface->d->preventGame = true;
	}
	return interface;
}

void WadseekerInterface::fileDownloadSuccessful(const ModFile &filename)
{
	successfulWads << filename;
	d->twWads->setFileSuccessful(filename.fileName());
}

void WadseekerInterface::initMessageColors()
{
	this->colorHtmlMessageNotice = gConfig.wadseeker.colorMessageNotice;
	this->colorHtmlMessageError = gConfig.wadseeker.colorMessageError;
	this->colorHtmlMessageFatalError = gConfig.wadseeker.colorMessageCriticalError;
	this->colorHtmlMessageNavigation = gConfig.wadseeker.colorMessageNavigation;

	d->loglevels = LogLevelMap();
	// The tags are translatable, but they should be kept short
	// regardless of the translation. 5 characters max is best.
	// Doomseeker refits all the tags to the same length, padding
	// them with spaces from the left.
	d->loglevels.insert(WadseekerLib::CriticalError, {
		tr("CRIT"),
		QString("color: %1; font-weight: bold;").arg(colorHtmlMessageFatalError),
	});
	d->loglevels.insert(WadseekerLib::Error, {
		tr("ERROR"),
		QString("color: %1;").arg(colorHtmlMessageError),
	});
	d->loglevels.insert(WadseekerLib::Notice, {
		tr("INFO"),
		QString("color: %1;").arg(colorHtmlMessageNotice),
	});
	d->loglevels.insert(WadseekerLib::NoticeImportant, {
		tr("NOTI"),
		QString("color: %1; font-weight: bold;").arg(colorHtmlMessageNotice),
	});
	d->loglevels.insert(WadseekerLib::Navigation, {
		tr("NAVI"),
		QString("color: %1;").arg(colorHtmlMessageNavigation),
	});
	d->loglevels.refit();
}

bool WadseekerInterface::isInstantiated()
{
	return currentInstance != nullptr;
}

void WadseekerInterface::registerUpdateRequest()
{
	updateProgressBar();
	updateTitle();
}

void WadseekerInterface::reject()
{
	switch (state)
	{
	case Downloading:
		wadseeker.abort();
		break;

	case Waiting:
		this->done(Rejected);
		break;
	}
}

void WadseekerInterface::resetTitleToDefault()
{
	setWindowTitle(tr("Wadseeker"));
}

void WadseekerInterface::seekStarted(const ModSet &filenames)
{
	QList<PWad> wads;
	QStringList names;
	for (ModFile modFile : filenames.modFiles())
	{
		wads << modFile;
		names << modFile.fileName();
	}
	d->teWadseekerOutput->clear();
	d->pbOverallProgress->setValue(0);
	d->taskbarProgress->setValue(0);
	appendLog("Seek started on filenames: " + names.join(", "), WadseekerLib::NoticeImportant, d->name);

	seekedWads = wads;
	successfulWads.clear();
	d->twSites->setRowCount(0);
	d->twWads->setRowCount(0);
	setStateDownloading();

	for (const PWad &wad : seekedWads)
	{
		d->twWads->addFile(wad.name());
	}
}

void WadseekerInterface::setStateDownloading()
{
	d->btnAbort->show();
	d->btnClose->hide();
	d->btnDownload->setEnabled(false);
	d->taskbarProgress->show();
	state = Downloading;
}

void WadseekerInterface::setStateWaiting()
{
	d->btnAbort->hide();
	d->btnClose->show();
	d->btnDownload->setEnabled(true);
	d->taskbarProgress->hide();
	state = Waiting;
}

void WadseekerInterface::setupAutomatic()
{
	bAutomatic = true;
	d->lblTop->hide();
	d->btnDownload->hide();
	d->leWadName->hide();
}

void WadseekerInterface::setWads(const QList<PWad> &wads)
{
	seekedWads = wads;
	if (!isAutomatic())
	{
		QStringList names;
		for (PWad wad : wads)
		{
			names << wad.name();
		}
		d->leWadName->setText(names.join(", "));
	}
}

void WadseekerInterface::setupIdgames()
{
	wadseeker.setIdgamesEnabled(gConfig.wadseeker.bSearchInIdgames);
	wadseeker.setIdgamesUrl(gConfig.wadseeker.idgamesURL);
}

void WadseekerInterface::showEvent(QShowEvent *event)
{
	Q_UNUSED(event);
	if (!bFirstShown)
	{
		d->taskbarButton->setWindow(windowHandle());
		bFirstShown = true;

		if (isAutomatic())
			startSeeking(seekedWads);
	}
}

void WadseekerInterface::serviceStarted(const QString &service)
{
	d->twSites->addService(service);
}

void WadseekerInterface::serviceFinished(const QString &service)
{
	d->twSites->removeService(service);
}

void WadseekerInterface::siteFinished(const QUrl &site)
{
	d->twSites->removeUrl(site);
}

void WadseekerInterface::siteProgress(const QUrl &site, qint64 bytes, qint64 total)
{
	d->twSites->setUrlProgress(site, bytes, total);
}

void WadseekerInterface::siteRedirect(const QUrl &oldUrl, const QUrl &newUrl)
{
	d->twSites->removeUrl(oldUrl);
	d->twSites->addUrl(newUrl);
}

void WadseekerInterface::siteStarted(const QUrl &site)
{
	d->twSites->addUrl(site);
}

void WadseekerInterface::startSeeking(const QList<PWad> &seekedFilesList)
{
	if (seekedFilesList.isEmpty())
		return;
	d->bCompletedSuccessfully = false;

	ModSet listWads;
	for (PWad seekedFile : seekedFilesList)
	{
		listWads.addModFile(seekedFile);
	}

	setupIdgames();

	wadseeker.setTargetDirectory(gDoomseekerTemplatedPathResolver().resolve(gConfig.wadseeker.targetDirectory));
	wadseeker.setCustomSites(customSites);
	wadseeker.setMaximumConcurrentSeeks(gConfig.wadseeker.maxConcurrentSiteDownloads);
	wadseeker.setMaximumConcurrentDownloads(gConfig.wadseeker.maxConcurrentWadDownloads);
	wadseeker.startSeek(listWads);
}

void WadseekerInterface::updateProgressBar()
{
	double totalPercentage = d->twWads->totalDonePercentage();
	auto progressBarValue = (unsigned)(totalPercentage * 100.0);

	d->pbOverallProgress->setValue(progressBarValue);
	d->taskbarProgress->setValue(progressBarValue);
}

void WadseekerInterface::updateTitle()
{
	switch (state)
	{
	case Downloading:
	{
		double totalPercentage = d->twWads->totalDonePercentage();
		if (totalPercentage < 0.0)
			totalPercentage = 0.0;

		setWindowTitle(tr("[%1%] Wadseeker").arg(totalPercentage, 6, 'f', 2));
		break;
	}

	default:
	case Waiting:
		resetTitleToDefault();
		break;
	}
}

QList<PWad> WadseekerInterface::unsuccessfulWads() const
{
	QList<PWad> allWads = seekedWads;
	for (PWad successfulWad : successfulWads)
	{
		for (int i = 0; i < allWads.size(); ++i)
		{
			if (allWads[i].name() == successfulWad.name())
			{
				allWads.removeAt(i);
				break;
			}
		}
	}
	return allWads;
}

void WadseekerInterface::showWadsTableContextMenu(const QPoint &position)
{
	const QModelIndex index = d->twWads->indexAt(position);
	WadseekerWadsTable::ContextMenu *menu = d->twWads->contextMenu(index, position);

	// Disable actions depending on Wadseeker's state.
	QString fileName = d->twWads->fileNameAtRow(index.row());
	if (!wadseeker.isDownloadingFile(fileName))
		menu->actionSkipCurrentSite->setEnabled(false);

	QAction *pResult = menu->exec();

	if (pResult == menu->actionSkipCurrentSite)
	{
		QString wadName = d->twWads->fileNameAtRow(index.row());
		d->twWads->setFileUrl(fileName, QUrl());

		wadseeker.skipFileCurrentUrl(wadName);
	}
	else if (pResult != nullptr)
		QMessageBox::warning(this, tr("Context menu error"), tr("Unknown action selected."));

	delete menu;
}
