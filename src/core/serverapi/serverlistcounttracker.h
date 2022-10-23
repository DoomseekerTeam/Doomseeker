//------------------------------------------------------------------------------
// serverlistcounttracker.h
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
#ifndef idd4447d5c_3443_4b8c_9902_82b5f6d51a47
#define idd4447d5c_3443_4b8c_9902_82b5f6d51a47

#include "dptr.h"
#include "serverapi/serverptr.h"

class EnginePlugin;

struct ServerListCount
{
	int numBots;
	int numHumanPlayers;
	int numPlayers;

	int numCustomServers;
	int numGenericServers;
	int numLanServers;
	int numServers;
	int numRefreshing;

	ServerListCount();

	void countPlayers(const ServerPtr &server);
	void discountPlayers(const ServerPtr &server);

	void countServer(const ServerPtr &server);
	void discountServer(const ServerPtr &server);

	int refreshedPercent() const;

	ServerListCount &operator+=(const ServerListCount &other);
};

class ServerListCountTracker : public QObject
{
	Q_OBJECT

public:
	ServerListCountTracker(QObject *parent = nullptr);

	const ServerListCount &count() const;
	void setPluginFilter(const EnginePlugin *plugin);

public slots:
	void deregisterServer(ServerPtr server);
	void registerServer(ServerPtr server);

signals:
	void updated();

private:
	DPtr<ServerListCountTracker> d;

private slots:
	void onServerBegunRefreshing(ServerPtr server);
	void onServerUpdated(ServerPtr server);
};

#endif
