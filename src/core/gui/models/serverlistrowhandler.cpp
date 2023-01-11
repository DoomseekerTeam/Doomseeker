//------------------------------------------------------------------------------
// serverlistrowhandler.cpp
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
#include "application.h"
#include "configuration/doomseekerconfig.h"
#include "gui/dockBuddiesList.h"
#include "gui/helpers/playersdiagram.h"
#include "gui/mainwindow.h"
#include "gui/serverlist.h"
#include "gui/widgets/serverlistview.h"
#include "ip2c/ip2c.h"
#include "log.h"
#include "serverapi/playerslist.h"
#include "serverapi/server.h"
#include "serverapi/serverstructs.h"
#include "serverlistcolumn.h"
#include "serverlistmodel.h"
#include "serverlistrowhandler.h"
#include <QPainter>

using namespace ServerListColumnId;

DClass<ServerListRowHandler>
{
public:
	ServerPtr server;

	QPixmap flag()
	{
		IP2C *ip2c = IP2C::instance();
		const IP2CCountry countryInfo = ip2c->countryInfoForServer(*server);
		return (countryInfo.isValid() && countryInfo.flag != nullptr) ?
			*countryInfo.flag : ip2c->flagUnknown;
	}
};

DPointered(ServerListRowHandler)

ServerListRowHandler::ServerListRowHandler(ServerListModel *parentModel,
	int rowIndex, const ServerPtr &server)
	: model(parentModel), row(rowIndex)
{
	d->server = server;
}

ServerListRowHandler::ServerListRowHandler(ServerListModel *parentModel, int rowIndex)
	: model(parentModel), row(rowIndex)
{
	d->server = serverFromList(parentModel, rowIndex);
}

ServerListRowHandler::~ServerListRowHandler()
{
}

void ServerListRowHandler::clearNonVitalFields()
{
	for (int i = 0; i < NUM_SERVERLIST_COLUMNS; ++i)
	{
		if (!ServerListColumns::isColumnVital(i))
			emptyItem(item(i));
	}
}

void ServerListRowHandler::emptyItem(QStandardItem *item)
{
	item->setData("", Qt::DisplayRole);
	item->setData(QVariant(), Qt::DecorationRole);
	item->setData(QVariant(), ServerListModel::SLDT_SORT);
}

QStringList ServerListRowHandler::extractValidGameCVarNames(const QList<GameCVar> &cvars)
{
	QStringList result;
	for (const GameCVar &cvar : cvars)
	{
		if (!cvar.isValid())
			result << cvar.name();
	}
	return result;
}

void ServerListRowHandler::fillAddressColumn()
{
	QStandardItem *pItem = item(IDAddress);
	fillItem(pItem, d->server->address(), d->server->hostName());
}

void ServerListRowHandler::fillItem(QStandardItem *item, const QString &str)
{
	QString strLowcase = str.toLower();
	item->setData(str, Qt::DisplayRole);
	item->setData(strLowcase, ServerListModel::SLDT_SORT);
}

void ServerListRowHandler::fillItem(QStandardItem *item, int sort, const QString &str)
{
	QVariant var = sort;

	fillItem(item, str);
	item->setData(sort, ServerListModel::SLDT_SORT);
}

void ServerListRowHandler::fillItem(QStandardItem *item, int num)
{
	QVariant var = num;

	item->setData(var, Qt::DisplayRole);
	item->setData(var, ServerListModel::SLDT_SORT);
}

void ServerListRowHandler::fillItem(QStandardItem *item, const QHostAddress &addr, const QString &actualDisplay)
{
	QVariant var = addr.toIPv4Address();

	if (actualDisplay.isEmpty())
		item->setData(addr.toString(), Qt::DisplayRole);
	else
		item->setData(actualDisplay, Qt::DisplayRole);
	item->setData(var, ServerListModel::SLDT_SORT);
}

void ServerListRowHandler::fillItem(QStandardItem *item, const QString &sort, const QPixmap &icon)
{
	item->setIcon(QIcon(icon));
	item->setData(sort, ServerListModel::SLDT_SORT);
}

