//------------------------------------------------------------------------------
// generalgamesetuppanel.h
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
#ifndef idb76a672e_9f54_4dbc_a933_33ea083e8f11
#define idb76a672e_9f54_4dbc_a933_33ea083e8f11

#include "dptr.h"
#include "serverapi/gamecreateparams.h"

#include <QWidget>

class CreateServerDialog;
class EnginePlugin;
class GameCreateParams;
class GameFileList;
class GameMode;
class Ini;

class GeneralGameSetupPanel : public QWidget
{
	Q_OBJECT

public:
	GeneralGameSetupPanel(QWidget *parent);
	~GeneralGameSetupPanel() override;

	GameMode currentGameMode() const;
	EnginePlugin *currentPlugin() const;
	void fillInParams(GameCreateParams &params);
	void loadConfig(Ini &config, bool loadingPrevious);
	QString mapName() const;
	void reloadAppConfig();
	void saveConfig(Ini &config);
	void setupForEngine(EnginePlugin *engine);
	void setupForHostMode(GameCreateParams::HostMode hostMode);
	void setCreateServerDialog(CreateServerDialog *dialog);
	void setIwadByName(const QString &iwad);
	QStringList getAllWadPaths() const;

signals:
	void gameModeChanged(const GameMode &gameMode);
	void pluginChanged(EnginePlugin *plugin);

protected:
	void showEvent(QShowEvent *event) override;

private:
	DPtr<GeneralGameSetupPanel> d;

	GameFileList gameExecutables() const;

	QString pathToExe();

	bool setEngine(const QString &engineName);
	void setupDifficulty(const EnginePlugin *engine);

private slots:
	void onGameModeChanged(int);
	void updateMapWarningVisibility();
};

#endif
