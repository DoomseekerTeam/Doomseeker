//------------------------------------------------------------------------------
// Turok2Exengineplugin.cpp
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
// Copyright (C) 2017 "Edward Richardson" <Edward850@crantime.org>
//------------------------------------------------------------------------------

#include <QDateTime>

#include "irc/entities/ircnetworkentity.h"
#include "plugins/engineplugin.h"

#include "turok2exengineplugin.h"
#include "turok2exgamehost.h"
#include "turok2exgameinfo.h"
#include "turok2exmasterclient.h"
#include "turok2exserver.h"

INSTALL_PLUGIN(Turok2ExEnginePlugin)

Turok2ExEnginePlugin::Turok2ExEnginePlugin()
{
	const // clear warnings
	#include "turok2ex.xpm"

	init("Turok 2 Remaster", turok2ex_xpm,
		EP_Author, "Night Dive Studios & The Doomseeker Team",
		EP_Version, PLUGIN_VERSION,
		EP_AboutProvider, new Turok2AboutProvider,

		EP_DefaultMaster, "t2ms.nightdivestudios.com:20700",
		EP_DefaultServerPort, 5029,
		EP_MasterClient, new Turok2ExMasterClient(),
		EP_SupportsRandomMapRotation,
		EP_ClientExeName, "horus_x64",
		EP_ServerExeName, "horus_x64_ded",
		EP_GameFileSearchSuffixes, "turok 2",
		EP_NoIwad,
		EP_NoClientSlots,
		EP_DifficultyProvider, new Turok2ExDifficultyProvider(),
		EP_CanonicalName, "turok2ex",
		EP_Done
	);
}

QList<GameMode> Turok2ExEnginePlugin::gameModes() const
{
	return Turok2ExGameInfo::gameModes();
}

QList<GameCVar> Turok2ExEnginePlugin::limits(const GameMode &mode) const
{
	return Turok2ExGameInfo::limits(mode);
}

GameHost *Turok2ExEnginePlugin::gameHost()
{
	return new Turok2ExGameHost();
}

ServerPtr Turok2ExEnginePlugin::mkServer(const QHostAddress &address, unsigned short port) const
{
	return ServerPtr(new Turok2ExServer(address, port));
}
