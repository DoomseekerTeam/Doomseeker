//------------------------------------------------------------------------------
// gamerulespanel.h
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef id880d5e15_1a53_40da_bdb9_11d9b4dc3222
#define id880d5e15_1a53_40da_bdb9_11d9b4dc3222

#include "dptr.h"
#include "serverapi/gamecreateparams.h"

#include <QWidget>

class CreateServerDialog;
class EnginePlugin;
class GameCreateParams;
class GameMode;
class Ini;
class MapListPanel;

class GameRulesPanel : public QWidget
{
	Q_OBJECT

public:
	GameRulesPanel(QWidget *parent);
	~GameRulesPanel() override;

	void fillInParams(GameCreateParams &params);
	bool isAnythingAvailable() const;
	MapListPanel *mapListPanel();
	void loadConfig(Ini &config);
	void saveConfig(Ini &config);
	void setupForEngine(const EnginePlugin *engine, const GameMode &gameMode);
	void setupForHostMode(GameCreateParams::HostMode hostMode);
	void setCreateServerDialog(CreateServerDialog *dialog);

private:
	DPtr<GameRulesPanel> d;

	void applyModeToUi();
	void fillInLimits(GameCreateParams &params);
	void fillInModifiers(GameCreateParams &params);
	void memorizeLimits();
	void loadMemorizedLimits(const EnginePlugin *engine);
	void removeLimitWidgets();
	void setupLimitWidgets(const EnginePlugin *engine, const GameMode &gameMode);
	void setupModifiers(const EnginePlugin *engine);
};

#endif
