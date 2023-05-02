//------------------------------------------------------------------------------
// player.cpp
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
#include "player.h"
#include "strings.hpp"

DClass<Player>
{
public:
	QString name;
	long score;
	unsigned short ping;
	bool spectator;
	bool bot;
	Player::PlayerTeam team;
};

DPointered(Player)

Player::Player()
{
	d->score = 0;
	d->ping = 0;
	d->spectator = false;
	d->bot = false;
	d->team = TEAM_NONE;
}

Player::Player(const QString &name, long score, unsigned long ping,
	PlayerTeam team, bool spectator, bool bot)
{
	d->name = name;
	d->score = score;
	d->ping = ping;
	d->spectator = spectator;
	d->bot = bot;
	d->team = team;
}

Player::Player(const Player &other)
{
	d = other.d;
}

Player &Player::operator=(const Player &other)
{
	d = other.d;
	return *this;
}

Player::~Player()
{
}

bool Player::isBot() const
{
	return d->bot;
}

bool Player::isSpectating() const
{
	return d->spectator;
}

bool Player::isTeamlessBot() const
{
	return d->bot && d->team == TEAM_NONE;
}

const QString &Player::name() const
{
	return d->name;
}

QString Player::nameColorTagsStripped() const
{
	QString ret;
	for (int i = 0; i < d->name.length(); ++i)
	{
		if (d->name[i] < 32 || d->name[i] > 126)
		{
			// Strip on \c.
			// Removing the control characters is still a good idea though.
			if (d->name[i] == ESCAPE_COLOR_CHAR)
			{
				// Find out what exactly needs to be removed. Either
				// it will be just one character, a bracket-enclosed
				// range or nothing if sequence is invalid.
				int colorCodeIdx = i + 1;
				bool range = false;
				for (; colorCodeIdx < d->name.length(); ++colorCodeIdx)
				{
					QChar symbol = d->name[colorCodeIdx];
					if (symbol == '[')
						range = true;
					else if ((range && symbol == ']') || !range)
						break;
				}
				if (range && colorCodeIdx >= d->name.length())
					++i; // We didn't find range end.
				else
					i = colorCodeIdx;
			}
			continue;
		}

		ret += d->name[i];
	}
	return ret;
}

QString Player::nameFormatted() const
{
	QString ret;
	for (const auto &c : d->name)
	{
		// cut out bad characters
		if ((c < 32 || c > 126) && c != ESCAPE_COLOR_CHAR)
			continue;

		switch (c.toLatin1())
		{
		case '<':
			ret += "&lt;";
			break;

		case '>':
			ret += "&gt;";
			break;

		default:
			ret += c;
			break;
		}
	}

	return Strings::colorizeString(ret);
}

unsigned long Player::ping() const
{
	return d->ping;
}

long Player::score() const
{
	return d->score;
}

Player::PlayerTeam Player::teamNum() const
{
	return d->team;
}

bool Player::operator==(const Player &other) const
{
	return name().compare(other.name(), Qt::CaseInsensitive) == 0;
}

//////////////////////////////////////////////////////////////////////////////

uint qHash(const Player &player)
{
	return qHash(player.name());
}
