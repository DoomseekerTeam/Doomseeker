//------------------------------------------------------------------------------
// maplistpanel.cpp
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "maplistpanel.h"
#include "ui_maplistpanel.h"

#include "commongui.h"
#include "gui/createserverdialog.h"
#include "ini/ini.h"
#include "maplistselector.h"
#include "plugins/engineplugin.h"
#include "serverapi/gamecreateparams.h"

#include <QStandardItemModel>
#include <QTimer>

DClass<MapListPanel> : public Ui::MapListPanel
{
public:
	CreateServerDialog *parentDialog;
};

DPointered(MapListPanel)

MapListPanel::MapListPanel(QWidget *parent)
	: QWidget(parent)
{
	d->setupUi(this);
	d->lstMaplist->setModel(new QStandardItemModel(this));
	this->connect(QApplication::instance(), SIGNAL(focusChanged(QWidget*,QWidget*)),
		SLOT(onFocusChanged(QWidget*,QWidget*)));

	d->parentDialog = nullptr;

	d->lblWarning->setText(MapListPanel::tr("Current map isn't present on map list. Game may misbehave."));
	d->lblWarning->setPixmap(QPixmap(":/icons/exclamation.png"));
	d->lblWarning->setWordWrap(true);
}

MapListPanel::~MapListPanel()
{
}

void MapListPanel::addMapFromEditBoxToList()
{
	addMapToMaplist(d->leMapname->text().trimmed());
}

void MapListPanel::addMapsFromLoadedWads()
{
	auto mapListSelector = new MapListSelector(this);
	mapListSelector->addPaths(d->parentDialog->wadPaths());
	if (mapListSelector->exec() == QDialog::Accepted)
	{
		for (QString map : mapListSelector->selectedMaps())
		{
			addMapToMaplist(map);
		}
	}
}

void MapListPanel::addMapToMaplist(const QString &map)
{
	if (map.isEmpty())
		return;
	auto model = static_cast<QStandardItemModel *>(d->lstMaplist->model());
	auto it = new QStandardItem(map);
	it->setDragEnabled(true);
	it->setDropEnabled(false);
	model->appendRow(it);
	updateMapWarningVisibility();
}

void MapListPanel::onFocusChanged(QWidget *old, QWidget *now)
{
	if (now == d->leMapname)
		d->btnAddMapToMaplist->setDefault(true);
	else if (old == d->leMapname)
		d->btnAddMapToMaplist->setDefault(false);
}

void MapListPanel::removeSelectedFromList()
{
	const bool bSelectNextLowest = true;
	CommonGUI::removeSelectedRowsFromStandardItemView(d->lstMaplist, bSelectNextLowest);
	updateMapWarningVisibility();
}

void MapListPanel::fillInParams(GameCreateParams &params)
{
	params.setMapList(CommonGUI::listViewStandardItemsToStringList(d->lstMaplist));
	params.setRandomMapRotation(d->cbRandomMapRotation->isChecked());
}

bool MapListPanel::hasMaps() const
{
	return d->lstMaplist->model()->rowCount() > 0;
}

bool MapListPanel::isMapOnList(const QString &mapName) const
{
	for (const QString &candidate : CommonGUI::listViewStandardItemsToStringList(d->lstMaplist))
	{
		if (candidate.compare(mapName, Qt::CaseInsensitive) == 0)
			return true;
	}
	return false;
}

void MapListPanel::showEvent(QShowEvent *event)
{
	Q_UNUSED(event);
	updateMapWarningVisibility();
}

void MapListPanel::updateMapWarningVisibility()
{
	d->lblWarning->setVisible(hasMaps() && !isMapOnList(d->parentDialog->mapName()));
}

void MapListPanel::loadConfig(Ini &config)
{
	IniSection section = config.section("Rules");
	QStringList stringList = section["maplist"].valueString().split(";");
	QAbstractItemModel *model = d->lstMaplist->model();
	model->removeRows(0, model->rowCount());
	for (QString s : stringList)
	{
		addMapToMaplist(s);
	}
	d->cbRandomMapRotation->setChecked(section["randomMapRotation"]);

	// Timer triggers slot after config is fully loaded.
	QTimer::singleShot(0, this, SLOT(updateMapWarningVisibility()));
}

void MapListPanel::saveConfig(Ini &config)
{
	IniSection section = config.section("Rules");
	QStringList stringList = CommonGUI::listViewStandardItemsToStringList(d->lstMaplist);
	section["maplist"] = stringList.join(";");
	section["randomMapRotation"] = d->cbRandomMapRotation->isChecked();
}

void MapListPanel::setCreateServerDialog(CreateServerDialog *dialog)
{
	d->parentDialog = dialog;
}

void MapListPanel::setupForEngine(const EnginePlugin *engine)
{
	d->cbRandomMapRotation->setVisible(engine->data()->supportsRandomMapRotation);
}
