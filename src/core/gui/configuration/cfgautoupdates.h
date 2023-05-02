//------------------------------------------------------------------------------
// cfgautoupdates.h
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
#ifndef DOOMSEEKER_GUI_CONFIGURATION_CFGAUTOPUDATES_H
#define DOOMSEEKER_GUI_CONFIGURATION_CFGAUTOPUDATES_H

#include "dptr.h"
#include "gui/configuration/configpage.h"
#include <QIcon>

class CFGAutoUpdates : public ConfigPage
{
	Q_OBJECT

public:
	CFGAutoUpdates(QWidget *parent = nullptr);
	~CFGAutoUpdates() override;

	QIcon icon() const override
	{
		return QIcon(":/icons/doomseeker_plus.png");
	}
	QString name() const override
	{
		return tr("Auto Updates");
	}

	void readSettings() override;

protected:
	void saveSettings() override;

private:
	void initUpdateChannels();

	DPtr<CFGAutoUpdates> d;
private slots:
	void onUpdateChannelChange(int index);
};

#endif
