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
// Copyright (C) 2022 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "ip2cparser_v2.h"

#include "ip2c/ip2cparser.h"
#include "ip2c/ip2crange.h"

#include "datastreamoperatorwrapper.h"
#include "global.h"
#include <QMap>

bool IP2CParserV2::parse(IP2CParser &self, QIODevice &dataBase)
{
	// Hardcode the URL to the current DB source. This may be inaccurate,
	// but version 2 didn't have the URL embedded.
	self.url_ = "https://doomseeker.drdteam.org/ip2c/IpToCountry_v2.dat.php";
	self.licences_[""] = "Please see the linked webpage for the licensing details of IP2C.\n" + self.url_;

	QDataStream dstream(&dataBase);
	dstream.setByteOrder(QDataStream::LittleEndian);
	DataStreamOperatorWrapper stream(&dstream);

	return readSectionIpv4(self, stream);
}

bool IP2CParserV2::readSectionIpv4(IP2CParser &self, DataStreamOperatorWrapper &stream)
{
	// We need to store the addresses in such hash table to make sure they
	// are ordered in proper, ascending order. Otherwise the whole library
	// will not work!
	QMap<unsigned, IP2CRange> hashTable;

	while (stream.hasRemaining())
	{
		// Discard the first field as it is the country name. As of Doomseeker
		// 1.3.3, the country names and their alpha-3 codes are hardcoded.
		QString::fromUtf8(stream.readRawUntilByte('\0'));
		QString countryCode = QString::fromUtf8(stream.readRawUntilByte('\0'));

		// Base entry for each IP read from the file
		IP2CRange baseEntry;
		baseEntry.country = countryCode;
		if (!stream.hasRemaining())
			return false;

		quint32 numOfIpBlocks = stream.readQUInt32();

		for (quint32 x = 0; x < numOfIpBlocks; ++x)
		{
			// Create new entry from the base.
			IP2CRange entry = baseEntry;

			entry.ipStart = stream.readQUInt32();
			if (!stream.hasRemaining())
				return false;
			entry.ipEnd = stream.readQUInt32();

			hashTable[entry.ipStart] = entry;
		}
	}

	self.ranges_ = hashTable.values();
	return true;
}
