//------------------------------------------------------------------------------
// zandronumgamehost.cpp
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
#include "zandronumengineplugin.h"
#include "zandronumgamehost.h"
#include "zandronumgameinfo.h"
#include "zandronumgamerunner.h"
#include "zandronumserver.h"
#include <serverapi/gamecreateparams.h>

#include <QFileInfo>
#include <cstdint>

ZandronumGameHost::ZandronumGameHost()
	: GameHost(ZandronumEnginePlugin::staticInstance())
{
	setArgForServerLaunch("-host");
	setArgForOptionalWadLoading("-optfile");

	set_addDMFlags(&ZandronumGameHost::addDMFlags);
}

void ZandronumGameHost::addDMFlags()
{
	const QList<DMFlagsSection> &dmFlags = params().dmFlags();
	const QString argNames[] = { "+dmflags", "+dmflags2", "+compatflags" };
	for (int i = 0; i < qMin(dmFlags.size(), 3); ++i)
	{
		int32_t flagsValue = 0;
		const DMFlagsSection &section = dmFlags[i];

		for (int j = 0; j < section.count(); ++j)
		{
			flagsValue |= section[i].value();
		}

		args() << argNames[i] << QString::number(flagsValue);
	}
}

void ZandronumGameHost::addExtra()
{
	args() << "+alwaysapplydmflags" << QString::number(1);

	if (params().skill() != Skill::UNDEFINED)
		args() << "-skill" << QString::number(params().skill() + 1); // from 1 to 5

	if (!params().loggingPath().isEmpty())
	{
		// Zandronum suffixes a date plus a ".log" extension. We'll add a prefix.
		args() << "+logfile" << QFileInfo(params().loggingPath()).absoluteFilePath() + "/Zandronum";
	}

	QString gameModeStr;
	switch (params().gameMode().index())
	{
	case GameMode::SGM_Cooperative:                     gameModeStr = "+cooperative"; break;
	case GameMode::SGM_CTF:                             gameModeStr = "+ctf"; break;
	case GameMode::SGM_Deathmatch:                      gameModeStr = "+deathmatch"; break;
	case GameMode::SGM_TeamDeathmatch:                  gameModeStr = "+teamplay"; break;
	case ZandronumGameInfo::GAMEMODE_DOMINATION:        gameModeStr = "+domination"; break;
	case ZandronumGameInfo::GAMEMODE_DUEL:              gameModeStr = "+duel"; break;
	case ZandronumGameInfo::GAMEMODE_INVASION:          gameModeStr = "+invasion"; break;
	case ZandronumGameInfo::GAMEMODE_LASTMANSTANDING:   gameModeStr = "+lastmanstanding"; break;
	case ZandronumGameInfo::GAMEMODE_ONEFLAGCTF:        gameModeStr = "+oneflagctf"; break;
	case ZandronumGameInfo::GAMEMODE_POSSESSION:        gameModeStr = "+possession"; break;
	case ZandronumGameInfo::GAMEMODE_SKULLTAG:          gameModeStr = "+skulltag"; break;
	case ZandronumGameInfo::GAMEMODE_SURVIVAL:          gameModeStr = "+survival"; break;
	case ZandronumGameInfo::GAMEMODE_TEAMGAME:          gameModeStr = "+teamgame"; break;
	case ZandronumGameInfo::GAMEMODE_TEAMLMS:           gameModeStr = "+teamlms"; break;
	case ZandronumGameInfo::GAMEMODE_TEAMPOSSESSION:    gameModeStr = "+teampossession"; break;
	case ZandronumGameInfo::GAMEMODE_TERMINATOR:        gameModeStr = "+terminator"; break;
	}
	if (!gameModeStr.isEmpty())
		args() << gameModeStr << "1";

	if (!params().map().isEmpty())
	{
		args() << "+map" << strArg(params().map());
	}

	for (QString mapRecord : params().mapList())
	{
		mapRecord = mapRecord.simplified();
		const QStringList mapArgs = mapRecord.split(" ");
		if (!mapArgs.isEmpty() && !mapArgs[0].isEmpty())
		{
			args() << "+addmap";
			for (const QString &mapArg : mapArgs)
				args() << strArg(mapArg);
		}
	}

	args() << "+sv_maprotation" << QString::number(
			static_cast<int>(!params().mapList().isEmpty()));
	args() << "+sv_randommaprotation" << QString::number(
			static_cast<int>(params().isRandomMapRotation()));

	if (params().hostMode() == GameCreateParams::Host)
		addHostModeParams();
}

void ZandronumGameHost::addHostModeParams()
{
	QString motd = params().motd();
	args() << "+sv_motd" << strArg(motd.replace("\n", "\\n"));
	args() << "+sv_hostemail" << strArg(params().email());
	args() << "+sv_hostname" << strArg(params().name());
	args() << "+sv_website" << strArg(params().url());

	{
		QString password = params().connectPassword();
		args() << "+sv_password" << strArg(password);
		args() << "+sv_forcepassword" << QString::number(static_cast<int>(!password.isEmpty()));
	}

	{
		QString password = params().ingamePassword();
		args() << "+sv_joinpassword" << strArg(password);
		args() << "+sv_forcejoinpassword" << QString::number(static_cast<int>(!password.isEmpty()));
	}

	args() << "+sv_rconpassword" << strArg(params().rconPassword());

	args() << "+sv_broadcast" << QString::number(static_cast<int>(params().isBroadcastToLan()));
	args() << "+sv_updatemaster" << QString::number(static_cast<int>(params().isBroadcastToMaster()));
	args() << "+sv_maxclients" << QString::number(params().maxTotalClientSlots());
	args() << "+sv_maxplayers" << QString::number(params().maxPlayers());

	if (params().upnp())
	{
		args() << "-upnp";
		if (params().upnpPort() > 0)
		{
			args() << QString::number(params().upnpPort());
		}
	}
}

ZandronumGameInfo::GameVersion ZandronumGameHost::gameVersion() const
{
	return static_cast<ZandronumGameInfo::GameVersion>(
		params().option(ZandronumGameInfo::OPTION_GAMEVERSION).toInt());
}

QString ZandronumGameHost::strArg(const QString &val)
{
	if (val.isEmpty())
	{
		// This will reset the value to nothing.
		if (gameVersion() == ZandronumGameInfo::GV_Zandronum2)
		{
			return "\"\"";
		}
		else
		{
			return "";
		}
	}
	else
	{
		return val;
	}
}
