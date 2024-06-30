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

#include <QDateTime>

#include "irc/entities/ircnetworkentity.h"
#include "plugins/engineplugin.h"

#include "odamexengineplugin.h"
#include "odamexgamehost.h"
#include "odamexgameinfo.h"
#include "odamexgamemode.h"
#include "odamexmasterclient.h"
#include "odamexserver.h"

namespace
{
/**
 * @brief Provides a (translated) About text for Odamex.
 */
class OdamexAboutProvider : public TextProvider
{
	Q_OBJECT

public:
	QString provide() override
	{
		QString aboutDescription = "Odamex (https://odamex.net/)\n\n";
		aboutDescription += tr("This plugin is distributed under the terms of the LGPL v2.1 or later.\n\n");
		return aboutDescription;
	}
};
}


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
		EP_MultiplayerDemoExtension, true, "odd",
		EP_ClientExeName, "odamex",
		EP_ServerExeName, "odasrv",
		EP_GameFileSearchSuffixes, "odamex",
		EP_AllowsUpnp,
		EP_AllowsLogging,
		EP_Done
	);
}

QList<DMFlagsSection> OdamexEnginePlugin::dmFlags() const
{
	return OdamexGameInfo::dmFlags();
}

QList<GameMode> OdamexEnginePlugin::gameModes() const
{
	return OdamexGameMode::base();
}

GameHost *OdamexEnginePlugin::gameHost()
{
	return new OdamexGameHost();
}

QList<GameCVar> OdamexEnginePlugin::limits(const GameMode &mode) const
{
	return OdamexGameInfo::extraSettings();
}

ServerPtr OdamexEnginePlugin::mkServer(const QHostAddress &address, unsigned short port) const
{
	return ServerPtr(new OdamexServer(address, port));
}

#include "odamexengineplugin.moc"
