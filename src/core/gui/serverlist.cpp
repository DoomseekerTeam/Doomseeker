//------------------------------------------------------------------------------
// serverlist.cpp
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
#include "serverlist.h"

#include "configuration/doomseekerconfig.h"
#include "gui/mainwindow.h"
#include "gui/models/serverlistcolumn.h"
#include "gui/models/serverlistmodel.h"
#include "gui/models/serverlistproxymodel.h"
#include "gui/remoteconsole.h"
#include "gui/widgets/serverlistcontextmenu.h"
#include "gui/widgets/serverlistview.h"
#include "refresher/refresher.h"
#include "serverapi/server.h"
#include "serverapi/tooltips/servertooltip.h"
#include "urlopener.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QToolTip>

using namespace ServerListColumnId;

ServerList::ServerList(ServerListView *serverTable, MainWindow *pMainWindow)
	: mainWindow(pMainWindow), model(nullptr), needsCleaning(false),
	proxyModel(nullptr), sortOrder(Qt::AscendingOrder),
	sortIndex(-1), table(serverTable)
{
	prepareServerTable();
	initCleanerTimer();
}

ServerList::~ServerList()
{
	saveColumnsWidthsSettings();
}

void ServerList::applyFilter(const ServerListFilterInfo &filterInfo)
{
	gConfig.serverFilter.info = filterInfo;
	proxyModel->setFilterInfo(filterInfo);
	needsCleaning = true;
}

bool ServerList::areColumnsWidthsSettingsChanged()
{
	for (int i = 0; i < NUM_SERVERLIST_COLUMNS; ++i)
	{
		if (ServerListColumns::columns[i].width != table->columnWidth(i))
			return true;
	}

	return false;
}

void ServerList::cleanUp()
{
	if (needsCleaning)
		cleanUpRightNow();
}

void ServerList::cleanUpRightNow()
{
	if (mainWindow->isEffectivelyActiveWindow())
		cleanUpForce();
}

void ServerList::cleanUpForce()
{
	if (table == nullptr || table->model() == nullptr)
		return;

	if (sortIndex >= 0)
	{
		auto pModel = static_cast<ServerListProxyModel *>(table->model());
		pModel->invalidate();
		pModel->sortServers(sortIndex, sortOrder);
	}

	setCountryFlagsIfNotPresent();
	needsCleaning = false;
}

void ServerList::clearAdditionalSorting()
{
	proxyModel->clearAdditionalSorting();
}

void ServerList::columnHeaderClicked(int index)
{
	if (isSortingByColumn(index))
		sortOrder = swappedCurrentSortOrder();
	else
		sortOrder = getColumnDefaultSortOrder(index);
	sortIndex = index;

	cleanUpRightNow();

	QHeaderView *header = table->horizontalHeader();
	header->setSortIndicator(sortIndex, sortOrder);
}

void ServerList::connectTableModelProxySlots()
{
	QHeaderView *header = table->horizontalHeader();
	this->connect(header, SIGNAL(sectionClicked(int)), SLOT(columnHeaderClicked(int)));

	this->connect(table->selectionModel(),
		SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
		SLOT(itemSelected(QItemSelection)));
	this->connect(table, SIGNAL(middleMouseClicked(QModelIndex,QPoint)),
		SLOT(tableMiddleClicked()));
	this->connect(table, SIGNAL(rightMouseClicked(QModelIndex,QPoint)),
		SLOT(tableRightClicked(QModelIndex,QPoint)));
	this->connect(table, SIGNAL(entered(QModelIndex)), SLOT(mouseEntered(QModelIndex)));
	this->connect(table, SIGNAL(leftMouseDoubleClicked(QModelIndex,QPoint)),
		SLOT(doubleClicked(QModelIndex)));
}

void ServerList::contextMenuAboutToHide()
{
	sender()->deleteLater();
}

