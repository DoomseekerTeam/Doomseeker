//------------------------------------------------------------------------------
// serverfilterdock.cpp
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
// Copyright (C) 2011 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "serverfilterdock.h"
#include "ui_serverfilterdock.h"

#include "configuration/doomseekerconfig.h"
#include "gui/commongui.h"
#include "gui/entity/serverlistfilterinfo.h"
#include "gui/icons.h"
#include "strings.hpp"

#include <QAction>
#include <QMessageBox>
#include <QTimer>

DClass<ServerFilterDock> : public Ui::ServerFilterDock
{
public:
	static const int CUSTOM_PRESET_IDX = 0;

	/**
	 * @brief Quick Search widget that is actually located outside the
	 * ServerFilterDock.
	 *
	 * ServerFilterDock needs to keep track of this widget in order to
	 * update the changing value appropriately.
	 */
	QLineEdit *leQuickSearch;
	/**
	 * Guard used to prevent multiple signals being generated while loading
	 * a filter with setFilterInfo.
	 */
	bool bDisableUpdate;

	void clearCustomPresets()
	{
		cboServerFilterPresets->blockSignals(true);
		for (int i = cboServerFilterPresets->count() - 1;
			 i > CUSTOM_PRESET_IDX;
			 --i)
		{
			cboServerFilterPresets->removeItem(i);
		}
		cboServerFilterPresets->blockSignals(false);
	}

	bool isCustomPreset(const ServerListFilterInfo &preset) const
	{
		return isCustomPresetName(preset.name);
	}

	bool isCustomPresetName(const QString &name) const
	{
		return name.isEmpty();
	}

	bool isCustomPresetIndex(int index) const
	{
		return index == CUSTOM_PRESET_IDX;
	}

	bool isCustomPresetSelected() const
	{
		return isCustomPresetIndex(cboServerFilterPresets->currentIndex());
	}

	ServerListFilterInfo presetAtIndex(int index) const
	{
		return ServerListFilterInfo::deserialize(cboServerFilterPresets->itemData(index));
	}

	ServerListFilterInfo currentPreset() const
	{
		return presetAtIndex(cboServerFilterPresets->currentIndex());
	}

	QString presetNameAtIndex(int index) const
	{
		return isCustomPresetIndex(index) ?
			QString() : presetAtIndex(index).name;
	}

	int presetIndex(const QString &name) const
	{
		if (isCustomPresetName(name))
			return CUSTOM_PRESET_IDX;
		for (int i = CUSTOM_PRESET_IDX + 1; i < cboServerFilterPresets->count(); ++i)
		{
			if (presetAtIndex(i).name.compare(name, Qt::CaseInsensitive) == 0)
			{
				return i;
			}
		}
		return -1;
	}

	QString currentPresetName() const
	{
		return presetNameAtIndex(cboServerFilterPresets->currentIndex());
	}
};

DPointeredNoCopy(ServerFilterDock)

ServerFilterDock::ServerFilterDock(QWidget *pParent)
	: QDockWidget(pParent)
{
	d->setupUi(this);
	d->leQuickSearch = nullptr;
	d->bDisableUpdate = false;
	d->cbGroupServersWithPlayersAtTop->setChecked(gConfig.doomseeker.bGroupServersWithPlayersAtTheTopOfTheList);

	d->btnClear->setIcon(Icons::clear());
	toggleViewAction()->setIcon(QIcon(":/icons/filter.png"));

	toggleViewAction()->setText(ServerFilterDock::tr("Server &filter"));
	toggleViewAction()->setShortcut(ServerFilterDock::tr("CTRL+F"));

	QTimer::singleShot(0, this, &ServerFilterDock::loadSettings);
}

ServerFilterDock::~ServerFilterDock()
{
}

void ServerFilterDock::addGameModeToComboBox(const QString &gameMode)
{
	addSortedNonDuplicate(d->cboGameMode, gameMode.trimmed());
	addSortedNonDuplicate(d->cboExcludeGameMode, gameMode.trimmed());
}

void ServerFilterDock::addSortedNonDuplicate(QComboBox *comboBox, const QString &text)
{
	if (comboBox->findText(text, Qt::MatchFixedString) < 0)
	{
		// Make sure combobox contents are sorted.
		for (int i = 0; i < comboBox->count(); ++i)
		{
			if (text < comboBox->itemText(i))
			{
				comboBox->insertItem(i, text);
				return;
			}
		}

		// The above routine didn't return.
		// This item belongs to the end of the list.
		comboBox->addItem(text);
	}
}

void ServerFilterDock::clear()
{
	setCustomPreset(ServerListFilterInfo());
	d->cboServerFilterPresets->setCurrentIndex(PrivData<ServerFilterDock>::CUSTOM_PRESET_IDX);
	this->setFilterInfo(ServerListFilterInfo());
}

