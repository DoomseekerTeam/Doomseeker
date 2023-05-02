//------------------------------------------------------------------------------
// broadcastmanager.cpp
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
// Copyright (C) 2015 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "broadcastmanager.h"

#include "log.h"
#include "plugins/engineplugin.h"
#include "refresher/refresher.h"
#include "serverapi/broadcast.h"
#include "serverapi/server.h"
#include <QSet>

DClass<BroadcastManager>
{
public:
	QSet<ServerPtr> servers;
};
DPointered(BroadcastManager)

BroadcastManager::BroadcastManager(QObject *parent)
	: QObject(parent)
{
}

BroadcastManager::~BroadcastManager()
{
}

void BroadcastManager::forgetServer(ServerPtr server)
{
	gLog << tr("%1 LAN server gone: %2, %3:%4").arg(server->plugin()->data()->name)
		.arg(server->name()).arg(server->address().toString())
		.arg(server->port());
	d->servers.remove(server);
	emit serverLost(server);
}

void BroadcastManager::registerServer(ServerPtr server, bool needsRefresh)
{
	if (!d->servers.contains(server))
	{
		server->setLan(true);
		d->servers.insert(server);
		gLog << tr("New %1 LAN server detected: %2:%3").arg(server->plugin()->data()->name)
			.arg(server->address().toString()).arg(server->port());
		emit newServerDetected(server, server->lastResponse());
	}
	if (needsRefresh)
	{
		gRefresher->registerServer(server.data());
	}
}

void BroadcastManager::registerPlugin(const EnginePlugin *plugin)
{
	this->connect(plugin->data()->broadcast,
		SIGNAL(serverLost(ServerPtr)),
		SLOT(forgetServer(ServerPtr)));
	this->connect(plugin->data()->broadcast,
		SIGNAL(serverDetected(ServerPtr,bool)),
		SLOT(registerServer(ServerPtr,bool)));
}

QList<ServerPtr> BroadcastManager::servers() const
{
	return d->servers.values();
}
