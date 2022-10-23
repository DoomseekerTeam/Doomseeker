//------------------------------------------------------------------------------
// chocolatedoomgameinfo.h
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
// Copyright (C) 2013 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------

#ifndef __CHOCOLATEDOOM_GAME_INFO_H__
#define __CHOCOLATEDOOM_GAME_INFO_H__

#include "serverapi/serverstructs.h"
#include "serverapi/textprovider.h"

class ChocolateDoomGameInfo
{
public:
	enum ChocolateDoomGameModes
	{
		MODE_COOPERATIVE,
		MODE_DEATHMATCH,
		MODE_ALTDEATH
	};
	static QList<GameMode> gameModes();
};

/**
 * @brief Following code was copy-pasted from Chocolate Doom source.
 */
namespace ChocolateDoom
{
// The "mission" controls what game we are playing.

typedef enum
{
	doom,            // Doom 1
	doom2,           // Doom 2
	pack_tnt,        // Final Doom: TNT: Evilution
	pack_plut,       // Final Doom: The Plutonia Experiment
	pack_chex,       // Chex Quest (modded doom)
	pack_hacx,       // Hacx (modded doom2)
	heretic,         // Heretic
	hexen,           // Hexen
	strife,          // Strife

	none
} GameMission_t;

// The "mode" allows more accurate specification of the game mode we are
// in: eg. shareware vs. registered.  So doom1.wad and doom.wad are the
// same mission, but a different mode.

typedef enum
{
	shareware,       // Doom/Heretic shareware
	registered,      // Doom/Heretic registered
	commercial,      // Doom II/Hexen
	retail,          // Ultimate Doom
	indetermined     // Unknown.
} GameMode_t;
} // close namespace

/**
 * @brief Provides a (translated) About text for Chocolatedoom.
 */
class ChocolateDoomAboutProvider : public TextProvider
{
	Q_OBJECT

public:
	QString provide() override;
};

#endif
