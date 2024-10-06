//------------------------------------------------------------------------------
// createserverdialog.cpp
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
#include "createserverdialog.h"
#include "ui_createserverdialog.h"

#include "apprunner.h"
#include "commandline.h"
#include "configuration/doomseekerconfig.h"
#include "copytextdlg.h"
#include "datapaths.h"
#include "gamedemo.h"
#include "gui/commongui.h"
#include "gui/configuration/doomseekerconfigurationdialog.h"
#include "gui/createserver/hostmodetraits.h"
#include "gui/widgets/createserverdialogpage.h"
#include "ini/ini.h"
#include "ini/settingsproviderqt.h"
#include "plugins/engineplugin.h"
#include "serverapi/gamecreateparams.h"
#include "serverapi/gamehost.h"
#include "serverapi/message.h"

#include <QFileDialog>
#include <QKeySequence>
#include <QMenuBar>
#include <QMessageBox>
#include <QScopedPointer>
#include <QStyle>
#include <QTimer>

DClass<CreateServerDialog> : public Ui::CreateServerDialog
{
public:
	QList<CreateServerDialogPage *> currentCustomPages;
	EnginePlugin *currentEngine;
	HostModeTraits hostMode;
	DemoStore demoStore;

	QMenu *modeMenu;
	QAction *hostModeAction;
	QAction *offlineModeAction;
};

DPointered(CreateServerDialog)

const QString CreateServerDialog::TEMP_GAME_CONFIG_FILENAME = "/tmpserver.ini";

CreateServerDialog::CreateServerDialog(GameCreateParams::HostMode hostMode, QWidget *parent)
	: QDialog(parent)
{
	CommonGUI::setupDialog(*this);
	// Have the window delete itself
	setAttribute(Qt::WA_DeleteOnClose);

	d->currentEngine = nullptr;
	d->hostMode = hostMode;
	d->demoStore = DemoStore();

	d->setupUi(this);
	setupMenu();

	applyModeToUi();

	d->generalSetupPanel->setCreateServerDialog(this);
	d->rulesPanel->setCreateServerDialog(this);

	d->tabWidget->setObjectName("createGameTabWidget");
	d->tabWidget->setStyleSheet("#createGameTabWidget::pane { border: 0; }");

	// This is a crude solution to the problem where message boxes appear
	// before the actual Create Game dialog. We need to give some time
	// for the dialog to appear. Unfortunately reimplementing
	// QDialog::showEvent() didn't work very well.
	QTimer::singleShot(1, this, SLOT(firstLoadConfigTimer()));
}

CreateServerDialog::~CreateServerDialog()
{
}

void CreateServerDialog::applyModeToUi()
{
	d->generalSetupPanel->setupForHostMode(d->hostMode);
	d->rulesPanel->setupForHostMode(d->hostMode);
	initServerTab();

	d->modeMenu->setEnabled(d->hostMode.canChangeMode());
	d->hostModeAction->setChecked(d->hostMode == GameCreateParams::Host);
	d->offlineModeAction->setChecked(d->hostMode == GameCreateParams::Offline);

	switch (d->hostMode.mode)
	{
	case GameCreateParams::Demo:
		setWindowTitle(tr("Doomseeker - Play Demo"));
		d->btnStart->setText(tr("Play demo"));
		break;
	case GameCreateParams::Host:
		setWindowTitle(tr("Doomseeker - Host Online Game"));
		d->btnStart->setText(tr("Host server"));
		break;
	case GameCreateParams::Offline:
		setWindowTitle(tr("Doomseeker - Play Offline Game"));
		d->btnStart->setText(tr("Play"));
		break;
	case GameCreateParams::Remote:
		setWindowTitle(tr("Doomseeker - Setup Remote Game"));
		d->btnStart->setText(tr("Play online"));
		break;
	default:
		setWindowTitle(tr("Doomseeker - Create Game"));
		d->btnStart->setText(tr("Create game"));
		break;
	}

	d->btnCommandLine->setVisible(d->hostMode.canShowCommandLine());
}

void CreateServerDialog::changeToHostMode()
{
	d->hostMode = GameCreateParams::Host;
	applyModeToUi();
}

void CreateServerDialog::changeToOfflineMode()
{
	d->hostMode = GameCreateParams::Offline;
	applyModeToUi();
}

