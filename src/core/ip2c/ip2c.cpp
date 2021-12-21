//------------------------------------------------------------------------------
// ip2c.cpp
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
#include "ip2c.h"

#include <QBuffer>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QResource>
#include <QTime>
#include <QTimeLine>

#include "log.h"

QMutex IP2C::instanceMutex;
IP2C *IP2C::staticInstance = nullptr;

IP2C *IP2C::instance()
{
	if (staticInstance == nullptr)
	{
		QMutexLocker locker(&instanceMutex);
		if (staticInstance == nullptr)
			staticInstance = new IP2C();
	}
	return staticInstance;
}

void IP2C::deinstantiate()
{
	if (staticInstance != nullptr)
	{
		delete staticInstance;
		staticInstance = nullptr;
	}
}

IP2C::IP2C()
	: flagLan(":flags/lan-small"), flagLocalhost(":flags/localhost-small"),
	flagUnknown(":flags/unknown-small")
{
}

IP2C::~IP2C()
{
}

const QPixmap &IP2C::flag(const QString &countryShortName)
{
	if (flags.contains(countryShortName))
		return flags[countryShortName];

	QString resName = ":flags/" + countryShortName;
	QResource res(resName);

	if (!res.isValid())
	{
		gLog << tr("No flag for country: %1").arg(countryShortName);
		flags[countryShortName] = flagUnknown;
		return flagUnknown;
	}

	flags[countryShortName] = QPixmap(resName);
	return flags[countryShortName];
}

bool IP2C::hasData() const
{
	return !database.empty();
}

IP2C::Lookup IP2C::howLookup(const QString &countryCode)
{
	QString code = countryCode.toUpper().trimmed();
	if (code.isEmpty() || code == "XIP")
		return LOOKUP_IP;
	else if (code == "XUN")
		return LOOKUP_DONT;
	else
		return LOOKUP_DIRECT;
}

const IP2C::IP2CData &IP2C::lookupIP(unsigned int ipaddress)
{
	if (database.empty())
		return invalidData;

	unsigned int upper = database.size() - 1;
	unsigned int lower = 0;
	unsigned int index = database.size() / 2;
	unsigned int lastIndex = 0xFFFFFFFF;
	while (index != lastIndex) // Infinite loop protection.
	{
		lastIndex = index;

		if (ipaddress < database[index].ipStart)
		{
			upper = index;
			index -= (index - lower) >> 1; // If we're concerning ourselves with speed >>1 is the same as /2, but should be faster.
			continue;
		}
		else if (ipaddress > database[index].ipEnd)
		{
			lower = index;
			index += (upper - index) >> 1;
			continue;
		}
		return database[index];
	}

	return invalidData;
}

IP2CCountryInfo IP2C::obtainCountryInfo(unsigned int ipaddress)
{
	if (isLocalhostAddress(ipaddress))
		return IP2CCountryInfo(&flagLocalhost, tr("Localhost"));

	if (isLANAddress(ipaddress))
		return IP2CCountryInfo(&flagLan, tr("LAN"));

	if (!hasData())
		return IP2CCountryInfo();

	const IP2CData &data = lookupIP(ipaddress);

	if (!data.isCountryKnown())
		return IP2CCountryInfo(&flagUnknown, tr("Unknown"));

	if (!data.isValid())
		return IP2CCountryInfo();

	const QPixmap *pFlag = &flag(data.country);
	return IP2CCountryInfo(pFlag, data.countryFullName);
}

bool IP2C::IP2CData::isCountryKnown() const
{
	return !country.isEmpty() && country != "ZZZ";
}
