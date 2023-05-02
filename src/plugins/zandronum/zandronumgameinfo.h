//------------------------------------------------------------------------------
// zandronumgameinfo.h
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
// Copyright (C) 2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_PLUGIN_ZANDRONUMGAMEINFO_H
#define DOOMSEEKER_PLUGIN_ZANDRONUMGAMEINFO_H

#include "serverapi/serverstructs.h"
#include "serverapi/textprovider.h"
#include <QObject>

class ZandronumGameInfo : public QObject
{
	Q_OBJECT

public:
	enum LMSAllowedWeapons
	{
		LMSAW_Pistol = 1 << 0,
		LMSAW_Shotgun = 1 << 1,
		LMSAW_SuperShotgun = 1 << 2,
		LMSAW_Chaingun = 1 << 3,
		LMSAW_Minigun = 1 << 4,
		LMSAW_RocketLauncher = 1 << 5,
		LMSAW_GrenadeLauncher = 1 << 6,
		LMSAW_PlasmaRifle = 1 << 7,
		LMSAW_Railgun = 1 << 8,
		LMSAW_Chainsaw = 1 << 9,
	};

	enum LMSSpectatorSettings
	{
		LMSSS_TalkToActivePlayers = 1 << 0,
		LMSSS_ViewTheGame = 1 << 1,
	};

	enum ZandronumGameMode
	{
		GAMEMODE_COOPERATIVE,
		GAMEMODE_SURVIVAL,
		GAMEMODE_INVASION,
		GAMEMODE_DEATHMATCH,
		GAMEMODE_TEAMPLAY,
		GAMEMODE_DUEL,
		GAMEMODE_TERMINATOR,
		GAMEMODE_LASTMANSTANDING,
		GAMEMODE_TEAMLMS,
		GAMEMODE_POSSESSION,
		GAMEMODE_TEAMPOSSESSION,
		GAMEMODE_TEAMGAME,
		GAMEMODE_CTF,
		GAMEMODE_ONEFLAGCTF,
		GAMEMODE_SKULLTAG,
		GAMEMODE_DOMINATION
	};

	/**
	 * This is stored in config and indexing cannot change between versions.
	 */
	enum GameVersion
	{
		GV_Zandronum2 = 1,
		GV_Zandronum3 = 2
	};

	static const QString OPTION_GAMEVERSION;

	static QList<GameMode> gameModes();
	static QList<GameCVar> gameModifiers();
};

/**
 * @brief Provides a (translated) About text for Zandronum.
 */
class ZandronumAboutProvider : public TextProvider
{
	Q_OBJECT

public:
	QString provide() override;
};

#endif
