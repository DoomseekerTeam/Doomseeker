//------------------------------------------------------------------------------
// Turok2Exgamehost.cpp
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
#include "turok2exgamehost.h"

#include "turok2exengineplugin.h"
#include <serverapi/gamecreateparams.h>
#include <serverapi/serverstructs.h>

Turok2ExGameHost::Turok2ExGameHost()
	: GameHost(Turok2ExEnginePlugin::staticInstance())
{
	set_addIwad(&Turok2ExGameHost::addIwad);
	set_addPwads(&Turok2ExGameHost::addPwads_prefixOnce);
}

void Turok2ExGameHost::addExtra()
{
	const QStringList &mapsList = params().mapList();
	if (!mapsList.isEmpty())
		args() << "+sv_maprotation" << mapsList.join(";");
	args() << "+shufflemaplist" << QString::number( static_cast<int>(params().isRandomMapRotation()));

	unsigned int modeNum = params().gameMode().index();
	if (modeNum != GameMode::SGM_Unknown)
		args() << "+sv_gamemode" << QString::number(modeNum);

	if (!params().map().isEmpty())
		args() << "+sv_map" << params().map();

	if (params().hostMode() == GameCreateParams::Host)
		addHostModeParams();
}

void Turok2ExGameHost::addHostModeParams()
{
	args() << "+sv_email" << params().email();
	args() << "+sv_directservername" << params().name();
	args() << "+sv_maxplayers" << QString::number(params().maxPlayers());
	args() << "+sv_website" << params().url();

	QString motd = params().motd();
	args() << "+sv_motd" << motd.replace("\n", "\\n");

	args() << "+sv_broadcast" << QString::number(static_cast<int>(params().isBroadcastToMaster()));
}

void Turok2ExGameHost::addIwad()
{
	//nada
}
