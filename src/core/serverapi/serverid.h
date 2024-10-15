//------------------------------------------------------------------------------
// serverid.h
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
// Copyright (C) 2024 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_CORE_SERVERAPI_SERVERID_H
#define DOOMSEEKER_CORE_SERVERAPI_SERVERID_H

#include <QHostAddress>
#include "plugins/engineplugin.h"
#include "serverapi/server.h"

class EnginePlugin;

/**
 * A runtime identity of a game server.
 *
 * This identity denotes the minimum set of attributes where if two different
 * Server instances have all of them equal, then they effectively refer to the
 * same, actual game server.
 *
 * In simpler words: if the program has discovered the same server twice, for
 * example: via the game's master server and via a LAN broadcast, it may end up
 * with two Server objects that effectively point to the same server. This
 * identity can be used to compare the Server objects, and if it proves to be
 * equal, the Server objects can be merged (or one of them discarded).
 *
 * The "runtime" part here refers to this being valid within the current
 * runtime only. For performance reasons it compares the server's game
 * by its plugin pointer.
 */
class ServerRuntimeId
{
public:
	static ServerRuntimeId fromServer(const ServerPtr server) { return fromServer(server.data()); }
	static ServerRuntimeId fromServer(const Server *server)
	{
		return ServerRuntimeId(
			server->plugin(),
			server->address(),
			server->port());
	}

	ServerRuntimeId(const EnginePlugin *plugin, const QHostAddress &address, unsigned short port)
		: m_plugin(plugin), m_address(address), m_port(port) {}

	bool operator==(const ServerRuntimeId &other) const
	{
		return m_plugin == other.m_plugin
			&& m_address == other.m_address
			&& m_port == other.m_port;
	}

	bool operator!=(const ServerRuntimeId &other) const
	{
		return !(*this == other);
	}

	const QHostAddress &address() const { return m_address; }
	const EnginePlugin *plugin() const { return m_plugin; }
	unsigned short port() const { return m_port; }

private:
	const EnginePlugin *m_plugin;
	QHostAddress m_address;
	unsigned short m_port;
};

#endif
