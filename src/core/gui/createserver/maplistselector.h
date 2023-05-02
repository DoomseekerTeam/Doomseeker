//------------------------------------------------------------------------------
// maplistselector.h
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
#ifndef __MAPLISTSELECTOR_H_
#define __MAPLISTSELECTOR_H_

#include "dptr.h"
#include "ui_maplistselector.h"
#include <QDialog>

/**
 * @brief Stores the name of the map and the mod where it came from.
 */
struct MapEntry
{
	MapEntry(const QString &name, const QString &originalFile, const bool &isIwad);
	QString name;
	QString originalFile;
	bool isIwad;
};

/**
 * @brief Prompts the user to select a list of maps to insert.
 */
class MapListSelector : public QDialog
{
	Q_OBJECT

public:
	MapListSelector(QWidget *parent = nullptr);
	~MapListSelector() override;

	/**
	 * @brief adds the paths to check for maps. Please note that it will
	 * immediately start checking them.
	 */
	void addPaths(const QStringList &paths);
	/**
	 * @brief returns the list of maps that were checked.
	 */
	const QStringList selectedMaps();
private:
	void addEntryToMapList(const MapEntry &newEntry);
	void setMapsOfCheckableList(const QList<MapEntry> &list);
	void setIfFinishedStateIsEnabled(const bool &state);
	DPtr<MapListSelector> d;
private slots:
	void accept() override;
	void reject() override;
	void performCheckStep();
	void selectAll();
	void listItemChanged();
	void showContextMenu(const QPoint &pos);
	void invertSelection();
};

#endif
