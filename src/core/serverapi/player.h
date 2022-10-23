//------------------------------------------------------------------------------
// player.h
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
// Copyright (C) 2010 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------
#ifndef __PLAYER_H_
#define __PLAYER_H_

#define MAX_TEAMS 4

#include "dptr.h"
#include "global.h"
#include <QHash>
#include <QString>

/**
 * @ingroup group_pluginapi
 * @brief Data structure that holds information about players in a server.
 */
class MAIN_EXPORT Player
{
public:
	enum PlayerTeam
	{
		TEAM_BLUE,
		TEAM_RED,
		TEAM_GREEN,
		TEAM_GOLD,

		TEAM_NONE = 0xFF
	};

	Player();
	Player(const QString &name, long score, unsigned long ping,
		PlayerTeam team = TEAM_NONE, bool spectator = false, bool bot = false);
	Player(const Player &other);
	Player &operator=(const Player &other);
	virtual ~Player();

	const QString &name() const;
	long score() const;
	unsigned long ping() const;
	bool isSpectating() const;
	bool isBot() const;
	bool isTeamlessBot() const;
	PlayerTeam teamNum() const;

	bool operator==(const Player &other) const;

	/**
	 * Formats string into HTML format.
	 */
	QString nameFormatted() const;

	/**
	 * Seeks for characters that are not from the <32; 126> range,
	 * removes them and the characters that appear after them,
	 * then returns new string.
	 */
	QString nameColorTagsStripped() const;

private:
	DPtr<Player> d;
};

uint qHash(const Player &player);

#endif
