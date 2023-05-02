//------------------------------------------------------------------------------
// maplistselector.cpp
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
#include "maplistselector.h"

#include "commongui.h"
#include "modreader.h"

#include <QFileInfo>
#include <QMenu>
#include <QScopedPointer>
#include <QSharedPointer>
#include <QTimer>

MapEntry::MapEntry(const QString &name, const QString &originalFile, const bool &isIwad)
{
	this->name = name;
	this->originalFile = originalFile;
	this->isIwad = isIwad;
}

DClass<MapListSelector> : public Ui::MapListSelector
{
public:
	QStringList paths;
	QList<MapEntry> mapList;
	QScopedPointer<QTimer> checkWadTimer;
};

DPointeredNoCopy(MapListSelector)

MapListSelector::MapListSelector(QWidget *parent) : QDialog(parent)
{
	d->checkWadTimer.reset(new QTimer);

	d->setupUi(this);

	connect(d->checkWadTimer.data(), SIGNAL(timeout()), this, SLOT(performCheckStep()));
}

MapListSelector::~MapListSelector()
{
}

void MapListSelector::addPaths(const QStringList &paths)
{
	d->paths << paths;
	d->progressBar->setMaximum(d->progressBar->value() + paths.size());
	setIfFinishedStateIsEnabled(false);
	d->checkWadTimer->start(0);
}

const QStringList MapListSelector::selectedMaps()
{
	QStringList tempSelectedMapList;
	for (int i = 0; i < d->listMapsWidget->count(); ++i)
	{
		QListWidgetItem *item = d->listMapsWidget->item(i);
		if (item->checkState() == Qt::Checked)
			tempSelectedMapList << item->text();
	}
	return tempSelectedMapList;
}

void MapListSelector::performCheckStep()
{
	if (d->paths.isEmpty())
	{
		d->checkWadTimer->stop();
		setMapsOfCheckableList(d->mapList);
		setIfFinishedStateIsEnabled(true);
		return;
	}
	QString path = d->paths.takeFirst();
	QSharedPointer<ModReader> modReader = ModReader::create(path);
	if (!modReader.isNull() && modReader->load())
	{
		for (const QString &tempMap : modReader->getAllMaps())
		{
			addEntryToMapList(MapEntry(tempMap, QFile(path).fileName(), modReader->isIwad()));
		}
	}
	d->progressBar->setValue(d->progressBar->value() + 1);
}

void MapListSelector::addEntryToMapList(const MapEntry &newEntry)
{
	bool equalEntryFound = false;
	for (int i = 0; i < d->mapList.size(); ++i)
	{
		if (d->mapList[i].name == newEntry.name)
		{
			d->mapList.replace(i, newEntry);
			equalEntryFound = true;
		}
	}
	if (!equalEntryFound)
		d->mapList.append(newEntry);
}

void MapListSelector::setMapsOfCheckableList(const QList<MapEntry> &list)
{
	d->listMapsWidget->clear();
	for (const MapEntry &mapEntry : list)
	{
		auto item = new QListWidgetItem(mapEntry.name, d->listMapsWidget);
		item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
		item->setCheckState(Qt::Checked);
		if (mapEntry.isIwad)
		{
			item->setBackground(Qt::lightGray);
			item->setForeground(Qt::black);
		}
		item->setToolTip("File: " + mapEntry.originalFile);
	}
}

void MapListSelector::setIfFinishedStateIsEnabled(const bool &state)
{
	d->btnAdd->setEnabled(state);
	d->checkAll->setEnabled(state);
	d->descriptionSelect->setVisible(state);
	d->listMapsWidget->setVisible(state);
	d->descriptionLoading->setVisible(!state);
	d->progressBar->setVisible(!state);
	adjustSize();
}

void MapListSelector::accept()
{
	done(Accepted);
}

void MapListSelector::reject()
{
	d->listMapsWidget->clear();
	d->checkWadTimer->stop();
	done(Rejected);
}

void MapListSelector::selectAll()
{
	Qt::CheckState checkState = d->checkAll->checkState();
	for (int i = 0; i < d->listMapsWidget->count(); ++i)
		d->listMapsWidget->item(i)->setCheckState(checkState);
}

void MapListSelector::listItemChanged()
{
	QStringList checkedMaps = selectedMaps();
	if (checkedMaps.size() > 0 && checkedMaps.size() < d->listMapsWidget->count())
		d->checkAll->setCheckState(Qt::PartiallyChecked);
	else
	{
		d->checkAll->setCheckState((checkedMaps.size() == 0) ?
			Qt::Unchecked : Qt::Checked);
	}
}

void MapListSelector::showContextMenu(const QPoint &pos)
{
	QPoint globalPos = d->listMapsWidget->mapToGlobal(pos);

	QMenu myMenu;
	myMenu.addAction(tr("Invert selection"), this, SLOT(invertSelection()));
	myMenu.exec(globalPos);
}

void MapListSelector::invertSelection()
{
	for (int i = 0; i < d->listMapsWidget->count(); ++i)
	{
		QListWidgetItem *item = d->listMapsWidget->item(i);
		if (item->isSelected())
			item->setCheckState(item->checkState() == Qt::Unchecked ?
				Qt::Checked : Qt::Unchecked);
	}
}