//------------------------------------------------------------------------------
// ip2c.h
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

#ifndef __IP2C_H__
#define __IP2C_H__

#include "global.h"
#include "ip2c/ip2ccountry.h"
#include "ip2c/ip2crange.h"

#include <QHash>
#include <QHostAddress>
#include <QList>
#include <QMutex>
#include <QPixmap>
#include <QString>

class Server;

/**
 * @brief IP to Country database handler.
 *
 * IP2C class provides an interface for translating IP addresses into country
 * names they are in. Additional methods allow to retrieve country's flag
 * picture for given IP.
 * @see IP2CParser
 */
class IP2C : public QObject
{
	Q_OBJECT

public:
	/**
	 * @brief Indicate how the IP2C resolution should occur.
	 */
	enum Lookup
	{
		/// Lookup the country by IP.
		LOOKUP_IP,
		/// The country info is known; use it directly.
		LOOKUP_DIRECT,
		/// Don't look-up the country; force an unknown country.
		LOOKUP_DONT,
	};

	static IP2C *instance();
	static void deinstantiate();

	/**
	 * Interpret the country code or a special value to see if and how
	 * the IP2C should be resolved.
	 */
	static Lookup howLookup(const QString &countryCode);

	const QPixmap flagLan;
	const QPixmap flagLocalhost;
	const QPixmap flagUnknown;

	const QPixmap &flag(const QString &countryCode);
	bool hasData() const;

	bool isDataAccessLocked() const
	{
		return bDataAccessLocked;
	}

	int numKnownEntries() const
	{
		return database.size();
	}

	/**
	 * Returns country information based on the ISO 3166-1 alpha-3 code.
	 */
	IP2CCountry countryInfo(const QString &countryCode);

	/**
	 * Returns country information based on given IP.
	 */
	IP2CCountry countryInfoForIPv4(unsigned int ipaddress);
	IP2CCountry countryInfoForAddress(const QHostAddress &ipaddress)
	{
		return countryInfoForIPv4(ipaddress.toIPv4Address());
	}
	IP2CCountry countryInfoForServer(const Server &server);

	void setDataAccessLockEnabled(bool b)
	{
		bDataAccessLocked = b;
	}

	/**
	 * @brief Sets the IP range database to the list specified.
	 *
	 * To avoid performance issues it is already assumed that the specified
	 * list is sorted.
	 */
	void setRangesDatabase(const QList<IP2CRange> &sortedRanges)
	{
		this->database = sortedRanges;
	}

private:
	static QMutex instanceMutex;
	static IP2C *staticInstance;

	/**
	 * @brief Performs only an informative role for the application.
	 *
	 * This might be set to true by either updater or parser. Application
	 * should not read from the database when this is true because
	 * data inside might still be invalid.
	 */
	bool bDataAccessLocked;
	QList<IP2CRange> database;
	QHash<QString, IP2CCountry> countries;

	// Caches all flags. It's important to not touch the flags structure
	// anymore after it's been created as the program will refer to the flags
	// by pointer.
	const QHash<QString, QPixmap> flags;

	const IP2CRange invalidRange;

	QSet<QString> unknownCountries;
	QSet<QString> unknownFlags;

	IP2C();
	~IP2C() override;
	Q_DISABLE_COPY(IP2C);

	/**
	 * Returns a reference to the structure describing the country.
	 */
	const IP2CRange &lookupIP(unsigned int ipaddress);
	IP2CCountry unknownCountry() const;

	void logUnknownCountry(const QString &countryCode);
	void logUnknownFlag(const QString &countryCode);

	inline bool isLANAddress(unsigned ipv4Address)
	{
		const static unsigned LAN_1_BEGIN = QHostAddress("10.0.0.0").toIPv4Address();
		const static unsigned LAN_1_END = QHostAddress("10.255.255.255").toIPv4Address();
		const static unsigned LAN_2_BEGIN = QHostAddress("172.16.0.0").toIPv4Address();
		const static unsigned LAN_2_END = QHostAddress("172.31.255.255").toIPv4Address();
		const static unsigned LAN_3_BEGIN = QHostAddress("192.168.0.0").toIPv4Address();
		const static unsigned LAN_3_END = QHostAddress("192.168.255.255").toIPv4Address();

		return
			(ipv4Address >= LAN_1_BEGIN && ipv4Address <= LAN_1_END)
			|| (ipv4Address >= LAN_2_BEGIN && ipv4Address <= LAN_2_END)
			|| (ipv4Address >= LAN_3_BEGIN && ipv4Address <= LAN_3_END)
		;
	}

	inline bool isLocalhostAddress(unsigned ipv4Address)
	{
		const static unsigned LOCALHOST_BEGIN = QHostAddress("127.0.0.0").toIPv4Address();
		const static unsigned LOCALHOST_END = QHostAddress("127.255.255.255").toIPv4Address();

		return ipv4Address >= LOCALHOST_BEGIN && ipv4Address <= LOCALHOST_END;
	}
};

#endif /* __IP2C_H__ */
