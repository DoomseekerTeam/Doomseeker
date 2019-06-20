//------------------------------------------------------------------------------
// odamexengineplugin.cpp
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
// Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------
#include "odamexengineplugin.h"
#include <QDateTime>

#include "irc/entities/ircnetworkentity.h"
#include "plugins/engineplugin.h"

#include "odamexgameexefactory.h"
#include "odamexgamehost.h"
#include "odamexgameinfo.h"
#include "odamexmasterclient.h"
#include "odamexserver.h"

#include <ini/inisection.h>
#include <ini/inivariable.h>
#include <datapaths.h>

INSTALL_PLUGIN(OdamexEnginePlugin)

OdamexEnginePlugin::OdamexEnginePlugin()
{
	const // clear warnings
	#include "odamex.xpm"

	init("Odamex", odamex_xpm,
		EP_Author, "The Doomseeker Team",
		EP_Version, PLUGIN_VERSION,
		EP_AboutProvider, new OdamexAboutProvider,

		EP_AllowsURL,
		EP_AllowsEmail,
		EP_AllowsJoinPassword,
		EP_AllowsRConPassword,
		EP_AllowsMOTD,
		EP_DefaultMaster, "master1.odamex.net:15000",
		EP_DefaultServerPort, 10666,
		EP_MasterClient, new OdamexMasterClient(),
		EP_IRCChannel, "Odamex", "irc.quakenet.org", "#odamex",
		EP_SupportsRandomMapRotation,
		EP_InGameFileDownloads,
		EP_DemoExtension, true, "odd",
		EP_ClientExeName, "odamex",
		EP_ServerExeName, "odasrv",
		EP_GameFileSearchSuffixes, "odamex",
		EP_AllowsUpnp,
		EP_AllowsLogging,
		#ifdef Q_OS_WIN32
		EP_AllowsInternalClients,
		#endif
		EP_Done
	);
}

void OdamexEnginePlugin::setupConfig(IniSection &config)
{
	#ifdef Q_OS_WIN32
	config.createSetting("EnableInternalClient", false);
	config.createSetting("InternalClientPath",
		DataPaths::defaultInstance()->pluginLocalDataLocationPath(*this));
	#endif
}

QList<DMFlagsSection> OdamexEnginePlugin::dmFlags() const
{
	return OdamexGameInfo::dmFlags();
}

QList<GameMode> OdamexEnginePlugin::gameModes() const
{
	return OdamexGameInfo::gameModes();
}

GameHost *OdamexEnginePlugin::gameHost()
{
	return new OdamexGameHost();
}

ServerPtr OdamexEnginePlugin::mkServer(const QHostAddress &address, unsigned short port) const
{
	return ServerPtr(new OdamexServer(address, port));
}

void OdamexEnginePlugin::start()
{
	EnginePlugin::start();
	setGameExeFactory(QSharedPointer<GameExeFactory>(new OdamexGameExeFactory(this)));
}
