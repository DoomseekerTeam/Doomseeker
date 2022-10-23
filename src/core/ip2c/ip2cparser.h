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
 * Compacted IP2C database file format parser.
 *
 * Detects the version of the database and uses the correct subparser.
 */
class IP2CParser
{
	friend class IP2CParserV2;
	friend class IP2CParserV3;

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
};

#endif
