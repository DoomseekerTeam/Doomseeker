//------------------------------------------------------------------------------
// enginezandronumconfigbox.h
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
#ifndef DOOMSEEKER_PLUGIN_ZANDRONUM_ENGINE_ZANDRONUM_CONFIG_H
#define DOOMSEEKER_PLUGIN_ZANDRONUM_ENGINE_ZANDRONUM_CONFIG_H

#include "gui/configuration/engineconfigpage.h"

class QCheckBox;
class QGroupBox;
class QLineEdit;
class QPushButton;

class EngineZandronumConfigBox : public EngineConfigPage
{
	Q_OBJECT

public:
	EngineZandronumConfigBox(EnginePlugin *plugin, IniSection &cfg, QWidget *parent = nullptr);

	void readSettings() override;


protected slots:
	void btnBrowseTestingPathClicked();

protected:

	void saveSettings() override;

	QGroupBox *groupTesting;
	QLineEdit *leTestingPath;
	QPushButton *btnBrowseTestingPath;
};

#endif
