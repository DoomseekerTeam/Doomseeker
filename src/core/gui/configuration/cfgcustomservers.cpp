//------------------------------------------------------------------------------
// cfgcustomservers.cpp
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
#include "cfgcustomservers.h"
#include "configuration/doomseekerconfig.h"
#include "plugins/engineplugin.h"
#include "plugins/pluginloader.h"
#include "ui_cfgcustomservers.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QStandardItemModel>
#include <QUrl>

const // clear warnings
#include "unknownengine.xpm"

DClass<CFGCustomServers> : public Ui::CFGCustomServers
{
public:
	QStandardItemModel *model;
};

DPointered(CFGCustomServers)

CFGCustomServers::CFGCustomServers(QWidget *parent)
	: ConfigPage(parent)
{
	d->setupUi(this);

	connect(d->btnAdd, SIGNAL(clicked()), this, SLOT(add()));
	connect(d->btnDisableSelected, SIGNAL(clicked()), this, SLOT(disableSelected()));
	connect(d->btnEnableSelected, SIGNAL(clicked()), this, SLOT(enableSelected()));
	connect(d->btnRemove, SIGNAL(clicked()), this, SLOT(remove()));
	connect(d->btnSetEngine, SIGNAL(clicked()), this, SLOT(setEngine()));

	prepareEnginesComboBox();
}

CFGCustomServers::~CFGCustomServers()
{
}

void CFGCustomServers::add()
{
	int pluginIndex = d->cboEngines->itemData(d->cboEngines->currentIndex()).toInt();
	const EnginePlugin *plugin = gPlugins->info(pluginIndex);

	QString engineName = d->cboEngines->itemText(d->cboEngines->currentIndex());

	add(engineName, "", plugin->data()->defaultServerPort, true);
}

void CFGCustomServers::add(const QString &engineName, const QString &host,
	unsigned short port, bool enabled)
{
	QList<QStandardItem *> record;

	auto engineItem = new QStandardItem();
	setEngineOnItem(engineItem, engineName);

	QString portString = QString::number(port);

	record.append(engineItem);
	record.append(new QStandardItem(host));
	record.append(new QStandardItem(portString));

	auto enabledItem = new QStandardItem();
	enabledItem->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
	enabledItem->setCheckable(true);
	enabledItem->setToolTip(tr("Toggle enabled state"));
	record.append(enabledItem);

	d->model->appendRow(record);
	d->tvServers->resizeRowsToContents();
}

CFGCustomServers::CheckAndFixPorts CFGCustomServers::checkAndFixPorts(int firstRow, int lastRow)
{
	CheckAndFixPorts returnValue = AllOk;
	for (int rowIndex = firstRow; rowIndex <= lastRow; ++rowIndex)
	{
		if (!isPortCorrect(rowIndex))
		{
			returnValue = AtLeastOneFixed;

			setPortToDefault(rowIndex);
		}
	}

	return returnValue;
}

void CFGCustomServers::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
	const QString MESSAGE_TITLE = tr("Doomseeker - pinned servers");

	int leftmostColumn = topLeft.column();
	int rightmostColumn = bottomRight.column();

	if (isPortColumnWithinRange(leftmostColumn, rightmostColumn))
	{
		switch (checkAndFixPorts(topLeft.row(), bottomRight.row()))
		{
		case AllOk:
			// Ignore
			break;

		case AtLeastOneFixed:
			QMessageBox::warning(this, MESSAGE_TITLE, tr("Port must be within range 1 - 65535"));
			break;

		default:
			QMessageBox::warning(this, MESSAGE_TITLE, tr("Unimplemented behavior!"));
			break;
		}
	}
}

void CFGCustomServers::disableSelected()
{
	setEnabledOnSelected(false);
}

void CFGCustomServers::enableSelected()
{
	setEnabledOnSelected(true);
}

void CFGCustomServers::setEnabledOnSelected(bool enabled)
{
	QItemSelectionModel *sel = d->tvServers->selectionModel();
	QModelIndexList indexList = sel->selectedRows();

	QModelIndexList::iterator it;
	for (it = indexList.begin(); it != indexList.end(); ++it)
	{
		QStandardItem *item = d->model->itemFromIndex(it->sibling(it->row(), EnabledIndex));
		item->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
	}
}

const EnginePlugin *CFGCustomServers::getPluginInfoForRow(int rowIndex)
{
	QStandardItem *itemEngine = d->model->item(rowIndex, EngineColumnIndex);
	QString engineName = itemEngine->data().toString();
	int pluginIndex = gPlugins->pluginIndexFromName(engineName);
	return gPlugins->info(pluginIndex);
}

bool CFGCustomServers::isPortColumnWithinRange(int leftmostColumnIndex, int rightmostColumnIndex)
{
	return leftmostColumnIndex <= PortColumnIndex && rightmostColumnIndex >= PortColumnIndex;
}

