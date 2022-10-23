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

#include "configuration/doomseekerconfig.h"
#include "serverapi/server.h"

#include <QBuffer>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QResource>
#include <QTime>
#include <QTimeLine>

#include "log.h"

QMutex IP2C::instanceMutex;
IP2C *IP2C::staticInstance = nullptr;

static QHash<QString, QPixmap> cacheFlags()
{
	QHash<QString, QPixmap> flags;
	QDirIterator it(":flags/country/");
	while (it.hasNext())
	{
		QString resName = it.next();
		QFileInfo resPath(resName);
		flags[resPath.fileName()] = QPixmap(resName);
	}
	return flags;
}

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
	flagUnknown(":flags/unknown-small"), countries(IP2CCountry::all()),
	flags(cacheFlags())
{
	for (auto it = countries.begin(); it != countries.end(); ++it)
	{
		const QString &code = it.key();
		auto flagIt = flags.find(code);
		if (flagIt != flags.end())
			it->flag = &flagIt.value();
	}
}

IP2C::~IP2C()
{
}

const QPixmap &IP2C::flag(const QString &countryCode)
{
	auto it = flags.find(countryCode);
	if (it != flags.end())
	{
		return *it;
	}
	else
	{
		logUnknownFlag(countryCode);
		return flagUnknown;
	}
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

const IP2CRange &IP2C::lookupIP(unsigned int ipaddress)
{
	if (database.empty())
		return invalidRange;

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

	return invalidRange;
}

IP2CCountry IP2C::countryInfo(const QString &countryCode)
{
	auto it = countries.find(countryCode);
	if (it != countries.end())
	{
		if (it->flag == nullptr)
			logUnknownFlag(countryCode);
		return *it;
	}
	else
	{
		logUnknownCountry(countryCode);
		return unknownCountry();
	}
}

IP2CCountry IP2C::countryInfoForIPv4(unsigned int ipaddress)
{
	if (isLocalhostAddress(ipaddress))
		return IP2CCountry(&flagLocalhost, tr("Localhost"));

	if (isLANAddress(ipaddress))
		return IP2CCountry(&flagLan, tr("LAN"));

	if (!hasData())
		return IP2CCountry();

	const IP2CRange &addressRange = lookupIP(ipaddress);

	if (!addressRange.isValid())
		return IP2CCountry();
	if (addressRange.country.isEmpty())
		return unknownCountry();

	return countryInfo(addressRange.country);
}

IP2CCountry IP2C::countryInfoForServer(const Server &server)
{
	if (gConfig.doomseeker.bHonorServerCountries)
	{
		const IP2C::Lookup lookup = howLookup(server.country());
		if (lookup == IP2C::LOOKUP_DONT)
		{
			return unknownCountry();
		}
		else if (lookup == IP2C::LOOKUP_DIRECT && !isDataAccessLocked())
		{
			return countryInfo(server.country());
		}
	}

	if (!isDataAccessLocked())
	{
		return countryInfoForAddress(server.address());
	}

	return IP2CCountry();
}

void IP2C::logUnknownCountry(const QString &countryCode)
{
	if (!unknownCountries.contains(countryCode))
	{
		unknownCountries.insert(countryCode);
		gLog << tr("Unknown country: %1").arg(countryCode);
	}
}

void IP2C::logUnknownFlag(const QString &countryCode)
{
	if (!unknownFlags.contains(countryCode))
	{
		unknownFlags.insert(countryCode);
		gLog << tr("No flag for country: %1").arg(countryCode);
	}
}

IP2CCountry IP2C::unknownCountry() const
{
	return IP2CCountry(&flagUnknown, tr("Unknown"));
}
