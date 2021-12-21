//------------------------------------------------------------------------------
// cfgip2c.cpp
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
// Copyright (C) 2021 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "cfgip2c.h"

#include "configuration/doomseekerconfig.h"

#include "ui_cfgip2c.h"

DClass<CFGIP2C> : public Ui::CFGIP2C {};
DPointered(CFGIP2C)

CFGIP2C::CFGIP2C(QWidget *parent)
	: ConfigPage(parent)
{
	d->setupUi(this);
}

CFGIP2C::~CFGIP2C()
{
}

void CFGIP2C::readSettings()
{
	d->cbAutoUpdate->setChecked(gConfig.doomseeker.bIP2CountryAutoUpdate);
	d->cbHonorServerCountries->setChecked(gConfig.doomseeker.bHonorServerCountries);
}

void CFGIP2C::saveSettings()
{
	gConfig.doomseeker.bIP2CountryAutoUpdate = d->cbAutoUpdate->isChecked();
	gConfig.doomseeker.bHonorServerCountries = d->cbHonorServerCountries->isChecked();
}
