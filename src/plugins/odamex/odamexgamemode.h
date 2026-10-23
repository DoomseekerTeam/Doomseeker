//------------------------------------------------------------------------------
// odamexgamemode.h
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
#ifndef DOOMSEEKER_ODAMEX_GAME_MODE_H
#define DOOMSEEKER_ODAMEX_GAME_MODE_H

#include <serverapi/serverstructs.h>
#include <QObject>
#include <QMap>

class OdamexGameMode : public QObject
{
	Q_OBJECT

public:
	/**
	 * The numbers here must correspond to the numbers used by Odamex.
	 *
	 * The values also must match the order and amount of modes returned
	 * by base().
	 */
	enum BaseId
	{
		BASE_COOPERATIVE = 0,
		BASE_DEATHMATCH = 1,
		BASE_TEAM_DEATHMATCH = 2,
		BASE_CTF = 3,
		BASE_HORDE = 4,
	};

	/**
	 * The values here must match the order and amount of modes returned
	 * by derived().
	 */
	enum DerivedId
	{
		DERIVED_COOPERATIVE,
		DERIVED_DEATHMATCH,
		DERIVED_TEAM_DEATHMATCH,
		DERIVED_CAPTURE_THE_FLAG,
		DERIVED_DUEL,
		DERIVED_SURVIVAL,
		DERIVED_LAST_MARINE_STANDING,
		DERIVED_TEAM_LAST_MARINE_STANDING,
		DERIVED_ATTACK_DEFEND_CTF,
		DERIVED_LMS_CTF,
		DERIVED_HORDE,
		DERIVED_SURVIVAL_HORDE,

		DERIVED_INVALID = INT_MAX,
	};

	/// List of game modes that can be used to host the game.
	static const QList<GameMode> &base();
	/**
	 * List of game modes interpreted from the servers. They're
	 * derived from the base() modes with applied mutator settings.
	 */
	static const QList<GameMode> &derived();

	/// Interpret server CVars and the player limit as a game mode.
	static GameMode fromServerSettings(const QMap<QString, QString> &cvars,
		unsigned int maxPlayers);

private:
	OdamexGameMode() = delete;
};

#endif
