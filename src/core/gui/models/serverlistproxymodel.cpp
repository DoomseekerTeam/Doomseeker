//------------------------------------------------------------------------------
// serverlistproxymodel.cpp
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
#include "serverlistproxymodel.h"

#include "gui/entity/serverlistfilterinfo.h"
#include "gui/mainwindow.h"
#include "gui/models/serverlistcolumn.h"
#include "gui/models/serverlistmodel.h"
#include "gui/serverlist.h"
#include "serverapi/playerslist.h"
#include "serverapi/server.h"
#include "serverapi/serverstructs.h"

#include <QWidget>

DClass<ServerListProxyModel>
{
public:
	QList<ColumnSort> additionalSortColumns;
	bool groupServersWithPlayersAtTop;
	int mainSortColumn;
	ServerList *parentHandler;
	ServerListFilterInfo filterInfo;
	Qt::SortOrder sortOrder;

	ColumnSort additionalSortForColumn(int column) const
	{
		for (const ColumnSort &sort : additionalSortColumns)
		{
			if (sort.columnId() == column)
				return sort;
		}
		return ColumnSort();
	}

	bool removeAdditionalColumnSorting(int column)
	{
		ColumnSort sort = additionalSortForColumn(column);
		if (sort.isValid())
		{
			additionalSortColumns.removeAll(sort);
			return true;
		}
		return false;
	}
};

DPointered(ServerListProxyModel)

ServerListProxyModel::ServerListProxyModel(ServerList *serverListHandler)
	: QSortFilterProxyModel(serverListHandler)
{
	d->groupServersWithPlayersAtTop = true;
	d->mainSortColumn = -1;
	d->parentHandler = serverListHandler;
}

ServerListProxyModel::~ServerListProxyModel()
{
}

void ServerListProxyModel::addAdditionalColumnSorting(int column, Qt::SortOrder order)
{
	if (d->mainSortColumn == column)
	{
		// No-op.
		return;
	}
	if (d->mainSortColumn >= 0)
	{
		d->removeAdditionalColumnSorting(column);
		d->additionalSortColumns << ColumnSort(column, order);
		emit additionalSortColumnsChanged();
	}
	else
	{
		d->mainSortColumn = column;
		d->sortOrder = order;
	}
	sort(d->mainSortColumn, d->sortOrder);
}

const QList<ColumnSort> &ServerListProxyModel::additionalSortColumns() const
{
	return d->additionalSortColumns;
}

void ServerListProxyModel::clearAdditionalSorting()
{
	if (!d->additionalSortColumns.isEmpty())
	{
		d->additionalSortColumns.clear();
		emit additionalSortColumnsChanged();
	}
}

#define RET_COMPARE(a, b) \
	{ \
		if ((a) < (b)) \
			return -1; \
		if ((a) == (b)) \
			return 0; \
		else \
			return 1; \
	}

int ServerListProxyModel::compareColumnSortData(QVariant &var1, QVariant &var2, int column) const
{
	using namespace ServerListColumnId;

	if (!(var1.isValid() || !var2.isValid()))
	{
		if (var1.isValid())
			return -1;
		if (var2.isValid())
			return 1;
		return 0;
	}

	switch (column)
	{
	case IDAddress:
		RET_COMPARE(var1.toUInt(), var2.toUInt());

	case IDPing:
	case IDPlayers:
		RET_COMPARE(var1.toInt(), var2.toInt());

	case IDPort:
	case IDGametype:
	case IDIwad:
	case IDMap:
	case IDServerName:
	case IDWads:
		RET_COMPARE(var1.toString(), var2.toString());

	default:
		return 0;
	}
}

