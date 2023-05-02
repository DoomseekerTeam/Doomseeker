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
#include "ip2c/entities/ip2ccountryinfo.h"

#include <QHash>
#include <QHostAddress>
#include <QList>
#include <QMutex>
#include <QPixmap>
#include <QProgressBar>
#include <QStatusBar>
#include <QString>
#include <QUrl>

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
	class IP2CData
	{
	public:
		unsigned int ipStart;
		unsigned int ipEnd;
		QString country;
		QString countryFullName;

		IP2CData()
		{
			ipStart = ipEnd = 0;
		}

		bool isCountryKnown() const;

		/**
		 * IP2CData class is valid when ipStart is different from
		 * ipEnd.
		 */
		bool isValid() const
		{
			return ipStart != ipEnd;
		}
	};

	static IP2C *instance();
	static void deinstantiate();

	const QPixmap flagLan;
	const QPixmap flagLocalhost;
	const QPixmap flagUnknown;

	/**
	 * @brief Adds new country entry to the database.
	 *
	 * Makes sure the database is sorted in ascending order.
	 */
	void appendEntryToDatabase(const IP2CData &entry);

	const QPixmap &flag(const QString &countryShortName);
	bool hasData() const;

	bool isDataAccessLocked() const
	{
		return bDataAccessLocked;
	}

	/**
	 * Returns a reference to the structure describing the country.
	 */
	const IP2CData &lookupIP(unsigned int ipaddress);
	const IP2CData &lookupIP(const QHostAddress &ipaddress)
	{
		return lookupIP(ipaddress.toIPv4Address());
	}

	int numKnownEntries() const
	{
		return database.size();
	}

	/**
	 * Returns country information based on given IP.
	 */
	IP2CCountryInfo obtainCountryInfo(unsigned int ipaddress);
	IP2CCountryInfo obtainCountryInfo(const QHostAddress &ipaddress)
	{
		return obtainCountryInfo(ipaddress.toIPv4Address());
	}

	void setDataAccessLockEnabled(bool b)
	{
		bDataAccessLocked = b;
	}

	/**
	 * @brief Sets database contents to the list specified.
	 *
	 * To avoid performance issues it is already assumed that the specified
	 * list is sorted.
	 */
	void setDatabase(const QList<IP2CData> &sortedCountryData)
	{
		QMutexLocker dataAccessMutexLocker(&dataAccessMutex);
		database = sortedCountryData;
	}

protected:
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
	QMutex dataAccessMutex;
	QList<IP2CData> database;
	QHash<QString, QPixmap> flags;
	const IP2CData invalidData;

	IP2C();
	~IP2C() override;
};

#endif /* __IP2C_H__ */