bool CreateServerDialog::commandLineArguments(QString &executable, QStringList &args,
	CommandLinePurpose purpose)
{
	const QString errorCapt = tr("Doomseeker - create game");
	if (d->currentEngine == nullptr)
	{
		QMessageBox::critical(nullptr, errorCapt, tr("No game selected"));
		return false;
	}

	GameCreateParams gameParams;
	if (createHostInfo(gameParams, purpose))
	{
		CommandLineInfo cli;
		QString error;

		GameHost *gameRunner = d->currentEngine->gameHost();
		Message message = gameRunner->createHostCommandLine(gameParams, cli);

		delete gameRunner;

		if (message.isError())
		{
			QMessageBox::critical(nullptr, tr("Doomseeker - error"), message.contents());
			return false;
		}
		else
		{
			executable = cli.executable.absoluteFilePath();
			args = cli.args;
			return true;
		}
	}
	return false;
}

bool CreateServerDialog::createHostInfo(GameCreateParams &params, CommandLinePurpose purpose)
{
	params.setHostMode(d->hostMode);
	d->generalSetupPanel->fillInParams(params);
	d->dmflagsPanel->fillInParams(params);

	if (!fillInParamsFromPluginPages(params))
		return false;

	d->customParamsPanel->fillInParams(params);
	if (d->hostMode.canChangeAnyGameRules())
	{
		if (d->hostMode.isCreatingServer())
		{
			d->serverPanel->fillInParams(params);
		}
		d->rulesPanel->fillInParams(params);
	}

	return createHostInfoDemoRecord(params, purpose);
}

bool CreateServerDialog::createHostInfoDemoRecord(GameCreateParams &params, CommandLinePurpose purpose)
{
	if (gConfig.doomseeker.bRecordDemo && d->hostMode.canRecordDemo())
	{
		const DemoRecord::Control demoMode = purpose == CommandLinePurpose::Run
			? DemoRecord::Managed : DemoRecord::Unmanaged;
		if (demoMode == DemoRecord::Managed)
		{
			if (!d->demoStore.ensureStorageExists(this))
			{
				return false;
			}
		}
		const QString demoPath = d->demoStore.mkDemoFullPath(demoMode, *d->currentEngine, false);
		if (demoPath.isEmpty())
		{
			return false;
		}

		params.setDemoPath(demoPath);
		params.setDemoRecord(demoMode);
	}
	return true;
}

GameMode CreateServerDialog::currentGameMode() const
{
	return d->generalSetupPanel->currentGameMode();
}

void CreateServerDialog::firstLoadConfigTimer()
{
	initEngineSpecific(d->generalSetupPanel->currentPlugin());
	QString tmpGameCfgPath = gDefaultDataPaths->programsDataDirectoryPath() + TEMP_GAME_CONFIG_FILENAME;

	QFileInfo fi(tmpGameCfgPath);
	if (fi.exists())
		loadConfig(tmpGameCfgPath, true);
}

void CreateServerDialog::initDMFlagsTabs()
{
	bool flagsAdded = d->hostMode.canChangeAnyGameRules() &&
		d->dmflagsPanel->initDMFlagsTabs(d->currentEngine);
	int tabIndex = d->tabWidget->indexOf(d->tabFlags);
	if (flagsAdded && tabIndex < 0)
		d->tabWidget->insertTab(d->tabWidget->indexOf(d->tabCustomParameters), d->tabFlags, tr("Flags"));
	else if (!flagsAdded && tabIndex >= 0)
		d->tabWidget->removeTab(tabIndex);
}

void CreateServerDialog::initEngineSpecific(EnginePlugin *engine)
{
	if (engine == d->currentEngine || engine == nullptr)
		return;

	d->currentEngine = engine;

	d->generalSetupPanel->setupForEngine(engine);
	initDMFlagsTabs();
	initEngineSpecificPages(engine);
	initServerTab();
	initRules();
}

void CreateServerDialog::initEngineSpecificPages(EnginePlugin *engine)
{
	// First, get rid of the original pages.
	for (CreateServerDialogPage *page : d->currentCustomPages)
		delete page;
	d->currentCustomPages.clear();

	if (!d->hostMode.canChangeAnyGameRules())
		return;

	// Add new custom pages to the dialog.
	d->currentCustomPages = engine->createServerDialogPages(this);
	for (CreateServerDialogPage *page : d->currentCustomPages)
	{
		int idxInsertAt = d->tabWidget->indexOf(d->tabCustomParameters);
		d->tabWidget->insertTab(idxInsertAt, page, page->name());
	}
}

