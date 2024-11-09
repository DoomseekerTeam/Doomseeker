//------------------------------------------------------------------------------
// generalgamesetuppanel.cpp
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
#include "generalgamesetuppanel.h"
#include "ui_generalgamesetuppanel.h"

#include "configuration/doomseekerconfig.h"
#include "gui/createserver/hostmodetraits.h"
#include "gui/createserver/maplistpanel.h"
#include "gui/createserverdialog.h"
#include "ini/ini.h"
#include "plugins/engineplugin.h"
#include "plugins/pluginloader.h"
#include "serverapi/gamecreateparams.h"
#include "serverapi/gameexefactory.h"
#include "serverapi/gamefile.h"
#include "filefilter.h"
#include "templatedpathresolver.h"
#include <cassert>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTimer>

DClass<GeneralGameSetupPanel> : public Ui::GeneralGameSetupPanel
{
public:
	EnginePlugin *currentEngine;
	HostModeTraits hostMode;
	bool iwadSetExplicitly;
	bool pwadsSetExplicitly;
	CreateServerDialog *parentDialog;

	int demoRow;
	int serverNameRow;
	int portRow;
	int hostModeSpacerRow;
	int gamemodeRow;
	int difficultyRow;
	int mapRow;

	bool isValidExecutable(const QString &executablePath)
	{
		QFileInfo fileInfo(executablePath);
		return !executablePath.isEmpty() && fileInfo.isFile();
	}
};

DPointered(GeneralGameSetupPanel)


GeneralGameSetupPanel::GeneralGameSetupPanel(QWidget *parent)
	: QWidget(parent)
{
	d->setupUi(this);
	d->iwadSetExplicitly = false;
	d->pwadsSetExplicitly = false;
	d->parentDialog = nullptr;

	d->demoPlayerExecutableInput->setAllowedExecutables(GameFile::Offline);
	d->hostExecutableInput->setAllowedExecutables(GameFile::Server);
	d->offlineExecutableInput->setAllowedExecutables(GameFile::Offline);
	d->remoteExecutableInput->setAllowedExecutables(GameFile::Client);

	d->demoPicker->setTitleVisible(false);
	d->demoPicker->setBrowseFileFilters(FileFilter::demos());
	d->demoPicker->setBrowseTitle(tr("Doomseeker - browse demo"));

	d->formLayout->getWidgetPosition(d->demoPicker, &d->demoRow, nullptr);
	d->formLayout->getWidgetPosition(d->leServerName, &d->serverNameRow, nullptr);
	d->formLayout->getWidgetPosition(d->portArea, &d->portRow, nullptr);
	d->formLayout->getWidgetPosition(d->hostModeSpacer, &d->hostModeSpacerRow, nullptr);
	d->formLayout->getWidgetPosition(d->cboGamemode, &d->gamemodeRow, nullptr);
	d->formLayout->getWidgetPosition(d->cboDifficulty, &d->difficultyRow, nullptr);
	d->formLayout->getWidgetPosition(d->mapPanel, &d->mapRow, nullptr);

	this->connect(d->cboEngine, SIGNAL(currentPluginChanged(EnginePlugin*)),
		SIGNAL(pluginChanged(EnginePlugin*)));
}

GeneralGameSetupPanel::~GeneralGameSetupPanel()
{
}

QStringList GeneralGameSetupPanel::getAllWadPaths() const
{
	QStringList paths;
	paths << d->iwadPicker->currentIwad();
	for (const PickedGameFile &file : d->wadsPicker->files())
	{
		paths << file.path;
	}
	return paths;
}

