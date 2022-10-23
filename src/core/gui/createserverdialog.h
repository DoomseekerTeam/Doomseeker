//------------------------------------------------------------------------------
// createserverdialog.h
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
// Copyright (C) 2009-2012 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_GUI_CREATESERVERDIALOG_H
#define DOOMSEEKER_GUI_CREATESERVERDIALOG_H

#include "dptr.h"
#include "serverapi/gamecreateparams.h"
#include "serverapi/serverstructs.h"
#include <QDialog>

class CreateServerDialogPage;
class EnginePlugin;
class GameCreateParams;
class MapListPanel;
class Message;
class Server;

/**
 * @brief Dialog window allowing user to create a game.
 */
class CreateServerDialog : public QDialog
{
	Q_OBJECT

public:
	CreateServerDialog(GameCreateParams::HostMode hostMode, QWidget *parent = nullptr);
	virtual ~CreateServerDialog();

	bool commandLineArguments(QString &executable, QStringList &args, bool offline);
	void makeRemoteGameSetupDialog(EnginePlugin *plugin);
	MapListPanel *mapListPanel();
	QString mapName() const;
	void setIwadByName(const QString &iwad);
	QStringList wadPaths() const;

private slots:
	void changeToHostMode();
	void changeToOfflineMode();
	void firstLoadConfigTimer();

	/**
	 * Called each time when a new engine in engine combo box is selected.
	 * Resets most of the controls and puts engine specific information
	 * and controls where applicable.
	 */
	void initEngineSpecific(EnginePlugin *engineInfo);
	void initGamemodeSpecific(const GameMode &gameMode);
	void showConfiguration();
	void showLoadConfig();
	void showSaveConfig();
	void showStartGameCommandLine();
	void startGame();

private:
	static const QString TEMP_GAME_CONFIG_FILENAME;

	DPtr<CreateServerDialog> d;

	/**
	 * Adds IWAD path to the IWAD ComboBox.
	 * If the path already exists no new items will be added but this path
	 * will be selected.
	 */
	void addIwad(const QString &path);
	void applyModeToUi();

	/**
	 * Sets host information for both server and hi objects. Both
	 * parameters obtain new information after this method is called.
	 * @return false if fail.
	 */
	bool createHostInfo(GameCreateParams &params, bool offline);
	void createHostInfoDemoRecord(GameCreateParams &params, bool offline);

	GameMode currentGameMode() const;

	bool fillInParamsFromPluginPages(GameCreateParams &params);
	void initDMFlagsTabs();

	/**
	 * @brief Loads pages specific for the given engine.
	 *
	 * @see CreateServerDialogPage
	 */
	void initEngineSpecificPages(EnginePlugin *engineInfo);

	void initServerTab();

	void initRules();

	bool loadConfig(const QString &filename, bool loadingPrevious);
	void runGame(bool offline);
	bool saveConfig(const QString &filename);
	void setupMenu();
	void showCommandLine(bool offline);
};

#endif
