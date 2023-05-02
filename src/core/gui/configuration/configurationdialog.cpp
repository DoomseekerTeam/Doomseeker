//------------------------------------------------------------------------------
// configurationdialog.cpp
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
#include "configurationdialog.h"
#include "ui_configurationdialog.h"

#include "configpage.h"
#include "qtmetapointer.h"
#include <cassert>
#include <QAbstractButton>
#include <QDebug>
#include <QKeyEvent>
#include <QStandardItem>
#include <QStandardItemModel>
#include <Qt>
#include <QTreeView>

DClass<ConfigurationDialog> : public Ui::ConfigurationDialog
{
public:
	enum Column
	{
		COL_FIRST = 0,
		COL_META = 0,
		COL_NAME = 0,
		COL_VALIDATION = 1,
		COLSIZE,
	};

	QList<ConfigPage *> configPages;
	ConfigPage *currentlyDisplayedPage;

	QModelIndex findPageModelIndex(const QModelIndex &rootIndex, ConfigPage *page)
	{
		QStandardItemModel *model = static_cast<QStandardItemModel *>(tvOptionsList->model());
		for (int row = 0; row < model->rowCount(rootIndex); ++row)
		{
			QModelIndex index = model->index(row, COL_META, rootIndex);
			ConfigPage *pageAtIndex = pageFromIndex(index);
			if (pageAtIndex == page)
				return index;
			QModelIndex childIndex = findPageModelIndex(index, page);
			if (childIndex.isValid())
				return childIndex;
		}
		return QModelIndex();
	}

	ConfigPage *pageFromIndex(const QModelIndex &index)
	{
		QModelIndex pageIndex = index.sibling(index.row(), COL_META);
		QtMetaPointer metaPointer = pageIndex.data(Qt::UserRole).value<QtMetaPointer>();
		void *pointer = metaPointer;
		return static_cast<ConfigPage *>(pointer);
	}

	QIcon validationIcon(ConfigPage::Validation validation) const
	{
		switch (validation)
		{
		case ConfigPage::VALIDATION_ERROR:
			return QIcon(":icons/exclamation_16.png");
		default:
			return QIcon();
		}
	}
};

DPointered(ConfigurationDialog)

ConfigurationDialog::ConfigurationDialog(QWidget *parent)
	: QDialog(parent)
{
	d->setupUi(this);

	QStandardItemModel *model = new QStandardItemModel(this);
	for (int column = PrivData<ConfigurationDialog>::COL_FIRST;
		column < PrivData<ConfigurationDialog>::COLSIZE; ++column)
	{
		model->setHorizontalHeaderItem(column, new QStandardItem());
	}
	d->tvOptionsList->setModel(model);

	d->tvOptionsList->header()->setSectionResizeMode(
		PrivData<ConfigurationDialog>::COL_NAME, QHeaderView::Stretch);
	d->tvOptionsList->header()->setSectionResizeMode(
		PrivData<ConfigurationDialog>::COL_VALIDATION, QHeaderView::Fixed);

	d->currentlyDisplayedPage = nullptr;
	connect(d->buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT (btnClicked(QAbstractButton*)));
	this->connect(d->tvOptionsList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
		SLOT(switchToItem(QModelIndex,QModelIndex)));
}

ConfigurationDialog::~ConfigurationDialog()
{
	for (int i = 0; i < d->configPages.count(); ++i)
		delete d->configPages[i];
}

QStandardItem *ConfigurationDialog::addConfigPage(
	QStandardItem *rootItem, ConfigPage *configPage, int position)
{
	if (!canConfigPageBeAdded(configPage))
		return nullptr;

	QStandardItemModel *pModel = (QStandardItemModel *)d->tvOptionsList->model();
	if (rootItem == nullptr)
		rootItem = pModel->invisibleRootItem();

	if (!this->hasItemOnList(rootItem))
		return nullptr;

	QList<QStandardItem *> row;

	QStandardItem *nameItem = new QStandardItem(configPage->name());
	nameItem->setIcon(configPage->icon());

	row.insert(PrivData<ConfigurationDialog>::COL_NAME, nameItem);

	QtMetaPointer metaPointer = configPage;
	nameItem->setData(qVariantFromValue(metaPointer), Qt::UserRole);

	row.insert(PrivData<ConfigurationDialog>::COL_VALIDATION, new QStandardItem());

	if (position < 0)
		rootItem->appendRow(row);
	else
		rootItem->insertRow(position, row);

	d->configPages << configPage;
	this->connect(configPage, SIGNAL(validationRequested()),
		SLOT(onPageValidationRequested()));

	if (!configPage->areSettingsAlreadyRead())
		configPage->read();
	validatePage(configPage);

	return nameItem;
}