void GeneralGameSetupPanel::fillInParams(GameCreateParams &params)
{
	params.setDemoPath(gDoomseekerTemplatedPathResolver().resolve(d->demoPicker->path()));
	params.setExecutablePath(gDoomseekerTemplatedPathResolver().resolve(pathToExe()));
	params.setIwadPath(gDoomseekerTemplatedPathResolver().resolve(d->iwadPicker->currentIwad()));
	params.setLoggingPath(gDoomseekerTemplatedPathResolver().resolve(d->logDirectoryPicker->validatedCurrentPath()));
	QStringList filePaths;
	QList<bool> optionalFiles;
	for (const PickedGameFile &file : d->wadsPicker->files())
	{
		filePaths << file.path;
		optionalFiles << file.optional;
	}
	params.setPwadsPaths(gDoomseekerTemplatedPathResolver().resolve(filePaths));
	params.setPwadsOptional(optionalFiles);
	params.setBroadcastToLan(d->cbBroadcastToLAN->isChecked());
	params.setBroadcastToMaster(d->cbBroadcastToMaster->isChecked());
	params.setMap(d->leMap->text());
	params.setName(d->leServerName->text());
	params.setPort(d->spinPort->isEnabled() ? d->spinPort->value() : 0);
	params.setGameMode(currentGameMode());
	params.setSkill(d->cboDifficulty->itemData(d->cboDifficulty->currentIndex()).toInt());
	params.setUpnp(d->cbUpnp->isChecked());
	params.setUpnpPort(d->spinUpnpPort->value());
}

void GeneralGameSetupPanel::loadConfig(Ini &config, bool loadingPrevious)
{
	IniSection general = config.section("General");

	// Engine
	const EnginePlugin *prevEngine = d->currentEngine;
	QString configEngineName = general["engine"];
	bool engineChanged = false;
	if (d->hostMode.canChangeGame())
	{
		if (!setEngine(configEngineName))
			return;
		engineChanged = (prevEngine != d->currentEngine);
	}

	// Executables
	bool changeExecutable = engineChanged || !d->cbLockExecutable->isChecked();
	if (!d->hostMode.canChangeGame())
	{
		// If we can't change the engine, also don't substitute the executable
		// when the config that is being loaded is for a different game.
		auto *plugin = gPlugins->plugin(gPlugins->pluginIndexFromName(configEngineName));
		if (plugin == nullptr || d->currentEngine != plugin->info())
			changeExecutable = false;
	}

	if (changeExecutable)
	{
		QString demoPlayerExecutablePath = *general["demoPlayerExecutable"];
		if (d->isValidExecutable(demoPlayerExecutablePath))
			d->demoPlayerExecutableInput->setPath(demoPlayerExecutablePath);

		QString hostExecutablePath = *general["hostExecutable"];
		if (d->isValidExecutable(hostExecutablePath))
			d->hostExecutableInput->setPath(hostExecutablePath);

		QString offlineExecutablePath = *general["offlineExecutable"];
		if (d->isValidExecutable(offlineExecutablePath))
			d->offlineExecutableInput->setPath(offlineExecutablePath);

		QString remoteExecutablePath = *general["remoteExecutable"];
		if (d->isValidExecutable(remoteExecutablePath))
			d->remoteExecutableInput->setPath(remoteExecutablePath);
	}

	// Other
	d->leServerName->setText(general["name"]);
	d->spinPort->setValue(general["port"]);

	int gameModeIndex = d->cboGamemode->findData(static_cast<gamemode_id>(general["gamemode"]));
	if (gameModeIndex >= 0)
		d->cboGamemode->setCurrentIndex(gameModeIndex);

	int difficultyIndex = d->cboDifficulty->findData(static_cast<int>(general["difficulty"]));
	d->cboDifficulty->setCurrentIndex(qMax(0, difficultyIndex));

	d->leMap->setText(general["map"]);

	if (!(loadingPrevious && d->iwadSetExplicitly))
		d->iwadPicker->addIwad(general["iwad"]);

	d->logDirectoryPicker->setLoggingEnabled(general["logEnabled"]);
	d->logDirectoryPicker->setPathAndUpdate(general["logDir"]);

	if (!(loadingPrevious && d->pwadsSetExplicitly))
	{
		QList<PickedGameFile> files;
		QStringList filesPaths = general["pwads"].valueString().split(";");
		QList<QString> optionalFilesTokens = general["pwadsOptional"].valueString().split(";");
		for (int i = 0; i < filesPaths.length(); ++i)
		{
			PickedGameFile file;
			file.path = filesPaths[i];
			if (i < optionalFilesTokens.length())
			{
				file.optional = (optionalFilesTokens[i] != "0");
			}
			files << file;
		}
		d->wadsPicker->setFiles(files);
	}

	d->cbBroadcastToLAN->setChecked(general["broadcastToLAN"]);
	d->cbBroadcastToMaster->setChecked(general["broadcastToMaster"]);
	d->cbUpnp->setChecked(general["upnp"]);
	d->spinUpnpPort->setValue(general["upnpPort"]);

	// Timer triggers slot after config is fully loaded.
	QTimer::singleShot(0, this, SLOT(updateMapWarningVisibility()));
}

