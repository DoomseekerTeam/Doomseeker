//------------------------------------------------------------------------------
// miscserversetuppanel.cpp
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "miscserversetuppanel.h"
#include "ui_miscserversetuppanel.h"

#include "configuration/doomseekerconfig.h"
#include "ini/ini.h"
#include "plugins/engineplugin.h"
#include "serverapi/gamecreateparams.h"

DClass<MiscServerSetupPanel> : public Ui::MiscServerSetupPanel
{
public:
	bool anythingAvailable;
};

DPointered(MiscServerSetupPanel)

MiscServerSetupPanel::MiscServerSetupPanel(QWidget *parent)
	: QWidget(parent)
{
	d->setupUi(this);
	d->anythingAvailable = false;

	d->cbHidePasswords->setChecked(gConfig.doomseeker.bHidePasswords);
}

MiscServerSetupPanel::~MiscServerSetupPanel()
{
}

void MiscServerSetupPanel::fillInParams(GameCreateParams &params)
{
	params.setEmail(d->leEmail->text());
	params.setMotd(d->pteMOTD->toPlainText());
	params.setConnectPassword(d->leConnectPassword->text());
	params.setIngamePassword(d->leJoinPassword->text());
	params.setRconPassword(d->leRConPassword->text());
	params.setUrl(d->leURL->text());
}

bool MiscServerSetupPanel::isAnythingAvailable() const
{
	return d->anythingAvailable;
}

void MiscServerSetupPanel::loadConfig(Ini &config)
{
	IniSection misc = config.section("Misc");
	d->leURL->setText(misc["URL"]);
	d->leEmail->setText(misc["eMail"]);
	d->leConnectPassword->setText(misc["connectPassword"]);
	d->leJoinPassword->setText(misc["joinPassword"]);
	d->leRConPassword->setText(misc["RConPassword"]);
	d->pteMOTD->document()->setPlainText(misc["MOTD"]);
}

void MiscServerSetupPanel::saveConfig(Ini &config)
{
	IniSection misc = config.section("Misc");
	misc["URL"] = d->leURL->text();
	misc["eMail"] = d->leEmail->text();
	misc["connectPassword"] = d->leConnectPassword->text();
	misc["joinPassword"] = d->leJoinPassword->text();
	misc["RConPassword"] = d->leRConPassword->text();
	misc["MOTD"] = d->pteMOTD->toPlainText();
}

void MiscServerSetupPanel::setupForEngine(const EnginePlugin *engine)
{
	d->anythingAvailable = false;
	bool visible = false;

	visible = engine->data()->allowsConnectPassword;
	d->labelConnectPassword->setVisible(visible);
	d->leConnectPassword->setVisible(visible);
	d->anythingAvailable = visible || d->anythingAvailable;

	visible = engine->data()->allowsEmail;
	d->labelEmail->setVisible(visible);
	d->leEmail->setVisible(visible);
	d->anythingAvailable = visible || d->anythingAvailable;

	visible = engine->data()->allowsJoinPassword;
	d->labelJoinPassword->setVisible(visible);
	d->leJoinPassword->setVisible(visible);
	d->anythingAvailable = visible || d->anythingAvailable;

	visible = engine->data()->allowsMOTD;
	d->labelMOTD->setVisible(visible);
	d->pteMOTD->setVisible(visible);
	d->anythingAvailable = visible || d->anythingAvailable;

	visible = engine->data()->allowsRConPassword;
	d->labelRConPassword->setVisible(visible);
	d->leRConPassword->setVisible(visible);
	d->anythingAvailable = visible || d->anythingAvailable;

	visible = engine->data()->allowsURL;
	d->labelURL->setVisible(visible);
	d->leURL->setVisible(visible);
	d->anythingAvailable = visible || d->anythingAvailable;
}

void MiscServerSetupPanel::setHidePasswords(bool hide)
{
	QLineEdit::EchoMode echoMode = hide ? QLineEdit::Password : QLineEdit::Normal;
	d->leConnectPassword->setEchoMode(echoMode);
	d->leJoinPassword->setEchoMode(echoMode);
	d->leRConPassword->setEchoMode(echoMode);
}
