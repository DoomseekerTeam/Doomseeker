//------------------------------------------------------------------------------
// srb2gameinfo.h
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
// Copyright (C) 2016 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef idbb44c6c8_a578_436c_adc7_dcba557f22ff
#define idbb44c6c8_a578_436c_adc7_dcba557f22ff

#include "serverapi/serverstructs.h"
#include "serverapi/textprovider.h"
#include <QObject>

class Srb2GameInfo : public QObject
{
	Q_OBJECT

	enum GameModeIndex
	{
		Competition = 1,
		Race = 2,
		Tag = 5,
		HideAndSeek = 6
	};

public:
	enum Flag
	{
		RespawnItems,
		RingSlinger,
		TouchTag,
		AllowExitLevel,
		AllowTeamChange,
		AllPlayersForExit,
		ListenServer,
		CasualServer,
		IngameWadDownloads,
		MouseLook,
		/// When enabled is similar to sv_unblockplayers.
		TailsPickup,
		FriendlyFire,
		RestrictSkinChange,
		JoinNextRound,
		PowerStones
	};

	static QString commandFromFlag(Flag flag);
	static QList<DMFlagsSection> dmFlags();
	static QList<GameMode> gameModes();
	static QList<GameCVar> limits(const GameMode &gameMode);
};

class Srb2DifficultyProvider : GameCVarProvider
{
public:
	QList<GameCVar> get(const QVariant &context) override
	{
		Q_UNUSED(context);
		return QList<GameCVar>();
	}
};

/**
 * @brief Provides a (translated) About text for Srb2.
 */
class Srb2AboutProvider : public TextProvider
{
	Q_OBJECT

public:
	QString provide() override;
};

#endif