void GeneralGameSetupPanel::saveConfig(Ini &config)
{
	IniSection general = config.section("General");
	general["engine"] = d->cboEngine->currentText();
	general["demoPlayerExecutable"] = d->demoPlayerExecutableInput->path();
	general["hostExecutable"] = d->hostExecutableInput->path();
	general["offlineExecutable"] = d->offlineExecutableInput->path();
	general["remoteExecutable"] = d->remoteExecutableInput->path();
	general["name"] = d->leServerName->text();
	general["port"] = d->spinPort->value();
	general["logDir"] = d->logDirectoryPicker->currentPath();
	general["logEnabled"] = d->logDirectoryPicker->isLoggingEnabled();
	general["gamemode"] = d->cboGamemode->itemData(d->cboGamemode->currentIndex()).toInt();
	general["map"] = d->leMap->text();
	general["difficulty"] = d->cboDifficulty->itemData(d->cboDifficulty->currentIndex()).toInt();
	general["iwad"] = d->iwadPicker->currentIwad();

	QList<PickedGameFile> files = d->wadsPicker->files();
	QStringList filePaths;
	QStringList optionalFiles;
	for (const auto &file : files)
	{
		filePaths << file.path;
		optionalFiles << (file.optional ? "1" : "0");
	}
	general["pwads"] = filePaths.join(";");
	general["pwadsOptional"] = optionalFiles.join(";");

	general["broadcastToLAN"] = d->cbBroadcastToLAN->isChecked();
	general["broadcastToMaster"] = d->cbBroadcastToMaster->isChecked();
	general["upnp"] = d->cbUpnp->isChecked();
	general["upnpPort"] = d->spinUpnpPort->value();
}

void GeneralGameSetupPanel::reloadAppConfig()
{
	d->demoPlayerExecutableInput->reloadExecutables();
	d->hostExecutableInput->reloadExecutables();
	d->offlineExecutableInput->reloadExecutables();
	d->remoteExecutableInput->reloadExecutables();
	d->iwadPicker->loadIwads();
}

void GeneralGameSetupPanel::setupDifficulty(const EnginePlugin *engine)
{
	QVariant oldDifficulty = d->cboDifficulty->itemData(d->cboDifficulty->currentIndex());
	d->cboDifficulty->clear();

	QList<GameCVar> levels = engine->data()->difficulty->get(QVariant());
	d->labelDifficulty->setVisible(!levels.isEmpty());
	d->cboDifficulty->setVisible(!levels.isEmpty());
	d->cboDifficulty->addItem(tr("< NONE >"), Skill::UNDEFINED);
	for (const GameCVar &level : levels)
	{
		d->cboDifficulty->addItem(level.name(), level.value());
	}
	int memorizedIndex = d->cboDifficulty->findData(oldDifficulty);
	if (memorizedIndex >= 0)
		d->cboDifficulty->setCurrentIndex(memorizedIndex);
}

