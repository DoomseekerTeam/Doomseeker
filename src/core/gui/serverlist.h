//------------------------------------------------------------------------------
// serverlist.h
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
#ifndef __SERVERLIST_H_
#define __SERVERLIST_H_

#include <QObject>
#include <QTimer>

#include "serverapi/serverptr.h"

class EnginePlugin;
class PWad;
class IniSection;
class MainWindow;
class Server;
class ServerListFilterInfo;
class ServerListModel;
class ServerListProxyModel;
class ServerListView;
class WadFindResult;
class QAction;
class QItemSelection;
class QModelIndex;
class QPoint;
class QSortFilterProxyModel;

class ServerList : public QObject
{
	Q_OBJECT

public:
	ServerList(ServerListView *serverTable, MainWindow *pMainWindow);
	~ServerList() override;

	void cleanUpForce();
	void cleanUpRightNow();

	MainWindow *getMainWindow()
	{
		return mainWindow;
	}
	bool hasAtLeastOneServer() const;

	bool isAnyColumnSortedAdditionally() const;
	bool isSortingAdditionallyByColumn(int column) const;
	bool isSortingByColumn(int columnIndex);

	void removeCustomServers();
	void removeNonSpecialServers();
	QList<ServerPtr> selectedServers() const;
	QList<ServerPtr> servers() const;
	QList<ServerPtr> serversForPlugin(const EnginePlugin *plugin) const;

	ServerPtr serverFromIndex(const QModelIndex &);

public slots:
	void applyFilter(const ServerListFilterInfo &filterInfo);
	void cleanUp();
	/**
	 * @brief Looks up hosts for all available servers.
	 */
	void lookupHosts();
	void redraw();
	void refreshSelected();
	void registerServer(ServerPtr server);
	void removeServer(const ServerPtr &server);
	/**
	 * @brief Sets country flags for servers that don't have flags
	 * present yet.
	 */
	void setCountryFlagsIfNotPresent();
	void setGroupServersWithPlayersAtTop(bool b);

	void tableMiddleClicked();
	void tableRightClicked(const QModelIndex &index, const QPoint &cursorPosition);
	void updateCountryFlags();
	void updateSearch(const QString &search);

signals:
	/**
	 * Emitted when a request for join command line show is called.
	 */
	void displayServerJoinCommandLine(const ServerPtr &);

	/**
	 * Request to force missing wads to be searched for (including
	 * optionals).
	 */
	void findMissingWADs(const ServerPtr &);

	void serverDeregistered(ServerPtr);
	void serverFilterModified(const ServerListFilterInfo &filter);
	/**
	 * @brief Emitted every time when a server info is updated through
	 * onServerUpdated().
	 */
	void serverInfoUpdated(const ServerPtr &);
	void serverRegistered(ServerPtr);
	void serverDoubleClicked(const ServerPtr &);
	void serversSelected(QList<ServerPtr> &);

private:
	QTimer cleanerTimer;
	MainWindow *mainWindow;
	ServerListModel *model;
	bool needsCleaning;
	ServerListProxyModel *proxyModel;
	Qt::SortOrder sortOrder;
	int sortIndex;
	ServerListView *table;

	bool areColumnsWidthsSettingsChanged();

	void connectTableModelProxySlots();
	void clearAdditionalSorting();

	ServerListModel *createModel();
	ServerListProxyModel *createSortingProxy(ServerListModel *serverListModel);

	Qt::SortOrder getColumnDefaultSortOrder(int columnId);

	void initCleanerTimer();

	void prepareServerTable();

	void removeAdditionalSortingForColumn(const QModelIndex &modelIndex);
	void saveColumnsWidthsSettings();

	void setupTableColumnWidths();
	void setupTableProperties(QSortFilterProxyModel *tableModel);

	void sortAdditionally(const QModelIndex &modelIndex, Qt::SortOrder order);

	Qt::SortOrder swappedCurrentSortOrder();

	void updateCountryFlags(bool force);

private slots:
	void columnHeaderClicked(int);
	void contextMenuAboutToHide();
	void contextMenuTriggered(QAction *action);
	void doubleClicked(const QModelIndex &);
	void itemSelected(const QItemSelection &);
	void mouseEntered(const QModelIndex &);
	void saveAdditionalSortingConfig();
	void onServerBegunRefreshing(const ServerPtr &server);
	void onServerUpdated(const ServerPtr &server);
	void updateHeaderTitles();
};

#endif