bool ServerListProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
	Q_UNUSED(sourceParent);
	ServerPtr s = serverFromList(sourceRow);
	if (s == nullptr)
		return false;

	const QString &nameFilter = d->filterInfo.serverName;
	if (!nameFilter.isEmpty())
	{
		if (!s->name().contains(nameFilter, Qt::CaseInsensitive))
			return false;
	}

	if (!d->filterInfo.bEnabled)
		return true;

	if (!s->isKnown())
	{
		if (d->filterInfo.bShowOnlyValid)
			return false;
	}
	else
	{
		// To make sure we perform an 'AND' query here all operations
		// must be negative. This means that if their test succeeds,
		// false must be returned.
		//
		// The ServerListFilterInfo copy constructor and operator= make
		// sure that all strings are trimmed.
		if (!d->filterInfo.bShowEmpty && s->isEmpty())
			return false;

		if (!d->filterInfo.bShowFull && s->isFull())
			return false;

		if (d->filterInfo.maxPing > 0 && d->filterInfo.maxPing < s->ping())
			return false;

		if (d->filterInfo.testingServers == Doomseeker::ShowOnly && !s->isTestingServer())
			return false;
		else if (d->filterInfo.testingServers == Doomseeker::ShowNone && s->isTestingServer())
			return false;

		if (!d->filterInfo.gameModes.isEmpty())
		{
			if (!d->filterInfo.gameModes.contains(s->gameMode().name(), Qt::CaseInsensitive))
				return false;
		}

		if (d->filterInfo.gameModesExcluded.contains(s->gameMode().name(), Qt::CaseInsensitive))
			return false;

		if (!d->filterInfo.wads.isEmpty())
		{
			bool bWadFound = false;

			// TODO
			// This may cause performance drops. Testing is required
			for (const QString &filteredWad : d->filterInfo.wads)
			{
				if (s->anyWadnameContains(filteredWad))
				{
					bWadFound = true;
					break;
				}
			}

			if (!bWadFound)
				return false;
		}

		if (!d->filterInfo.wadsExcluded.isEmpty())
		{
			for (const QString &filteredWad : d->filterInfo.wadsExcluded)
			{
				if (s->anyWadnameContains(filteredWad))
					return false;
			}
		}
	}

	return true;
}

const ServerListFilterInfo &ServerListProxyModel::filterInfo() const
{
	return d->filterInfo;
}

bool ServerListProxyModel::isAnyColumnSortedAdditionally() const
{
	return !d->additionalSortColumns.isEmpty();
}

bool ServerListProxyModel::isSortingAdditionallyByColumn(int column) const
{
	return d->additionalSortForColumn(column).isValid();
}

bool ServerListProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
	if (!d->parentHandler->getMainWindow()->isEffectivelyActiveWindow())
		return false;

	auto model = static_cast<ServerListModel *>(sourceModel());

	ServerPtr s1 = serverFromList(left);
	ServerPtr s2 = serverFromList(right);

	if (s1 != nullptr && s2 != nullptr)
	{
		if (s1->isCustom() && !s2->isCustom())
			return d->sortOrder == Qt::AscendingOrder;
		else if (!s1->isCustom() && s2->isCustom())
			return d->sortOrder == Qt::DescendingOrder;

		if (s1->isLan() && !s2->isLan())
			return d->sortOrder == Qt::AscendingOrder;
		else if (!s1->isLan() && s2->isLan())
			return d->sortOrder == Qt::DescendingOrder;
	}

	ServerListModel::ServerGroup sg1 = model->serverGroup(left.row());
	ServerListModel::ServerGroup sg2 = model->serverGroup(right.row());

	if (sg1 != sg2 && sg1 != 0 && sg2 != 0)
	{
		if (sg1 > sg2)
			return d->sortOrder == Qt::AscendingOrder;
		else
			return d->sortOrder == Qt::DescendingOrder;
	}

	if (d->groupServersWithPlayersAtTop)
	{
		// Using data stored in column will honor user settings declaring
		// whether bots should be treated as players or not.
		int numPlayers1 = sourceModel()->data(left.sibling(left.row(),
			ServerListColumnId::IDPlayers), sortRole()).toInt();
		int numPlayers2 = sourceModel()->data(right.sibling(right.row(),
			ServerListColumnId::IDPlayers), sortRole()).toInt();
		if (numPlayers1 > 0 && numPlayers2 == 0)
			return d->sortOrder == Qt::AscendingOrder;
		else if (numPlayers1 == 0 && numPlayers2 > 0)
			return d->sortOrder == Qt::DescendingOrder;
	}

	QVariant leftVar = sourceModel()->data(left, sortRole());
	QVariant rightVar = sourceModel()->data(right, sortRole());
	int comparison = compareColumnSortData(leftVar, rightVar, left.column());
	if (comparison == 0)
	{
		for (const ColumnSort &additionalSort : d->additionalSortColumns)
		{
			QModelIndex additionalLeft = left.sibling(left.row(), additionalSort.columnId());
			QModelIndex additionalRight = right.sibling(right.row(), additionalSort.columnId());
			leftVar = sourceModel()->data(additionalLeft, sortRole());
			rightVar = sourceModel()->data(additionalRight, sortRole());
			comparison = compareColumnSortData(leftVar, rightVar, additionalSort.columnId());
			if (comparison != 0)
			{
				if (additionalSort.order() == Qt::DescendingOrder)
					comparison *= -1;
				break;
			}
		}
	}
	return comparison < 0;
}