void GeneralGameSetupPanel::setupForEngine(EnginePlugin *engine)
{
	d->currentEngine = engine;

	d->cboEngine->setPluginByName(engine->data()->name);
	d->labelIwad->setVisible(engine->data()->hasIwad);
	d->iwadPicker->setVisible(engine->data()->hasIwad);
	d->labelLogging->setVisible(engine->data()->allowsLogging);
	d->logDirectoryPicker->setVisible(engine->data()->allowsLogging);
	d->upnpArea->setVisible(engine->data()->allowsUpnp);
	d->spinUpnpPort->setVisible(engine->data()->allowsUpnpPort);

	d->demoPlayerExecutableInput->setPlugin(engine);
	d->hostExecutableInput->setPlugin(engine);
	d->offlineExecutableInput->setPlugin(engine);
	d->remoteExecutableInput->setPlugin(engine);

	d->spinPort->setValue(d->currentEngine->data()->defaultServerPort);

	d->cboGamemode->clear();
	d->cboGamemode->addItem(tr("< NONE >"), GameMode::SGM_Unknown);

	QList<GameMode> gameModes = d->currentEngine->gameModes();
	for (int i = 0; i < gameModes.count(); ++i)
		d->cboGamemode->addItem(gameModes[i].name(), gameModes[i].index());
	setupDifficulty(engine);
}

void GeneralGameSetupPanel::setupForHostMode(GameCreateParams::HostMode hostMode)
{
	d->hostMode = hostMode;

	d->cboEngine->setDisabled(!d->hostMode.canChangeGame());

	d->demoPlayerExecutableInput->hide();
	d->hostExecutableInput->hide();
	d->offlineExecutableInput->hide();
	d->remoteExecutableInput->hide();
	switch (hostMode)
	{
	case GameCreateParams::Demo: d->demoPlayerExecutableInput->show(); break;
	default:
	case GameCreateParams::Host: d->hostExecutableInput->show(); break;
	case GameCreateParams::Offline: d->offlineExecutableInput->show(); break;
	case GameCreateParams::Remote: d->remoteExecutableInput->show(); break;
	}

	const bool canSetupServer = d->hostMode.isCreatingServer() && d->hostMode.canChangeAnyGameRules();
	d->labelServerName->setVisible(canSetupServer);
	d->leServerName->setVisible(canSetupServer);
	d->labelPort->setVisible(canSetupServer);
	d->portArea->setVisible(canSetupServer);
	d->hostModeSpacer->setVisible(canSetupServer);
	d->broadcastOptionsArea->setVisible(canSetupServer);

	// Insert/remove operations are necessary to get rid of extra
	// spacing remaining when hiding form elements.
	if (canSetupServer)
	{
		d->formLayout->insertRow(d->serverNameRow, d->labelServerName, d->leServerName);
		d->formLayout->insertRow(d->portRow, d->labelPort, d->portArea);
		d->formLayout->insertRow(d->hostModeSpacerRow, d->hostModeSpacer,
			static_cast<QWidget *>(nullptr));
	}
	else
	{
		d->formLayout->removeWidget(d->labelServerName);
		d->formLayout->removeWidget(d->leServerName);
		d->formLayout->removeWidget(d->labelPort);
		d->formLayout->removeWidget(d->portArea);
		d->formLayout->removeWidget(d->hostModeSpacer);
	}

	d->labelDemo->setVisible(d->hostMode == GameCreateParams::Demo);
	d->demoPicker->setVisible(d->hostMode == GameCreateParams::Demo);

	if (d->hostMode == GameCreateParams::Demo)
	{
		d->formLayout->insertRow(d->demoRow, d->labelDemo, d->demoPicker);
	}
	else
	{
		d->formLayout->removeWidget(d->labelDemo);
		d->formLayout->removeWidget(d->demoPicker);
	}

	d->labelGamemode->setVisible(d->hostMode.canChangeAnyGameRules());
	d->cboGamemode->setVisible(d->hostMode.canChangeAnyGameRules());
	d->labelDifficulty->setVisible(d->hostMode.canChangeAnyGameRules());;
	d->cboDifficulty->setVisible(d->hostMode.canChangeAnyGameRules());
	d->labelMap->setVisible(d->hostMode.canChangeAnyGameRules());
	d->mapPanel->setVisible(d->hostMode.canChangeAnyGameRules());

	if (d->hostMode.canChangeAnyGameRules())
	{
		d->formLayout->insertRow(d->gamemodeRow, d->labelGamemode, d->cboGamemode);
		d->formLayout->insertRow(d->difficultyRow, d->labelDifficulty, d->cboDifficulty);
		d->formLayout->insertRow(d->mapRow, d->labelMap, d->mapPanel);
	}
	else
	{
		d->formLayout->removeWidget(d->labelGamemode);
		d->formLayout->removeWidget(d->cboGamemode);
		d->formLayout->removeWidget(d->labelDifficulty);
		d->formLayout->removeWidget(d->cboDifficulty);
		d->formLayout->removeWidget(d->labelMap);
		d->formLayout->removeWidget(d->mapPanel);
	}
}

