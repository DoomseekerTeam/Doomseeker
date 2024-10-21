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
#include "ip2cparser_v3.h"

#include "ip2c/ip2cparser.h"
#include "ip2c/ip2crange.h"

#include "datastreamoperatorwrapper.h"
#include "global.h"
#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <QMap>

#include <cstdint>

enum class Section : uint8_t
{
	LICENCE_TEXTS = 0,
	IPV4 = 1,
	URL_DB = 2,
};

bool IP2CParserV3::parse(IP2CParser &self, QIODevice &dataBase)
{
	QDataStream dstream(&dataBase);
	dstream.setByteOrder(QDataStream::LittleEndian);
	DataStreamOperatorWrapper stream(&dstream);

	while (stream.hasRemaining())
	{
		auto id = stream.readQUInt8();
		auto size = stream.readQUInt64();
		switch (id)
		{
		case decltype(id)(Section::LICENCE_TEXTS):
			if (!readSectionLegalNotice(self, stream.readRaw(size)))
				return false;
			break;
		case decltype(id)(Section::IPV4):
			if (!readSectionIpv4(self, stream.readRaw(size)))
				return false;
			break;
		case decltype(id)(Section::URL_DB):
			if (!readSectionUrl(self, stream.readRaw(size)))
				return false;
			break;
		default:
			stream.skipRawData(size);
			break;
		}
	}

	return true;
}

bool IP2CParserV3::readSectionIpv4(IP2CParser &self, QByteArray &&section)
{
	QBuffer buffer(&section);
	buffer.open(QIODevice::ReadOnly);
	QDataStream dstream(&buffer);
	dstream.setByteOrder(QDataStream::LittleEndian);
	DataStreamOperatorWrapper stream(&dstream);
	// We need to store the addresses in such hash table to make sure they
	// are ordered in proper, ascending order. Otherwise the whole library
	// will not work!
	QMap<unsigned, IP2CRange> hashTable;

	while (stream.hasRemaining())
	{
		auto countryCode = stream.readUtf8CString();

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

bool IP2CParserV3::readSectionLegalNotice(IP2CParser &self, QByteArray &&section)
{
	QBuffer buffer(&section);
	buffer.open(QIODevice::ReadOnly);
	QDataStream dstream(&buffer);
	dstream.setByteOrder(QDataStream::LittleEndian);
	DataStreamOperatorWrapper stream(&dstream);

	while (stream.hasRemaining())
	{
		auto locale = stream.readUtf8CString();
		if (stream.hasRemaining())
		{
			auto text = stream.readUtf8CString();
			self.licences_.insert(std::move(locale), std::move(text));
		}
		else
			return false;
	}
	return true;
}

bool IP2CParserV3::readSectionUrl(IP2CParser &self, QByteArray &&section)
{
	QBuffer buffer(&section);
	buffer.open(QIODevice::ReadOnly);
	QDataStream dstream(&buffer);
	dstream.setByteOrder(QDataStream::LittleEndian);
	DataStreamOperatorWrapper stream(&dstream);
	self.url_ = DataStreamOperatorWrapper(&dstream).readUtf8CString();
	return true;
}
