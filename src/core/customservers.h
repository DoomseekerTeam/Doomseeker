//------------------------------------------------------------------------------
// customservers.h
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
#ifndef __CUSTOMSERVERS_H_
#define __CUSTOMSERVERS_H_

#include "serverapi/masterclient.h"
#include <QtContainerFwd>

class IniSection;

/**
 * If server for unknown engine is found, the engineIndex should be set
 * to a negative value.
 */
class CustomServerInfo
{
public:
	QString engine;
	QString host;
	unsigned short port;
	int engineIndex;
	bool enabled;

	static CustomServerInfo fromServer(const Server *server);

	/**
	 * Returns true if the other server has same plugin, host and port.
	 */
	bool isSameServer(const CustomServerInfo &other) const;
};

/**
 * A 'fake' master server that is used to handle
 * custom servers from various engines.
 */
class CustomServers : public MasterClient
{
	Q_OBJECT

public:
	CustomServers() : MasterClient() {}

	/**
	 * Reads data in format `(<engine_name>;<host_name>;<port>);(...)...`
	 * and splits it to a list of CustomServerInfo objects. If a server
	 * for unknown engine is found it will be appended anyway
	 * but the CustomServerInfo::engineIndex will be set to a
	 * negative value.
	 * @param str - concatenated string in required format
	 * @return list of custom servers
	 */
	static void decodeConfigEntries(const QString &str, QList<CustomServerInfo> &outCustomServerInfoList);

	/**
	 * Returns true if server with the same plugin, host and port is known.
	 */
	bool hasSameServer(const Server *otherServer) const;

	static bool isServerPinned(const CustomServerInfo &serverInfo);
	static void setServerPinned(const CustomServerInfo &serverInfo, bool pinned);

	const EnginePlugin *plugin() const override
	{
		return nullptr;
	}

	/**
	 * Convenience method - tries to read config seeking for
	 * "CustomServers" entry, decode it and launch setServers() method.
	 */
	QList<ServerPtr> readConfig();

	/**
	 * Since this is not required here (there's no real
	 * master to refresh) this does nothing.
	 */
	void refreshStarts() override {}

	/**
	 * Sets a list of custom servers.
	 * @param csiList - list of custom servers. If element's
	 *     engineIndex is < 0 the element is discarded. Also
	 *     the element will be discarded if its hostname cannot
	 *     be resolved.
	 */
	QList<ServerPtr> setServers(const QList<CustomServerInfo> &serverDefs);

protected:
	QByteArray createServerListRequest() override
	{
		return QByteArray();
	}
	Response readMasterResponse(const QByteArray &data) override
	{
		Q_UNUSED(data);
		return RESPONSE_BAD;
	}
};

#endif
