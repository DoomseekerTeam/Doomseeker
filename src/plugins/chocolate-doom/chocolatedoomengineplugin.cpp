//------------------------------------------------------------------------------
// chocolatedoomengineplugin.cpp
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

#include "plugins/engineplugin.h"

#include "chocolatedoomengineplugin.h"
#include "chocolatedoomgameexefactory.h"
#include "chocolatedoomgamehost.h"
#include "chocolatedoomgameinfo.h"
#include "chocolatedoommasterclient.h"
#include "chocolatedoomserver.h"
#include <QSharedPointer>

INSTALL_PLUGIN(ChocolateDoomEnginePlugin)

ChocolateDoomEnginePlugin::ChocolateDoomEnginePlugin()
{
	const // clear warnings
	#include "chocolatedoom.xpm"

	init("Chocolate Doom", chocolatedoom_xpm,
		EP_Author, "The Doomseeker Team",
		EP_Version, PLUGIN_VERSION,
		EP_AboutProvider, new ChocolateDoomAboutProvider,

		EP_DefaultMaster, "master.chocolate-doom.org:2342",
		EP_DefaultServerPort, 2342,
		EP_MasterClient, new ChocolateDoomMasterClient(),
		EP_ServerExeName, "chocolate-doom",
		EP_ClientExeName, "chocolate-doom",
		EP_GameFileSearchSuffixes, "chocolate-doom;chocolate doom;chocolatedoom;chocolate_doom",
		EP_NoMapList,
		EP_NoClientSlots,
		EP_NoPlayerSlots,
		EP_CanonicalName, "chocolatedoom",
		EP_Done
	);
}

GameHost *ChocolateDoomEnginePlugin::gameHost()
{
	return new ChocolateDoomGameHost();
}

QList<GameMode> ChocolateDoomEnginePlugin::gameModes() const
{
	return ChocolateDoomGameInfo::gameModes();
}

ServerPtr ChocolateDoomEnginePlugin::mkServer(const QHostAddress &address, unsigned short port) const
{
	return ServerPtr(new ChocolateDoomServer(address, port));
}

void ChocolateDoomEnginePlugin::start()
{
	EnginePlugin::start();
	setGameExeFactory(QSharedPointer<GameExeFactory>(new ChocolateDoomGameExeFactory(this)));
}