bool CFGCustomServers::isPortCorrect(int rowIndex)
{
	const int MIN_PORT = 1;
	const int MAX_PORT = 65535;

	QStandardItem *item = d->model->item(rowIndex, PortColumnIndex);
	bool ok;
	int port = item->text().toInt(&ok);

	return ok && port >= MIN_PORT && port <= MAX_PORT;
}

void CFGCustomServers::prepareEnginesComboBox()
{
	d->cboEngines->clear();

	for (unsigned i = 0; i < gPlugins->numPlugins(); ++i)
	{
		const EnginePlugin *plugin = gPlugins->info(i);
		d->cboEngines->addItem(plugin->icon(), plugin->data()->name, i);
	}

	if (d->cboEngines->count() > 0)
		d->cboEngines->setCurrentIndex(0);
}

void CFGCustomServers::prepareTable()
{
	d->model = new QStandardItemModel(this);

	connect(d->model, SIGNAL(dataChanged(const QModelIndex&,const QModelIndex&)), this, SLOT(dataChanged(const QModelIndex&,const QModelIndex&)));

	QStringList labels;
	labels << "" << tr("Host") << tr("Port") << "";
	d->model->setHorizontalHeaderLabels(labels);

	d->tvServers->setModel(d->model);

	d->tvServers->setColumnWidth(EngineColumnIndex, 23);
	d->tvServers->setColumnWidth(AddressColumnIndex, 180);
	d->tvServers->setColumnWidth(PortColumnIndex, 60);
	d->tvServers->setColumnWidth(EnabledIndex, 30);

	QList<ColumnIndices> fixedSizeColumns;
	fixedSizeColumns << EngineColumnIndex << EnabledIndex;
	for (ColumnIndices columnIdx : fixedSizeColumns)
		d->tvServers->horizontalHeader()->setSectionResizeMode(columnIdx, QHeaderView::Fixed);
	d->tvServers->horizontalHeader()->setHighlightSections(false);

	d->tvServers->verticalHeader()->hide();
}

void CFGCustomServers::readSettings()
{
	prepareTable();

	QList<CustomServerInfo> customServersList = gConfig.doomseeker.customServers.toList();
	QList<CustomServerInfo>::iterator it;
	for (it = customServersList.begin(); it != customServersList.end(); ++it)
		add(it->engine, it->host, it->port, it->enabled);
}

void CFGCustomServers::remove()
{
	QItemSelectionModel *selModel = d->tvServers->selectionModel();
	QModelIndexList indexList = selModel->selectedRows();
	selModel->clear();

	QList<QStandardItem *> itemList;
	for (int i = 0; i < indexList.count(); ++i)
		itemList << d->model->item(indexList[i].row(), 0);

	for (int i = 0; i < itemList.count(); ++i)
	{
		QModelIndex index = d->model->indexFromItem(itemList[i]);
		d->model->removeRow(index.row());
	}
}

void CFGCustomServers::saveSettings()
{
	gConfig.doomseeker.customServers = this->tableGetServers();
}

void CFGCustomServers::setEngine()
{
	QItemSelectionModel *sel = d->tvServers->selectionModel();
	QModelIndexList indexList = sel->selectedRows();

	QModelIndexList::iterator it;
	for (it = indexList.begin(); it != indexList.end(); ++it)
	{
		QStandardItem *item = d->model->itemFromIndex(*it);
		QString engineName = d->cboEngines->itemText(d->cboEngines->currentIndex());
		setEngineOnItem(item, engineName);
	}
}

void CFGCustomServers::setEngineOnItem(QStandardItem *item, const QString &engineName)
{
	int engineId = gPlugins->pluginIndexFromName(engineName);

	item->setData(engineName);
	item->setToolTip(engineName);
	if (engineId >= 0)
	{
		const EnginePlugin *plugin = gPlugins->info(engineId);
		item->setIcon(plugin->icon());
	}
	else
		item->setIcon(QPixmap(unknownengine_xpm));

	item->setEditable(false);
	item->setText("");
}

void CFGCustomServers::setPortToDefault(int rowIndex)
{
	const EnginePlugin *pluginInfo = getPluginInfoForRow(rowIndex);
	QString defaultPort = QString::number(pluginInfo->data()->defaultServerPort);

	QStandardItem *itemPort = d->model->item(rowIndex, PortColumnIndex);
	itemPort->setText(defaultPort);
}

QVector<CustomServerInfo> CFGCustomServers::tableGetServers()
{
	QVector<CustomServerInfo> servers;
	for (int i = 0; i < d->model->rowCount(); ++i)
	{
		CustomServerInfo customServer;

		QStandardItem *item = d->model->item(i, EngineColumnIndex);
		customServer.engine = item->data().toString();

		customServer.engineIndex = gPlugins->pluginIndexFromName(customServer.engine);

		item = d->model->item(i, AddressColumnIndex);
		customServer.host = item->text();

		item = d->model->item(i, PortColumnIndex);
		customServer.port = item->text().toUShort();

		item = d->model->item(i, EnabledIndex);
		customServer.enabled = (item->checkState() == Qt::Checked);

		servers << customServer;
	}

	return servers;
}