void ServerListRowHandler::fillItem(QStandardItem *item, int sort, const QPixmap &image)
{
	item->setData(image, Qt::DecorationRole);
	item->setData(sort, ServerListModel::SLDT_SORT);
}

void ServerListRowHandler::fillPlayerColumn()
{
	QStandardItem *pItem = item(IDPlayers);

	QString style = gConfig.doomseeker.slotStyle;
	bool botsAreNotPlayers = gConfig.doomseeker.bBotsAreNotPlayers;

	const PlayersList &players = d->server->players();
	int sortValue = 0;

	if (botsAreNotPlayers)
		sortValue = players.numClientsWithoutBots();
	else
		sortValue = players.numClients();

	if (!PlayersDiagram::isNumericStyle(style))
		fillItem(pItem, sortValue, PlayersDiagram(d->server).pixmap());
	else
	{
		fillItem(pItem, sortValue, QString("%1/%2").arg(players.numClients())
			.arg(d->server->maxClients()));
	}

	// Unset some data if it has been set before.
	pItem->setData(QVariant(QVariant::Invalid), PlayersDiagram::isNumericStyle(style) ? Qt::DecorationRole : Qt::DisplayRole);
	pItem->setData(PlayersDiagram::isNumericStyle(style) ? 0 : USERROLE_RIGHTALIGNDECORATION, Qt::UserRole);
}

void ServerListRowHandler::fillPortIconColumn()
{
	QStandardItem *pItem = item(IDPort);
	QPixmap icon = d->server->icon();
	if (d->server->isKnown())
	{
		if (d->server->isLockedAnywhere()) // Draw a key if it is locked.
		{
			QPainter iconPainter(&icon);
			iconPainter.drawPixmap(0, 0, QPixmap(":/locked.png"));
			iconPainter.end();
		}
		else if (d->server->isSecure())
		{
			QPainter iconPainter(&icon);
			iconPainter.drawPixmap(0, 0, QPixmap(":/shield.png"));
			iconPainter.end();
		}
		// 't' is drawn on a different part of the logo therefore it can be
		// drawn together with other icons
		if (d->server->isTestingServer())
		{
			QPainter iconPainter(&icon);
			iconPainter.drawPixmap(0, 0, QPixmap(":/t.png"));
			iconPainter.end();
		}
	}
	fillItem(pItem, d->server->metaObject()->className(), icon);
}

void ServerListRowHandler::fillServerPointer()
{
	QStandardItem *pItem = item(COL_META);
	QVariant savePointer = QVariant::fromValue(d->server);
	pItem->setData(savePointer, ServerListModel::SLDT_POINTER_TO_SERVER_STRUCTURE);
}

QStandardItem *ServerListRowHandler::item(int columnIndex)
{
	return model->item(row, columnIndex);
}

void ServerListRowHandler::redraw()
{
	updateServer();

	// Since updateServer doesn't do anything with the flags we need to
	// explicitly redraw it here.
	setCountryFlag();
}

ServerPtr ServerListRowHandler::server()
{
	return d->server;
}

void ServerListRowHandler::setBackgroundColor()
{
	QString color;
	if (d->server->isCustom())
		color = gConfig.doomseeker.customServersColor;
	else if (d->server->isLan())
		color = gConfig.doomseeker.lanServersColor;
	else if (gConfig.doomseeker.bMarkServersWithBuddies &&
		gApp->mainWindow() != nullptr &&
		gApp->mainWindow()->buddiesList()->hasBuddy(d->server))
	{
		color = gConfig.doomseeker.buddyServersColor;
	}

	for (int column = 0; column < NUM_SERVERLIST_COLUMNS; ++column)
	{
		QBrush brush = !color.isEmpty()
			? QBrush(QColor(color))
			: Qt::NoBrush;
		QStandardItem *pItem = item(column);
		pItem->setBackground(brush);
	}
}

void ServerListRowHandler::setBad()
{
	QStandardItem *qstdItem;

	clearNonVitalFields();

	qstdItem = item(IDServerName);
	fillItem(qstdItem, tr("<ERROR>"));

	item(COL_META)->setData(SGBad, ServerListModel::SLDT_SERVER_GROUP);
}

