//------------------------------------------------------------------------------
// odamexgameinfo.h
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
#ifndef __ODAMEX_GAME_INFO_H_
#define __ODAMEX_GAME_INFO_H_

#include "serverapi/serverstructs.h"
#include "serverapi/textprovider.h"
#include <QObject>

class OdamexGameInfo : public QObject
{
	Q_OBJECT

public:
	enum OdamexGameModes
	{
		MODE_COOPERATIVE,
		MODE_DEATHMATCH,
		MODE_DEATHMATCH2,
		MODE_TEAM_DEATHMATCH,
		MODE_CAPTURE_THE_FLAG,
		MODE_DUEL
	};

	static QList<DMFlagsSection> dmFlags();
	static QList<GameMode> gameModes();
	static GameMode gameModeDuel();
};

/**
 * @brief Provides a (translated) About text for Odamex.
 */
class OdamexAboutProvider : public TextProvider
{
	Q_OBJECT

public:
	QString provide() override;
};

class OdamexWarningInternalClientsProvider : public TextProvider
{
	Q_OBJECT

public:
	QString provide() override;
};

#endif
