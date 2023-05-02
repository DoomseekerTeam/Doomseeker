//------------------------------------------------------------------------------
// odamexgamemode.cpp
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
// Copyright (C) 2021 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "odamexgamemode.h"

const QList<GameMode> &OdamexGameMode::base()
{
	static QList<GameMode> modes;
	if (modes.isEmpty())
	{
		modes << GameMode::mkCooperative();
		modes << GameMode::mkDeathmatch();
		modes << GameMode::mkTeamDeathmatch();
		modes << GameMode::mkCaptureTheFlag();
		modes << GameMode::ffaGame(DERIVED_HORDE, tr("Horde"));
	}
	return modes;
}

const QList<GameMode> &OdamexGameMode::derived()
{
	static QList<GameMode> modes;
	if (modes.isEmpty())
	{
		modes << GameMode::mkCooperative();
		modes << GameMode::mkDeathmatch();
		modes << GameMode::mkTeamDeathmatch();
		modes << GameMode::mkCaptureTheFlag();
		modes << GameMode::ffaGame(DERIVED_DUEL, tr("Duel"));
		modes << GameMode::ffaGame(DERIVED_SURVIVAL, tr("Survival"));
		modes << GameMode::ffaGame(DERIVED_LAST_MARINE_STANDING, tr("Last Marine Standing"));
		modes << GameMode::teamGame(DERIVED_TEAM_LAST_MARINE_STANDING, tr("Teams Last Marine Standing"));
		modes << GameMode::teamGame(DERIVED_ATTACK_DEFEND_CTF, tr("Attack & Defend CTF"));
		modes << GameMode::teamGame(DERIVED_LMS_CTF, tr("LMS Capture The Flag"));
		modes << GameMode::ffaGame(DERIVED_HORDE, tr("Horde"));
		modes << GameMode::ffaGame(DERIVED_SURVIVAL_HORDE, tr("Survival Horde"));
	}
	return modes;
}
