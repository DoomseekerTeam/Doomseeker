//------------------------------------------------------------------------------
// serverlistcontextmenu.h
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
#ifndef __SERVER_LIST_CONTEXT_MENU_H_
#define __SERVER_LIST_CONTEXT_MENU_H_

#include "dptr.h"
#include "serverapi/serverptr.h"
#include <QList>
#include <QObject>

class Server;
class ServerFilterBuilderMenu;
class ServerListFilterInfo;
class ServerList;
class QAction;
class QMenu;
class QModelIndex;
class QPoint;

class ServerListContextMenu : public QObject
{
	Q_OBJECT

public:
	enum Result
	{
		/// This is returned when something was copied to clipboard.
		DataCopied,

		Join,
		OpenRemoteConsole,
		OpenURL,
		Refresh,
		ShowJoinCommandLine,
		SortAdditionallyAscending,
		SortAdditionallyDescending,
		ClearAdditionalSorting,
		RemoveAdditionalSortingForColumn,
		FindMissingWADs,
		TogglePinServers,

		/// This is returned upon cancel.
		NothingHappened
	};

	ServerListContextMenu(ServerPtr serverAtIndex,
		const ServerListFilterInfo &filter,
		const QModelIndex &modelIndex,
		const QList<ServerPtr> &servers,
		ServerList *parent);
	~ServerListContextMenu() override;

	const QModelIndex &modelIndex() const;
	void popup(const QPoint &point);
	ServerPtr server() const;
	const QList<ServerPtr> &servers() const;
	const ServerListFilterInfo &serverFilter() const;
	Result translateQMenuResult(QAction *resultAction);

signals:
	void aboutToHide();
	void triggered(QAction *action);

private:
	DPtr<ServerListContextMenu> d;

	QMenu *createCopyMenu(QWidget *parent);
	void createMembers();
	void initializeMembers();
};

#endif
