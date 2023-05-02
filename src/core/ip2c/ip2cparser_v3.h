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
// Copyright (C) 2022 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __IP2CPARSER_V3_H__
#define __IP2CPARSER_V3_H__

#include <QByteArray>
#include <QIODevice>

class IP2CParser;
/**
 * Compacted database file format, version 3
 */
class IP2CParserV3
{
public:
	bool parse(IP2CParser &self, QIODevice &dataBase);

private:
	bool readSectionIpv4(IP2CParser &self, QByteArray &&section);
	bool readSectionLegalNotice(IP2CParser &self, QByteArray &&section);
	bool readSectionUrl(IP2CParser &self, QByteArray &&section);
};

#endif
