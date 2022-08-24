//------------------------------------------------------------------------------
// ip2cparser.cpp
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
#include "ip2cparser.h"

#include "ip2c/ip2cparser_v2.h"
#include "ip2c/ip2cparser_v3.h"

#include "global.h"

bool IP2CParser::parse(QIODevice &dataBase)
{
	QByteArray dataArray = dataBase.readAll();

	// Read version.
	int pos = 4;
	if (pos >= dataArray.size())
		return false;

	const char *data = dataArray.constData();
	unsigned short version = READINT16(&data[pos]);

	dataBase.seek(6); // TODO proper fix for this
	switch (version)
	{
	case 2:
		return IP2CParserV2().parse(*this, dataBase);
	case 3:
		return IP2CParserV3().parse(*this, dataBase);
	default:
		return false;
	}
}