void GeneralGameSetupPanel::setCreateServerDialog(CreateServerDialog *dialog)
{
	d->parentDialog = dialog;
}

void GeneralGameSetupPanel::setDemoPath(const QString &demoPath)
{
	d->demoPicker->setPath(demoPath);
}

void GeneralGameSetupPanel::setIwadByName(const QString &iwad)
{
	d->iwadSetExplicitly = true;
	d->iwadPicker->setIwadByName(iwad);
}

void GeneralGameSetupPanel::setIwadByPath(const QString &iwadPath)
{
	d->iwadSetExplicitly = true;
	d->iwadPicker->setIwadByPath(iwadPath);
}

void GeneralGameSetupPanel::setPwads(const QList<PickedGameFile> &pwads)
{
	d->pwadsSetExplicitly = true;
	d->wadsPicker->setFiles(pwads);
}

void GeneralGameSetupPanel::showEvent(QShowEvent *event)
{
	Q_UNUSED(event);
	updateMapWarningVisibility();
}

QString GeneralGameSetupPanel::mapName() const
{
	return d->leMap->text();
}

QString GeneralGameSetupPanel::pathToExe()
{
	switch (d->hostMode.mode)
	{
	case GameCreateParams::Demo: return d->demoPlayerExecutableInput->path();
	default:
	case GameCreateParams::Host: return d->hostExecutableInput->path();
	case GameCreateParams::Offline: return d->offlineExecutableInput->path();
	case GameCreateParams::Remote: return d->remoteExecutableInput->path();
	}
}

void GeneralGameSetupPanel::onGameModeChanged(int index)
{
	if (index >= 0)
		emit gameModeChanged(currentGameMode());
}

GameMode GeneralGameSetupPanel::currentGameMode() const
{
	QList<GameMode> gameModes = d->currentEngine->gameModes();
	for (const GameMode &mode : gameModes)
	{
		if (mode.index() == d->cboGamemode->itemData(d->cboGamemode->currentIndex()).toInt())
			return mode;
	}
	return GameMode::mkUnknown();
}

EnginePlugin *GeneralGameSetupPanel::currentPlugin() const
{
	return d->cboEngine->currentPlugin();
}

bool GeneralGameSetupPanel::setEngine(const QString &engineName)
{
	if (!d->cboEngine->setPluginByName(engineName))
	{
		QMessageBox::critical(this, tr("Doomseeker - load server config"),
			tr("Plugin for engine \"%1\" is not present!").arg(engineName));
		return false;
	}
	return true;
}

void GeneralGameSetupPanel::updateMapWarningVisibility()
{
	assert(d->parentDialog != nullptr);
	MapListPanel *mapList = d->parentDialog->mapListPanel();
	d->lblMapWarning->setVisible(mapList->hasMaps() && !mapList->isMapOnList(mapName()));
}
