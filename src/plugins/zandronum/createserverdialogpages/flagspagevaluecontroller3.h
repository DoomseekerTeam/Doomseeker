//------------------------------------------------------------------------------
// flagspagevaluecontroller3.h
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
// Copyright (C) 2016 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_PLUGIN_ZANDRONUM_CREATESERVERDIALOGPAGES_FLAGSPAGEVALUECONTROLLER3_H
#define DOOMSEEKER_PLUGIN_ZANDRONUM_CREATESERVERDIALOGPAGES_FLAGSPAGEVALUECONTROLLER3_H

#include <QAbstractButton>
#include <QMap>

#include "createserverdialogpages/flagspagevaluecontroller.h"

#include <cstdint>

class FlagsPage;

namespace Zandronum3
{
/**
 * @brief Converts numerical flags values to widget representation
 *        and vice-versa.
 */
class FlagsPageValueController : public ::FlagsPageValueController
{
public:
	FlagsPageValueController(FlagsPage *flagsPage);

	void convertNumericalToWidgets() override;
	void convertWidgetsToNumerical() override;
	void setVisible(bool visible) override;

private:
	int32_t compatflags;
	int32_t compatflags2;
	int32_t zandronumCompatflags;

	int32_t dmflags;
	int32_t dmflags2;
	int32_t zandronumDmflags;

	int32_t lmsAllowedWeapons;
	int32_t lmsSpectatorSettings;

	QMap<int32_t, QAbstractButton *> compatflagsCheckboxes;
	QMap<int32_t, QAbstractButton *> compatflags2Checkboxes;
	QMap<int32_t, QAbstractButton *> dmflagsCheckboxes;
	QMap<int32_t, QAbstractButton *> dmflags2Checkboxes;
	QMap<int32_t, QAbstractButton *> lmsAllowedWeaponsCheckboxes;
	QMap<int32_t, QAbstractButton *> lmsSpectatorSettingsCheckboxes;
	QMap<int32_t, QAbstractButton *> zandronumCompatflagsCheckboxes;
	QMap<int32_t, QAbstractButton *> zandronumDmflagsCheckboxes;

	FlagsPage *flagsPage;

	void convertToNumericalGeneral();
	void convertToNumericalPlayers();
	void convertToNumericalCooperative();
	void convertToNumericalDeathmatch();

	void convertCheckboxesToNumerical(const QMap<int32_t, QAbstractButton *> &checkboxMap,
		int32_t &flagsValue);
	void convertNumericalToCheckboxes(QMap<int32_t, QAbstractButton *> &checkboxMap,
		int32_t flagsValue);

	void convertToWidgetGeneral();
	void convertToWidgetPlayers();
	void convertToWidgetCooperative();
	void convertToWidgetDeathmatch();

	void readFlagsFromTextInputs();
};
}

#endif
