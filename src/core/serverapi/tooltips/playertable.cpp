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

#include "capsqt.h"
#include "serverapi/server.h"
#include "serverapi/serverstructs.h"
#include <climits>

#define PLAYERTABLE_COLSPAN "4"

// Games can come up with their own team names; don't trust them too much.
static const int MAX_TEAMNAME_SIZE = 100;

DClass<PlayerTable>
{
public:
	ServerCPtr server;

	bool isTeamGame() const
	{
		return server->gameMode().isTeamGame();
	}
};

DPointered(PlayerTable)

PlayerTable::PlayerTable(const ServerCPtr &server)
{
	d->server = server;
}

PlayerTable::~PlayerTable()
{
}

QString PlayerTable::generateHTML()
{
	static const QString css = "<style>"
		".player-table {"
		"    background-color: #FFFFFF;"
		"    color: #000000;"
		"}\n"
		".header-row th {"
		"    border-bottom: 1px solid black;"
		"    padding-bottom: 2px;"
		"}\n"
		".section-header-row th {"
		"    padding-left: 20px;"
		"    padding-top: 4px;"
		"    text-align: left;"
		"}\n"
		"th {"
		"    text-align: left;"
		"    white-space: nowrap;"
		"}\n"
		"th, td {"
		"    border-right: 1px dashed gray;"
		"    padding: 0px 4px;"
		"}\n"
		".number-cell {"
		"    text-align: right;"
		"}\n"
		"</style>";

	const bool isTeamGame = d->isTeamGame();
	const PlayersList &players = d->server->players();

	PlayersByTeams playersByTeams;
	PlayersList bots, spectators;

	players.inGamePlayersByTeams(playersByTeams);
	players.botsWithoutTeam(bots);
	players.spectators(spectators);

	QString table = R"(<table class="player-table" cellspacing="0" width="100%">)";
	table += tableHeader();
	bool separator = false;
	for (int i : playersByTeams.keys())
	{
		const PlayersList &playersList = playersByTeams[i];
		if (isTeamGame)
		{
			table += teamHeader(d->server->teamName(i));
		}
		table += createPlayerRows(playersList);
	}
	if (bots.count() > 0)
	{
		table += sectionHeader(tr("Bots"));
		table += createPlayerRows(bots);
	}
	if (spectators.count() > 0)
	{
		table += sectionHeader(tr("Spectators"));
		table += createPlayerRows(spectators);
	}
	table += "</table>";
	return css + table;
}

QString PlayerTable::createPlayerRows(const PlayersList &playerList) const
{
	QString rows;
	for (const Player &player : playerList.players())
	{
		rows += createPlayerRow(player);
	}
	return rows;
}

QString PlayerTable::createPlayerRow(const Player &player) const
{
	QString status = "";
	if (player.isBot())
	{
		status = tr("BOT");
	}
	else if (player.isSpectating())
	{
		status = tr("SPECTATOR");
	}

	QString ping;
	if (player.ping() != USHRT_MAX)
	{
		ping = QString::number(player.ping());
	}

	return QString(
		"<tr>"
		R"(<td>%1</td>)"
		R"(<td class="number-cell" width="60">%2</td>)"
		R"(<td class="number-cell" width="60">%3</td>)"
		R"(<td width="100">%4</td>)"
		"</tr>")
			.arg(player.nameFormatted())
			.arg(player.score())
			.arg(ping)
			.arg(status);
}

QString PlayerTable::tableHeader() const
{
	static const bool canCssBorders = CapsQt::canCssCellBorders();

	const QString PLAYER = tr("Player");
	const QString SCORE = tr("Score");
	const QString PING = tr("Ping");
	const QString STATUS = tr("Status");

	QString header = QString(
		R"(<tr class="header-row">)"
		R"(<th>%1</th>)"
		R"(<th class="number-cell" width="60">&nbsp;%2</th>)"
		R"(<th class="number-cell" width="60">&nbsp;%3</th>)"
		R"(<th width="100">%4</th>)"
		"</tr>")
			.arg(PLAYER)
			.arg(SCORE)
			.arg(PING)
			.arg(STATUS);
	if (!canCssBorders)
	{
		// If borders can't be drawn, add a horizontal line separator
		// to imitate them.
		header += R"(<tr><th colspan="4"><hr width="100%"></th></tr>)";
	}
	return header;
}

QString PlayerTable::teamHeader(const QString &teamName)
{
	// Don't trust the team name and sanitize it because
	// it can be procured by the game.
	return sectionHeader(tr("Team %1").arg(
		teamName.toHtmlEscaped().left(MAX_TEAMNAME_SIZE)));
}

QString PlayerTable::sectionHeader(const QString &title)
{
	return QString(
		R"(<tr class="section-header-row">)"
		"<th>%1</th>"
		"<th>&nbsp;</th>"
		"<th>&nbsp;</th>"
		"<th>&nbsp;</th>"
		"</tr>")
			.arg(title);
}
