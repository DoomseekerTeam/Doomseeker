//------------------------------------------------------------------------------
// servertooltip.cpp
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
// Copyright (C) 2015 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "servertooltip.h"

#include "configuration/doomseekerconfig.h"
#include "pathfinder/pathfinder.h"
#include "pathfinder/wadpathfinder.h"
#include "serverapi/playerslist.h"
#include "serverapi/server.h"
#include "serverapi/serverstructs.h"
#include "serverapi/tooltips/tooltipgenerator.h"

namespace ServerTooltip
{
QString FONT_COLOR_MISSING = "#ff0000";
QString FONT_COLOR_WARNING = "#ff9f00";
QString FONT_COLOR_FOUND = "#009f00";
}

QString ServerTooltip::createIwadToolTip(ServerPtr server)
{
	if (!server->isKnown())
	{
		return QString();
	}

	bool bFindIwad = gConfig.doomseeker.bTellMeWhereAreTheWADsWhenIHoverCursorOverWADSColumn;
	if (bFindIwad)
	{
		static const QString FORMAT_TEMPLATE = "<font color=\"%1\">%2</font>";

		WadFindResult path = findWad(server, server->iwad());
		if (path.isValid())
		{
			QString msg = path.path();
			if (path.isAlias())
			{
				msg += " " + L10n::tr("(alias of: %1)").arg(server->iwad());
			}
			return FORMAT_TEMPLATE.arg(FONT_COLOR_FOUND, msg);
		}
		else
		{
			return FORMAT_TEMPLATE.arg(FONT_COLOR_MISSING, L10n::tr("MISSING"));
		}
	}

	return QString();
}

QString ServerTooltip::createPlayersToolTip(ServerCPtr server)
{
	if (server == nullptr || !server->isKnown())
	{
		return QString();
	}

	TooltipGenerator *tooltipGenerator = server->tooltipGenerator();

	QString ret;
	ret = "<div style='white-space: pre'>";
	ret += tooltipGenerator->gameInfoTableHTML();
	if (server->players().numClients() != 0)
	{
		ret += tooltipGenerator->playerTableHTML();
	}
	ret += "</div>";

	delete tooltipGenerator;
	return ret;
}

QString ServerTooltip::createPortToolTip(ServerCPtr server)
{
	if (server == nullptr || !server->isKnown())
		return QString();

	QString ret;
	if (server->isLocked())
		ret += "Password protected\n";
	if (server->isLockedInGame())
		ret += "Password protected in-game\n";
	if (server->isSecure())
		ret += "Enforces master bans\n";
	return ret.trimmed();
}

QString ServerTooltip::createPwadsToolTip(ServerPtr server)
{
	if (server == nullptr || !server->isKnown() || server->numWads() == 0)
	{
		return QString();
	}

	// Prepare initial formatting.
	static const QString toolTip = "<div style='white-space: pre'>%1</div>";
	QString content;

	const QList<PWad> &pwads = server->wads();

	// Check if we should seek and colorize.
	bool bFindWads = gConfig.doomseeker.bTellMeWhereAreTheWADsWhenIHoverCursorOverWADSColumn;

	// Engage!
	if (bFindWads)
	{
		QStringList pwadsFormatted;
		PathFinder pathFinder = server->wadPathFinder();
		WadPathFinder wadFinder(pathFinder);
		for (const PWad &wad : pwads)
		{
			pwadsFormatted << createPwadToolTipInfo(wad, wadFinder);
		}

		content = "<table cellspacing=1>";
		content += pwadsFormatted.join("\n");
		content += "</table>";
	}
	else
	{
		for (const PWad &wad : pwads)
		{
			content += wad.name() + "\n";
		}
		content.chop(1); // Get rid of extra \n.
	}

	return toolTip.arg(content);
}

QString ServerTooltip::createPwadToolTipInfo(const PWad &pwad, WadPathFinder &wadFinder)
{
	WadFindResult findResult = wadFinder.find(pwad.name());

	QString fontColor = "#777777";
	QStringList cells;

	cells << pwad.name();
	if (findResult.isValid())
	{
		fontColor = FONT_COLOR_FOUND;
		cells << findResult.path();
	}
	else
	{
		if (pwad.isOptional())
		{
			fontColor = FONT_COLOR_WARNING;
			cells << L10n::tr("OPTIONAL");
		}
		else
		{
			fontColor = FONT_COLOR_MISSING;
			cells << L10n::tr("MISSING");
		}
	}
	if (findResult.isAlias())
	{
		cells << L10n::tr("ALIAS");
	}
	else
	{
		cells << "";
	}

	QString formattedStringBegin = QString("<tr style=\"color: %1;\">").arg(fontColor);
	QString formattedStringMiddle;
	QString space = "";
	for (const QString &cell : cells)
	{
		formattedStringMiddle += QString("<td style=\"padding-right: 5;\">%1</td>").arg(cell);
		space = " ";
	}
	return formattedStringBegin + formattedStringMiddle + "</tr>";
}

QString ServerTooltip::createServerNameToolTip(ServerCPtr server)
{
	if (server == nullptr)
	{
		return QString();
	}

	TooltipGenerator *tooltipGenerator = server->tooltipGenerator();

	QString ret;
	QString generalInfo = tooltipGenerator->generalInfoHTML();

	if (!generalInfo.isEmpty())
	{
		ret = "<div style='white-space: pre'>";
		ret += generalInfo;
		ret += "</div>";
	}

	delete tooltipGenerator;
	return ret;
}
