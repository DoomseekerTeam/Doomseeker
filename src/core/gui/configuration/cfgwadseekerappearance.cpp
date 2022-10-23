//------------------------------------------------------------------------------
// cfgwadseekerappearance.cpp
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
#include "cfgwadseekerappearance.h"
#include "configuration/doomseekerconfig.h"
#include "ui_cfgwadseekerappearance.h"

DClass<CFGWadseekerAppearance> : public Ui::CFGWadseekerAppearance
{
};

DPointered(CFGWadseekerAppearance)

CFGWadseekerAppearance::CFGWadseekerAppearance(QWidget *parent)
	: ConfigPage(parent)
{
	d->setupUi(this);
}

CFGWadseekerAppearance::~CFGWadseekerAppearance()
{
}

void CFGWadseekerAppearance::readSettings()
{
	d->btnNoticeMessageColor->setColorHtml(gConfig.wadseeker.colorMessageNotice);
	d->btnErrorMessageColor->setColorHtml(gConfig.wadseeker.colorMessageError);
	d->btnCriticalErrorMessageColor->setColorHtml(gConfig.wadseeker.colorMessageCriticalError);
}

void CFGWadseekerAppearance::saveSettings()
{
	gConfig.wadseeker.colorMessageNotice = d->btnNoticeMessageColor->colorHtml();
	gConfig.wadseeker.colorMessageError = d->btnErrorMessageColor->colorHtml();
	gConfig.wadseeker.colorMessageCriticalError = d->btnCriticalErrorMessageColor->colorHtml();
}
