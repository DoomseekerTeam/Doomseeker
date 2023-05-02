//------------------------------------------------------------------------------
// gameconfigerrorbox.cpp
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
#include "gameconfigerrorbox.h"

#include "application.h"
#include "gui/configuration/doomseekerconfigurationdialog.h"
#include "gui/mainwindow.h"

#include <QMessageBox>
#include <QPushButton>

void GameConfigErrorBox::show(EnginePlugin *game, const QString &title,
	const QString &message, bool showConfigureButton, QWidget *parent)
{
	QMessageBox msgBox(QMessageBox::Critical, title, message, QMessageBox::NoButton, parent);
	QPushButton *btnConfigure;
	if (showConfigureButton)
	{
		btnConfigure = new QPushButton(
			QIcon(":/icons/preferences-system-4.png"),
			tr("Configure &game"), &msgBox);
		msgBox.addButton(btnConfigure, QMessageBox::AcceptRole);
		msgBox.addButton(QMessageBox::Close);
		msgBox.setDefaultButton(btnConfigure);
	}
	else
	{
		msgBox.addButton(QMessageBox::Ok);
	}
	msgBox.exec();
	if (btnConfigure != nullptr && msgBox.clickedButton() == btnConfigure)
	{
		DoomseekerConfigurationDialog::openConfiguration(
			parent != nullptr ? parent : gApp->mainWindow(),
			game);
	}
}
