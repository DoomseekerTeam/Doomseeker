//------------------------------------------------------------------------------
// serverlistmodel.cpp
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
#include "configuration/doomseekerconfig.h"
#include "gui/helpers/playersdiagram.h"
#include "gui/mainwindow.h"
#include "gui/models/serverlistcolumn.h"
#include "gui/models/serverlistmodel.h"
#include "gui/models/serverlistrowhandler.h"
#include "gui/serverlist.h"
#include "gui/widgets/serverlistview.h"
#include "log.h"
#include "serverapi/server.h"
#include <QItemDelegate>
#include <QTime>

using namespace ServerListColumnId;

//////////////////////////////////////////////////////////////

ServerListModel::ServerListModel(ServerList *parent)
	: QStandardItemModel(parent),
	parentHandler(parent)
{
	setSortRole(SLDT_SORT);
}

int ServerListModel::addServer(ServerPtr server)
{
	QList<QStandardItem *> columns = ServerListColumns::generateListOfCells();
	appendRow(columns);

	// Country flag is set only once. Set it here and avoid setting it in
	// updateServer() method.

	QModelIndex index = indexFromItem(columns[0]);
	ServerListRowHandler rowHandler(this, index.row(), server);
	if (parentHandler->getMainWindow()->isEffectivelyActiveWindow())
		rowHandler.setCountryFlag();

	return rowHandler.updateServer();
}

ServerPtr ServerListModel::findSameServer(const Server *server)
{
	CustomServerInfo serverInfo = CustomServerInfo::fromServer(server);
	for (int i = 0; i < rowCount(); ++i)
	{
		ServerPtr serverOnList = serverFromList(i);
		CustomServerInfo serverOnListInfo = CustomServerInfo::fromServer(serverOnList.data());
		if (serverOnListInfo.isSameServer(serverInfo))
			return serverOnList;
	}
	return ServerPtr();
}

int ServerListModel::findServerOnTheList(const Server *server)
{
	if (server != nullptr)
	{
		for (int i = 0; i < rowCount(); ++i)
		{
			ServerCPtr serverOnList = serverFromList(i);
			if (server == serverOnList)
				return i;
		}
	}
	return -1;
}

void ServerListModel::redraw(int row)
{
	ServerPtr server = serverFromList(row);
	ServerListRowHandler rowHandler(this, row, server);
	rowHandler.redraw();
}

void ServerListModel::redraw(Server *server)
{
	int row = findServerOnTheList(server);
	if (row >= 0)
		redraw(row);
}

void ServerListModel::redrawAll()
{
	PlayersDiagram::loadImages(gConfig.doomseeker.slotStyle);

	for (int i = 0; i < rowCount(); ++i)
		redraw(i);
}

QList<ServerPtr> ServerListModel::customServers() const
{
	QList<ServerPtr> servers;
	for (int i = 0; i < rowCount(); ++i)
	{
		ServerPtr server = serverFromList(i);
		if (server->isCustom())
			servers << server;
	}
	return servers;
}

QList<ServerPtr> ServerListModel::nonSpecialServers() const
{
	QList<ServerPtr> servers;
	for (int i = 0; i < rowCount(); ++i)
	{
		ServerPtr server = serverFromList(i);
		if (!server->isCustom() && !server->isLan())
			servers << server;
	}
	return servers;
}

QList<ServerPtr> ServerListModel::servers() const
{
	QList<ServerPtr> servers;
	for (int i = 0; i < rowCount(); ++i)
		servers << serverFromList(i);
	return servers;
}

QList<ServerPtr> ServerListModel::serversForPlugin(const EnginePlugin *plugin) const
{
	QList<ServerPtr> servers;
	for (int i = 0; i < rowCount(); ++i)
	{
		ServerPtr server = serverFromList(i);
		if (server->plugin() == plugin)
			servers << server;
	}
	return servers;
}

void ServerListModel::removeServer(const ServerPtr &server)
{
	int index = findServerOnTheList(server.data());
	if (index >= 0)
		removeRow(index);
}

ServerPtr ServerListModel::serverFromList(int rowIndex) const
{
	return ServerListRowHandler::serverFromList(this, rowIndex);
}

ServerPtr ServerListModel::serverFromList(const QModelIndex &index) const
{
	return ServerListRowHandler::serverFromList(this, index.row());
}

ServerListModel::ServerGroup ServerListModel::serverGroup(int row)
{
	QStandardItem *qstdItem = item(row, COL_META);
	return static_cast<ServerListModel::ServerGroup>(qstdItem->data(SLDT_SERVER_GROUP).toInt());
}

void ServerListModel::setRefreshing(ServerPtr server)
{
	int rowIndex = findServerOnTheList(server.data());
	if (rowIndex >= 0)
	{
		ServerListRowHandler rowHandler(this, rowIndex, server);
		rowHandler.setRefreshing();
	}
}

void ServerListModel::prepareHeaders()
{
	setHorizontalHeaderLabels(ServerListColumns::generateColumnHeaderLabels());
}

int ServerListModel::updateServer(int row, ServerPtr server)
{
	ServerListRowHandler rowHandler(this, row, server);
	rowHandler.updateServer();

	return row;
}

QVariant ServerListModel::columnSortData(int row, int column)
{
	QStandardItem *it = item(row, column);
	return it->data(SLDT_SORT);
}

void ServerListModel::updateFlag(int row, bool force)
{
	ServerPtr server = serverFromList(row);
	ServerListRowHandler rowHandler(this, row, server);
	QStandardItem *itm = item(row, IDServerName);

	if (force || itm->icon().isNull())
		rowHandler.setCountryFlag();
}