void ServerList::contextMenuTriggered(QAction *action)
{
	auto contextMenu = static_cast<ServerListContextMenu *>(sender());
	ServerPtr server = contextMenu->server();
	// 1. This is a bit convoluted, but emitting the serverFilterModified
	//    signal leads to a call to applyFilter() in this class.
	// 2. Since the menu modifies existing server filter, the worst that can
	//    happen is that we set the same filter again.
	emit serverFilterModified(contextMenu->serverFilter());

	ServerListContextMenu::Result contextMenuResult = contextMenu->translateQMenuResult(action);
	switch (contextMenuResult)
	{
	case ServerListContextMenu::DataCopied:
		// Do nothing.
		break;

	case ServerListContextMenu::FindMissingWADs:
		emit findMissingWADs(server);
		break;

	case ServerListContextMenu::Join:
		emit serverDoubleClicked(server);
		break;

	case ServerListContextMenu::OpenRemoteConsole:
		new RemoteConsole(server);
		break;

	case ServerListContextMenu::OpenURL:
		// Calling QDesktopServices::openUrl() here directly resulted
		// in a crash somewhere in Qt libraries. UrlOpener defers the
		// call with a timer and this fixes the crash.
		UrlOpener::instance()->open(server->webSite());
		break;

	case ServerListContextMenu::NothingHappened:
		// Do nothing; ignore.
		break;

	case ServerListContextMenu::Refresh:
		refreshSelected();
		break;

	case ServerListContextMenu::ShowJoinCommandLine:
		emit displayServerJoinCommandLine(server);
		break;

	case ServerListContextMenu::SortAdditionallyAscending:
		sortAdditionally(contextMenu->modelIndex(), Qt::AscendingOrder);
		break;

	case ServerListContextMenu::SortAdditionallyDescending:
		sortAdditionally(contextMenu->modelIndex(), Qt::DescendingOrder);
		break;

	case ServerListContextMenu::RemoveAdditionalSortingForColumn:
		removeAdditionalSortingForColumn(contextMenu->modelIndex());
		break;

	case ServerListContextMenu::ClearAdditionalSorting:
		clearAdditionalSorting();
		break;

	case ServerListContextMenu::TogglePinServers:
		for (const ServerPtr &server : contextMenu->servers())
		{
			model->redraw(server.data());
		}
		break;

	default:
		QMessageBox::warning(mainWindow, tr("Doomseeker - context menu warning"),
			tr("Unhandled behavior in ServerList::contextMenuTriggered()"));
		break;
	}
}

ServerListModel *ServerList::createModel()
{
	auto serverListModel = new ServerListModel(this);
	serverListModel->prepareHeaders();
	return serverListModel;
}

ServerListProxyModel *ServerList::createSortingProxy(ServerListModel *serverListModel)
{
	auto proxy = new ServerListProxyModel(this);
	this->connect(proxy, SIGNAL(additionalSortColumnsChanged()),
		SLOT(updateHeaderTitles()));
	this->connect(proxy, SIGNAL(additionalSortColumnsChanged()),
		SLOT(saveAdditionalSortingConfig()));
	proxy->setSourceModel(serverListModel);
	proxy->setSortRole(ServerListModel::SLDT_SORT);
	proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
	proxy->setFilterKeyColumn(IDServerName);

	return proxy;
}

void ServerList::doubleClicked(const QModelIndex &index)
{
	emit serverDoubleClicked(serverFromIndex(index));
}

Qt::SortOrder ServerList::getColumnDefaultSortOrder(int columnId)
{
	// Right now we can assume that columnIndex == columnId.
	return ServerListColumns::columns[columnId].defaultSortOrder;
}

bool ServerList::hasAtLeastOneServer() const
{
	return model->rowCount() > 0;
}

void ServerList::initCleanerTimer()
{
	cleanerTimer.setInterval(200);
	cleanerTimer.start();
	connect(&cleanerTimer, SIGNAL(timeout()), this, SLOT (cleanUp()));
}

bool ServerList::isAnyColumnSortedAdditionally() const
{
	return proxyModel->isAnyColumnSortedAdditionally();
}

bool ServerList::isSortingAdditionallyByColumn(int column) const
{
	return proxyModel->isSortingAdditionallyByColumn(column);
}

bool ServerList::isSortingByColumn(int columnIndex)
{
	return sortIndex == columnIndex;
}

void ServerList::itemSelected(const QItemSelection &selection)
{
	auto pModel = static_cast<QSortFilterProxyModel *>(table->model());
	QModelIndexList indexList = selection.indexes();

	QList<ServerPtr> servers;
	for (int i = 0; i < indexList.count(); ++i)
	{
		QModelIndex realIndex = pModel->mapToSource(indexList[i]);
		ServerPtr server = model->serverFromList(realIndex);
		servers.append(server);
	}
	emit serversSelected(servers);
}