void ServerListProxyModel::removeAdditionalColumnSorting(int column)
{
	bool anythingRemoved = d->removeAdditionalColumnSorting(column);
	if (d->mainSortColumn > 0)
		sort(d->mainSortColumn, d->sortOrder);
	if (anythingRemoved)
		emit additionalSortColumnsChanged();
}

void ServerListProxyModel::setFilterInfo(const ServerListFilterInfo &filterInfo)
{
	d->filterInfo = filterInfo;
	invalidate();
}

void ServerListProxyModel::setGroupServersWithPlayersAtTop(bool b)
{
	d->groupServersWithPlayersAtTop = b;
	invalidate();
}

ServerPtr ServerListProxyModel::serverFromList(const QModelIndex &index) const
{
	return serverFromList(index.row());
}

ServerPtr ServerListProxyModel::serverFromList(int row) const
{
	auto model = static_cast<ServerListModel *>(sourceModel());
	return model->serverFromList(row);
}

void ServerListProxyModel::setAdditionalSortColumns(const QList<ColumnSort> &columns)
{
	d->additionalSortColumns = columns;
	emit additionalSortColumnsChanged();
}

void ServerListProxyModel::sortServers(int column, Qt::SortOrder order)
{
	d->mainSortColumn = column;
	d->sortOrder = order;
	if (d->removeAdditionalColumnSorting(column))
		emit additionalSortColumnsChanged();
	sort(column, order);
}
///////////////////////////////////////////////////////////////////////////////
ColumnSort::ColumnSort()
{
	columnId_ = -1;
	order_ = Qt::AscendingOrder;
}

ColumnSort::ColumnSort(int columnId, Qt::SortOrder order)
{
	columnId_ = columnId;
	order_ = order;
}

int ColumnSort::columnId() const
{
	return columnId_;
}

ColumnSort ColumnSort::deserializeQVariant(const QVariant &v)
{
	QVariantMap map = v.toMap();
	return ColumnSort(map["columnId"].toInt(),
		static_cast<Qt::SortOrder>(map["order"].toInt())
	);
}

bool ColumnSort::isValid() const
{
	return columnId() >= 0;
}

Qt::SortOrder ColumnSort::order() const
{
	return order_;
}

bool ColumnSort::operator==(const ColumnSort &other) const
{
	return order() == other.order() && columnId() == other.columnId();
}

QVariant ColumnSort::serializeQVariant() const
{
	QVariantMap map;
	map["columnId"] = columnId();
	map["order"] = order();
	return map;
}
