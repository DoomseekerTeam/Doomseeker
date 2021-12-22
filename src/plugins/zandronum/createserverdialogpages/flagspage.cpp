//------------------------------------------------------------------------------
// flagspage.cpp
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
#include "flagspage.h"

#include <ini/ini.h>
#include <ini/inisection.h>
#include <log.h>
#include <QDebug>
#include <QSharedPointer>
#include <QValidator>
#include <serverapi/gamecreateparams.h>

#include "createserverdialogpages/flagsid.h"
#include "createserverdialogpages/flagspagevaluecontroller2.h"
#include "createserverdialogpages/flagspagevaluecontroller3.h"
#include "createserverdialogpages/votingsetupwidget.h"
#include "zandronumgameinfo.h"
#include <cstdint>

const int32_t DEFAULT_LMSALLOWEDWEAPONS = 1023;
const int32_t DEFAULT_LMSSPECTATORSETTINGS = 3;

class DmflagsValidator : public QValidator
{
public:
	void fixup(QString &input) const override
	{
		if (input.trimmed().isEmpty())
		{
			input = "0";
		}
	}

	State validate(QString &input, int &pos) const override
	{
		Q_UNUSED(pos);
		if (input.trimmed().isEmpty() || input.trimmed() == "-")
		{
			return QValidator::Intermediate;
		}

		bool bOk;
		input.toInt(&bOk);
		return bOk ? QValidator::Acceptable : QValidator::Invalid;
	}
};
////////////////////////////////////////////////////////////////////////////////
class FlagsPage::PrivData
{
public:
	DmflagsValidator validator;
	QSharedPointer<FlagsPageValueController> flagsController;
};

////////////////////////////////////////////////////////////////////////////////
FlagsPage::FlagsPage(CreateServerDialog *pParentDialog)
	: CreateServerDialogPage(pParentDialog, tr("Zandronum"))
{
	setupUi(this);

	d = new PrivData();

	setTabOrder(tabWidget, votingPage);
	setTabOrder(votingPage, leDmflags);

	FlagsId flagsId(this);
	flagsId.assign();

	// The validator makes sure that the line edit will accept only
	// 32-bit signed values.
	leDmflags->setValidator(&d->validator);
	leDmflags2->setValidator(&d->validator);
	leZandronumDmflags->setValidator(&d->validator);
	leCompatflags->setValidator(&d->validator);
	leCompatflags2->setValidator(&d->validator);
	leZandronumCompatflags->setValidator(&d->validator);
	leLMSAllowedWeapons->setValidator(&d->validator);
	leLMSSpectatorSettings->setValidator(&d->validator);

	// Version-specific widgets should all be invisible at this stage.
	Zandronum2::FlagsPageValueController(this).setVisible(false);
	Zandronum3::FlagsPageValueController(this).setVisible(false);

	// Init values for widgets.
	cboGameVersion->addItem(tr("Zandronum 3"), ZandronumGameInfo::GV_Zandronum3);
	cboGameVersion->addItem(tr("Zandronum 2 (old)"), ZandronumGameInfo::GV_Zandronum2);

	cboFallingDamage->insertItem(FDT_None, tr("None"));
	cboFallingDamage->insertItem(FDT_Old, tr("Old (ZDoom)"));
	cboFallingDamage->insertItem(FDT_Hexen, tr("Hexen"));
	cboFallingDamage->insertItem(FDT_Strife, tr("Strife"));
	cboFallingDamage->setCurrentIndex(FDT_None);

	initJumpCrouchComboBoxes(cboJumping);
	initJumpCrouchComboBoxes(cboCrouching);

	cboPlayerBlock->addItem(tr("Players block each other normally"), PB_Block);
	cboPlayerBlock->addItem(tr("Players can walk through each other"), PB_Noclip);
	cboPlayerBlock->addItem(tr("Allies can walk through each other"), PB_AllyNoclip);

	cboPrivateChat->addItem(tr("Allowed"), PrivChat_Allowed);
	cboPrivateChat->addItem(tr("Only team"), PrivChat_Team);
	cboPrivateChat->addItem(tr("Forbidden"), PrivChat_Forbidden);

	setGameVersion(ZandronumGameInfo::GV_Zandronum3);

	// Widget states
	spinMonsterKillPercentage->setEnabled(false);
}

