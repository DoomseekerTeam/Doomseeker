//------------------------------------------------------------------------------
// odamexgameinfo.cpp
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
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "odamexgameinfo.h"

QList<DMFlagsSection> OdamexGameInfo::dmFlags()
{
	DMFlagsSection section("DMFlags");
	section << DMFlag("Items respawn", 1 << 0, tr("Items respawn"));
	section << DMFlag("Weapons stay", 1 << 1, tr("Weapons stay"));
	section << DMFlag("Friendly fire", 1 << 2, tr("Friendly fire"));
	section << DMFlag("Allow exit", 1 << 3, tr("Allow exit"));
	section << DMFlag("Infinite ammo", 1 << 4, tr("Infinite ammo"));
	section << DMFlag("No monsters", 1 << 5, tr("No monsters"));
	section << DMFlag("Monsters respawn", 1 << 6, tr("Monsters respawn"));
	section << DMFlag("Fast monsters", 1 << 7, tr("Fast monsters"));
	section << DMFlag("Jumping allowed", 1 << 8, tr("Jumping allowed"));
	section << DMFlag("Freelook allowed", 1 << 9, tr("Freelook allowed"));
	section << DMFlag("Wad can be downloaded", 1 << 10, tr("Wad can be downloaded"));
	section << DMFlag("Server resets on empty", 1 << 11, tr("Server resets on empty"));
	section << DMFlag("Clean Maps", 1 << 12, tr("Clean Maps"));
	section << DMFlag("Kill anyone who tries to leave the level", 1 << 13,
		tr("Kill anyone who tries to leave the level"));

	QList<DMFlagsSection> result;
	result << section;
	return result;
}

QList<GameCVar> OdamexGameInfo::extraSettings()
{
	QList<GameCVar> settings;
	settings << GameCVar(tr("Lives"), "+g_lives", 0);
	settings << GameCVar(tr("Teams"), "+sv_teamsinplay", 2);
	settings << GameCVar(tr("Attack && Defend"), "+g_sides", false);
	return settings;
}
