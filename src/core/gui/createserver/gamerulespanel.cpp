//------------------------------------------------------------------------------
// gamerulespanel.cpp
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
#include "gamerulespanel.h"
#include "ui_gamerulespanel.h"

#include "gui/createserver/gamecvaredit.h"
#include "ini/ini.h"
#include "plugins/engineplugin.h"
#include "serverapi/gamecreateparams.h"
#include "serverapi/serverstructs.h"

#include <QVariant>
#include <memory>

namespace
{
struct CVarUi
{
	std::shared_ptr<QLabel> label;
	std::shared_ptr<GameCVarEdit> edit;

	QString command() const { return cvar().command(); }
	GameCVar cvar() const { return edit->cvar(); }
	QVariant value() const { return edit->value(); }
	void setValue(const QVariant &value) { edit->setValue(value); }
};
}

DClass<GameRulesPanel> : public Ui::GameRulesPanel
{
public:
	bool anythingAvailable;
	const EnginePlugin *engine;
	GameCreateParams::HostMode hostMode;
	GameMode gameMode;
	QList<GameCVar> gameModifiers;
	QList<std::shared_ptr<CVarUi> > cvarWidgets;
	QMap<QString, QMap<QString, QVariant> > memorizedCVars;
};

DPointered(GameRulesPanel)

GameRulesPanel::GameRulesPanel(QWidget *parent)
	: QWidget(parent)
{
	d->setupUi(this);
	d->anythingAvailable = true;
	d->engine = nullptr;
	d->hostMode = GameCreateParams::Host;
}

GameRulesPanel::~GameRulesPanel()
{
	d->cvarWidgets.clear();
}

void GameRulesPanel::applyModeToUi()
{
	bool engineAllowsSlotsLimits = false;
	bool engineHasModifiers = false;
	bool engineHasMapList = false;

	if (d->engine != nullptr)
	{
		setupModifiers(d->engine);
		d->mapListPanel->setupForEngine(d->engine);
		setupCVarWidgets(d->engine, d->gameMode);

		auto ecfg = d->engine->data();

		d->labelMaxClients->setVisible(ecfg->allowsClientSlots);
		d->spinMaxClients->setVisible(ecfg->allowsClientSlots);
		d->labelMaxPlayers->setVisible(ecfg->allowsPlayerSlots);
		d->spinMaxPlayers->setVisible(ecfg->allowsPlayerSlots);

		// Remove hidden widgets to prevent creating extra spacing
		// in the form layout.
		d->hostingLimitsLayout->removeWidget(d->labelMaxClients);
		d->hostingLimitsLayout->removeWidget(d->spinMaxClients);
		d->hostingLimitsLayout->removeWidget(d->labelMaxPlayers);
		d->hostingLimitsLayout->removeWidget(d->spinMaxPlayers);

		if (ecfg->allowsClientSlots)
			d->hostingLimitsLayout->addRow(d->labelMaxClients, d->spinMaxClients);
		if (ecfg->allowsPlayerSlots)
			d->hostingLimitsLayout->addRow(d->labelMaxPlayers, d->spinMaxPlayers);

		engineAllowsSlotsLimits = ecfg->allowsClientSlots || ecfg->allowsPlayerSlots;
		engineHasMapList = ecfg->hasMapList;
		engineHasModifiers = !d->engine->gameModifiers().isEmpty();
	}

	d->extraSettingsBox->setVisible(!d->cvarWidgets.isEmpty());

	bool slotLimitsBoxAvailable = d->hostMode == GameCreateParams::Host && engineAllowsSlotsLimits;
	d->hostLimitsBox->setVisible(slotLimitsBoxAvailable);
	d->mapListBox->setVisible(engineHasMapList);

	d->anythingAvailable = !d->cvarWidgets.isEmpty()
		|| engineHasModifiers
		|| engineHasMapList
		|| slotLimitsBoxAvailable;
}

void GameRulesPanel::fillInParams(GameCreateParams &params)
{
	params.setMaxClients(d->spinMaxClients->value());
	params.setMaxPlayers(d->spinMaxPlayers->value());

	fillInCVars(params);
	fillInModifiers(params);

	d->mapListPanel->fillInParams(params);
}

void GameRulesPanel::fillInCVars(GameCreateParams &params)
{
	for (auto &cvarUi : d->cvarWidgets)
	{
		params.cvars() << cvarUi->cvar();
	}
}