QStandardItem *ConfigurationDialog::addLabel(QStandardItem *rootItem, const QString &label, int position)
{
	QStandardItemModel *pModel = (QStandardItemModel *)d->tvOptionsList->model();
	if (rootItem == nullptr)
		rootItem = pModel->invisibleRootItem();

	if (!this->hasItemOnList(rootItem))
		return nullptr;

	QtMetaPointer metaPointer = (void *)nullptr;
	QVariant variantMetaPointer = qVariantFromValue(metaPointer);

	QList<QStandardItem *> row;

	QStandardItem *nameItem = new QStandardItem(label);
	nameItem->setData(variantMetaPointer, Qt::UserRole);
	row.insert(PrivData<ConfigurationDialog>::COL_NAME, nameItem);
	row.insert(PrivData<ConfigurationDialog>::COL_VALIDATION, new QStandardItem());

	if (position < 0)
		rootItem->appendRow(row);
	else
		rootItem->insertRow(position, row);

	return nameItem;
}

void ConfigurationDialog::btnClicked(QAbstractButton *button)
{
	// Figure out what button we pressed and perform its action.
	switch (d->buttonBox->standardButton(button))
	{
	default:
		break;

	case QDialogButtonBox::Ok: // Also does the same as Apply
		if (this->validate())
		{
			this->accept();
			this->saveSettings();
		}
		break;

	case QDialogButtonBox::Apply:
		if (this->validate())
			this->saveSettings();
		break;

	case QDialogButtonBox::Cancel:
		this->reject();
		break;
	}
}

bool ConfigurationDialog::canConfigPageBeAdded(ConfigPage *configPage)
{
	return isConfigPageValid(configPage) && !hasConfigPage(configPage);
}

QModelIndex ConfigurationDialog::findPageModelIndex(ConfigPage *page)
{
	QStandardItemModel *model = static_cast<QStandardItemModel *>(d->tvOptionsList->model());
	return d->findPageModelIndex(model->indexFromItem(model->invisibleRootItem()), page);
}

bool ConfigurationDialog::isConfigPageValid(ConfigPage *configPage)
{
	return configPage != nullptr && !configPage->name().isEmpty();
}

bool ConfigurationDialog::hasConfigPage(ConfigPage *configPage)
{
	for (ConfigPage *addedPage : d->configPages)
	{
		if (configPage == addedPage)
			return true;
	}

	return false;
}

void ConfigurationDialog::keyPressEvent(QKeyEvent *e)
{
	switch (e->key())
	{
	case Qt::Key_Enter:
	case Qt::Key_Return:
		// Suppress the dialog being accepted on pressing ENTER key.
		// Dialog would close even in line edits that had "returnPressed()"
		// signals connected. That wasn't good.
		e->ignore();
		break;
	default:
		QDialog::keyPressEvent(e);
	}
}

bool ConfigurationDialog::hasItemOnList(QStandardItem *pItem) const
{
	if (pItem == nullptr)
		return false;

	QStandardItemModel *pModel = (QStandardItemModel *)d->tvOptionsList->model();

	// Calling index methods on the invisible root items will always return
	// invalid values.

	return pModel->invisibleRootItem() == pItem
		|| pModel->indexFromItem(pItem).isValid();
}

void ConfigurationDialog::onPageValidationRequested()
{
	ConfigPage *page = qobject_cast<ConfigPage *>(sender());
	assert(page != nullptr);
	validatePage(page);
}

void ConfigurationDialog::reject()
{
	for (ConfigPage *page : d->configPages)
	{
		page->reject();
	}
	QDialog::reject();
}

void ConfigurationDialog::switchToItem(const QModelIndex &current, const QModelIndex &previous)
{
	if (current.isValid() && current != previous)
	{
		ConfigPage *configPage = d->pageFromIndex(current);

		// Something with sense was selected, display this something
		// and hide previous box.
		if (isConfigPageValid(configPage))
			showConfigPage(configPage);
	}
}

QTreeView *ConfigurationDialog::optionsTree()
{
	return d->tvOptionsList;
}

void ConfigurationDialog::saveSettings()
{
	// Iterate through every engine and execute it's saving method
	for (int i = 0; i < d->configPages.count(); ++i)
		d->configPages[i]->save();

	doSaveSettings();

	if (isVisible())
	{
		// Allow panels such as the one for Wadseeker update their contents.
		for (int i = 0; i < d->configPages.count(); ++i)
		{
			d->configPages[i]->read();
			validatePage(d->configPages[i]);
		}
	}
}

void ConfigurationDialog::showConfigPage(ConfigPage *page)
{
	if (d->currentlyDisplayedPage != nullptr)
	{
		validatePage(d->currentlyDisplayedPage);
		d->currentlyDisplayedPage->hide();
		d->mainPanel->setTitle(QString());
	}
	d->currentlyDisplayedPage = page;

	if (page != nullptr)
	{
		page->setAllowSave(true);
		validatePage(page);
		d->mainPanel->layout()->addWidget(page);
		d->mainPanel->setTitle(page->title());
		page->show();
	}
}

void ConfigurationDialog::validatePage(ConfigPage *page)
{
	assert(page != nullptr);
	QModelIndex pageIndex = findPageModelIndex(page);
	assert(pageIndex.isValid());

	QModelIndex validationIndex = pageIndex.sibling(pageIndex.row(), PrivData<ConfigurationDialog>::COL_VALIDATION);
	assert(validationIndex.isValid());

	QStandardItemModel *model = static_cast<QStandardItemModel *>(d->tvOptionsList->model());
	QStandardItem *validationItem = model->itemFromIndex(validationIndex);
	validationItem->setIcon(d->validationIcon(page->validate()));
}
