//------------------------------------------------------------------------------
// serverlistcontextmenu.cpp
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
#include "serverlistcontextmenu.h"

#include "clipboard.h"
#include "customservers.h"
#include "gui/entity/serverlistfilterinfo.h"
#include "gui/serverlist.h"
#include "gui/widgets/serverfilterbuildermenu.h"
#include "serverapi/server.h"
#include "strings.hpp"
#include <cassert>
#include <QApplication>
#include <QModelIndex>

DClass<ServerListContextMenu>
{
public:
	QAction *copyAddress;
	QAction *copyEmail;
	QAction *copyName;
	QAction *copyUrl;
	QAction *clearAdditionalSorting;
	QAction *findMissingWads;
	QAction *join;
	QAction *openUrlInDefaultBrowser;
	QAction *rcon;
	QAction *refresh;
	QAction *removeAdditionalSortingForColumn;
	QAction *showJoinCommandLine;
	QAction *sortAdditionallyAscending;
	QAction *sortAdditionallyDescending;
	QAction *toggleServerPinned;
	QMenu *menu;
	ServerFilterBuilderMenu *filterBuilder;
	ServerPtr serverAtIndex;
	QModelIndex modelIndex;
	ServerListFilterInfo serverFilter;
	QList<ServerPtr> servers;
	ServerList *parent;

	bool isServerPinned() const
	{
		return CustomServers::isServerPinned(
			CustomServerInfo::fromServer(serverAtIndex.data()));
	}

	void togglePinAllServers()
	{
		bool toggleTo = !isServerPinned();
		for (const ServerPtr &server : servers)
		{
			CustomServers::setServerPinned(
				CustomServerInfo::fromServer(server.data()),
				toggleTo);
			server->setCustom(toggleTo);
		}
	}
};

DPointered(ServerListContextMenu)

ServerListContextMenu::ServerListContextMenu(ServerPtr serverAtIndex,
	const ServerListFilterInfo &filter,
	const QModelIndex &modelIndex,
	const QList<ServerPtr> &servers,
	ServerList *parent)
	: QObject(parent)
{
	d->serverAtIndex = serverAtIndex;
	d->parent = parent;
	d->servers = servers;
	d->serverFilter = filter;
	d->modelIndex = modelIndex;
	initializeMembers();
	createMembers();
}

ServerListContextMenu::~ServerListContextMenu()
{
	delete d->menu;
}

QMenu *ServerListContextMenu::createCopyMenu(QWidget *parent)
{
	QMenu *copyMenu = new QMenu(tr("Copy"), parent);
	d->copyAddress = copyMenu->addAction(tr("Copy Address"));

	if (!d->serverAtIndex->email().isEmpty())
		d->copyEmail = copyMenu->addAction(tr("Copy E-Mail"));

	if (!d->serverAtIndex->webSite().isEmpty())
		d->copyUrl = copyMenu->addAction(tr("Copy URL"));

	d->copyName = copyMenu->addAction(tr("Copy Name"));

	return copyMenu;
}

void ServerListContextMenu::createMembers()
{
	d->menu = new QMenu();
	this->connect(d->menu, SIGNAL(aboutToHide()), SIGNAL(aboutToHide()));
	this->connect(d->menu, SIGNAL(triggered(QAction*)), SIGNAL(triggered(QAction*)));

	d->refresh = d->menu->addAction(tr("Refresh"));
	d->join = d->menu->addAction(tr("Join"));
	d->showJoinCommandLine = d->menu->addAction(tr("Show join command line"));
	d->findMissingWads = d->menu->addAction(tr("Find missing WADs"));

	// Website.
	const QString &webSite = d->serverAtIndex->webSite();
	bool bShouldAllowOpenUrl = !webSite.isEmpty() && Strings::isUrlSafe(webSite);

	if (bShouldAllowOpenUrl)
		d->openUrlInDefaultBrowser = d->menu->addAction(tr("Open URL in browser"));

	// Pinning ("marking as favourite").
	QString pinnedLabel = !d->isServerPinned() ? tr("Pin") : tr ("Unpin");
	d->toggleServerPinned = d->menu->addAction(pinnedLabel);

	// Copy menu.
	QMenu *copyMenu = createCopyMenu(d->menu);
	d->menu->addMenu(copyMenu);

	// Filter builder.
	d->filterBuilder = new ServerFilterBuilderMenu(*d->serverAtIndex, d->serverFilter, d->menu);
	if (d->serverAtIndex->isKnown() && !d->filterBuilder->isEmpty())
		d->menu->addMenu(d->filterBuilder);

	d->rcon = nullptr;
	if (d->serverAtIndex->hasRcon())
	{
		d->menu->addSeparator();
		d->rcon = d->menu->addAction(tr("Remote Console"));
	}

	// Sorts.
	d->menu->addSeparator();
	if (!d->parent->isSortingByColumn(d->modelIndex.column()))
	{
		d->sortAdditionallyAscending = d->menu->addAction(tr("Sort additionally ascending"));
		d->sortAdditionallyDescending = d->menu->addAction(tr("Sort additionally descending"));
	}
	if (d->parent->isSortingAdditionallyByColumn(d->modelIndex.column()))
	{
		d->removeAdditionalSortingForColumn = d->menu->addAction(
			tr("Remove additional sorting for column"));
	}
	if (d->parent->isAnyColumnSortedAdditionally())
		d->clearAdditionalSorting = d->menu->addAction(tr("Clear additional sorting"));
}

