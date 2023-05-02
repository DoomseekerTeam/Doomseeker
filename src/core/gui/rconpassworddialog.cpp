//------------------------------------------------------------------------------
// rconpassworddialog.cpp
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
// Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------
#include "configuration/passwordscfg.h"
#include "gui/commongui.h"
#include "plugins/engineplugin.h"
#include "plugins/pluginloader.h"
#include "rconpassworddialog.h"
#include "serverapi/server.h"
#include "ui_rconpassworddialog.h"

DClass<RconPasswordDialog> : public Ui::RconPasswordDialog
{
};

DPointered(RconPasswordDialog)

RconPasswordDialog::RconPasswordDialog(QWidget *parent, bool connection)
	: QDialog(parent)
{
	d->setupUi(this);
	CommonGUI::setupDialog(*this);
	this->connect(d->cbHidePassword, SIGNAL(toggled(bool)), SLOT(setHidePassword(bool)));

	if (connection)
	{
		// Populate engines box.
		d->engines->clear();
		for (unsigned int i = 0; i < gPlugins->numPlugins(); i++)
		{
			const EnginePlugin *info = gPlugins->plugin(i)->info();
			if (info->server(QHostAddress("localhost"), 0)->hasRcon())
				d->engines->addItem(info->icon(), info->data()->name, i);
		}
	}
	else
		d->connectionBox->hide();

	PasswordsCfg passCfg;
	setHidePassword(passCfg.isHidingPasswords());

	// Adjust the size and prevent resizing.
	adjustSize();
	setMinimumHeight(height());
	setMaximumHeight(height());
}

RconPasswordDialog::~RconPasswordDialog()
{
}

QString RconPasswordDialog::connectPassword() const
{
	return d->lePassword->text();
}

const EnginePlugin *RconPasswordDialog::selectedEngine() const
{
	int pluginIndex = d->engines->itemData(d->engines->currentIndex()).toInt();
	const PluginLoader::Plugin *plugin = gPlugins->plugin(pluginIndex);
	if (plugin == nullptr)
		return nullptr;

	return plugin->info();
}

QString RconPasswordDialog::serverAddress() const
{
	return d->leServerAddress->text();
}

void RconPasswordDialog::setHidePassword(bool hide)
{
	d->cbHidePassword->blockSignals(true);
	d->cbHidePassword->setChecked(hide);
	d->cbHidePassword->blockSignals(false);
	d->lePassword->setEchoMode(hide ? QLineEdit::Password : QLineEdit::Normal);
}