void ServerList::lookupHosts()
{
	for (int i = 0; i < model->rowCount(); ++i)
	{
		ServerPtr server = model->serverFromList(i);
		server->lookupHost();
	}
}

void ServerList::mouseEntered(const QModelIndex &index)
{
	auto pModel = static_cast<QSortFilterProxyModel *>(table->model());
	QModelIndex realIndex = pModel->mapToSource(index);
	ServerPtr server = model->serverFromList(realIndex);
	QString tooltip;

	// Functions inside cases perform checks on the server structure
	// to see if any tooltip should be generated. Empty string is returned
	// in case if it should be not.
	switch (index.column())
	{
	case IDPort:
		tooltip = ServerTooltip::createPortToolTip(server);
		break;

	case IDAddress:
		tooltip = server->hostName(true);
		break;

	case IDPlayers:
		tooltip = ServerTooltip::createPlayersToolTip(server);
		break;

	case IDServerName:
		tooltip = ServerTooltip::createServerNameToolTip(server);
		break;

	case IDIwad:
		tooltip = ServerTooltip::createIwadToolTip(server);
		break;

	case IDWads:
		tooltip = ServerTooltip::createPwadsToolTip(server);
		break;

	default:
		tooltip = "";
		break;
	}

	QToolTip::showText(QCursor::pos(), tooltip, nullptr);
}

void ServerList::prepareServerTable()
{
	model = createModel();
	proxyModel = createSortingProxy(model);

	columnHeaderClicked(IDPlayers);
	table->setModel(proxyModel);
	table->setupTableProperties();

	if (gConfig.doomseeker.serverListSortIndex >= 0)
	{
		sortIndex = gConfig.doomseeker.serverListSortIndex;
		sortOrder = static_cast<Qt::SortOrder>(gConfig.doomseeker.serverListSortDirection);
	}

	connectTableModelProxySlots();
	proxyModel->setAdditionalSortColumns(gConfig.doomseeker.additionalSortColumns());
}

void ServerList::redraw()
{
	model->redrawAll();
}

void ServerList::refreshSelected()
{
	for (const ServerPtr &server : selectedServers())
	{
		gRefresher->registerServer(server.data());
	}
}

void ServerList::registerServer(ServerPtr server)
{
	ServerPtr serverOnList = model->findSameServer(server.data());
	if (serverOnList != nullptr)
	{
		serverOnList->setCustom(server->isCustom() || serverOnList->isCustom());
		model->redraw(serverOnList.data());
		return;
	}
	this->connect(server.data(), SIGNAL(updated(ServerPtr,int)),
		SLOT(onServerUpdated(ServerPtr)));
	this->connect(server.data(), SIGNAL(begunRefreshing(ServerPtr)),
		SLOT(onServerBegunRefreshing(ServerPtr)));
	model->addServer(server);
	emit serverRegistered(server);
}

void ServerList::removeServer(const ServerPtr &server)
{
	server->disconnect(this);
	model->removeServer(server);
	emit serverDeregistered(server);
}

void ServerList::removeCustomServers()
{
	for (ServerPtr server : model->customServers())
	{
		removeServer(server);
	}
}

void ServerList::removeNonSpecialServers()
{
	for (ServerPtr server : model->nonSpecialServers())
	{
		removeServer(server);
	}
}

void ServerList::removeAdditionalSortingForColumn(const QModelIndex &modelIndex)
{
	proxyModel->removeAdditionalColumnSorting(modelIndex.column());
}

void ServerList::saveAdditionalSortingConfig()
{
	gConfig.doomseeker.setAdditionalSortColumns(proxyModel->additionalSortColumns());
}

void ServerList::saveColumnsWidthsSettings()
{
	gConfig.doomseeker.serverListColumnState = table->horizontalHeader()->saveState().toBase64();
	gConfig.doomseeker.serverListSortIndex = sortIndex;
	gConfig.doomseeker.serverListSortDirection = sortOrder;
}

