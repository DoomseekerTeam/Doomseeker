//------------------------------------------------------------------------------
// doomseekerconfigurationdialog.h
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
// Copyright (C) 2010 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __DOOMSEEKERCONFIGURATIONDIALOG_H__
#define __DOOMSEEKERCONFIGURATIONDIALOG_H__

#include "gui/configuration/configurationdialog.h"

class EnginePlugin;

class DoomseekerConfigurationDialog : public ConfigurationDialog
{
	Q_OBJECT

public:
	DoomseekerConfigurationDialog(QWidget *parent = nullptr);

	QStandardItem *addConfigPage(QStandardItem *rootItem, ConfigPage *configPage, int position = -1) override;
	bool addEngineConfiguration(ConfigPage *configPage);
	bool customServersChanged() const
	{
		return bCustomServersChanged;
	}
	void initOptionsList();
	void showPluginConfiguration(const EnginePlugin *plugin);
	bool wasAppearanceChanged() const
	{
		return bAppearanceChanged;
	}
	bool isRestartNeeded() const
	{
		return bRestartNeeded;
	}

	static void openConfiguration(QWidget *parent, const EnginePlugin *showPlugin = nullptr);
	static bool isOpen();

signals:
	void appearanceChanged();

private:
	bool bAppearanceChanged;
	bool bRestartNeeded;
	bool bCustomServersChanged;
	ConfigPage *customServersCfgBox;
	QStandardItem *enginesRoot;

	void appendFilePathsConfigurationBoxes();
	void appendWadseekerConfigurationBoxes();
	void doSaveSettings() override;

private slots:
	/**
	 *	When called, sets bAppearanceChanged to true.
	 */
	void appearanceChangedSlot();
	/**
	 *	When called, sets bRestartNeeded to true.
	 */
	void restartNeededSlot();
};

#endif
