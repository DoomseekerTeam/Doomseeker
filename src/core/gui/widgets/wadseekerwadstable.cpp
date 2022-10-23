//------------------------------------------------------------------------------
// wadseekerwadstable.cpp
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
// Copyright (C) 2011 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "wadseekerwadstable.h"

#include "speedcalculator.h"
#include "strings.hpp"
#include "wadseeker/entities/modfile.h"

#include <QDebug>
#include <QHeaderView>
#include <QProgressBar>
#include <QUrl>

WadseekerWadsTable::WadseekerWadsTable(QWidget *pParent)
	: TableWidgetMouseAware(pParent)
{
	d.bAlreadyShownOnce = false;
	d.updateClock.start();
}

WadseekerWadsTable::~WadseekerWadsTable()
{
	QMap<QString, SpeedCalculator *>::iterator it;
	for (it = d.speedCalculators.begin(); it != d.speedCalculators.end(); ++it)
		delete it.value();
}

void WadseekerWadsTable::addFile(const QString &filename)
{
	// Add new row to table, but only if file is not yet added.
	if (findFileRow(filename) < 0)
	{
		insertRow(rowCount());
		int rowIndex = rowCount() - 1;

		// Create the row contents.
		setSortingEnabled(false);

		auto pBar = new QProgressBar();
		pBar->setAlignment(Qt::AlignCenter);

		setItem(rowIndex, IDX_NAME_COLUMN, new QTableWidgetItem(filename));
		setItem(rowIndex, IDX_URL_COLUMN, new QTableWidgetItem());
		setCellWidget(rowIndex, IDX_PROGRESS_COLUMN, pBar);
		setItem(rowIndex, IDX_SPEED_COLUMN, new QTableWidgetItem());
		setItem(rowIndex, IDX_ETA_COLUMN, new QTableWidgetItem(tr("N/A")));
		setItem(rowIndex, IDX_SIZE_COLUMN, new QTableWidgetItem(tr("N/A")));

		auto pCalculator = new SpeedCalculator();
		pCalculator->start();

		d.speedCalculators.insert(filename, pCalculator);

		setSortingEnabled(true);
	}
}

WadseekerWadsTable::ContextMenu *WadseekerWadsTable::contextMenu(const QModelIndex &index, const QPoint &cursorPosition)
{
	auto menu = new ContextMenu(this);
	QPoint displayPoint = this->viewport()->mapToGlobal(cursorPosition);
	menu->move(displayPoint);

	if (!index.isValid())
		menu->actionSkipCurrentSite->setEnabled(false);

	return menu;
}

qint64 WadseekerWadsTable::expectedDataSize(int row) const
{
	if (row < 0 || row >= this->rowCount())
		return -1;

	QString fileName = fileNameAtRow(row);
	return d.speedCalculators[fileName]->expectedDataSize();
}

QString WadseekerWadsTable::fileNameAtRow(int row) const
{
	if (row < 0 || row >= this->rowCount())
		return QString();

	return item(row, IDX_NAME_COLUMN)->text();
}

int WadseekerWadsTable::findFileRow(const QString &filename)
{
	QList<QTableWidgetItem *> list = findItems(filename, Qt::MatchFixedString);
	if (!list.isEmpty())
		return list.first()->row();

	return -1;
}

void WadseekerWadsTable::setFileDownloadFinished(const ModFile &filename)
{
	int row = findFileRow(filename.fileName());

	if (row >= 0)
	{
		// Update ETA
		// ETA will be changed to DONE if wad is installed successfully.
		item(row, IDX_ETA_COLUMN)->setText(tr("N/A"));
		item(row, IDX_SPEED_COLUMN)->setText(tr("N/A"));

		item(row, IDX_URL_COLUMN)->setText(tr("Awaiting URLs"));
		item(row, IDX_URL_COLUMN)->setToolTip(tr("Awaiting URLs"));

		const bool FORCE = true;
		updateDataInfoValues(FORCE);
	}
}

void WadseekerWadsTable::setFileFailed(const ModFile &filename)
{
	int row = findFileRow(filename.fileName());

	if (row >= 0)
	{
		item(row, IDX_NAME_COLUMN)->setIcon(QIcon(":/icons/x.png"));

		item(row, IDX_URL_COLUMN)->setText("");
	}
}

void WadseekerWadsTable::setFileProgress(const ModFile &filename, qint64 current, qint64 total)
{
	int row = findFileRow(filename.fileName());

	if (row >= 0)
	{
		auto pBar = (QProgressBar *) this->cellWidget(row, IDX_PROGRESS_COLUMN);
		pBar->setMaximum(total);
		pBar->setValue(current);

		// Update ETA and speed
		SpeedCalculator *pCalculator = d.speedCalculators.value(filename.fileName());
		pCalculator->setExpectedDataSize(total);
		pCalculator->registerDataAmount(current);

		const bool FORCE = true;
		updateDataInfoValues(!FORCE);
	}
}

