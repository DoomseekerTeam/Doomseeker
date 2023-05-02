//------------------------------------------------------------------------------
// serverlistmodel.h
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
#ifndef __SERVER_LIST_MODEL_H_
#define __SERVER_LIST_MODEL_H_

#include "serverapi/serverptr.h"
#include <QStandardItemModel>

class EnginePlugin;
class Server;
class ServerListSortFilterProxyModel;
class ServerList;

class ServerListModel : public QStandardItemModel
{
	Q_OBJECT

	friend class ServerList;
	friend class ServerListProxyModel;

public:
	/**
	 * Servers from the same group will be always kept together
	 * and sorted only inside this group. Group order is always descending:
	 * SG_NORMAL servers will be always on the top of the list, after them
	 * will be SG_WAIT servers, etc.
	 *
	 * @b WARNING:
	 * Exception: custom servers will always be on top of the list
	 * and will be sorted inside their own group independentedly.
	 */
	enum ServerGroup
	{
		SG_NORMAL = 200,
		SG_WAIT = 175,
		SG_BANNED = 150,
		SG_TIMEOUT = 125,
		SG_BAD = 100,
		SG_FIRST_QUERY = 50,
	};

	enum ServerListDataTypes
	{
		// Pointer to the server structure is always stored in the first column
		SLDT_POINTER_TO_SERVER_STRUCTURE = Qt::UserRole + 1,
		SLDT_SORT = Qt::UserRole + 2,
		SLDT_SERVER_GROUP = Qt::UserRole + 3
	};

	ServerListModel(ServerList *parent);

	/**
	 * @return New row index.
	 */
	int addServer(ServerPtr server);
	QList<ServerPtr> customServers() const;

	/**
	 * Finds "the same" server (plugin, host, port) that
	 * isn't necessarilly the same object.
	 */
	ServerPtr findSameServer(const Server *server);

	/**
	 * @brief Finds index of the row where server is contained.
	 *
	 * @return -1 in case of a failure or index of the row otherwise.
	 */
	int findServerOnTheList(const Server *server);

	ServerList *handler()
	{
		return parentHandler;
	}


	QList<ServerPtr> nonSpecialServers() const;
	QList<ServerPtr> servers() const;
	QList<ServerPtr> serversForPlugin(const EnginePlugin *plugin) const;

	/**
	 * Enforces update of a given row. No modificiation is done
	 * to the server info itself. Can be used to redraw things like
	 * background.
	 */
	void redraw(int row);

	/**
	 * Redraws row for the specified server. Does nothing if
	 * server is not found in the model.
	 */
	void redraw(Server *server);

	void redrawAll();

	void removeServer(const ServerPtr &server);

	/**
	 *  Updates flag on given row.
	 *  @param row - index of row to update
	 */
	void updateFlag(int row, bool force = true);

	/**
	 * Returns row index.
	 */
	int updateServer(int row, ServerPtr server);

	ServerPtr serverFromList(int rowIndex) const;
	ServerPtr serverFromList(const QModelIndex &) const;

	void setRefreshing(ServerPtr server);

signals:
	void allRowsContentChanged();
	void rowContentChanged(int row);

private:
	void prepareHeaders();
	ServerGroup serverGroup(int row);

	QVariant columnSortData(int row, int column);

	ServerList *parentHandler;
};

#endif
