//------------------------------------------------------------------------------
// cfgircappearance.cpp
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
#include "cfgircappearance.h"
#include "irc/configuration/ircconfig.h"
#include "ui_cfgircappearance.h"

DClass<CFGIRCAppearance> : public Ui::CFGIRCAppearance
{
};

DPointered(CFGIRCAppearance)

CFGIRCAppearance::CFGIRCAppearance(QWidget *parent)
	: ConfigPage(parent)
{
	d->setupUi(this);
}

CFGIRCAppearance::~CFGIRCAppearance()
{
}

void CFGIRCAppearance::readSettings()
{
	// Colors
	d->btnBackgroundColor->setColorHtml(gIRCConfig.appearance.backgroundColor);
	d->btnChannelActionColor->setColorHtml(gIRCConfig.appearance.channelActionColor);
	d->btnCtcpColor->setColorHtml(gIRCConfig.appearance.ctcpColor);
	d->btnDefaultTextColor->setColorHtml(gIRCConfig.appearance.defaultTextColor);
	d->btnErrorColor->setColorHtml(gIRCConfig.appearance.errorColor);
	d->btnNetworkActionColor->setColorHtml(gIRCConfig.appearance.networkActionColor);
	d->btnUrlColor->setColorHtml(gIRCConfig.appearance.urlColor);
	d->btnUserListSelectedTextColor->setColorHtml(gIRCConfig.appearance.userListSelectedTextColor);
	d->btnUserListSelectedBackgroundColor->setColorHtml(gIRCConfig.appearance.userListSelectedBackgroundColor);

	// Fonts
	d->btnMainFont->setSelectedFont(gIRCConfig.appearance.mainFont);
	d->btnUserListFont->setSelectedFont(gIRCConfig.appearance.userListFont);

	// Others
	d->cbTimestamps->setChecked(gIRCConfig.appearance.timestamps);
	d->cbWindowAlertOnImportantEvent->setChecked(gIRCConfig.appearance.windowAlertOnImportantChatEvent);
}

void CFGIRCAppearance::saveSettings()
{
	// Colors
	gIRCConfig.appearance.backgroundColor = d->btnBackgroundColor->colorHtml();
	gIRCConfig.appearance.channelActionColor = d->btnChannelActionColor->colorHtml();
	gIRCConfig.appearance.ctcpColor = d->btnCtcpColor->colorHtml();
	gIRCConfig.appearance.defaultTextColor = d->btnDefaultTextColor->colorHtml();
	gIRCConfig.appearance.errorColor = d->btnErrorColor->colorHtml();
	gIRCConfig.appearance.networkActionColor = d->btnNetworkActionColor->colorHtml();
	gIRCConfig.appearance.urlColor = d->btnUrlColor->colorHtml();
	gIRCConfig.appearance.userListSelectedTextColor = d->btnUserListSelectedTextColor->colorHtml();
	gIRCConfig.appearance.userListSelectedBackgroundColor = d->btnUserListSelectedBackgroundColor->colorHtml();

	// Fonts
	gIRCConfig.appearance.mainFont = d->btnMainFont->selectedFont();
	gIRCConfig.appearance.userListFont = d->btnUserListFont->selectedFont();

	// Others
	gIRCConfig.appearance.timestamps = d->cbTimestamps->isChecked();
	gIRCConfig.appearance.windowAlertOnImportantChatEvent = d->cbWindowAlertOnImportantEvent->isChecked();
}
