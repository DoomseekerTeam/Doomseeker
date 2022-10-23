//------------------------------------------------------------------------------
// srb2gameinfo.cpp
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
// Copyright (C) 2016 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "srb2gameinfo.h"

QString Srb2GameInfo::commandFromFlag(Flag flag)
{
	switch (flag)
	{
	case AllPlayersForExit:
		return "+playersforexit";
	case AllowExitLevel:
		return "+allowexitlevel";
	case AllowTeamChange:
		return "+allowteamchange";
	case FriendlyFire:
		return "+friendlyfire";
	case IngameWadDownloads:
		return "+downloading";
	case JoinNextRound:
		return "+joinnextround";
	case MouseLook:
		return "+allowmlook";
	case PowerStones:
		return "+powerstones";
	case RespawnItems:
		return "+respawnitem";
	case RestrictSkinChange:
		return "+restrictskinchange";
	case RingSlinger:
		return "+ringslinger";
	case TailsPickup:
		return "+tailspickup";
	case TouchTag:
		return "+touchtag";
	default:
		return "";
	}
}

QList<DMFlagsSection> Srb2GameInfo::dmFlags()
{
	DMFlagsSection section("flags", tr("Flags"));
	section << DMFlag("All players must reach exit", AllPlayersForExit,
		tr("All players must reach exit"));
	section << DMFlag("Allow attacking in all game modes", RingSlinger,
		tr("Allow attacking in all game modes"));
	section << DMFlag("Allow mouse-look", MouseLook, tr("Allow mouse-look"));
	section << DMFlag("Allow level exit in all game modes", AllowExitLevel,
		tr("Allow level exit in all game modes"));
	section << DMFlag("Allow team change", AllowTeamChange, tr("Allow team change"));
	section << DMFlag("Tag players by simply touching them", TouchTag,
		tr("Tag players by simply touching them"));
	section << DMFlag("Respawn items", RespawnItems, tr("Respawn items"));
	section << DMFlag("Listen server", ListenServer, tr("Listen server"));
	section << DMFlag("Casual server", CasualServer, tr("Casual server"));
	section << DMFlag("Allow in-game WAD downloads", IngameWadDownloads,
		tr("Allow in-game WAD downloads"));
	section << DMFlag("Players are non-solid and can be picked up", TailsPickup,
		tr("Players are non-solid and can be picked up"));
	section << DMFlag("Friendly fire", FriendlyFire, tr("Friendly fire"));
	section << DMFlag("Restrict skin change", RestrictSkinChange,
		tr("Restrict skin change (game mode specific)"));
	section << DMFlag("Join game only on next round", JoinNextRound,
		tr("Join game only on next round"));
	section << DMFlag("Spawn chaos emeralds in DM and TDM and CTF", PowerStones,
		tr("Spawn chaos emeralds (powerstones) in DM, TDM and CTF"));

	QList<DMFlagsSection> flags;
	flags << section;
	return flags;
}

QList<GameMode> Srb2GameInfo::gameModes()
{
	QList<GameMode> modes;
	modes << GameMode::mkCooperative();
	modes << GameMode::ffaGame(Competition, tr("Competition"));
	modes << GameMode::ffaGame(Race, tr("Race"));
	modes << GameMode::mkDeathmatch();
	modes << GameMode::mkTeamDeathmatch();
	modes << GameMode::ffaGame(Tag, tr("Tag"));
	modes << GameMode::ffaGame(HideAndSeek, tr("Hide & Seek"));
	modes << GameMode::mkCaptureTheFlag();
	return modes;
}

QList<GameCVar> Srb2GameInfo::limits(const GameMode &gameMode)
{
	QList<GameCVar> limits;
	switch (gameMode.index())
	{
	case GameMode::SGM_Cooperative:
	case Competition:
		limits << GameCVar(tr("Starting lives"), "+startinglives", 3);
		break;
	case Race:
		limits << GameCVar(tr("Laps"), "+numlaps", 4);
		limits << GameCVar(tr("Countdown time"), "+countdowntime", 60);
		break;
	case Tag:
	case HideAndSeek:
		limits << GameCVar(tr("Hide time"), "+hidetime", 30); // Intentional fall-through.
	case GameMode::SGM_Deathmatch:
	case GameMode::SGM_TeamDeathmatch:
	case GameMode::SGM_CTF:
		if (gameMode.index() == GameMode::SGM_CTF)
			limits << GameCVar(tr("Team difference autobalance"), "+autobalance", 0);
		limits << GameCVar(tr("Point limit"), "+pointlimit", 0);
		limits << GameCVar(tr("Time limit"), "+timelimit", 0);
		break;
	}
	if (gameMode.index() == GameMode::SGM_CTF)
		limits << GameCVar(tr("Flag auto-return time"), "+flagtime", 30);
	limits << GameCVar(tr("Force server character (skin)"), "+forceskin", -1);
	limits << GameCVar(tr("Respawn item time"), "+respawnitemtime", 30);
	limits << GameCVar(tr("Intermission time"), "+inttime", 20);
	limits << GameCVar(tr("Max ping"), "+maxping", 0);
	limits << GameCVar(tr("Max upload size in kB"), "+maxsend", 1024);
	limits << GameCVar(tr("Tics until client timeout"), "+nettimeout", 525);
	limits << GameCVar(tr("Resynch attempts before kicking clients"), "+resynchattempts", 10);
	return limits;
}

QString Srb2AboutProvider::provide()
{
	QString aboutDescription = "Sonic Robo Blast 2 (https://www.srb2.org/)\n\n";
	aboutDescription += tr("This plugin is distributed under the terms of the LGPL v2.1 or later.\n\n");
	return aboutDescription;
}