FlagsPage::~FlagsPage()
{
	delete d;
}

void FlagsPage::applyWidgetsChange()
{
	if (d->flagsController != nullptr)
		d->flagsController->convertWidgetsToNumerical();
}

void FlagsPage::fillInGameCreateParams(GameCreateParams &gameCreateParams)
{
	QStringList params;

	QList<std::pair<QString, QString>> flags;
	flags << std::make_pair("dmflags", leDmflags->text());
	flags << std::make_pair("dmflags2", leDmflags2->text());
	flags << std::make_pair("zadmflags", leZandronumDmflags->text());
	flags << std::make_pair("compatflags", leCompatflags->text());
	if (leCompatflags2->isEnabled() && leCompatflags2->isVisible())
		flags << std::make_pair("compatflags2", leCompatflags2->text());
	flags << std::make_pair("zacompatflags", leZandronumCompatflags->text());
	flags << std::make_pair("lmsallowedweapons", leLMSAllowedWeapons->text());
	flags << std::make_pair("lmsspectatorsettings", leLMSSpectatorSettings->text());

	for (auto flag : flags)
	{
		const QString name = flag.first;
		const QString valueAsStr = flag.second;
		bool ok = false;
		const int32_t value = valueAsStr.toLong(&ok);
		if (!ok)
			continue;

		if (value >= 0)
		{
			params << QString("+%1").arg(name) << valueAsStr;
		}
		else
		{
			params << QString("+%1 %2").arg(name).arg(value);
		}
	}

	if (cbMonstersMustBeKilledToExit->isChecked())
	{
		params << "+sv_killallmonsters_percentage"
			<< QString::number(spinMonsterKillPercentage->value());
	}
	params << "+sv_afk2spec" <<
		QString::number(spinForceInactivePlayersSpectatingMins->value());
	params << "+sv_coop_damagefactor" <<
		QString::number(spinMonstersDamageFactor->value());

	if (cbRespawnDelayEnabled->isChecked())
		params << "+sv_respawndelaytime" << QString::number(spinRespawnDelay->value());

	params << "+sv_defaultdmflags" << (cbDefaultDmflags->isChecked() ? "1" : "0");
	if (gameVersion() == ZandronumGameInfo::GV_Zandronum3)
	{
		params << "+sv_allowprivatechat" << cboPrivateChat->currentData().toString();
	}

	if (!leServerCountry->text().trimmed().isEmpty())
		params << "+sv_country" << leServerCountry->text().trimmed();

	params << votingPage->generateGameRunParameters();

	gameCreateParams.setOption("GameVersion", gameVersion());
	gameCreateParams.customParameters() << params;
}

void FlagsPage::initJumpCrouchComboBoxes(QComboBox *pComboBox)
{
	pComboBox->insertItem(JCA_Default, tr("Default"));
	pComboBox->insertItem(JCA_No, tr("No"));
	pComboBox->insertItem(JCA_Yes, tr("Yes"));
}

void FlagsPage::insertFlagsIfValid(QLineEdit *dst, QString flags, int32_t valIfInvalid)
{
	int dummy;
	if (d->validator.validate(flags, dummy) == QValidator::Acceptable)
	{
		dst->setText(flags);
	}
	else
	{
		dst->setText(QString::number(valIfInvalid));
	}
}

