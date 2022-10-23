//------------------------------------------------------------------------------
// customservers.cpp
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
#include "configuration/doomseekerconfig.h"
#include "customservers.h"
#include "log.h"
#include "plugins/engineplugin.h"
#include "plugins/pluginloader.h"
#include "serverapi/server.h"
#include <cassert>
#include <QHostInfo>
#include <QUrl>

CustomServerInfo CustomServerInfo::fromServer(const Server *server)
{
	assert(server != nullptr);
	CustomServerInfo obj;
	obj.engine = server->plugin()->data()->name;
	obj.engineIndex = gPlugins->pluginIndexFromName(obj.engine);
	obj.host = server->address().toString();
	obj.port = server->port();
	obj.enabled = true;
	return obj;
}

bool CustomServerInfo::isSameServer(const CustomServerInfo &other) const
{
	return engine == other.engine
		&& port == other.port
		&& host == other.host;
}

////////////////////////////////////////

void CustomServers::decodeConfigEntries(const QString &str, QList<CustomServerInfo> &outCustomServerInfoList)
{
	outCustomServerInfoList.clear();

	int openingBracketIndex = 0;
	int closingBracketIndex = 0;
	bool bSeekClosingBracket = false;
	for (int charIdx = 0; charIdx < str.length(); ++charIdx)
	{
		if (!bSeekClosingBracket && str[charIdx] == '(')
		{
			openingBracketIndex = charIdx;
			bSeekClosingBracket = true;
		}
		else if (bSeekClosingBracket && str[charIdx] == ')')
		{
			closingBracketIndex = charIdx;
			bSeekClosingBracket = false;

			QString entry = str.mid(openingBracketIndex + 1, closingBracketIndex - (openingBracketIndex + 1));
			QStringList entryList = entry.split(";");

			if (entryList.size() >= 3)
			{
				CustomServerInfo customServerInfo;
				customServerInfo.engine = QUrl::fromPercentEncoding(entryList[0].toUtf8());

				int engineIndex = gPlugins->pluginIndexFromName(customServerInfo.engine);
				customServerInfo.engineIndex = engineIndex;

				customServerInfo.host = QUrl::fromPercentEncoding(entryList[1].toUtf8());
				customServerInfo.enabled = true;
				if (entryList.size() >= 4)
					customServerInfo.enabled = entryList[3].toInt() != 0;

				bool ok = false;
				int port = QString(entryList[2]).toInt(&ok);
				if (ok && port >= 1 && port <= 65535)
					customServerInfo.port = port;
				else if (engineIndex >= 0)
				{
					const PluginLoader::Plugin *pPlugin = gPlugins->plugin(engineIndex);
					customServerInfo.port = pPlugin->info()->data()->defaultServerPort;
				}
				else
					customServerInfo.port = 1;

				outCustomServerInfoList << customServerInfo;
			} // end of if
		} // end of else if
	} // end of for
}

bool CustomServers::isServerPinned(const CustomServerInfo &serverInfo)
{
	for (const CustomServerInfo &knownPinned : gConfig.doomseeker.customServers)
	{
		if (knownPinned.isSameServer(serverInfo))
			return knownPinned.enabled;
	}
	return false;
}

bool CustomServers::hasSameServer(const Server *otherServer) const
{
	CustomServerInfo otherServerInfo = CustomServerInfo::fromServer(otherServer);
	for (ServerCPtr knownServer : servers())
	{
		if (CustomServerInfo::fromServer(knownServer.data()).isSameServer(otherServerInfo))
			return true;
	}
	return false;
}

QList<ServerPtr> CustomServers::readConfig()
{
	return setServers(gConfig.doomseeker.customServers.toList());
}

void CustomServers::setServerPinned(const CustomServerInfo &serverInfo, bool pinned)
{
	// Check if the server has already been memorized.
	for (int serverIdx = 0; serverIdx < gConfig.doomseeker.customServers.size(); ++serverIdx)
	{
		CustomServerInfo &knownPinned = gConfig.doomseeker.customServers[serverIdx];
		if (knownPinned.isSameServer(serverInfo))
		{
			knownPinned.enabled = pinned;
			return;
		}
	}
	// Server is not yet on the memorized servers list.
	// If we're about to pin it, just add it to the list.
	if (pinned)
		gConfig.doomseeker.customServers << serverInfo;
}

QList<ServerPtr> CustomServers::setServers(const QList<CustomServerInfo> &serverDefs)
{
	emptyServerList();

	QList<ServerPtr> servers;
	for (const CustomServerInfo &customServerInfo : serverDefs)
	{
		if (!customServerInfo.enabled)
			continue;
		if (customServerInfo.engineIndex < 0)
		{
			// Unknown engine.
			gLog << tr("Unknown game for custom server %1:%2")
				.arg(customServerInfo.host).arg(customServerInfo.port);
			continue;
		}

		QHostAddress address;
		if (!address.setAddress(customServerInfo.host))
		{
			QHostInfo hostInfo(QHostInfo::fromName(customServerInfo.host));
			if (hostInfo.addresses().size() == 0)
			{
				// Can't decipher address.
				gLog << tr("Failed to resolve address for custom server %1:%2")
					.arg(customServerInfo.host).arg(customServerInfo.port);
				continue;
			}
			address = hostInfo.addresses().first();
		}

		const EnginePlugin *pInterface = gPlugins->plugin(customServerInfo.engineIndex)->info();
		ServerPtr p = pInterface->server(address, customServerInfo.port);
		if (p == nullptr)
		{
			gLog << tr("Plugin returned nullptr \"Server*\" for custom server %1:%2. "
					"This is a problem with the plugin.")
				.arg(customServerInfo.host).arg(customServerInfo.port);
			continue;
		}
		p->setCustom(true);

		registerNewServer(p);
		servers << p;
	}
	return servers;
}
