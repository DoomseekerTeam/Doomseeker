//------------------------------------------------------------------------------
// cfgwadseekersites.cpp
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
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "cfgwadseekersites.h"
#include "configuration/doomseekerconfig.h"
#include "ui_cfgwadseekersites.h"
#include "wadseeker/wadseeker.h"
#include <QCompleter>
#include <QDebug>
#include <QDirModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QUrl>

DClass<CFGWadseekerSites> : public Ui::CFGWadseekerSites
{
};

DPointered(CFGWadseekerSites)

CFGWadseekerSites::CFGWadseekerSites(QWidget *parent)
	: ConfigPage(parent)
{
	d->setupUi(this);

	d->lstUrls->setModel(new QStandardItemModel());

	connect(d->btnUrlAdd, SIGNAL(clicked()), this, SLOT(btnUrlAddClicked()));
	connect(d->btnUrlDefault, SIGNAL(clicked()), this, SLOT(btnUrlDefaultClicked()));
	connect(d->btnUrlRemove, SIGNAL(clicked()), this, SLOT(btnUrlRemoveClicked()));
	connect(d->leUrl, SIGNAL(returnPressed()), this, SLOT(btnUrlAddClicked()));
}

CFGWadseekerSites::~CFGWadseekerSites()
{
}

void CFGWadseekerSites::btnUrlAddClicked()
{
	insertUrl(d->leUrl->text());
}

void CFGWadseekerSites::btnUrlDefaultClicked()
{
	for (int i = 0; !Wadseeker::defaultSites[i].isEmpty(); ++i)
		insertUrl(Wadseeker::defaultSites[i]);
}

void CFGWadseekerSites::btnUrlRemoveClicked()
{
	QItemSelectionModel *selModel = d->lstUrls->selectionModel();
	QModelIndexList indexList = selModel->selectedIndexes();
	selModel->clear();

	auto model = static_cast<QStandardItemModel *>(d->lstUrls->model());
	QList<QStandardItem *> itemList;
	for (int i = 0; i < indexList.count(); ++i)
		itemList << model->itemFromIndex(indexList[i]);

	for (int i = 0; i < itemList.count(); ++i)
	{
		QModelIndex index = model->indexFromItem(itemList[i]);
		model->removeRow(index.row());
	}
}

void CFGWadseekerSites::insertUrl(const QString &url)
{
	if (url.isEmpty())
		return;

	// first we check whether the URL is already in the box.
	auto model = static_cast<QStandardItemModel *>(d->lstUrls->model());
	for (int i = 0; i < model->rowCount(); ++i)
	{
		QUrl existingUrl(model->item(i)->text());
		if (existingUrl == url)
			return;
	}

	auto it = new QStandardItem(url);

	it->setDragEnabled(true);
	it->setDropEnabled(false);
	it->setToolTip(url);

	model->appendRow(it);
}

void CFGWadseekerSites::readSettings()
{
	const QStringList &urlList = gConfig.wadseeker.searchURLs;
	for (const QString &url : urlList)
	{
		this->insertUrl(url);
	}
	d->cbAlwaysUseDefaultSites->setChecked(gConfig.wadseeker.bAlwaysUseDefaultSites);
}

void CFGWadseekerSites::saveSettings()
{
	QStringList urlList;
	auto model = static_cast<QStandardItemModel *>(d->lstUrls->model());
	for (int i = 0; i < model->rowCount(); ++i)
		urlList << model->item(i)->text();

	gConfig.wadseeker.searchURLs = urlList;
	gConfig.wadseeker.bAlwaysUseDefaultSites = d->cbAlwaysUseDefaultSites->isChecked();
}
