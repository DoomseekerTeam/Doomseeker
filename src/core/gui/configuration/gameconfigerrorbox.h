//------------------------------------------------------------------------------
// gameconfigerrorbox.h
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
// Copyright (C) 2022 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef DOOMSEEKER_CORE_GUI_CONFIGURATION_GAMECONFIGERRORBOX_H
#define DOOMSEEKER_CORE_GUI_CONFIGURATION_GAMECONFIGERRORBOX_H

#include <QObject>
#include <QString>

class EnginePlugin;

class GameConfigErrorBox : public QObject
{
	Q_OBJECT

public:
	static void show(EnginePlugin *game, const QString &title, const QString &message,
		bool showConfigureButton, QWidget *parent = nullptr);

private:
	GameConfigErrorBox() = delete;
};

#endif