QList<ServerPtr> ServerList::selectedServers() const
{
	QModelIndexList indexList = table->selectionModel()->selectedRows();

	QList<ServerPtr> servers;
	for (int i = 0; i < indexList.count(); ++i)
	{
		QModelIndex realIndex = proxyModel->mapToSource(indexList[i]);
		ServerPtr server = model->serverFromList(realIndex);
		servers.append(server);
	}
	return servers;
}

void ServerList::onServerBegunRefreshing(const ServerPtr &server)
{
	model->setRefreshing(server);
}

QList<ServerPtr> ServerList::servers() const
{
	return model->servers();
}

ServerPtr ServerList::serverFromIndex(const QModelIndex &index)
{
	auto pModel = static_cast<QSortFilterProxyModel *>(table->model());
	QModelIndex indexReal = pModel->mapToSource(index);
	return model->serverFromList(indexReal);
}

QList<ServerPtr> ServerList::serversForPlugin(const EnginePlugin *plugin) const
{
	return model->serversForPlugin(plugin);
}

void ServerList::onServerUpdated(const ServerPtr &server)
{
	int rowIndex = model->findServerOnTheList(server.data());
	if (rowIndex >= 0)
		rowIndex = model->updateServer(rowIndex, server);
	else
		rowIndex = model->addServer(server);

	needsCleaning = true;
	emit serverInfoUpdated(server);
}

void ServerList::setCountryFlagsIfNotPresent()
{
	const bool FORCE = true;
	updateCountryFlags(!FORCE);
}

void ServerList::setGroupServersWithPlayersAtTop(bool b)
{
	proxyModel->setGroupServersWithPlayersAtTop(b);
}

void ServerList::sortAdditionally(const QModelIndex &modelIndex, Qt::SortOrder order)
{
	auto model = static_cast<ServerListProxyModel *>(table->model());
	model->addAdditionalColumnSorting(modelIndex.column(), order);
}

Qt::SortOrder ServerList::swappedCurrentSortOrder()
{
	return sortOrder == Qt::AscendingOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
}

void ServerList::tableMiddleClicked()
{
	refreshSelected();
}

void ServerList::tableRightClicked(const QModelIndex &index, const QPoint &cursorPosition)
{
	ServerPtr server = serverFromIndex(index);

	ServerListContextMenu *contextMenu = new ServerListContextMenu(server,
		proxyModel->filterInfo(), index, selectedServers(), this);
	this->connect(contextMenu, SIGNAL(aboutToHide()), SLOT(contextMenuAboutToHide()));
	this->connect(contextMenu, SIGNAL(triggered(QAction*)), SLOT(contextMenuTriggered(QAction*)));

	QPoint displayPoint = table->viewport()->mapToGlobal(cursorPosition);
	contextMenu->popup(displayPoint);
}

void ServerList::updateCountryFlags()
{
	const bool FORCE = true;
	updateCountryFlags(FORCE);
}

void ServerList::updateCountryFlags(bool force)
{
	for (int i = 0; i < model->rowCount(); ++i)
		model->updateFlag(i, force);
}

void ServerList::updateHeaderTitles()
{
	const QList<ColumnSort> &sortings = proxyModel->additionalSortColumns();
	for (int i = 0; i < ServerListColumnId::NUM_SERVERLIST_COLUMNS; ++i)
	{
		// Clear header icons.
		model->setHeaderData(i, Qt::Horizontal, QIcon(), Qt::DecorationRole);
	}
	QStringList labels = ServerListColumns::generateColumnHeaderLabels();
	for (int i = 0; i < sortings.size(); ++i)
	{
		const ColumnSort &sort = sortings[i];
		labels[sort.columnId()] = QString("[%1] %2").arg(i + 1).arg(labels[sort.columnId()]);
		QIcon icon = sort.order() == Qt::AscendingOrder ?
			QIcon(":/icons/ascending.png") :
			QIcon(":/icons/descending.png");
		model->setHeaderData(sort.columnId(), Qt::Horizontal, icon, Qt::DecorationRole);
	}
	model->setHorizontalHeaderLabels(labels);
}

void ServerList::updateSearch(const QString &search)
{
	QRegExp pattern(QString("*") + search + "*", Qt::CaseInsensitive, QRegExp::Wildcard);
	proxyModel->setFilterRegExp(pattern);
}
