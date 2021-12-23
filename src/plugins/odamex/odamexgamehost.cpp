//------------------------------------------------------------------------------
// odamexgamehost.cpp
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
#include "odamexgamehost.h"

#include "odamexengineplugin.h"
#include "odamexgameinfo.h"
#include "odamexgamemode.h"
#include <climits>
#include <QDateTime>
#include <serverapi/gamecreateparams.h>
#include <serverapi/serverstructs.h>

OdamexGameHost::OdamexGameHost()
	: GameHost(OdamexEnginePlugin::staticInstance())
{
	setArgForDemoPlayback("-netplay");
	setArgForDemoRecord("-netrecord");
}

void OdamexGameHost::addExtra()
{
	if (params().skill() != Skill::UNDEFINED)
		args() << "-skill" << QString::number(params().skill() + 1); // from 1 to 5

	if (!params().loggingPath().isEmpty())
	{
		args() << "+logfile" << params().loggingPath() + "/Odamex__" +
			QDateTime::currentDateTime().toString("yyyy_MM_dd-HH_mm_ss") + ".log";
	}

	const QStringList &mapsList = params().mapList();
	if (!mapsList.isEmpty())
	{
		for (QString map : mapsList)
		{
			args() << "+addmap" << map;
		}
	}
	args() << "+shufflemaplist" << QString::number( static_cast<int>(params().isRandomMapRotation()));

	const int gameModeIdx = [&](){
		for (int i = 0; i < OdamexGameMode::base().size(); ++i)
		{
			if (params().gameMode().index() == OdamexGameMode::base()[i].index())
				return i;
		}
		return INT_MAX;
	}();
	if (gameModeIdx != INT_MAX)
		args() << "+sv_gametype" << QString::number(gameModeIdx);

	if (!params().map().isEmpty())
		args() << "+map" << params().map();

	if (params().hostMode() == GameCreateParams::Host)
		addHostModeParams();
}

void OdamexGameHost::addHostModeParams()
{
	args() << "+set" << "sv_upnp" << (params().upnp() ? "1" : "0");
	args() << "+join_password" << "\"" + params().ingamePassword() + "\"";
	args() << "+rcon_password" << "\"" + params().rconPassword() + "\"";
	args() << "+sv_email" << "\"" + params().email() + "\"";
	args() << "+sv_hostname" << "\"" + params().name() + "\"";
	args() << "+sv_maxclients" << QString::number(params().maxTotalClientSlots());
	args() << "+sv_maxplayers" << QString::number(params().maxPlayers());
	args() << "+sv_website" << "\"" + params().url() + "\"";

	QString motd = params().motd();
	args() << "+sv_motd" << "\"" + motd.replace("\n", "\\n") + "\"";

	args() << "+sv_usemasters" << QString::number(static_cast<int>(params().isBroadcastToMaster()));
}
