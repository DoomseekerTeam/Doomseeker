//------------------------------------------------------------------------------
// playerslist.h
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
#ifndef __PLAYERS_LIST_H_
#define __PLAYERS_LIST_H_

#include "dptr.h"
#include "global.h"
#include "player.h"
#include <QList>
#include <QMap>

class PlayersList;

/**
 * Key - Team number.
 * Value - List of players.
 */
typedef QMap<int, PlayersList> PlayersByTeams;

/**
 * @ingroup group_pluginapi
 */
class MAIN_EXPORT PlayersList
{
public:
	PlayersList();
	PlayersList(const PlayersList &other);
	PlayersList &operator=(const PlayersList &other);
	virtual ~PlayersList();
	/**
	 * @brief Lists all bots regardless if they are on a team or not.
	 */
	void bots(PlayersList &botsList) const;

	/**
	 * @brief Lists only those bots that are not on a team.
	 */
	void botsWithoutTeam(PlayersList &botsList) const;
	void clear();
	int count() const
	{
		return size();
	}

	int numBots() const;
	int numBotsOnTeam(int team) const;
	int numBotsWithoutTeam() const;

	/**
	 * @brief Overall number of people and bots on the server.
	 */
	int numClients() const;
	int numClientsWithoutBots() const;

	int numHumansInGame() const;
	int numHumansOnTeam(int team) const;
	int numHumansWithoutTeam() const;
	int numPlayersOnTeam(int team) const;
	int numSpectators() const;

	PlayersList &operator<<(const Player &player);
	Player &operator[](int index);
	const Player &operator[](int index) const;
	const QList<Player> &players() const;

	/**
	 * @brief Divides players and bots to lists ordered by teams.
	 *
	 * Bots that are not assigned to teams will not be listed here. To get
	 * them use botsWithoutTeam() method.
	 *
	 * @param playersListMap [out] - New PlayersList objects will be stored
	 *     in this map.
	 */
	void inGamePlayersByTeams(PlayersByTeams &playersListMap) const;

	int size() const;
	void spectators(PlayersList &spectatorsList) const;
private:
	DPtr<PlayersList> d;
};

#endif
