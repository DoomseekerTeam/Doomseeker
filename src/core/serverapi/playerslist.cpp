//------------------------------------------------------------------------------
// playerslist.cpp
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
#include "playerslist.h"

DClass<PlayersList>
{
public:
	QList<Player> players;
};

DPointered(PlayersList)

PlayersList::PlayersList()
{
	// Nothing to store yet. Future backward compatibility with plugins.
}

PlayersList::PlayersList(const PlayersList &other)
{
	d = other.d;
}

PlayersList &PlayersList::operator=(const PlayersList &other)
{
	d = other.d;
	return *this;
}

PlayersList::~PlayersList()
{
}

void PlayersList::bots(PlayersList &botsList) const
{
	botsList.clear();

	for (Player player : players())
	{
		if (player.isBot())
		{
			botsList << player;
		}
	}
}

void PlayersList::botsWithoutTeam(PlayersList &botsList) const
{
	botsList.clear();

	for (Player player : players())
	{
		if (player.isBot() && player.teamNum() == Player::TEAM_NONE)
		{
			botsList << player;
		}
	}
}

void PlayersList::clear()
{
	d->players.clear();
}

int PlayersList::numBots() const
{
	int bots = 0;
	for (Player player : players())
	{
		if (player.isBot())
		{
			++bots;
		}
	}

	return bots;
}

int PlayersList::numBotsOnTeam(int team) const
{
	int bots = 0;
	for (Player player : players())
	{
		if (player.isBot() && player.teamNum() == team)
		{
			++bots;
		}
	}

	return bots;
}

int PlayersList::numBotsWithoutTeam() const
{
	int bots = 0;
	for (Player player : players())
	{
		if (player.isBot() && player.teamNum() == Player::TEAM_NONE)
		{
			++bots;
		}
	}

	return bots;
}

int PlayersList::numClients() const
{
	return size();
}

int PlayersList::numClientsWithoutBots() const
{
	return size() - numBots();
}

int PlayersList::numHumansInGame() const
{
	int humansInGame = 0;

	for (Player player : players())
	{
		if (!player.isBot() && !player.isSpectating())
		{
			++humansInGame;
		}
	}

	return humansInGame;
}

int PlayersList::numHumansOnTeam(int team) const
{
	int humans = 0;
	for (Player player : players())
	{
		if (!player.isBot()
			&& !player.isSpectating()
			&& player.teamNum() == team)
		{
			++humans;
		}
	}

	return humans;
}

int PlayersList::numHumansWithoutTeam() const
{
	int humans = 0;
	for (Player player : players())
	{
		if (!player.isBot()
			&& !player.isSpectating()
			&& player.teamNum() == Player::TEAM_NONE)
		{
			++humans;
		}
	}

	return humans;
}

int PlayersList::numPlayersOnTeam(int team) const
{
	int teamSize = 0;
	for (Player player : players())
	{
		if (player.teamNum() == team)
		{
			++teamSize;
		}
	}

	return teamSize;
}

int PlayersList::numSpectators() const
{
	int spectators = 0;

	for (Player player : players())
	{
		if (player.isSpectating())
		{
			++spectators;
		}
	}

	return spectators;
}

PlayersList &PlayersList::operator<<(const Player &player)
{
	d->players << player;
	return *this;
}

Player &PlayersList::operator[](int index)
{
	return d->players[index];
}

const Player &PlayersList::operator[](int index) const
{
	return d->players[index];
}

const QList<Player> &PlayersList::players() const
{
	return d->players;
}

void PlayersList::inGamePlayersByTeams(PlayersByTeams &playersListMap) const
{
	playersListMap.clear();

	for (const Player &player : players())
	{
		if (!player.isSpectating() && !player.isTeamlessBot())
		{
			int teamIndex = player.teamNum();
			if (playersListMap.contains(teamIndex))
			{
				playersListMap[teamIndex] << player;
			}
			else
			{
				PlayersList newList;
				newList << player;
				playersListMap.insert(teamIndex, newList);
			}
		}
	}
}

int PlayersList::size() const
{
	return d->players.size();
}

void PlayersList::spectators(PlayersList &spectatorsList) const
{
	spectatorsList.clear();

	for (Player player : players())
	{
		if (player.isSpectating())
		{
			spectatorsList << player;
		}
	}
}
