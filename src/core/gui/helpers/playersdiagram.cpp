// playerdiagram.cpp
//------------------------------------------------------------------------------
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
#include "playersdiagram.h"

#include "datapaths.h"
#include "log.h"
#include "serverapi/playerslist.h"
#include "serverapi/server.h"
#include "strings.hpp"
#include <cassert>
#include <QDir>
#include <QPainter>
#include <QResource>
#include <utility>

QImage PlayersDiagram::openImage;
QImage PlayersDiagram::openSpecImage;
QImage PlayersDiagram::botImage;
QImage PlayersDiagram::playerImage;
QImage PlayersDiagram::spectatorImage;

const QString PlayersDiagram::DEFAULT_STYLE = "blocks";
QString PlayersDiagram::currentlyLoadedStyle;

PlayersDiagram::PlayersDiagram(ServerCPtr server)
	: server(std::move(server))
{
	if (openImage.isNull())
		return;

	obtainPlayerNumbers();
	draw();
}

PlayersDiagram::~PlayersDiagram()
{
}

QList<PlayersDiagramStyle> PlayersDiagram::availableSlotStyles()
{
	QList<PlayersDiagramStyle> list;

	// Built-ins.
	list << PlayersDiagramStyle("numeric", tr("Numeric"));
	list << PlayersDiagramStyle("blocks", tr("Blocks"));

	// Extra.
	QStringList knownNames;
	for (QDir dir : stylePaths())
	{
		QStringList extraSlots = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
		for (QString slotsName : extraSlots)
		{
			if (!knownNames.contains(slotsName, Qt::CaseInsensitive))
			{
				knownNames << slotsName;
				list << PlayersDiagramStyle(slotsName);
			}
		}
	}

	return list;
}

QImage PlayersDiagram::colorizePlayer(QImage image, const QColor &color)
{
	QVector<QRgb> colors = image.colorTable();
	QColor destinationColor = color.toHsv();
	for (QRgb &color : colors)
	{
		// Cyan has no red so move on if this color has red.
		if (qRed(color) != 0 || qAlpha(color) == 0)
			continue;

		int hue = 0;
		int saturation = 0;
		int value = 0;
		destinationColor.getHsv(&hue, &saturation, &value);
		destinationColor.setHsv(hue, saturation, QColor(color).toHsv().value());
		color = destinationColor.rgb();
	}
	image.setColorTable(colors);

	return image;
}

void PlayersDiagram::draw()
{
	// Don't bother trying to draw an empty image.
	if (server->numTotalSlots() == 0)
		return;

	diagram = QPixmap(server->numTotalSlots() * playerImage.width(), playerImage.height());
	diagram.fill(Qt::transparent);

	slotSize = playerImage.width();
	position = diagram.width() - slotSize;
	painter = new QPainter(&diagram);

	for (int team = 0; team < MAX_TEAMS; ++team)
	{
		drawTeam(Human, team, numHumansOnTeam[team]);
		drawTeam(Bot, team, numBotsOnTeam[team]);
	}

	drawTeam(Human, Player::TEAM_NONE, numHumansWithoutTeam);
	drawTeam(Bot, Player::TEAM_NONE, numBotsWithoutTeam);

	if (numSpectators > 0)
		drawPictures(spectatorImage, numSpectators);

	if (numFreeJoinSlots > 0)
		drawPictures(openImage, numFreeJoinSlots);

	if (numFreeSpectatorSlots > 0)
		drawPictures(openSpecImage, numFreeSpectatorSlots);

	delete painter;
}

void PlayersDiagram::drawTeam(PlayerType playerType, int team, int howMany)
{
	if (howMany > 0)
	{
		QImage baseImage;

		switch (playerType)
		{
		case Bot:
			baseImage = botImage;
			break;

		case Human:
			baseImage = playerImage;
			break;

		default:
			gLog << "Error inside PlayersDiagram::drawTeam(): unknown PlayerType";
			return;
		}

		const QImage picture = colorizePlayer(baseImage, QColor(server->teamColor(team)));
		drawPictures(picture, howMany);
	}
}

void PlayersDiagram::drawPictures(const QImage &image, int howMany)
{
	for (; howMany > 0; --howMany)
	{
		painter->drawImage(position, 0, image);
		position -= slotSize;
	}
}

bool PlayersDiagram::isNumericStyle(const QString &style)
{
	return style == "numeric";
}

void PlayersDiagram::loadImages(const QString &style)
{
	if (style == currentlyLoadedStyle)
		return;
	if (isNumericStyle(style))
		return;

	openImage = loadImage(style, "open");
	openSpecImage = loadImage(style, "specopen");
	botImage = loadImage(style, "bot");
	playerImage = loadImage(style, "player");
	spectatorImage = loadImage(style, "spectator");
	currentlyLoadedStyle = style;
}

QImage PlayersDiagram::loadImage(const QString &style, const QString &name)
{
	QImage image;
	if (style != DEFAULT_STYLE)
	{
		QString resourcePath;
		for (const QString &dir : stylePaths())
		{
			image = QImage(Strings::combinePaths(dir, style + "/" + name + ".png"));
			if (!image.isNull())
				return image;
		}
	}
	if (image.isNull())
		image = QImage(":/slots/" + DEFAULT_STYLE + "/" + name);
	return image;
}

void PlayersDiagram::obtainPlayerNumbers()
{
	memset(numBotsOnTeam, 0, sizeof(int) * MAX_TEAMS);
	memset(numHumansOnTeam, 0, sizeof(int) * MAX_TEAMS);

	const PlayersList &players = server->players();

	numBotsWithoutTeam = players.numBotsWithoutTeam();
	numFreeJoinSlots = server->numFreeJoinSlots();
	numFreeSpectatorSlots = server->numFreeSpectatorSlots();
	numHumansWithoutTeam = players.numHumansWithoutTeam();
	numSpectators = players.numSpectators();

	for (int i = 0; i < MAX_TEAMS; ++i)
	{
		numBotsOnTeam[i] = players.numBotsOnTeam(i);
		numHumansOnTeam[i] = players.numHumansOnTeam(i);
	}
}

QStringList PlayersDiagram::stylePaths()
{
	return gDefaultDataPaths->staticDataSearchDirs("theme/slots");
}
