//------------------------------------------------------------------------------
// configpage.cpp
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
#include "configpage.h"

DClass<ConfigPage>
{
public:
	bool bAllowSave;
	bool bSettingsAlreadyRead;

	void (ConfigPage::*reject)();
};

DPointered(ConfigPage)

POLYMORPHIC_DEFINE(void, ConfigPage, reject, (), ())

ConfigPage::ConfigPage(QWidget *parent)
	: QWidget(parent)
{
	set_reject(&ConfigPage::reject_default);
	d->bAllowSave = false;
	d->bSettingsAlreadyRead = false;
	hide();
}

ConfigPage::~ConfigPage()
{
}

bool ConfigPage::allowSave()
{
	return d->bAllowSave;
}

bool ConfigPage::areSettingsAlreadyRead()
{
	return d->bSettingsAlreadyRead;
}

void ConfigPage::reject_default()
{
	// no-op
}

void ConfigPage::read()
{
	d->bSettingsAlreadyRead = true;
	readSettings();
}

bool ConfigPage::save()
{
	if (d->bAllowSave)
	{
		saveSettings();
		return true;
	}
	else
		return false;
}

void ConfigPage::setAllowSave(bool b)
{
	d->bAllowSave = b;
}

QString ConfigPage::title() const
{
	return name();
}

ConfigPage::Validation ConfigPage::validate()
{
	return VALIDATION_OK;
}
