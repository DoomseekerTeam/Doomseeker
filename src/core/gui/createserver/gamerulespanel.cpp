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

#include "ini/ini.h"
#include "plugins/engineplugin.h"
#include "serverapi/gamecreateparams.h"
#include "serverapi/serverstructs.h"

#include <climits>
#include <QVariant>

DClass<GameRulesPanel> : public Ui::GameRulesPanel
{
public:
	class GameLimitWidget
	{
	public:
		QWidget *label;
		QSpinBox *spinBox;
		GameCVar limit;
	};

	bool anythingAvailable;
	const EnginePlugin *engine;
	GameCreateParams::HostMode hostMode;
	GameMode gameMode;
	QList<GameCVar> gameModifiers;
	QList<GameLimitWidget *> limitWidgets;
	QMap<QString, QMap<QString, int> > memorizedLimits;
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
	qDeleteAll(d->limitWidgets);
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
		setupLimitWidgets(d->engine, d->gameMode);

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

	d->gameLimitsBox->setVisible(!d->limitWidgets.isEmpty());

	bool slotLimitsBoxAvailable = d->hostMode == GameCreateParams::Host && engineAllowsSlotsLimits;
	d->hostLimitsBox->setVisible(slotLimitsBoxAvailable);
	d->mapListBox->setVisible(engineHasMapList);

	d->anythingAvailable = !d->limitWidgets.isEmpty()
		|| engineHasModifiers
		|| engineHasMapList
		|| slotLimitsBoxAvailable;
}

void GameRulesPanel::fillInParams(GameCreateParams &params)
{
	params.setMaxClients(d->spinMaxClients->value());
	params.setMaxPlayers(d->spinMaxPlayers->value());

	fillInLimits(params);
	fillInModifiers(params);

	d->mapListPanel->fillInParams(params);
}

void GameRulesPanel::fillInLimits(GameCreateParams &params)
{
	for (PrivData<GameRulesPanel>::GameLimitWidget *p : d->limitWidgets)
	{
		p->limit.setValue(p->spinBox->value());
		params.cvars() << p->limit;
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

void GameRulesPanel::memorizeLimits()
{
	if (d->engine != nullptr)
	{
		if (!d->memorizedLimits.contains(d->engine->nameCanonical()))
			d->memorizedLimits[d->engine->nameCanonical()] = QMap<QString, int>();
		QMap<QString, int> &limits = d->memorizedLimits[d->engine->nameCanonical()];
		for (const PrivData<GameRulesPanel>::GameLimitWidget *limitWidget : d->limitWidgets)
		{
			limits[limitWidget->limit.command()] = limitWidget->spinBox->value();
		}
	}
}

void GameRulesPanel::loadMemorizedLimits(const EnginePlugin *engine)
{
	if (d->memorizedLimits.contains(engine->nameCanonical()))
	{
		QMap<QString, int> &limits = d->memorizedLimits[engine->nameCanonical()];
		for (const PrivData<GameRulesPanel>::GameLimitWidget *limitWidget : d->limitWidgets)
		{
			if (limits.contains(limitWidget->limit.command()))
				limitWidget->spinBox->setValue(limits[limitWidget->limit.command()]);
		}
	}
}

void GameRulesPanel::loadConfig(Ini &config)
{
	IniSection section = config.section("Rules");

	d->cboModifier->setCurrentIndex(section["modifier"]);
	d->spinMaxClients->setValue(section["maxClients"]);
	d->spinMaxPlayers->setValue(section["maxPlayers"]);
	for (PrivData<GameRulesPanel>::GameLimitWidget *widget : d->limitWidgets)
	{
		widget->spinBox->setValue(section[widget->limit.command()]);
	}

	d->mapListPanel->loadConfig(config);
}

void GameRulesPanel::saveConfig(Ini &config)
{
	IniSection section = config.section("Rules");

	section["modifier"] = d->cboModifier->currentIndex();
	section["maxClients"] = d->spinMaxClients->value();
	section["maxPlayers"] = d->spinMaxPlayers->value();
	for (PrivData<GameRulesPanel>::GameLimitWidget *widget : d->limitWidgets)
	{
		section[widget->limit.command()] = widget->spinBox->value();
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

void GameRulesPanel::removeLimitWidgets()
{
	for (PrivData<GameRulesPanel>::GameLimitWidget *widget : d->limitWidgets)
	{
		delete widget->label;
		delete widget->spinBox;
		delete widget;
	}

	d->limitWidgets.clear();
}

void GameRulesPanel::setupLimitWidgets(const EnginePlugin *engine, const GameMode &gameMode)
{
	memorizeLimits();
	removeLimitWidgets();
	QList<GameCVar> limits = engine->limits(gameMode);

	for (const GameCVar &limit : limits)
	{
		QLabel *label = new QLabel(this);
		label->setText(limit.name());
		auto spinBox = new QSpinBox(this);
		spinBox->setMaximum(INT_MAX);
		spinBox->setMinimum(INT_MIN);
		spinBox->setCorrectionMode(QAbstractSpinBox::CorrectToNearestValue);
		spinBox->setValue(limit.value().toInt());

		d->limitsLayout->addRow(label, spinBox);

		auto glw = new PrivData<GameRulesPanel>::GameLimitWidget();
		glw->label = label;
		glw->spinBox = spinBox;
		glw->limit = limit;
		d->limitWidgets << glw;
	}

	loadMemorizedLimits(engine);
}
