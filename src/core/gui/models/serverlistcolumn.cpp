//------------------------------------------------------------------------------
// serverlistcolumn.cpp
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
#include "serverlistcolumn.h"

#include <QStandardItem>
#include <QStringList>

using namespace ServerListColumnId;

#define HIDDEN true
#define RESIZEABLE true

const ServerListColumn ServerListColumns::columns[] =
{
	{ IDPort, 24, !HIDDEN, !RESIZEABLE, Qt::AscendingOrder },
	{ IDPlayers, 60, !HIDDEN, RESIZEABLE, Qt::DescendingOrder },
	{ IDPing, 50, !HIDDEN, RESIZEABLE, Qt::AscendingOrder },
	{ IDServerName, 200, !HIDDEN, RESIZEABLE, Qt::AscendingOrder },
	{ IDAddress, 120, !HIDDEN, RESIZEABLE, Qt::AscendingOrder },
	{ IDIwad, 90, !HIDDEN, RESIZEABLE, Qt::AscendingOrder },
	{ IDMap, 70, !HIDDEN, RESIZEABLE, Qt::AscendingOrder },
	{ IDWads, 120, !HIDDEN, RESIZEABLE, Qt::AscendingOrder },
	{ IDGametype, 150, !HIDDEN, RESIZEABLE, Qt::AscendingOrder },
	{ IDHiddenGroup, 0, HIDDEN, !RESIZEABLE, Qt::DescendingOrder },
	{ IDHiddenServerPointer, 0, HIDDEN, !RESIZEABLE, Qt::AscendingOrder }
};

QString ServerListColumns::columnLabel(int columnId)
{
	// Column labels need to initialized after the translation is loaded
	// or they won't get traslated. This piece of code was initialized
	// statically before.
	switch (columnId)
	{
	case IDPort:
		return "";
	case IDPlayers:
		return tr("Players");
	case IDPing:
		return tr("Ping");
	case IDServerName:
		return tr("Server Name");
	case IDAddress:
		return tr("Address");
	case IDIwad:
		return tr("IWAD");
	case IDMap:
		return tr("Map");
	case IDWads:
		return tr("WADs");
	case IDGametype:
		return tr("Game Type");
	case IDHiddenGroup:
		return "SORT_GROUP";
	case IDHiddenServerPointer:
		return "SERVER_POINTER";
	default:
		return "UNKNOWN_COLUMN";
	}
}

QStringList ServerListColumns::generateColumnHeaderLabels()
{
	QStringList labels;
	for (int i = 0; i < NUM_SERVERLIST_COLUMNS; ++i)
		labels << columnLabel(i);
	return labels;
}

QList<QStandardItem *> ServerListColumns::generateListOfCells()
{
	QList<QStandardItem *> cells;
	for (int x = 0; x < NUM_SERVERLIST_COLUMNS; ++x)
		cells.append(new QStandardItem());
	return cells;
}

bool ServerListColumns::isColumnVital(int columnId)
{
	// We assume that columnIndex == columnId.
	return columns[columnId].bHidden || columnId == IDAddress || columnId == IDPort;
}