bool FlagsPage::loadConfig(Ini &ini)
{
	IniSection section = ini.section("dmflags");
	loadGameVersion(static_cast<ZandronumGameInfo::GameVersion>((int)section["gameversion"]));

	// The below numerical flag inserts are here to support old configs.
	insertFlagsIfValid(leDmflags, section["dmflags"]);
	insertFlagsIfValid(leDmflags2, section["dmflags2"]);
	insertFlagsIfValid(leZandronumDmflags, section["zandronumDmflags"]);
	insertFlagsIfValid(leCompatflags, section["compatflags"]);
	insertFlagsIfValid(leZandronumCompatflags, section["zandronumCompatflags"]);
	insertFlagsIfValid(leLMSAllowedWeapons, section["lmsallowedweapons"], DEFAULT_LMSALLOWEDWEAPONS);
	insertFlagsIfValid(leLMSSpectatorSettings, section["lmsspectatorsettings"], DEFAULT_LMSSPECTATORSETTINGS);
	propagateFlagsInputsChanges();
	// End of old config support.

	FlagsId flagsId(this);
	flagsId.load(section);

	IniVariable varKillMonstersPercentage = section["killmonsters_percentage"];
	if (!varKillMonstersPercentage.value().isNull())
	{
		spinMonsterKillPercentage->setValue(varKillMonstersPercentage);
	}
	IniVariable varForceInactivePlayersSpectatingMins = section["force_inactive_players_spectating_mins"];
	if (!varForceInactivePlayersSpectatingMins.value().isNull())
	{
		spinForceInactivePlayersSpectatingMins->setValue(varForceInactivePlayersSpectatingMins);
	}
	IniVariable varRespawnDelaySecs = section["respawn_delay_secs"];
	cbRespawnDelayEnabled->setChecked(!varRespawnDelaySecs.value().isNull());
	if (!varRespawnDelaySecs.value().isNull())
	{
		spinRespawnDelay->setValue(varRespawnDelaySecs);
	}
	IniVariable varMonstersDamageFactor = section["monsters_damage_factor"];
	if (!varMonstersDamageFactor.value().isNull())
	{
		spinMonstersDamageFactor->setValue(varMonstersDamageFactor);
	}
	IniVariable varPrivateChat = section["private_chat"];
	if (!varPrivateChat.value().isNull())
	{
		const int idx = cboPrivateChat->findData(varPrivateChat.value().toInt());
		if (idx >= 0)
		{
			cboPrivateChat->setCurrentIndex(idx);
		}
	}

	if (section.hasSetting("falling_damage_type"))
		cboFallingDamage->setCurrentIndex(section["falling_damage_type"]);
	if (section.hasSetting("jump_ability"))
		cboJumping->setCurrentIndex(section["jump_ability"]);
	if (section.hasSetting("crouch_ability"))
		cboCrouching->setCurrentIndex(section["crouch_ability"]);
	if (section.hasSetting("country"))
		leServerCountry->setText(section["country"]);
	setPlayerBlock(static_cast<PlayerBlock>(section.value("player_block", PB_NotSet).toInt()));
	setLevelExit(static_cast<LevelExit>(section.value("level_exit", EXIT_NotSet).toInt()));

	cbDefaultDmflags->setChecked((bool)section["defaultdmflags"]);

	bool loaded = votingPage->loadConfig(ini);
	applyWidgetsChange();
	return loaded;
}

void FlagsPage::propagateFlagsInputsChanges()
{
	if (d->flagsController != nullptr)
		d->flagsController->convertNumericalToWidgets();
}

bool FlagsPage::saveConfig(Ini &ini)
{
	IniSection section = ini.section("dmflags");

	// Remove obsolete settings that were created by old method of saving
	// dmflags. That way the subsequent loads of this file will not trigger
	// the backward-compatibility fallbacks.
	QStringList oldSettings;
	oldSettings << "dmflags" << "dmflags2" << "zandronumDmflags" << "compatflags"
		<< "zandronumCompatflags" << "lmsallowedweapons" << "lmsspectatorsettings";
	for (const QString &oldSetting : oldSettings)
	{
		section.deleteSetting(oldSetting);
	}

	FlagsId flagsId(this);
	flagsId.save(section);

	section["gameversion"] = cboGameVersion->itemData(cboGameVersion->currentIndex()).toInt();
	section["defaultdmflags"] = cbDefaultDmflags->isChecked();
	section["falling_damage_type"] = cboFallingDamage->currentIndex();
	section["jump_ability"] = cboJumping->currentIndex();
	section["crouch_ability"] = cboCrouching->currentIndex();
	if (playerBlock() != PB_NotSet)
		section["player_block"] = playerBlock();
	if (levelExit() != EXIT_NotSet)
		section["level_exit"] = levelExit();

	section["killmonsters_percentage"] = spinMonsterKillPercentage->value();
	section["force_inactive_players_spectating_mins"] =
		spinForceInactivePlayersSpectatingMins->value();
	if (cbRespawnDelayEnabled->isChecked())
		section["respawn_delay_secs"] = spinRespawnDelay->value();
	section["monsters_damage_factor"] =
		static_cast<float>(spinMonstersDamageFactor->value());
	section["private_chat"] = cboPrivateChat->currentData().toInt();
	if (!leServerCountry->text().trimmed().isEmpty())
		section["country"] = leServerCountry->text().trimmed();

	return votingPage->saveConfig(ini);
}

