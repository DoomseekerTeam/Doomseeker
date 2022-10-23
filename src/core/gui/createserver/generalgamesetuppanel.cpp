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
#include "filefilter.h"
#include "gui/createserver/maplistpanel.h"
#include "gui/createserverdialog.h"
#include "ini/ini.h"
#include "plugins/engineplugin.h"
#include "plugins/pluginloader.h"
#include "serverapi/gamecreateparams.h"
#include "serverapi/gameexefactory.h"
#include "serverapi/gamefile.h"
#include <cassert>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QTimer>

DClass<GeneralGameSetupPanel> : public Ui::GeneralGameSetupPanel
{
public:
	EnginePlugin *currentEngine;
	GameCreateParams::HostMode hostMode;
	bool iwadSetExplicitly;
	CreateServerDialog *parentDialog;

	int serverNameRow;
	int portRow;
	int hostModeSpacerRow;

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
	d->hostMode = GameCreateParams::Host;
	d->iwadSetExplicitly = false;
	d->parentDialog = nullptr;

	d->hostExecutableInput->setAllowedExecutables(GameFile::Server);
	d->offlineExecutableInput->setAllowedExecutables(GameFile::Offline);
	d->remoteExecutableInput->setAllowedExecutables(GameFile::Client);

	d->formLayout->getWidgetPosition(d->leServerName, &d->serverNameRow, nullptr);
	d->formLayout->getWidgetPosition(d->portArea, &d->portRow, nullptr);
	d->formLayout->getWidgetPosition(d->hostModeSpacer, &d->hostModeSpacerRow, nullptr);

	this->connect(d->cboEngine, SIGNAL(currentPluginChanged(EnginePlugin*)),
		SIGNAL(pluginChanged(EnginePlugin*)));
}

GeneralGameSetupPanel::~GeneralGameSetupPanel()
{
}

QStringList GeneralGameSetupPanel::getAllWadPaths() const
{
	QStringList paths;
	paths << d->iwadPicker->currentIwad() << d->wadsPicker->filePaths();
	return paths;
}

void GeneralGameSetupPanel::fillInParams(GameCreateParams &params)
{
	params.setExecutablePath(pathToExe());
	params.setIwadPath(d->iwadPicker->currentIwad());
	params.setLoggingPath(d->logDirectoryPicker->validatedCurrentPath());
	params.setPwadsPaths(d->wadsPicker->filePaths());
	params.setPwadsOptional(d->wadsPicker->fileOptional());
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
	if (d->hostMode != GameCreateParams::Remote)
	{
		if (!setEngine(configEngineName))
			return;
		engineChanged = (prevEngine != d->currentEngine);
	}

	// Executables
	bool changeExecutable = engineChanged || !d->cbLockExecutable->isChecked();
	if (d->hostMode == GameCreateParams::Remote)
	{
		// If we're configuring a remote game the engine cannot be changed.
		// Don't substitute the executable when the config that is being
		// loaded is for a different game.
		auto *plugin = gPlugins->plugin(gPlugins->pluginIndexFromName(configEngineName));
		if (plugin == nullptr || d->currentEngine != plugin->info())
			changeExecutable = false;
	}

	if (changeExecutable)
	{
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

	QList<bool> optionalWads;
	for (QString value : general["pwadsOptional"].valueString().split(";"))
	{
		optionalWads << (value != "0");
	}
	d->wadsPicker->setFilePaths(general["pwads"].valueString().split(";"), optionalWads);

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

	general["pwads"] = d->wadsPicker->filePaths().join(";");
	QList<bool> optionalWads = d->wadsPicker->fileOptional();
	QStringList optionalList;
	for (bool optional : optionalWads)
	{
		optionalList << (optional ? "1" : "0");
	}
	general["pwadsOptional"] = optionalList.join(";");

	general["broadcastToLAN"] = d->cbBroadcastToLAN->isChecked();
	general["broadcastToMaster"] = d->cbBroadcastToMaster->isChecked();
	general["upnp"] = d->cbUpnp->isChecked();
	general["upnpPort"] = d->spinUpnpPort->value();
}

void GeneralGameSetupPanel::reloadAppConfig()
{
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

	d->cboEngine->setDisabled(hostMode == GameCreateParams::Remote);

	d->hostExecutableInput->hide();
	d->offlineExecutableInput->hide();
	d->remoteExecutableInput->hide();
	switch (hostMode)
	{
	default:
	case GameCreateParams::Host: d->hostExecutableInput->show(); break;
	case GameCreateParams::Offline: d->offlineExecutableInput->show(); break;
	case GameCreateParams::Remote: d->remoteExecutableInput->show(); break;
	}

	d->labelServerName->setVisible(hostMode == GameCreateParams::Host);
	d->leServerName->setVisible(hostMode == GameCreateParams::Host);
	d->labelPort->setVisible(hostMode == GameCreateParams::Host);
	d->portArea->setVisible(hostMode == GameCreateParams::Host);
	d->hostModeSpacer->setVisible(hostMode == GameCreateParams::Host);
	d->broadcastOptionsArea->setVisible(hostMode == GameCreateParams::Host);

	// Insert/remove operations are necessary to get rid of extra
	// spacing remaining when hiding form elements.
	if (hostMode == GameCreateParams::Host)
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
}

void GeneralGameSetupPanel::setCreateServerDialog(CreateServerDialog *dialog)
{
	d->parentDialog = dialog;
}

void GeneralGameSetupPanel::setIwadByName(const QString &iwad)
{
	d->iwadSetExplicitly = true;
	d->iwadPicker->setIwadByName(iwad);
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
	switch (d->hostMode)
	{
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
