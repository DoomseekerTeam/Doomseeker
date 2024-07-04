//------------------------------------------------------------------------------
// cfggames.h
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
// Copyright (C) 2024 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_GUI_CONFIGURATION_CFGGAMES_H
#define DOOMSEEKER_GUI_CONFIGURATION_CFGGAMES_H

#include "gui/configuration/configpage.h"
#include <QIcon>

class CFGGames : public ConfigPage
{
	Q_OBJECT

public:
	CFGGames(QWidget *parent);
	~CFGGames() override;

	QIcon icon() const override
	{
		return QIcon(":/icons/joystick.png");
	}
	QString name() const override
	{
		return tr("Games");
	}
	void readSettings() override;

protected:
	void saveSettings() override;

private:
	DPtr<CFGGames> d;
};

#endif