void ServerListRowHandler::setBanned()
{
	QStandardItem *qstdItem;

	clearNonVitalFields();

	qstdItem = item(IDServerName);
	fillItem(qstdItem, tr("You are banned from this server!"));

	item(COL_META)->setData(SGBanned, ServerListModel::SLDT_SERVER_GROUP);
}

void ServerListRowHandler::setCountryFlag()
{
	item(IDServerName)->setIcon(d->flag());
}

void ServerListRowHandler::setFirstQuery()
{
	item(COL_META)->setData(SGFirstQuery, ServerListModel::SLDT_SERVER_GROUP);
}

void ServerListRowHandler::setGood()
{
	QStandardItem *qstdItem;
	QString strTmp;

	fillPlayerColumn();

	qstdItem = item(IDPing);
	fillItem(qstdItem, d->server->ping());

	qstdItem = item(IDServerName);
	fillItem(qstdItem, d->server->name());

	qstdItem = item(IDIwad);
	fillItem(qstdItem, d->server->iwad());

	qstdItem = item(IDMap);
	fillItem(qstdItem, d->server->map());

	strTmp.clear();
	for (const PWad &wad : d->server->wads())
	{
		if (wad.isOptional())
			strTmp += QString("[%1] ").arg(wad.name());
		else
			strTmp += wad.name() + " ";
	}
	strTmp.chop(1);
	qstdItem = item(IDWads);
	fillItem(qstdItem, strTmp);

	qstdItem = item(IDGametype);
	QString fullGameModeName = d->server->gameMode().name();
	QStringList modifierNames = extractValidGameCVarNames(d->server->modifiers());
	if (!modifierNames.isEmpty())
		fullGameModeName += QString(" (%1)").arg(modifierNames.join(", "));
	fillItem(qstdItem, fullGameModeName);

	item(COL_META)->setData(SGNormal, ServerListModel::SLDT_SERVER_GROUP);
}

void ServerListRowHandler::setRefreshing()
{
	QStandardItem *qstdItem = item(IDServerName);
	qstdItem->setText(tr("<REFRESHING>"));
}

void ServerListRowHandler::setTimeout()
{
	QStandardItem *qstdItem;

	clearNonVitalFields();

	qstdItem = item(IDServerName);
	fillItem(qstdItem, tr("<NO RESPONSE>"));

	item(COL_META)->setData(SGTimeout, ServerListModel::SLDT_SERVER_GROUP);
}

void ServerListRowHandler::setWait()
{
	QStandardItem *qstdItem;

	clearNonVitalFields();

	qstdItem = item(IDServerName);
	fillItem(qstdItem, tr("<Refreshed too soon, wait a while and try again>"));

	item(COL_META)->setData(SGWait, ServerListModel::SLDT_SERVER_GROUP);
}

ServerPtr ServerListRowHandler::serverFromList(const ServerListModel *parentModel, int rowIndex)
{
	QStandardItem *pItem = parentModel->item(rowIndex, COL_META);
	if (pItem == nullptr)
		return ServerPtr();
	QVariant pointer = pItem->data(ServerListModel::SLDT_POINTER_TO_SERVER_STRUCTURE);
	if (!pointer.isValid())
		return ServerPtr();
	return pointer.value<ServerPtr>();
}

int ServerListRowHandler::updateServer()
{
	fillServerPointer();
	fillPortIconColumn();
	fillAddressColumn();

	switch (d->server->lastResponse())
	{
	case Server::RESPONSE_BAD:
		setBad();
		break;

	case Server::RESPONSE_BANNED:
		setBanned();
		break;

	case Server::RESPONSE_GOOD:
		setGood();
		break;

	case Server::RESPONSE_WAIT:
		if (d->server->isKnown())
			setGood();
		else
			setWait();
		break;

	case Server::RESPONSE_TIMEOUT:
		setTimeout();
		break;

	case Server::RESPONSE_NO_RESPONSE_YET:
		setFirstQuery();
		if (d->server->isRefreshing())
			setRefreshing();
		break;

	default:
		gLog << tr("Unknown server response (%1): %2:%3").arg(d->server->lastResponse())
			.arg(d->server->address().toString()).arg(d->server->port());
		break;
	}

	setBackgroundColor();

	return row;
}
