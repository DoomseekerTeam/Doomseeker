//------------------------------------------------------------------------------
// generalinfotip.cpp
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
#include "generalinfotip.h"

#include "ip2c/ip2c.h"
#include "serverapi/server.h"

DClass<GeneralInfoTip>
{
public:
	ServerCPtr server;
};

DPointered(GeneralInfoTip)

GeneralInfoTip::GeneralInfoTip(const ServerCPtr &server)
{
	d->server = server;
}

GeneralInfoTip::~GeneralInfoTip()
{
}

QString GeneralInfoTip::generateHTML()
{
	QString ret;
	if (d->server->isKnown())
	{
		ret += d->server->name().toHtmlEscaped() + "\n\n";
		ret += labelString(tr("Version"), d->server->gameVersion());
		ret += labelString(tr("E-mail"), d->server->email());
		ret += labelString(tr("URL"), d->server->webSite());
	}

	IP2CCountry countryInfo = IP2C::instance()->countryInfoForServer(*d->server);
	if (countryInfo.isValid())
	{
		ret += labelString(tr("Location"), countryInfo.name);
	}

	return ret;
}

QString GeneralInfoTip::labelString(QString label, QString valueString)
{
	if (valueString.isEmpty())
	{
		return QString();
	}
	else
	{
		return QString("<b>%1:</b> %2\n")
			.arg(label)
			.arg(valueString.toHtmlEscaped());
	}
}
