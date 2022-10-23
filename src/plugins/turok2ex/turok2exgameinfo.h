//------------------------------------------------------------------------------
// Turok2Exgameinfo.h
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
// Copyright (C) 2017 "Edward Richardson" <Edward850@crantime.org>
//------------------------------------------------------------------------------
#ifndef __TUROK2EX_GAME_INFO_H_
#define __TUROK2EX_GAME_INFO_H_

#include "serverapi/serverstructs.h"
#include "serverapi/textprovider.h"
#include <QObject>

class Turok2ExGameInfo : public QObject
{
	Q_OBJECT

public:
	enum Turok2ExGameModes
	{
		MODE_SINGLEPLAYER, // start of list
		MODE_ROKMATCH,
		MODE_TEAMROKMATCH,
		MODE_FRAGTAG,
		MODE_LASTTUROKSTANDING,
		MODE_ROKMIX,
		MODE_INSTAROK,
		MODE_RAPTORFEST,
		MODE_NUMMODES // end of list
	};

	static QList<GameMode> gameModes();
	static QList<GameCVar> limits(const GameMode &gameMode);
};

class Turok2ExDifficultyProvider : GameCVarProvider
{
public:
	QList<GameCVar> get(const QVariant &context) override
	{
		Q_UNUSED(context);
		return QList<GameCVar>();
	}
};

/**
 * @brief Provides a (translated) About text for Turok2.
 */
class Turok2AboutProvider : public TextProvider
{
	Q_OBJECT

public:
	QString provide() override;
};

#endif