void GameRulesPanel::fillInModifiers(GameCreateParams &params)
{
	int modIndex = d->cboModifier->currentIndex();
	if (modIndex > 0) // Index zero is always "< NONE >"
	{
		--modIndex;
		d->gameModifiers[modIndex].setValue(1);
		params.cvars() << d->gameModifiers[modIndex];
	}
}

bool GameRulesPanel::isAnythingAvailable() const
{
	return d->anythingAvailable;
}

MapListPanel *GameRulesPanel::mapListPanel()
{
	return d->mapListPanel;
}

void GameRulesPanel::memorizeCVars()
{
	if (d->engine != nullptr)
	{
		const QString &engineName = d->engine->nameCanonical();
		QMap<QString, QVariant> &cvars = d->memorizedCVars[engineName];
		for (auto &cvarUi : d->cvarWidgets)
			cvars[cvarUi->command()] = cvarUi->value();
	}
}

void GameRulesPanel::loadMemorizedCVars(const EnginePlugin *engine)
{
	if (d->memorizedCVars.contains(engine->nameCanonical()))
	{
		const QMap<QString, QVariant> &cvars = d->memorizedCVars[engine->nameCanonical()];
		for (auto &cvarUi : d->cvarWidgets)
		{
			if (cvars.contains(cvarUi->command()))
				cvarUi->setValue(cvars[cvarUi->command()]);
		}
	}
}

void GameRulesPanel::loadConfig(Ini &config)
{
	IniSection section = config.section("Rules");

	d->cboModifier->setCurrentIndex(section["modifier"]);
	d->spinMaxClients->setValue(section["maxClients"]);
	d->spinMaxPlayers->setValue(section["maxPlayers"]);
	for (auto &cvarUi : d->cvarWidgets)
	{
		cvarUi->setValue(section[cvarUi->command()].value());
	}

	d->mapListPanel->loadConfig(config);
}

void GameRulesPanel::saveConfig(Ini &config)
{
	IniSection section = config.section("Rules");

	section["modifier"] = d->cboModifier->currentIndex();
	section["maxClients"] = d->spinMaxClients->value();
	section["maxPlayers"] = d->spinMaxPlayers->value();
	for (auto &cvarUi : d->cvarWidgets)
	{
		section[cvarUi->command()].setValue(cvarUi->value());
	}

	d->mapListPanel->saveConfig(config);
}

void GameRulesPanel::setCreateServerDialog(CreateServerDialog *dialog)
{
	d->mapListPanel->setCreateServerDialog(dialog);
}

void GameRulesPanel::setupForEngine(const EnginePlugin *engine, const GameMode &gameMode)
{
	d->engine = engine;
	d->gameMode = gameMode;
	applyModeToUi();
}

void GameRulesPanel::setupForHostMode(GameCreateParams::HostMode hostMode)
{
	d->hostMode = hostMode;
	applyModeToUi();
}

void GameRulesPanel::setupModifiers(const EnginePlugin *engine)
{
	QString selectedModifier = d->cboModifier->currentText();
	d->cboModifier->clear();
	d->gameModifiers.clear();

	QList<GameCVar> modifiers = engine->gameModifiers();

	if (!modifiers.isEmpty())
	{
		d->modifierBox->show();

		d->cboModifier->addItem(tr("< NONE >"));

		for (const GameCVar &cvar : modifiers)
		{
			d->cboModifier->addItem(cvar.name());
			d->gameModifiers << cvar;
		}
		if (!selectedModifier.isEmpty())
		{
			int modifierIndex = d->cboModifier->findText(selectedModifier);
			if (modifierIndex >= 0)
				d->cboModifier->setCurrentIndex(modifierIndex);
		}
	}
	else
	{
		d->modifierBox->hide();
	}
}

void GameRulesPanel::setupCVarWidgets(const EnginePlugin *engine, const GameMode &gameMode)
{
	memorizeCVars();

	d->cvarWidgets.clear();
	QList<GameCVar> cvars = engine->limits(gameMode);

	for (const GameCVar &cvar : cvars)
	{
		std::shared_ptr<CVarUi> cvarUi(new CVarUi);
		cvarUi->edit.reset(new GameCVarEdit(cvar, this));
		if (cvarUi->edit->externalLabel())
		{
			cvarUi->label.reset(new QLabel(this));
			cvarUi->label->setText(cvar.name());
		}

		d->extraSettingsLayout->addRow(cvarUi->label.get(), cvarUi->edit.get());
		d->cvarWidgets << std::move(cvarUi);
	}

	loadMemorizedCVars(engine);
}
