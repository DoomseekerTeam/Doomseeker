//------------------------------------------------------------------------------
// flagspage.h
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
#ifndef DOOMSEEKER_PLUGIN_ZANDRONUMQ_CREATESERVERDIALOGPAGES_FLAGSPAGE_H
#define DOOMSEEKER_PLUGIN_ZANDRONUMQ_CREATESERVERDIALOGPAGES_FLAGSPAGE_H

#include "ui_flagspage.h"
#include "zandronumqgameinfo.h"
#include <gui/widgets/createserverdialogpage.h>

namespace zandronumq1
{
class FlagsPageValueController;
}

class GameCreateParams;

class FlagsPage : public CreateServerDialogPage, private Ui::FlagsPage
{
	friend class FlagsId;
	friend class zandronumq1::FlagsPageValueController;

	Q_OBJECT

public:
	/**
	 * This is stored in config and indexing cannot change between versions.
	 */
	enum FallingDamageType
	{
		FDT_None = 0,
		FDT_Old = 1,
		FDT_Hexen = 2,
		FDT_Strife = 3
	};

	/**
	 * This is stored in config and indexing cannot change between versions.
	 */
	enum JumpCrouchAbility
	{
		JCA_Default = 0,
		JCA_No = 1,
		JCA_Yes = 2
	};

	/**
	 * This is stored in config and indexing cannot change between versions.
	 */
	enum PlayerBlock
	{
		PB_NotSet = 0,
		PB_Noclip = 1,
		PB_AllyNoclip = 2,
		PB_Block = 3
	};

	/**
	 * This is stored in config and indexing cannot change between versions.
	 */
	enum LevelExit
	{
		EXIT_NotSet = 0,
		EXIT_NextMap = 1,
		EXIT_RestartMap = 2,
		EXIT_KillPlayer = 3
	};

	static const ZandronumqGameInfo::GameVersion DEFAULT_GAME_VERSION = ZandronumqGameInfo::GV_zandronumq1;

	FlagsPage(CreateServerDialog *pParentDialog);
	~FlagsPage() override;

	void fillInGameCreateParams(GameCreateParams &params) override;
	bool loadConfig(Ini &ini) override;
	bool saveConfig(Ini &ini) override;

private:
	class PrivData;

	PrivData *d;

	void initJumpCrouchComboBoxes(QComboBox *pComboBox);
	void insertFlagsIfValid(QLineEdit *dst, QString flags, unsigned valIfInvalid = 0);

	ZandronumqGameInfo::GameVersion gameVersion() const;
	void loadGameVersion(ZandronumqGameInfo::GameVersion version);
	void setGameVersion(ZandronumqGameInfo::GameVersion version);

	PlayerBlock playerBlock() const;
	void setPlayerBlock(PlayerBlock playerBlock);

	LevelExit levelExit() const;
	void setLevelExit(LevelExit levelExit);

private slots:
	void applyGameVersion();

	/**
	 * @brief Extracts dmflags values from widgets and inserts their
	 *        numerical values into the text input widgets.
	 */
	void applyWidgetsChange();

	/**
	 * @brief Takes the numerical values of flags and applies them to
	 *        widgets.
	 */
	void propagateFlagsInputsChanges();
};

#endif
