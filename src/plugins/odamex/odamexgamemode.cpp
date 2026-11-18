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

#include "odamexcvar.h"

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

GameMode OdamexGameMode::fromServerSettings(const QVariantMap &cvars,
	unsigned int maxPlayers)
{
	/*
	  Define the gamemode.
	  From: https://github.com/odamex/odamex/blob/f0e0416fc42ef06ce8ff1f1c462df6a69fc65fd1/common/g_gametype.cpp#L53
	  Do not use g_gametypename.
	*/
	const DerivedId derivedId = [&]()
	{
		const uint mode = OdamexCvar::valueToUInt(cvars.value("sv_gametype"));
		const uint lives = OdamexCvar::valueToUInt(cvars.value("g_lives"));
		// Keep the numeric fallback for legacy replies: QVariant::toBool()
		// would treat nonnumeric text such as "invalid" as true.
		const QVariant sidesSetting = cvars.value("g_sides");
		const bool sides = sidesSetting.userType() == QMetaType::Bool
			? sidesSetting.toBool() : OdamexCvar::valueToUInt(sidesSetting) != 0;
		if (mode == OdamexGameMode::BASE_COOPERATIVE && lives > 0)
			return OdamexGameMode::DERIVED_SURVIVAL;
		else if (mode == OdamexGameMode::BASE_COOPERATIVE)
			return OdamexGameMode::DERIVED_COOPERATIVE;
		else if (mode == OdamexGameMode::BASE_DEATHMATCH && lives > 0)
			return OdamexGameMode::DERIVED_LAST_MARINE_STANDING;
		else if (mode == OdamexGameMode::BASE_DEATHMATCH && maxPlayers <= 2)
			return OdamexGameMode::DERIVED_DUEL;
		else if (mode == OdamexGameMode::BASE_DEATHMATCH)
			return OdamexGameMode::DERIVED_DEATHMATCH;
		else if (mode == OdamexGameMode::BASE_TEAM_DEATHMATCH && lives > 0)
			return OdamexGameMode::DERIVED_TEAM_LAST_MARINE_STANDING;
		else if (mode == OdamexGameMode::BASE_TEAM_DEATHMATCH)
			return OdamexGameMode::DERIVED_TEAM_DEATHMATCH;
		else if (mode == OdamexGameMode::BASE_CTF && sides)
			return OdamexGameMode::DERIVED_ATTACK_DEFEND_CTF;
		else if (mode == OdamexGameMode::BASE_CTF && lives > 0)
			return OdamexGameMode::DERIVED_LMS_CTF;
		else if (mode == OdamexGameMode::BASE_CTF)
			return OdamexGameMode::DERIVED_CAPTURE_THE_FLAG;
		else if (mode == OdamexGameMode::BASE_HORDE && lives > 0)
			return OdamexGameMode::DERIVED_SURVIVAL_HORDE;
		else if (mode == OdamexGameMode::BASE_HORDE)
			return OdamexGameMode::DERIVED_HORDE;
		else
			return OdamexGameMode::DERIVED_COOPERATIVE; // Arbitrary default
	}();
	return derived()[derivedId];
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
