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

#include "datastreamoperatorwrapper.h"
#include "global.h"
#include <QBuffer>
#include <QFile>
#include <QMap>

enum class Section : uint8_t
{
	LICENCE_TEXTS = 0,
	IPV4 = 1,
	URL_DB = 2,
};

bool IP2CParser::parse(QIODevice &dataBase)
{
	QByteArray dataArray = dataBase.readAll();

	// Read version.
	int pos = 4;
	if (pos >= dataArray.size())
		return false;

	const char *data = dataArray.constData();
	unsigned short version = READINT16(&data[pos]);

	switch (version)
	{
	case 3:
		return readDatabaseVersion3(dataArray);
	case 2:
		return readDatabaseVersion2(dataArray);
	default:
		return false;
	}
}

bool IP2CParser::readDatabaseVersion2(const QByteArray &dataArray)
{
	// Hardcode the URL to the current DB source. This may be inaccurate,
	// but version 2 didn't have the URL embedded.
	url_ = "https://mailfud.org/geoip-legacy/";

	QBuffer buffer;
	buffer.setData(dataArray);
	buffer.open(QIODevice::ReadOnly);
	QDataStream dstream(&buffer);
	dstream.setByteOrder(QDataStream::LittleEndian);
	DataStreamOperatorWrapper stream(&dstream);
	stream.skipRawData(6); // skip file tag and version number

	return readSectionIpv4(stream.readRawAll());
}

bool IP2CParser::readDatabaseVersion3(const QByteArray &dataArray)
{
	QBuffer buffer;
	buffer.setData(dataArray);
	buffer.open(QIODevice::ReadOnly);
	QDataStream dstream(&buffer);
	dstream.setByteOrder(QDataStream::LittleEndian);
	DataStreamOperatorWrapper stream(&dstream);
	stream.skipRawData(6); // skip file tag and version number

	while (stream.hasRemaining())
	{
		auto id = stream.readQUInt8();
		auto size = stream.readQUInt64();
		switch (id)
		{
		case decltype(id)(Section::LICENCE_TEXTS):
			if (!readSectionLegalNotice(stream.readRaw(size)))
				return false;
			break;
		case decltype(id)(Section::IPV4):
			if (!readSectionIpv4(stream.readRaw(size)))
				return false;
			break;
		case decltype(id)(Section::URL_DB):
			if (!readSectionUrl(stream.readRaw(size)))
				return false;
			break;
		default:
			stream.skipRawData(size);
		}
	}

	return true;
}

bool IP2CParser::readSectionIpv4(QByteArray &&section)
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

	this->ranges_ = hashTable.values();

	return true;
}

bool IP2CParser::readSectionLegalNotice(QByteArray &&section)
{
	QBuffer buffer(&section);
	buffer.open(QIODevice::ReadOnly);
	QDataStream dstream(&buffer);
	dstream.setByteOrder(QDataStream::LittleEndian);
	DataStreamOperatorWrapper stream(&dstream);

	while (stream.hasRemaining())
	{
		auto locale = QString::fromUtf8(stream.readRawUntilByte('\0'));
		if (stream.hasRemaining())
		{
			auto text = QString::fromUtf8(stream.readRawUntilByte('\0'));
			licences_.insert(std::move(locale), std::move(text));
		}
		else
			return false;
	}
	return true;
}

bool IP2CParser::readSectionUrl(QByteArray &&section)
{
	QBuffer buffer(&section);
	buffer.open(QIODevice::ReadOnly);
	QDataStream dstream(&buffer);
	dstream.setByteOrder(QDataStream::LittleEndian);
	DataStreamOperatorWrapper stream(&dstream);
	url_ = QString::fromUtf8(DataStreamOperatorWrapper(&dstream).readRawUntilByte('\0'));
	return true;
}