void WadseekerWadsTable::setFileSuccessful(const ModFile &filename)
{
	int row = findFileRow(filename.fileName());

	if (row >= 0)
	{
		// Set progress bar to 100%.
		auto pBar = (QProgressBar *) this->cellWidget(row, IDX_PROGRESS_COLUMN);
		SpeedCalculator *pCalculator = d.speedCalculators[filename.fileName()];
		if (pCalculator->expectedDataSize() == 0)
			pCalculator->setExpectedDataSize(1);

		pCalculator->registerDataAmount(pCalculator->expectedDataSize());

		pBar->setMaximum(pCalculator->expectedDataSize());
		pBar->setValue(pCalculator->expectedDataSize());

		item(row, IDX_NAME_COLUMN)->setIcon(QIcon(":/icons/ok.png"));
		item(row, IDX_URL_COLUMN)->setText("");

		item(row, IDX_ETA_COLUMN)->setText(tr("Done"));
		item(row, IDX_SPEED_COLUMN)->setText("");

		const bool FORCE = true;
		updateDataInfoValues(FORCE);
	}
}

void WadseekerWadsTable::setFileUrl(const ModFile &filename, const QUrl &url)
{
	// At this point we know that a new download has started.
	// We should reset certain values.
	int row = findFileRow(filename.fileName());

	if (row >= 0)
	{
		QTableWidgetItem *pItem = this->item(row, IDX_URL_COLUMN);
		pItem->setText(url.toString());
		pItem->setToolTip(url.toString());

		auto pBar = (QProgressBar *) this->cellWidget(row, IDX_PROGRESS_COLUMN);
		pBar->setMaximum(0);
		pBar->setValue(0);

		SpeedCalculator *pCalculator = d.speedCalculators[filename.fileName()];
		pCalculator->start();
	}
}

void WadseekerWadsTable::showEvent(QShowEvent *pEvent)
{
	Q_UNUSED(pEvent);
	if (!d.bAlreadyShownOnce)
	{
		// Events in this block must occur after the widget has been
		// constructed, but only once.
		QHeaderView *pHeader = horizontalHeader();

		// Setup resizing
		pHeader->setSectionResizeMode(IDX_URL_COLUMN, QHeaderView::Stretch);

		pHeader->resizeSection(IDX_NAME_COLUMN, 140);
		pHeader->resizeSection(IDX_PROGRESS_COLUMN, 85);
		pHeader->resizeSection(IDX_ETA_COLUMN, 85);
		pHeader->resizeSection(IDX_SIZE_COLUMN, 150);
		pHeader->resizeSection(IDX_SPEED_COLUMN, 85);
	}
}

double WadseekerWadsTable::totalDonePercentage() const
{
	double sumDownloadPercentages = 0.0;

	if (this->rowCount() == 0)
		return -1.0;

	for (int i = 0; i < this->rowCount(); ++i)
	{
		const auto pBar = (const QProgressBar *) this->cellWidget(i, IDX_PROGRESS_COLUMN);
		if (pBar != nullptr)
		{
			int val = pBar->value();
			int max = pBar->maximum();

			if (max > 0)
			{
				double curPercentage =  (double) val / (double) max;
				sumDownloadPercentages += curPercentage;
			}
		}
	}

	// We need to multiply the value by 100 to get actual percents.
	sumDownloadPercentages *= 100.0;
	double averageDownloadPercentages = sumDownloadPercentages / (double) this->rowCount();

	return averageDownloadPercentages;
}

void WadseekerWadsTable::updateDataInfoValues(bool bForce)
{
	// Make sure updates are not performed before certain interval passes.
	if (d.updateClock.elapsed() > UPDATE_INTERVAL_MS || bForce)
	{
		d.updateClock.start();

		for (int i = 0; i < this->rowCount(); ++i)
		{
			// Find the calculator for specified row.
			QString filename = this->item(i, IDX_NAME_COLUMN)->text();
			SpeedCalculator *pCalculator = d.speedCalculators.value(filename);

			// Update data amount.
			QString strCurrent = Strings::formatDataAmount(pCalculator->lastRegisterAttemptedDataAmount());
			QString strTotal = Strings::formatDataAmount(pCalculator->expectedDataSize());

			QString size = QString("%1 / %2").arg(strCurrent, strTotal);
			item(i, IDX_SIZE_COLUMN)->setText(size);

			// Update ETA and speed.
			if (pCalculator->expectedDataSize() != pCalculator->lastRegisterAttemptedDataAmount())
			{
				// If both above values are equal it means we have either
				// finished the download or didn't start it yet. In either case
				// we shouldn't change the speed and ETA displays.
				long double ldEta = pCalculator->estimatedTimeUntilArrival();
				long double ldSpeed = pCalculator->getSpeed();

				if (ldEta >= 0.0)
				{
					QString strEta = Strings::formatTime(pCalculator->estimatedTimeUntilArrival());
					item(i, IDX_ETA_COLUMN)->setText(strEta);
				}
				else
					item(i, IDX_ETA_COLUMN)->setText(tr("N/A"));

				if (ldSpeed >= 0.0)
				{
					QString strSpeed = Strings::formatDataSpeed(pCalculator->getSpeed());
					item(i, IDX_SPEED_COLUMN)->setText(strSpeed);
				}
				else
					item(i, IDX_SPEED_COLUMN)->setText(tr("N/A"));
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////
WadseekerWadsTable::ContextMenu::ContextMenu(QWidget *pParent)
	: QMenu(pParent)
{
	this->actionSkipCurrentSite = new QAction(tr("Skip current URL"), this);

	this->addAction(this->actionSkipCurrentSite);
}