void ServerListContextMenu::initializeMembers()
{
	d->clearAdditionalSorting = nullptr;
	d->removeAdditionalSortingForColumn = nullptr;
	d->sortAdditionallyAscending = nullptr;
	d->sortAdditionallyDescending = nullptr;
	d->copyAddress = nullptr;
	d->copyEmail = nullptr;
	d->copyName = nullptr;
	d->copyUrl = nullptr;
	d->filterBuilder = nullptr;
	d->findMissingWads = nullptr;
	d->join = nullptr;
	d->menu = nullptr;
	d->openUrlInDefaultBrowser = nullptr;
	d->rcon = nullptr;
	d->refresh = nullptr;
	d->showJoinCommandLine = nullptr;
	d->toggleServerPinned = nullptr;
}

const QModelIndex &ServerListContextMenu::modelIndex() const
{
	return d->modelIndex;
}

void ServerListContextMenu::popup(const QPoint &point)
{
	d->menu->popup(point);
}

ServerPtr ServerListContextMenu::server() const
{
	return d->serverAtIndex;
}

const QList<ServerPtr> &ServerListContextMenu::servers() const
{
	return d->servers;
}

const ServerListFilterInfo &ServerListContextMenu::serverFilter() const
{
	assert(d->filterBuilder);
	return d->filterBuilder->filter();
}

ServerListContextMenu::Result ServerListContextMenu::translateQMenuResult(QAction *resultAction)
{
	if (resultAction == nullptr)
		return NothingHappened;

	// Now perform checks against menu items.
	if (resultAction == d->refresh)
		return Refresh;
	else if (resultAction == d->join)
		return Join;
	else if (resultAction == d->showJoinCommandLine)
		return ShowJoinCommandLine;
	else if (resultAction == d->openUrlInDefaultBrowser)
		return OpenURL;
	else if (resultAction == d->copyAddress)
	{
		QString addr = QString("%1:%2").arg(d->serverAtIndex->address().toString()).arg(d->serverAtIndex->port());
		Clipboard::setText(addr);
		return DataCopied;
	}
	else if (resultAction == d->copyEmail)
	{
		Clipboard::setText(d->serverAtIndex->email());
		return DataCopied;
	}
	else if (resultAction == d->copyName)
	{
		Clipboard::setText(d->serverAtIndex->name());
		return DataCopied;
	}
	else if (resultAction == d->copyUrl)
	{
		Clipboard::setText(d->serverAtIndex->webSite());
		return DataCopied;
	}
	else if (resultAction == d->findMissingWads)
		return FindMissingWADs;
	else if (resultAction == d->rcon)
		return OpenRemoteConsole;
	else if (resultAction == d->sortAdditionallyAscending)
		return SortAdditionallyAscending;
	else if (resultAction == d->sortAdditionallyDescending)
		return SortAdditionallyDescending;
	else if (resultAction == d->clearAdditionalSorting)
		return ClearAdditionalSorting;
	else if (resultAction == d->removeAdditionalSortingForColumn)
		return RemoveAdditionalSortingForColumn;
	else if (resultAction == d->toggleServerPinned)
	{
		d->togglePinAllServers();
		return TogglePinServers;
	}

	return NothingHappened;
}