QLineEdit *ServerFilterDock::createQuickSearch()
{
	if (d->leQuickSearch == nullptr)
	{
		auto qs = new QLineEdit();
		qs->setText(d->leServerName->text());

		connect(d->leServerName, SIGNAL(textEdited(const QString&)), qs, SLOT(setText(const QString&)));
		connect(qs, SIGNAL(textEdited(const QString&)), d->leServerName, SLOT(setText(const QString&)));

		d->leQuickSearch = qs;
	}

	return d->leQuickSearch;
}

void ServerFilterDock::emitUpdated()
{
	if (d->bDisableUpdate)
		return;

	ServerListFilterInfo filterInfo = this->filterInfo();
	if (d->isCustomPresetSelected())
	{
		setCustomPreset(filterInfo);
	}
	else
	{
		const ServerListFilterInfo currentPreset = d->currentPreset();
		d->cboServerFilterPresets->setItemText(d->cboServerFilterPresets->currentIndex(),
			currentPreset.isFilteringEquivalent(filterInfo) ?
				currentPreset.name :
				QString("%1 *").arg(currentPreset.name));
	}

	emit filterUpdated(filterInfo);
}

void ServerFilterDock::enableFilter()
{
	d->cbFilteringEnabled->setChecked(true);
	emitUpdated();
}

ServerListFilterInfo ServerFilterDock::filterInfo() const
{
	ServerListFilterInfo filterInfo;

	filterInfo.bEnabled = d->cbFilteringEnabled->isChecked();
	filterInfo.bShowEmpty = d->cbShowEmpty->isChecked();
	filterInfo.bShowFull = d->cbShowFull->isChecked();
	filterInfo.bShowOnlyValid = d->cbShowOnlyValid->isChecked();
	filterInfo.bShowBannedServers = d->cbShowBannedServers->isChecked();
	filterInfo.bShowTooSoonServers = d->cbShowTooSoonServers->isChecked();
	filterInfo.bShowNotRespondingServers = d->cbShowNotRespondingServers->isChecked();
	filterInfo.gameModes = d->cboGameMode->selectedItemTexts();
	filterInfo.gameModesExcluded = d->cboExcludeGameMode->selectedItemTexts();
	filterInfo.lockedServers = Doomseeker::checkboxTristateToShowMode(d->cbShowLockedServers->checkState());
	filterInfo.maxPing = d->spinMaxPing->value();
	filterInfo.testingServers = Doomseeker::checkboxTristateToShowMode(d->cbShowTestingServers->checkState());
	filterInfo.serverName = d->leServerName->text();
	filterInfo.wads = d->leWads->text().trimmed().split(",", Qt::SkipEmptyParts);
	filterInfo.wadsExcluded = d->leExcludeWads->text().trimmed().split(",", Qt::SkipEmptyParts);

	return filterInfo;
}

void ServerFilterDock::onServerGroupingChange()
{
	gConfig.doomseeker.bGroupServersWithPlayersAtTheTopOfTheList = d->cbGroupServersWithPlayersAtTop->isChecked();
	emit nonEmptyServerGroupingAtTopToggled(d->cbGroupServersWithPlayersAtTop->isChecked());
}

int ServerFilterDock::addPreset(const ServerListFilterInfo &preset)
{
	if (d->isCustomPreset(preset))
	{
		setCustomPreset(preset);
		return PrivData<ServerFilterDock>::CUSTOM_PRESET_IDX;
	}

	int index = d->presetIndex(preset.name);
	if (index > PrivData<ServerFilterDock>::CUSTOM_PRESET_IDX)
	{
		d->cboServerFilterPresets->setItemData(index, preset.serialize());
	}
	else
	{
		index = PrivData<ServerFilterDock>::CUSTOM_PRESET_IDX + 1;
		for (; index < d->cboServerFilterPresets->count(); ++index)
		{
			if (d->presetAtIndex(index).name.compare(
				preset.name, Qt::CaseInsensitive) > 0)
			{
				break;
			}
		}
		d->cboServerFilterPresets->insertItem(index, preset.name, preset.serialize());
	}
	return index;
}

void ServerFilterDock::setCustomPreset(const ServerListFilterInfo &preset)
{
	static const int idxCustom = PrivData<ServerFilterDock>::CUSTOM_PRESET_IDX;
	d->cboServerFilterPresets->setItemText(idxCustom,
		preset.isFilteringAnything() ? tr("[custom]") : tr("[no filter]"));
	d->cboServerFilterPresets->setItemData(idxCustom, preset.serialize());
}

