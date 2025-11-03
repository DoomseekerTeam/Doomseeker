//------------------------------------------------------------------------------
// zandronumgameinfo.cpp
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "zandronumgameinfo.h"

#include <QDebug>
#include <QVector>

namespace
{
GameMode findByZandronumEnglishName(const QString &name)
{
	static QVector<QString> englishNames;
	if (englishNames.isEmpty())
	{
		englishNames
			<< "Cooperative"
			<< "Survival"
			<< "Invasion"
			<< "Deathmatch"
			<< "Team Deathmatch"
			<< "Duel"
			<< "Terminator"
			<< "Last Man Standing"
			<< "Team Last Man Standing"
			<< "Possession"
			<< "Team Possession"
			<< "Team Game"
			<< "Capture the Flag"
			<< "1-Flag CTF"
			<< "Skulltag"
			<< "Domination"
		;
	}

	for (int i = 0; i < englishNames.size(); ++i)
	{
		if (englishNames[i].compare(name, Qt::CaseInsensitive) == 0)
		{
			const QList<GameMode> gameModes = ZandronumGameInfo::gameModes();
			if (i >= 0 && i < gameModes.size())
			{
				return gameModes[i];
			}
			else
			{
				// We have a bug: lists are in desync.
				qDebug() << "Matched game mode name" << name
					<< "to Zandronum-English hardcoded name" << englishNames[i]
					<< "at index" << i
					<< "but there are only" << gameModes.size()
					<< "available.";
				break;
			}
		}
	}
	return GameMode();
}
}

const QString ZandronumGameInfo::OPTION_GAMEVERSION = "GameVersion";

QList<GameCVar> ZandronumGameInfo::extraSettings(const GameMode &gameMode)
{
	QList<GameCVar> settings;

	const int modeIdx = gameMode.index();

	if (modeIdx != GameMode::SGM_Cooperative
		&& modeIdx != GAMEMODE_INVASION
		&& modeIdx != GAMEMODE_SURVIVAL)
	{
		settings << GameCVar(tr("Time limit"), "+timelimit", 0);
	}

	if (modeIdx == GameMode::SGM_Deathmatch
		|| modeIdx == GAMEMODE_DUEL
		|| modeIdx == GameMode::SGM_TeamDeathmatch
		|| modeIdx == GAMEMODE_TERMINATOR)
	{
		settings << GameCVar(tr("Frag limit"), "+fraglimit", 0);
	}

	if (modeIdx == GameMode::SGM_CTF
		|| modeIdx == GAMEMODE_DOMINATION
		|| modeIdx == GAMEMODE_ONEFLAGCTF
		|| modeIdx == GAMEMODE_POSSESSION
		|| modeIdx == GAMEMODE_SKULLTAG
		|| modeIdx == GAMEMODE_TEAMGAME
		|| modeIdx == GAMEMODE_TEAMPOSSESSION)
	{
		settings << GameCVar(tr("Point limit"), "+pointlimit", 0);
	}

	if (modeIdx == GAMEMODE_DUEL
		|| modeIdx == GAMEMODE_LASTMANSTANDING
		|| modeIdx == GAMEMODE_TEAMLMS)
	{
		settings << GameCVar(tr("Win limit"), "+winlimit", 0);
	}

	if (modeIdx == GAMEMODE_DUEL)
	{
		settings << GameCVar(tr("Duel limit"), "+duellimit", 0);
	}

	settings << GameCVar(tr("Max. lives"), "+sv_maxlives", 0);

	return settings;
}

QList<GameMode> ZandronumGameInfo::gameModes()
{
	QList<GameMode> gameModesList;
	gameModesList << GameMode::mkCooperative();
	gameModesList << GameMode::ffaGame(GAMEMODE_SURVIVAL, tr("Survival"));
	gameModesList << GameMode::ffaGame(GAMEMODE_INVASION, tr("Invasion"));
	gameModesList << GameMode::mkDeathmatch();
	gameModesList << GameMode::mkTeamDeathmatch();
	gameModesList << GameMode::ffaGame(GAMEMODE_DUEL, tr("Duel"));
	gameModesList << GameMode::ffaGame(GAMEMODE_TERMINATOR, tr("Terminator"));
	gameModesList << GameMode::ffaGame(GAMEMODE_LASTMANSTANDING, tr("LMS"));
	gameModesList << GameMode::teamGame(GAMEMODE_TEAMLMS, tr("Team LMS"));
	gameModesList << GameMode::ffaGame(GAMEMODE_POSSESSION, tr("Possession"));
	gameModesList << GameMode::teamGame(GAMEMODE_TEAMPOSSESSION, tr("Team Poss"));
	gameModesList << GameMode::teamGame(GAMEMODE_TEAMGAME, tr("Team Game"));
	gameModesList << GameMode::mkCaptureTheFlag();
	gameModesList << GameMode::teamGame(GAMEMODE_ONEFLAGCTF, tr("One Flag CTF"));
	gameModesList << GameMode::teamGame(GAMEMODE_SKULLTAG, tr("Skulltag"));
	gameModesList << GameMode::teamGame(GAMEMODE_DOMINATION, tr("Domination"));
	return gameModesList;
}

QList<GameCVar> ZandronumGameInfo::gameModifiers()
{
	QList<GameCVar> gameModifiersList;
	gameModifiersList << GameCVar("Buckshot", "+buckshot");
	gameModifiersList << GameCVar("Instagib", "+instagib");
	return gameModifiersList;
}

QString ZandronumGameInfo::tryDoomseekerizeGameModeName(
	const GameMode &gameMode, const QString &name)
{
	if (name.isEmpty())
		return gameMode.name();

	GameMode matchedMode = findByZandronumEnglishName(name);
	if (matchedMode.isValid())
		return matchedMode.name();

	return name;
}

QString ZandronumAboutProvider::provide()
{
	QString aboutDescription = "Zandronum (https://zandronum.com/)\n\n";
	aboutDescription += tr("This plugin is distributed under the terms of the LGPL v2.1 or later.\n\n");
	return aboutDescription;
}
