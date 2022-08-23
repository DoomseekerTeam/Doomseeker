//------------------------------------------------------------------------------
// ip2cparser.h
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
#ifndef __IP2CPARSER_H__
#define __IP2CPARSER_H__

#include "ip2crange.h"

#include <QIODevice>
#include <QList>
#include <QMap>
#include <QString>

/**
 * Compacted database file format, version 2:
 * (all strings are null terminated)
 * Header:
 * @code
 * TYPE			LENGTH		DESCRIPTION
 * -----------------------------------------------------
 * unsigned long	4			'I' 'P' '2' 'C' bytes
 * unsigned short	2			Version (equal to 2)
 * @endcode
 *
 * Block repeated until EOF:
 * @code
 * TYPE			LENGTH		DESCRIPTION
 * -----------------------------------------------------
 * string			N/A		Country full name, UTF-8
 * string			N/A		Country abbreviation (3 letters version)
 * unsigned long	4			Number of IP Blocks (N_IP_BLOCKS)
 *
 * -- BLOCK: repeated N_IP_BLOCKS times.
 * unsigned long	4			Beginning of an IP range
 * unsigned long	4			End of an IP range
 * -- END OF BLOCK
 * @endcode
 */
class IP2CParser
{
public:
	const QList<IP2CRange> ranges() const
	{
		return ranges_;
	}

	const QMap<QString, QString> licences() const
	{
		return licences_;
	}

	const QString url() const
	{
		return url_;
	}

	bool parse(QIODevice &dataBase);

private:
	QList<IP2CRange> ranges_;
	QMap<QString, QString> licences_;
	QString url_;

	bool readDatabaseVersion2(const QByteArray &dataArray);
	bool readDatabaseVersion3(const QByteArray &dataArray);
	bool readSectionIpv4(QByteArray &&section);
	bool readSectionLegalNotice(QByteArray &&section);
	bool readSectionUrl(QByteArray &&section);
};

#endif
