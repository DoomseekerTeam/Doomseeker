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

#include "application.h"
#include "capsqt.h"
#include "gui/dockBuddiesList.h"
#include "gui/mainwindow.h"
#include "serverapi/server.h"
#include "serverapi/serverstructs.h"
#include "serverapi/tooltips/tooltiprenderhint.h"

#include <QFont>
#include <QFontMetrics>
#include <QRect>

#include <climits>

// Games can come up with their own team names; don't trust them too much.
static const int MAX_TEAMNAME_SIZE = 100;

DClass<PlayerTable>
{
public:
	TooltipRenderHint renderHint;
	ServerCPtr server;

	bool isTeamGame() const
	{
		return server->gameMode().isTeamGame();
	}

	int maxShownPlayers() const
	{
		if (this->renderHint.boundingRect().isNull())
			return -1;

		QFontMetrics fontMetrics(this->renderHint.font());

		// Permit to take only about 80% of the viewport.
		int viewportHeight = this->renderHint.boundingRect().height();
		viewportHeight = qMax(300, viewportHeight * 80 / 100);

		return viewportHeight / fontMetrics.height();
	}
};

DPointered(PlayerTable)

PlayerTable::PlayerTable(const TooltipRenderHint &renderHint, const ServerCPtr &server)
{
	d->renderHint = renderHint;
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
		"    border-top: 1px solid black;"
		"    border-bottom: 1px solid black;"
		"    padding: 2px 4px;"
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

	const int maxShownPlayers = d->maxShownPlayers();
	const int numSections = playersByTeams.size()
		+ qMin(spectators.size(), 1);
	int totalPlayers = bots.count() + spectators.count();
	for (const PlayersList &players : playersByTeams.values())
	{
		totalPlayers += players.size();
	}
	const int maxShownPlayersBySection = (maxShownPlayers > 0 && totalPlayers > maxShownPlayers)
		? (maxShownPlayers / numSections)
		: -1;

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
		table += createPlayerRows(playersList, maxShownPlayersBySection);
	}
	if (bots.count() > 0)
	{
		table += sectionHeader(tr("Bots"));
		table += createPlayerRows(bots, maxShownPlayersBySection);
	}
	if (spectators.count() > 0)
	{
		table += sectionHeader(tr("Spectators"));
		table += createPlayerRows(spectators, maxShownPlayersBySection);
	}
	table += "</table>";
	return css + table;
}

QString PlayerTable::createPlayerRows(const PlayersList &playerList, int maxShown) const
{
	DockBuddiesList *buddiesList = (gApp != nullptr && gApp->mainWindow() != nullptr)
		? gApp->mainWindow()->buddiesList()
		: nullptr;

	QList<Player> sorted;
	const QList<Player> *players = &playerList.players();
	if (maxShown > 0 && playerList.size() > maxShown)
	{
		// If we can't display everyone, sort them so that buddies are first
		// and bots are last.
		sorted = playerList.players();
		// std::sort() crashes here!
		qSort(sorted.begin(), sorted.end(),
			[buddiesList](const Player &p1, const Player &p2)
			{
				if (buddiesList != nullptr)
				{
					if (buddiesList->isBuddy(p1) && !buddiesList->isBuddy(p2))
						return false;
				}
				if (!p1.isBot() && p2.isBot())
					return false;
				return true;
			});
		players = &sorted;
	}

	QString rows;
	int totalShown = 0;
	for (const Player &player : *players)
	{
		rows += createPlayerRow(player);
		if (maxShown > 0 && ++totalShown >= maxShown)
			break;
	}
	if (maxShown > 0)
	{
		int remaining = static_cast<int>(playerList.size()) - maxShown;
		if (remaining > 0)
		{
			rows += createMoreRow(remaining);
		}
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

QString PlayerTable::createMoreRow(int count) const
{
	QString text = tr("(and %n more ...)", nullptr, count);
	return QString(
		"<tr>"
		"<td>%1</td>"
		R"(<td width="60">&nbsp;</td>)"
		R"(<td width="60">&nbsp;</td>)"
		R"(<td width="100">&nbsp;</td>)"
		"</tr>")
			.arg(text);
}

QString PlayerTable::tableHeader() const
{
	static const bool canCssBorders = CapsQt::canCssCellBorders();

	const QString PLAYER = tr("Player");
	const QString SCORE = tr("Score");
	const QString PING = tr("Ping");
	const QString STATUS = tr("Status");

	QString header;
	if (!canCssBorders)
	{
		// If borders can't be drawn, add a horizontal line separator
		// to imitate them.
		header += R"(<tr><th colspan="4"><hr width="100%"></th></tr>)";
	}
	header += QString(
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
