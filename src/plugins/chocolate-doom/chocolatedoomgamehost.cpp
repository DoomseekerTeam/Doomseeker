//------------------------------------------------------------------------------
// chocolatedoomgamehost.cpp
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
// Copyright (C) 2013 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "chocolatedoomgamehost.h"

#include "chocolatedoomengineplugin.h"
#include "chocolatedoomgameinfo.h"
#include <QString>
#include <serverapi/gamecreateparams.h>

ChocolateDoomGameHost::ChocolateDoomGameHost()
	: GameHost(ChocolateDoomEnginePlugin::staticInstance())
{
	setArgForServerLaunch("-server");
}

void ChocolateDoomGameHost::addExtra()
{
	if (params().skill() != Skill::UNDEFINED)
		args() << "-skill" << QString::number(params().skill() + 1); // from 1 to 5

	switch (params().gameMode().index())
	{
	default: break;
	case GameMode::SGM_Deathmatch:
		args() << "-deathmatch";
		break;
	case ChocolateDoomGameInfo::MODE_ALTDEATH:
		args() << "-altdeath";
		break;
	}
	addWarp();
}

void ChocolateDoomGameHost::addWarp()
{
	// Convert map name to proper number for -warp
	// -warp format differs between IWADs (version 2.2.1 of Chocolate Game).
	// - -warp <episode> <mission> for episode based games (Doom, Heretic)
	// - -warp <map> for streamlined games (Doom 2)
	QString mapname = params().map().trimmed().toUpper();
	if (mapname.length() == 5 && mapname.startsWith("MAP"))
		args() << "-warp" << mapname.right(2);
	else if (mapname.length() == 4 && mapname[0] == 'E' && mapname[2] == 'M')
		args() << "-warp" << QString("%1").arg(mapname[1]) << QString("%1").arg(mapname[3]);
}