void CreateServerDialog::initGamemodeSpecific(const GameMode &gameMode)
{
	d->rulesPanel->setupForEngine(d->currentEngine, gameMode);
}

void CreateServerDialog::initServerTab()
{
	if (d->currentEngine != nullptr)
		d->serverPanel->setupForEngine(d->currentEngine);
	d->tabWidget->setTabEnabled(d->tabWidget->indexOf(d->tabServer),
		d->serverPanel->isAnythingAvailable()
		&& d->hostMode.isSettingUpOnlineGame()
		&& d->hostMode.canChangeAnyGameRules());
}

void CreateServerDialog::initRules()
{
	d->rulesPanel->setupForEngine(d->currentEngine, currentGameMode());
	d->tabWidget->setTabEnabled(d->tabWidget->indexOf(d->tabRules),
		d->rulesPanel->isAnythingAvailable()
		&& d->hostMode.canChangeAnyGameRules());
}

bool CreateServerDialog::loadConfig(const QString &filename, bool loadingPrevious)
{
	QSettings settingsFile(filename, QSettings::IniFormat);
	SettingsProviderQt settingsProvider(&settingsFile);
	Ini ini(&settingsProvider);

	d->generalSetupPanel->loadConfig(ini, loadingPrevious);
	d->rulesPanel->loadConfig(ini);
	d->serverPanel->loadConfig(ini);
	d->dmflagsPanel->loadConfig(ini);

	for (CreateServerDialogPage *page : d->currentCustomPages)
		page->loadConfig(ini);

	d->customParamsPanel->loadConfig(ini);

	if (d->hostMode.canChangeMode())
	{
		d->hostMode = HostModeTraits::fromCfgName(ini.section("General")["hostMode"]);
		applyModeToUi();
	}

	return true;
}

void CreateServerDialog::makeDemoPlaybackSetupDialog(EnginePlugin *plugin,
	const GameDemo &demo, const QString &iwad, const QList<PickedGameFile> &wads)
{
	initEngineSpecific(plugin);
	switchToMode(GameCreateParams::Demo);
	d->generalSetupPanel->setDemoPath(demo.demopath);
	d->generalSetupPanel->setIwadByPath(iwad);
	d->generalSetupPanel->setPwads(wads);
}

void CreateServerDialog::makeRemoteGameSetupDialog(EnginePlugin *plugin)
{
	initEngineSpecific(plugin);
	switchToMode(GameCreateParams::Remote);
}

MapListPanel *CreateServerDialog::mapListPanel()
{
	return d->rulesPanel->mapListPanel();
}

QString CreateServerDialog::mapName() const
{
	return d->generalSetupPanel->mapName();
}

QStringList CreateServerDialog::wadPaths() const
{
	return d->generalSetupPanel->getAllWadPaths();
}

bool CreateServerDialog::fillInParamsFromPluginPages(GameCreateParams &params)
{
	for (CreateServerDialogPage *page : d->currentCustomPages)
	{
		if (page->validate())
			page->fillInGameCreateParams(params);
		else
		{
			// Pages must take care of displaying their own error messages.
			d->tabWidget->setCurrentIndex(d->tabWidget->indexOf(page));
			return false;
		}
	}
	return true;
}

void CreateServerDialog::runGame()
{
	const QString errorCapt = tr("Doomseeker - create game");
	if (d->currentEngine == nullptr)
	{
		QMessageBox::critical(nullptr, errorCapt, tr("No game selected"));
		return;
	}

	GameCreateParams gameParams;
	if (createHostInfo(gameParams, CommandLinePurpose::Run))
	{
		QScopedPointer<GameHost> gameRunner(d->currentEngine->gameHost());
		Message message = gameRunner->host(gameParams);

		if (message.isError())
		{
			QMessageBox::critical(nullptr, errorCapt, message.contents());
		}
		else
		{
			QString tmpGameConfigPath = gDefaultDataPaths->programsDataDirectoryPath() + TEMP_GAME_CONFIG_FILENAME;
			saveConfig(tmpGameConfigPath);
		}
	}
}