void ServerFilterDock::setFilterInfo(const ServerListFilterInfo &filterInfo)
{
	d->bDisableUpdate = true;

	d->cbFilteringEnabled->setChecked(filterInfo.bEnabled);
	d->cbShowEmpty->setChecked(filterInfo.bShowEmpty);
	d->cbShowFull->setChecked(filterInfo.bShowFull);
	d->cbShowOnlyValid->setChecked(filterInfo.bShowOnlyValid);
	d->cbShowBannedServers->setChecked(filterInfo.bShowBannedServers);
	d->cbShowTooSoonServers->setChecked(filterInfo.bShowTooSoonServers);
	d->cbShowNotRespondingServers->setChecked(filterInfo.bShowNotRespondingServers);

	for (const QString &gameMode : filterInfo.gameModes)
	{
		addGameModeToComboBox(gameMode);
	}
	d->cboGameMode->setSelectedTexts(filterInfo.gameModes);

	for (const QString &gameMode : filterInfo.gameModesExcluded)
	{
		addGameModeToComboBox(gameMode);
	}
	d->cboExcludeGameMode->setSelectedTexts(filterInfo.gameModesExcluded);

	d->spinMaxPing->setValue(filterInfo.maxPing);
	if (d->leQuickSearch != nullptr)
		d->leQuickSearch->setText(filterInfo.serverName.trimmed());

	d->leServerName->setText(filterInfo.serverName.trimmed());
	d->leWads->setText(filterInfo.wads.join(",").trimmed());
	d->leExcludeWads->setText(filterInfo.wadsExcluded.join(",").trimmed());

	d->cbShowLockedServers->setCheckState(Doomseeker::showModeToCheckboxState(
		filterInfo.lockedServers));
	d->cbShowTestingServers->setCheckState(Doomseeker::showModeToCheckboxState(
		filterInfo.testingServers));

	d->bDisableUpdate = false;
	emitUpdated();
}

void ServerFilterDock::removeFilterPreset()
{
	const QString name = d->currentPresetName();
	if (QMessageBox::Yes == QMessageBox::question(this,
		tr("Doomseeker - Remove filter preset"),
		tr("Are you sure you wish to remove the filter preset \"%1\"?").arg(name)))
	{
		if (!name.isEmpty() && !d->isCustomPresetName(name))
		{
			gConfig.serverFilter.presets.remove(name);
			d->cboServerFilterPresets->removeItem(d->presetIndex(name));
		}
	}
}

void ServerFilterDock::saveFilterAsPreset()
{
	bool ok = false;
	const QString name = CommonGUI::getText(this, tr("Doomseeker - Save filter preset"),
		tr("Enter name for the filter preset (will overwrite if exists):"),
		QLineEdit::Normal, d->currentPresetName(), &ok).trimmed();

	if (!ok || name.isEmpty())
		return;

	ServerListFilterInfo filter = this->filterInfo();
	filter.name = name;
	gConfig.serverFilter.presets[filter.name] = filter.serialize();
	gConfig.saveToFile();
	const int index = addPreset(filter);
	d->cboServerFilterPresets->setCurrentIndex(index);
	d->cboServerFilterPresets->setItemText(index, name); // reset the * if any
}

void ServerFilterDock::loadSettings()
{
	// Presets.
	d->clearCustomPresets();
	for (const QVariant &serialized : gConfig.serverFilter.presets)
	{
		ServerListFilterInfo preset = ServerListFilterInfo::deserialize(serialized);
		if (!preset.name.isEmpty())
			addPreset(preset);
	}

	// The custom filter.
	setCustomPreset(gConfig.serverFilter.info);

	// Now select the current filter.
	int currentPresetIdx = d->presetIndex(gConfig.serverFilter.currentPreset);
	if (currentPresetIdx < 0 || currentPresetIdx >= d->cboServerFilterPresets->count())
	{
		currentPresetIdx = PrivData<ServerFilterDock>::CUSTOM_PRESET_IDX;
	}
	selectFilterPreset(currentPresetIdx);
}

void ServerFilterDock::selectFilterPreset(int index)
{
	d->cboServerFilterPresets->blockSignals(true);
	d->cboServerFilterPresets->setCurrentIndex(index);
	d->cboServerFilterPresets->blockSignals(false);
	d->btnRemovePreset->setEnabled(!d->isCustomPresetIndex(index) && index > 0);
	gConfig.serverFilter.currentPreset = d->currentPresetName();
	for (int i = PrivData<ServerFilterDock>::CUSTOM_PRESET_IDX + 1;
		 i < d->cboServerFilterPresets->count();
		 ++i)
	{
		// Reset any '*'
		d->cboServerFilterPresets->setItemText(i, d->presetNameAtIndex(i));
	}
	setFilterInfo(ServerListFilterInfo::deserialize(
		d->cboServerFilterPresets->itemData(index)));
}
