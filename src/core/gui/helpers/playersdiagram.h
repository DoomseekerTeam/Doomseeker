//------------------------------------------------------------------------------
// playerdiagram.h
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
#ifndef __PLAYERS_DIAGRAM_H_
#define __PLAYERS_DIAGRAM_H_

#include "serverapi/player.h"
#include "serverapi/serverptr.h"
#include <QPixmap>
#include <utility>

class Server;
class QImage;

struct PlayersDiagramStyle
{
public:
	QString name;
	QString displayName;

	PlayersDiagramStyle() {}
	PlayersDiagramStyle(QString name)
		: name(name)
	{
		displayName = name.toLower();
		displayName[0] = displayName[0].toUpper();
	}
	PlayersDiagramStyle(QString name, QString displayName)
		: name(std::move(name)), displayName(std::move(displayName)) {}
};

class PlayersDiagram : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(PlayersDiagram)

public:
	static QList<PlayersDiagramStyle> availableSlotStyles();
	/**
	 * Loads all the images used to build a diagram. Previous images
	 * will be freed. This is be used to change the diagram appearance
	 * when Configuration box indicates that such action is required.
	 */
	static void loadImages(const QString &style);
	static bool isNumericStyle(const QString &style);

	PlayersDiagram(ServerCPtr server);

	~PlayersDiagram() override;

	QPixmap pixmap() const
	{
		return diagram;
	}

protected:
	enum PlayerType
	{
		Bot,
		Human
	};

	static QImage openImage, openSpecImage, botImage, playerImage, spectatorImage;

	/**
	 * Colorizes the image to color.  This works is a fairly hacky way.  It
	 * colorizes cyan areas only.  To detect cyan it simply checks if a red
	 * component is present.  If so it is not cyan.
	 *
	 * Colorization is done by keeping the hue and saturation if the passed
	 * in color and applying the value of the color in the image.
	 */
	QImage colorizePlayer(QImage image, const QColor &color);

	void draw();
	void drawTeam(PlayerType playerType, int team, int howMany);
	void drawPictures(const QImage &image, int howMany);

	void obtainPlayerNumbers();

	int numBotsOnTeam[MAX_TEAMS];
	int numBotsWithoutTeam;
	int numFreeJoinSlots;
	int numFreeSpectatorSlots;
	int numHumansWithoutTeam;
	int numHumansOnTeam[MAX_TEAMS];
	int numSpectators;

private:
	static const QString DEFAULT_STYLE;
	static QString currentlyLoadedStyle;

	ServerCPtr server;
	QPixmap diagram;
	QPainter *painter;
	int position;
	int slotSize;

	static QImage loadImage(const QString &style, const QString &name);
	static QStringList stylePaths();
};

#endif
