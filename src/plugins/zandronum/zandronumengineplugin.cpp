//------------------------------------------------------------------------------
// zandronumengineplugin.cpp
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
// Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net> (skulltagengineplugin.cpp)
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------

#include <datapaths.h>
#include <ini/ini.h>
#include <pathfinder/pathfinder.h>
#include <plugins/engineplugin.h>
#include <strings.hpp>

#include "createserverdialogpages/flagspage.h"
#include "huffman/huffman.h"
#include "zandronumbinaries.h"
#include "zandronumbroadcast.h"
#include "zandronumengineplugin.h"
#include "zandronumgameexefactory.h"
#include "zandronumgamehost.h"
#include "zandronumgameinfo.h"
#include "zandronummasterclient.h"
#include "zandronumserver.h"

DClass<ZandronumEnginePlugin>
{
public:
	ZandronumBroadcast *broadcast;
};
DPointered(ZandronumEnginePlugin)

INSTALL_PLUGIN(ZandronumEnginePlugin)

ZandronumEnginePlugin::ZandronumEnginePlugin()
{
	HUFFMAN_Construct();
	d->broadcast = new ZandronumBroadcast();

	const // clear warnings
	#include "zandronum.xpm"

	init("Zandronum", zandronum_xpm,
		EP_Author, "The Doomseeker Team",
		EP_Version, PLUGIN_VERSION,
		EP_AboutProvider, new ZandronumAboutProvider,

		EP_AllowsURL,
		EP_AllowsEmail,
		EP_AllowsConnectPassword,
		EP_AllowsJoinPassword,
		EP_AllowsLogging,
		EP_AllowsRConPassword,
		EP_AllowsMOTD,
		EP_AllowsUpnp,
		EP_AllowsUpnpPort,
		#ifndef Q_OS_MAC
		EP_AllowsInternalClients,
		#endif
		#if defined(Q_OS_WIN32) || defined(Q_OS_MAC)
		EP_ClientOnly,
		#endif
		EP_ClientExeName, "zandronum",
		EP_ServerExeName, "zandronum-server",
		EP_GameFileSearchSuffixes, "zandronum",
		EP_DontCreateDMFlagsPagesAutomatic,
		EP_DefaultServerPort, 10666,
		EP_DefaultMaster, "master.zandronum.com:15300",
		EP_SupportsRandomMapRotation,
		EP_IRCChannel, "Zandronum", "irc.zandronum.com", "#zandronum",
		EP_RefreshThreshold, 10,
		EP_DemoExtension, false, "cld",
		EP_URLScheme, "zan",
		EP_Broadcast, d->broadcast,
		EP_MasterClient, new ZandronumMasterClient(),
		EP_Done
	);
}

void ZandronumEnginePlugin::setupConfig(IniSection &config)
{
	config.createSetting("Masterserver", data()->defaultMaster);
	config.createSetting("AllowServersToDisplayMyCountry", false);
	#ifndef Q_OS_DARWIN
	config.createSetting("EnableInternalClient", false);
	config.createSetting("InternalClientPath",
		DataPaths::defaultInstance()->pluginLocalDataLocationPath(*this));
	#endif
}

QList<CreateServerDialogPage *> ZandronumEnginePlugin::createServerDialogPages(CreateServerDialog *pDialog)
{
	QList<CreateServerDialogPage *> pages;

	pages << new FlagsPage(pDialog);

	return pages;
}

GameHost *ZandronumEnginePlugin::gameHost()
{
	return new ZandronumGameHost();
}

QList<GameMode> ZandronumEnginePlugin::gameModes() const
{
	return ZandronumGameInfo::gameModes();
}

QList<GameCVar> ZandronumEnginePlugin::gameModifiers() const
{
	return ZandronumGameInfo::gameModifiers();
}

QList<GameCVar> ZandronumEnginePlugin::limits(const GameMode &gm) const
{
	return ZandronumGameInfo::extraSettings(gm);
}

ServerPtr ZandronumEnginePlugin::mkServer(const QHostAddress &address, unsigned short port) const
{
	return ServerPtr(new ZandronumServer(address, port));
}

void ZandronumEnginePlugin::start()
{
	EnginePlugin::start();
	setGameExeFactory(QSharedPointer<GameExeFactory>(new ZandronumGameExeFactory(this)));
	d->broadcast->start();
}