bool CreateServerDialog::saveConfig(const QString &filename)
{
	QSettings settingsFile(filename, QSettings::IniFormat);
	SettingsProviderQt settingsProvider(&settingsFile);
	Ini ini(&settingsProvider);

	d->generalSetupPanel->saveConfig(ini);
	d->rulesPanel->saveConfig(ini);
	d->serverPanel->saveConfig(ini);
	d->dmflagsPanel->saveConfig(ini);

	for (CreateServerDialogPage *page : d->currentCustomPages)
		page->saveConfig(ini);

	d->customParamsPanel->saveConfig(ini);

	ini.section("General")["hostMode"] = d->hostMode.cfgName();

	if (settingsFile.isWritable())
	{
		settingsFile.sync();
		return true;
	}
	return false;
}

void CreateServerDialog::setIwadByName(const QString &iwad)
{
	d->generalSetupPanel->setIwadByName(iwad);
}

void CreateServerDialog::setupMenu()
{
	QMenuBar *mainMenu = new QMenuBar(this);

	d->modeMenu = mainMenu->addMenu(tr("&Mode"));
	d->hostModeAction = d->modeMenu->addAction(tr("&Host server"),
		this, SLOT(changeToHostMode()));
	d->hostModeAction->setCheckable(true);
	d->offlineModeAction = d->modeMenu->addAction(tr("&Play offline"),
		this, SLOT(changeToOfflineMode()));
	d->offlineModeAction->setCheckable(true);

	auto *settingsMenu = mainMenu->addMenu(tr("&Settings"));

	auto *loadConfigAction = settingsMenu->addAction(tr("&Load game configuration"),
		this, SLOT(showLoadConfig()));
	loadConfigAction->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
	loadConfigAction->setShortcut(QKeySequence("Ctrl+O"));

	auto *saveConfigAction = settingsMenu->addAction(tr("&Save game configuration"),
		this, SLOT(showSaveConfig()));
	saveConfigAction->setIcon(QIcon(":/icons/diskette.png"));
	saveConfigAction->setShortcut(QKeySequence("Ctrl+S"));
	settingsMenu->addSeparator();

	auto *programSettings = settingsMenu->addAction(tr("&Program settings"),
		this, SLOT(showConfiguration()));
	programSettings->setIcon(QIcon(":/icons/preferences-system-4.png"));

	d->dialogLayout->setMenuBar(mainMenu);
}

void CreateServerDialog::showConfiguration()
{
	DoomseekerConfigurationDialog::openConfiguration(this, d->currentEngine);
	d->generalSetupPanel->reloadAppConfig();
}

void CreateServerDialog::showCommandLine()
{
	QString executable;
	QStringList args;
	if (commandLineArguments(executable, args, CommandLinePurpose::Show))
	{
		// Lines below directly modify the passed values.
		CommandLine::escapeExecutable(executable);
		CommandLine::escapeArgs(args);

		QString title = !d->hostMode.isCreatingServer() ?
			tr("Run game command line:") :
			tr("Host server command line:");
		CopyTextDlg ctd(executable + " " + args.join(" "), title, this);
		ctd.exec();
	}
}

void CreateServerDialog::showLoadConfig()
{
	QString dialogDir = gConfig.doomseeker.previousCreateServerConfigDir;
	QString strFile = QFileDialog::getOpenFileName(this, tr("Doomseeker - load game setup config"), dialogDir, tr("Config files (*.ini)"));

	if (!strFile.isEmpty())
	{
		QFileInfo fi(strFile);
		gConfig.doomseeker.previousCreateServerConfigDir = fi.absolutePath();

		loadConfig(strFile, false);
	}
}

void CreateServerDialog::showSaveConfig()
{
	QString dialogDir = gConfig.doomseeker.previousCreateServerConfigDir;
	QString strFile = QFileDialog::getSaveFileName(this, tr("Doomseeker - save game setup config"), dialogDir, tr("Config files (*.ini)"));
	if (!strFile.isEmpty())
	{
		QFileInfo fi(strFile);
		gConfig.doomseeker.previousCreateServerConfigDir = fi.absolutePath();

		if (fi.suffix().isEmpty())
			strFile += ".ini";

		if (!saveConfig(strFile))
			QMessageBox::critical(nullptr, tr("Doomseeker - save game setup config"), tr("Unable to save game setup configuration!"));
	}
}

void CreateServerDialog::startGame()
{
	// TODO this should probably be controlled by a separate
	// setting where the plugin would tell the box if the box can
	// start the game or if it must let the plugin do it.
	if (d->hostMode != GameCreateParams::Remote)
		runGame();
	else
		accept();
}

void CreateServerDialog::switchToMode(GameCreateParams::HostMode mode)
{
	d->hostMode = mode;
	applyModeToUi();
}
