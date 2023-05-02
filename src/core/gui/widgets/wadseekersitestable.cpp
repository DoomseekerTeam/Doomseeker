//------------------------------------------------------------------------------
// wadseekersitestable.cpp
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
#include "wadseekersitestable.h"

#include <QHeaderView>
#include <QProgressBar>
#include <QPushButton>
#include <QUrl>

WadseekerSitesTable::WadseekerSitesTable(QWidget *pParent)
	: TableWidgetMouseAware(pParent)
{
	d.bAlreadyShownOnce = false;
}

void WadseekerSitesTable::addUrl(const QUrl &url)
{
	// Add new row to table, but only if URL is not yet added.
	if (findRow(url) < 0)
	{
		setSortingEnabled(false);

		insertRow(rowCount());
		int rowIndex = rowCount() - 1;

		QProgressBar *pBar = new QProgressBar();
		pBar->setAlignment(Qt::AlignCenter);
		pBar->setMinimum(0);
		pBar->setMaximum(0);

		QPushButton *abortButton = new QPushButton(tr("Abort"));

		connect (abortButton, &QPushButton::clicked, [this, url]()
		{
			this->requestUrlAbort(url.toString());
		});

		setItem(rowIndex, IDX_URL_COLUMN, new QTableWidgetItem(url.toString()));
		setCellWidget(rowIndex, IDX_PROGRESS_COLUMN, pBar);
		setCellWidget(rowIndex, IDX_ABORT_COLUMN, abortButton);

		setSortingEnabled(true);
	}
}

int WadseekerSitesTable::findRow(const QString &text)
{
	QList<QTableWidgetItem *> list = findItems(text, Qt::MatchFixedString);
	if (!list.isEmpty())
		return list.first()->row();

	return -1;
}

int WadseekerSitesTable::findRow(const QUrl &url)
{
	return findRow(url.toString());
}


void WadseekerSitesTable::removeUrl(const QUrl &url)
{
	int row = findRow(url);
	if (row >= 0)
		removeRow(row);
}

void WadseekerSitesTable::addService(const QString &service)
{
	if (findRow(service) < 0)
	{
		setSortingEnabled(false);

		insertRow(rowCount());
		int rowIndex = rowCount() - 1;

		QProgressBar *pBar = new QProgressBar();
		pBar->setAlignment(Qt::AlignCenter);
		pBar->setMinimum(0);
		pBar->setMaximum(0);

		QPushButton *abortButton = new QPushButton(tr("Abort"));

		connect (abortButton, &QPushButton::clicked, [this, service]()
		{
			this->serviceAbortRequested(service);
		});

		setItem(rowIndex, IDX_URL_COLUMN, new QTableWidgetItem(service));
		setCellWidget(rowIndex, IDX_PROGRESS_COLUMN, pBar);
		setCellWidget(rowIndex, IDX_ABORT_COLUMN, abortButton);

		setSortingEnabled(true);
	}
}

void WadseekerSitesTable::removeService(const QString &service)
{
	int row = findRow(service);
	if (row >= 0)
		removeRow(row);
}

void WadseekerSitesTable::requestUrlAbort(const QString &urlAsString)
{
	emit urlAbortRequested(urlAsString);
}

void WadseekerSitesTable::setUrlProgress(const QUrl &url, qint64 current, qint64 total)
{
	int row = findRow(url);

	if (row >= 0)
	{
		QProgressBar *pBar = (QProgressBar *) this->cellWidget(row, IDX_PROGRESS_COLUMN);
		pBar->setMaximum(total);
		pBar->setValue(current);
	}
}

void WadseekerSitesTable::showEvent(QShowEvent *pEvent)
{
	if (!d.bAlreadyShownOnce)
	{
		// Events in this block must occur after the widget has been
		// constructed, but only once.
		QHeaderView *pHeader = horizontalHeader();

		// Setup resizing
		pHeader->setSectionResizeMode(IDX_URL_COLUMN, QHeaderView::Stretch);

		pHeader->resizeSection(IDX_PROGRESS_COLUMN, 85);
	}
}
