//------------------------------------------------------------------------------
// gameinfotip.cpp
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
#include "gameinfotip.h"
#include "serverapi/playerslist.h"
#include "serverapi/server.h"
#include "serverapi/serverstructs.h"

DClass<GameInfoTip>
{
public:
	ServerCPtr server;
};

DPointered(GameInfoTip)

const QString GameInfoTip::UNLIMITED = QObject::tr("Unlimited");

GameInfoTip::GameInfoTip(const ServerCPtr &server)
{
	d->server = server;
}

GameInfoTip::~GameInfoTip()
{
}

QString GameInfoTip::generateHTML()
{
	QString table = "<table>";
	table += timelimitHTML();
	table += scorelimitHTML();
	table += teamScoresHTML();
	table += playersHTML();
	table += "</table>";

	return table;
}

QString GameInfoTip::playersHTML()
{
	const QString PLAYERS = tr("Players");

	const PlayersList &players = d->server->players();
	const int canJoin = qMax(0, d->server->numFreeJoinSlots());
	return "<tr><td>" + PLAYERS + ":&nbsp;</td><td>"
		+ tr("%1 / %2 (%3 can join)").arg(players.numClients()).arg(d->server->maxClients()).arg(canJoin)
		+ "</td></tr>";
}

QString GameInfoTip::limitHTML(QString limitName, QString valueArgsTemplate, int value)
{
	QString row = "<tr><td>" + limitName + ":&nbsp;</td><td>" + valueArgsTemplate + "</td></tr>";

	if (value == 0)
	{
		row = row.arg(UNLIMITED);
	}
	else
	{
		row = row.arg(value);
	}

	return row;
}

QString GameInfoTip::scorelimitHTML()
{
	const QString SCORELIMIT = tr("Scorelimit");
	QString row = limitHTML(SCORELIMIT, "%1", d->server->scoreLimit());

	return row;
}

QString GameInfoTip::teamScoresHTML()
{
	QString teamScores;
	if (d->server->gameMode().isTeamGame())
	{
		teamScores = "<tr><td colspan=\"2\">%1</td></tr>";
		QString teams;
		bool bPrependBar = false;
		for (int i = 0; i < MAX_TEAMS; ++i)
		{
			if (d->server->players().numPlayersOnTeam(i) != 0)
			{
				if (bPrependBar)
				{
					teams += " | ";
				}
				teams += d->server->teamName(i) + ": " + QString::number(d->server->score(i));
				bPrependBar = true;
			}
		}
		teamScores = teamScores.arg(teams);
	}

	return teamScores;
}

QString GameInfoTip::timelimitHTML()
{
	const QString TIMELIMIT = tr("Timelimit");
	int timeLimit = d->server->timeLimit();
	QString row = limitHTML(TIMELIMIT, "%1 %2", timeLimit);

	QString timeLeft = "";
	if (timeLimit != 0)
	{
		timeLeft = tr("(%1 left)").arg(d->server->timeLeft());
	}
	row = row.arg(timeLeft);

	return row;
}