void FlagsPage::applyGameVersion()
{
	setGameVersion(static_cast<ZandronumGameInfo::GameVersion>(
		cboGameVersion->itemData(cboGameVersion->currentIndex()).toInt()));
}

void FlagsPage::loadGameVersion(ZandronumGameInfo::GameVersion version)
{
	int index = cboGameVersion->findData(version);
	if (index < 0)
	{
		gLog << tr("Unknown Zandronum version in the config. Reverting to default.");
		version = DEFAULT_GAME_VERSION;
		index = cboGameVersion->findData(version);
		if (index < 0)
		{
			gLog << QString("Zandronum: FlagsPage::loadGameVersion() - oops, "
					"a bug!, GameVersion = %1").arg(version);
			return;
		}
	}
	setGameVersion(version);
}

void FlagsPage::setGameVersion(ZandronumGameInfo::GameVersion version)
{
	cboGameVersion->blockSignals(true);
	int index = cboGameVersion->findData(version);
	if (index >= 0)
		cboGameVersion->setCurrentIndex(index);
	cboGameVersion->blockSignals(false);
	if (d->flagsController != nullptr)
	{
		d->flagsController->setVisible(false);
	}
	switch (version)
	{
	default:
		gLog << tr("Tried to set unknown Zandronum version. Reverting to default."); // intentional fall-through
	case ZandronumGameInfo::GV_Zandronum2:
		d->flagsController = QSharedPointer<FlagsPageValueController>(
			new Zandronum2::FlagsPageValueController(this));
		break;
	case ZandronumGameInfo::GV_Zandronum3:
		d->flagsController = QSharedPointer<FlagsPageValueController>(
			new Zandronum3::FlagsPageValueController(this));
		break;
	}
	d->flagsController->setVisible(true);
	d->flagsController->convertWidgetsToNumerical();
}

ZandronumGameInfo::GameVersion FlagsPage::gameVersion() const
{
	#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
	return static_cast<ZandronumGameInfo::GameVersion>(cboGameVersion->currentData().toInt());
	#else
	return static_cast<ZandronumGameInfo::GameVersion>(cboGameVersion->itemData(cboGameVersion->currentIndex()).toInt());
	#endif
}

FlagsPage::PlayerBlock FlagsPage::playerBlock() const
{
	return static_cast<FlagsPage::PlayerBlock>(cboPlayerBlock->currentData().toInt());
}

void FlagsPage::setPlayerBlock(PlayerBlock playerBlock)
{
	const int idx = cboPlayerBlock->findData(static_cast<int>(playerBlock));
	if (idx >= 0)
	{
		cboPlayerBlock->setCurrentIndex(idx);
	}
	else
	{
		cboPlayerBlock->setCurrentIndex(cboPlayerBlock->findData(PB_Block));
	}
}

FlagsPage::LevelExit FlagsPage::levelExit() const
{
	if (rbContinueToTheNextMap->isChecked())
		return EXIT_NextMap;
	if (rbRestartTheCurrentLevel->isChecked())
		return EXIT_RestartMap;
	if (rbKillThePlayer->isChecked())
		return EXIT_KillPlayer;
	return EXIT_NotSet;
}

void FlagsPage::setLevelExit(LevelExit levelExit)
{
	switch (levelExit)
	{
	case EXIT_NotSet:
		// do nothing
		break;
	case EXIT_NextMap:
		rbContinueToTheNextMap->setChecked(true);
		break;
	case EXIT_RestartMap:
		rbRestartTheCurrentLevel->setChecked(true);
		break;
	case EXIT_KillPlayer:
		rbKillThePlayer->setChecked(true);
		break;
	default:
		qDebug() << "FlagsPage::setLevelExit - unhandled LevelExit " << levelExit;
		break;
	}
}
