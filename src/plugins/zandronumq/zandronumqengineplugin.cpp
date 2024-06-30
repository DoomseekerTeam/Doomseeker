//------------------------------------------------------------------------------
// zandronumqengineplugin.cpp
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
#include "enginezandronumqconfigbox.h"
#include "huffman/huffman.h"
#include "zandronumqbroadcast.h"
#include "zandronumqengineplugin.h"
#include "zandronumqgameexefactory.h"
#include "zandronumqgamehost.h"
#include "zandronumqgameinfo.h"
#include "zandronumqmasterclient.h"
#include "zandronumqserver.h"

DClass<ZandronumqEnginePlugin>
{
public:
	ZandronumqBroadcast *broadcast;
};
DPointered(ZandronumqEnginePlugin)

INSTALL_PLUGIN(ZandronumqEnginePlugin)

ZandronumqEnginePlugin::ZandronumqEnginePlugin()
{
	HUFFMAN_Construct();
	d->broadcast = new ZandronumqBroadcast();

	const // clear warnings
	#include "zandronumq.xpm"

	init("Q-Zandronum", zandronumq_xpm,
		EP_Author, "geNia",
		EP_Version, PLUGIN_VERSION,
		EP_CanonicalName, "zandronumq",
		EP_AboutProvider, new ZandronumqAboutProvider,

		EP_AllowsURL,
		EP_AllowsEmail,
		EP_AllowsConnectPassword,
		EP_AllowsJoinPassword,
		EP_AllowsLogging,
		EP_AllowsRConPassword,
		EP_AllowsMOTD,
		EP_AllowsUpnp,
		EP_AllowsUpnpPort,
		#if defined(Q_OS_WIN32) || defined(Q_OS_DARWIN)
		EP_ClientOnly,
		#endif
		EP_ClientExeName, "q-zandronum",
		EP_ServerExeName, "q-zandronum-server",
		EP_GameFileSearchSuffixes, "q-zandronum",
		EP_DontCreateDMFlagsPagesAutomatic,
		EP_DefaultServerPort, 10666,
		EP_DefaultMaster, "master.qzandronum.com:15300",
		EP_SupportsRandomMapRotation,
		EP_RefreshThreshold, 10,
		EP_DemoExtension, false, "lmp",
		EP_MultiplayerDemoExtension, false, "cld",
		EP_URLScheme, "zan",
		EP_Broadcast, d->broadcast,
		EP_MasterClient, new ZandronumqMasterClient(),
		EP_Done
	);
}

void ZandronumqEnginePlugin::setupConfig(IniSection &config)
{
	config.createSetting("Masterserver", data()->defaultMaster);
}

ConfigPage *ZandronumqEnginePlugin::configuration(QWidget *parent)
{
	return new EngineZandronumqConfigBox(staticInstance(), *data()->pConfig, parent);
}

QList<CreateServerDialogPage *> ZandronumqEnginePlugin::createServerDialogPages(CreateServerDialog *pDialog)
{
	QList<CreateServerDialogPage *> pages;

	pages << new FlagsPage(pDialog);

	return pages;
}

GameHost *ZandronumqEnginePlugin::gameHost()
{
	return new ZandronumqGameHost();
}

QList<GameMode> ZandronumqEnginePlugin::gameModes() const
{
	return ZandronumqGameInfo::gameModes();
}

QList<GameCVar> ZandronumqEnginePlugin::gameModifiers() const
{
	return ZandronumqGameInfo::gameModifiers();
}

QList<GameCVar> ZandronumqEnginePlugin::limits(const GameMode &gm) const
{
	return ZandronumqGameInfo::extraSettings(gm);
	QList<GameCVar> gl;

	int m = gm.index();

	if (m != GameMode::SGM_Cooperative
		&& m != ZandronumqGameInfo::GAMEMODE_INVASION
		&& m != ZandronumqGameInfo::GAMEMODE_SURVIVAL)
	{
		gl << GameCVar(QObject::tr("Time limit:"), "+timelimit");
	}

	if (m == GameMode::SGM_Deathmatch
		|| m == ZandronumqGameInfo::GAMEMODE_DUEL
		|| m == GameMode::SGM_TeamDeathmatch
		|| m == ZandronumqGameInfo::GAMEMODE_TERMINATOR)
	{
		gl << GameCVar(QObject::tr("Frag limit:"), "+fraglimit");
	}

	if (m == GameMode::SGM_CTF
		|| m == ZandronumqGameInfo::GAMEMODE_DOMINATION
		|| m == ZandronumqGameInfo::GAMEMODE_ONEFLAGCTF
		|| m == ZandronumqGameInfo::GAMEMODE_POSSESSION
		|| m == ZandronumqGameInfo::GAMEMODE_SKULLTAG
		|| m == ZandronumqGameInfo::GAMEMODE_TEAMGAME
		|| m == ZandronumqGameInfo::GAMEMODE_TEAMPOSSESSION)
	{
		gl << GameCVar(QObject::tr("Point limit:"), "+pointlimit");
	}

	if (m == ZandronumqGameInfo::GAMEMODE_DUEL
		|| m == ZandronumqGameInfo::GAMEMODE_LASTMANSTANDING
		|| m == ZandronumqGameInfo::GAMEMODE_TEAMLMS)
	{
		gl << GameCVar(QObject::tr("Win limit:"), "+winlimit");
	}

	if (m == ZandronumqGameInfo::GAMEMODE_DUEL)
	{
		gl << GameCVar(QObject::tr("Duel limit:"), "+duellimit");
	}

	gl << GameCVar(QObject::tr("Max. lives:"), "+sv_maxlives");

	return gl;
}

ServerPtr ZandronumqEnginePlugin::mkServer(const QHostAddress &address, unsigned short port) const
{
	return ServerPtr(new ZandronumqServer(address, port));
}

void ZandronumqEnginePlugin::start()
{
	EnginePlugin::start();
	setGameExeFactory(QSharedPointer<GameExeFactory>(new ZandronumqGameExeFactory(this)));
	d->broadcast->start();
}
