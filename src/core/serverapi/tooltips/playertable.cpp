//------------------------------------------------------------------------------
// playertable.cpp
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
#include "playertable.h"
#include "serverapi/server.h"
#include "serverapi/serverstructs.h"
#include <climits>

DClass<PlayerTable>
{
public:
	int numOfColumns;
	ServerCPtr server;
};

DPointered(PlayerTable)

PlayerTable::PlayerTable(const ServerCPtr &server)
{
	d->numOfColumns = 0;
	d->server = server;
	setNumberOfColumns();
}

PlayerTable::~PlayerTable()
{
}

QString PlayerTable::generateHTML()
{
	QString tableStyle = "background-color: #FFFFFF; color: #000000";
	QString table = R"(<table cellspacing="4" style=")" + tableStyle + R"(">)";
	table += tableHeader();
	table += tableContent();
	table += "</table>";
	return table;
}

void PlayerTable::setNumberOfColumns()
{
	if (d->server->gameMode().isTeamGame())
	{
		d->numOfColumns = 5;
	}
	else
	{
		d->numOfColumns = 4;
	}
}

QString PlayerTable::spawnPartOfPlayerTable(const PlayersList &list, bool bAppendEmptyRowAtBeginning)
{
	QString ret;
	if (list.count() != 0)
	{
		if (bAppendEmptyRowAtBeginning)
		{
			ret = QString("<tr><td colspan=\"%1\">&nbsp;</td></tr>").arg(d->numOfColumns);
		}

		for (int i = 0; i < list.count(); ++i)
		{
			const Player &player = list[i];

			QString status = "";
			if (player.isBot())
			{
				status = tr("BOT");
			}
			else if (player.isSpectating())
			{
				status = tr("SPECTATOR");
			}

			QString strPlayer = "<tr>";
			if (d->server->gameMode().isTeamGame())
			{
				strPlayer += QString("<td>%1</td>").arg(d->server->teamName(player.teamNum()));
			}
			strPlayer += R"(<td>%1</td><td align="right">%2</td><td align="right">%3</td><td>%4</td></tr>)";
			QString ping;
			if (player.ping() != USHRT_MAX)
			{
				ping =  QString::number(player.ping());
			}
			strPlayer = strPlayer.arg(player.nameFormatted()).arg(player.score()).arg(ping);
			strPlayer = strPlayer.arg(status);

			ret += strPlayer;
		}
	}

	return ret;
}

QString PlayerTable::spawnPlayersRows(const PlayersByTeams &playersByTeams)
{
	QString playersRows;

	bool bAppendEmptyRowAtBeginning = false;
	for (const PlayersList &playersList : playersByTeams.values())
	{
		playersRows += spawnPartOfPlayerTable(playersList, bAppendEmptyRowAtBeginning);
		if (!bAppendEmptyRowAtBeginning)
		{
			bAppendEmptyRowAtBeginning = true;
		}
	}

	return playersRows;
}

QString PlayerTable::tableContent()
{
	PlayersByTeams playersByTeams;
	PlayersList bots, spectators;

	const PlayersList &players = d->server->players();

	players.inGamePlayersByTeams(playersByTeams);
	players.botsWithoutTeam(bots);
	players.spectators(spectators);

	bool bAppendEmptyRowAtBeginning = false;
	QString playersRows = spawnPlayersRows(playersByTeams);

	bAppendEmptyRowAtBeginning = !playersRows.isEmpty();
	QString botsRows = spawnPartOfPlayerTable(bots, bAppendEmptyRowAtBeginning);

	bAppendEmptyRowAtBeginning = !(botsRows.isEmpty() && playersRows.isEmpty());
	QString spectatorsRows = spawnPartOfPlayerTable(spectators, bAppendEmptyRowAtBeginning);

	QString content = playersRows + botsRows + spectatorsRows;

	return content;
}

QString PlayerTable::tableHeader()
{
	const QString TEAM = tr("Team");
	const QString PLAYER = tr("Player");
	const QString SCORE = tr("Score");
	const QString PING = tr("Ping");
	const QString STATUS = tr("Status");

	QString teamHeader;
	QString header = "<tr>";

	if (d->server->gameMode().isTeamGame())
	{
		teamHeader = "<td>" + TEAM + "</td>";
	}

	header += teamHeader;
	header += "<td>" + PLAYER + "</td>";
	header += "<td align=\"right\">&nbsp;" + SCORE + "</td>";
	header += "<td align=\"right\">&nbsp;" + PING + "</td>";
	header += "<td>" + STATUS + "</td>";
	header += "</tr>";
	header += QString(R"(<tr><td colspan="%1"><hr width="100%"></td></tr>)").arg(d->numOfColumns);

	return header;
}
