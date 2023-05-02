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
#include "log.h"
#include <QBuffer>
#include <QFile>
#include <QMap>
#include <QMutexLocker>
#include <QTime>

IP2CParser::IP2CParser(IP2C *pTargetDatabase)
{
	currentParsingThread = nullptr;
	this->pTargetDatabase = pTargetDatabase;
	bIsParsing = false;
}

bool IP2CParser::doReadDatabase(const QString &filePath)
{
	QMutexLocker mutexLocker(&thisLock);

	// This will set proper state whenever and wherever this method finishes.
	ConstructorDestructorParserStateSetter stateSetter(this);

	QFile dataBase(filePath);
	gLog << tr("Parsing IP2C database: %1").arg(dataBase.fileName());

	if (!dataBase.exists()
		|| !dataBase.open(QIODevice::ReadOnly)
		|| !dataBase.isReadable())
	{
		gLog << tr("Unable to open IP2C file.");
		return false;
	}

	QTime time;
	time.start();

	QByteArray dataArray = dataBase.readAll();

	// Read version.
	int pos = 4;
	if (pos >= dataArray.size())
		return false;

	const char *data = dataArray.constData();
	unsigned short version = READINT16(&data[pos]);

	bool wasReadSuccessful = false;
	switch (version)
	{
	case 2:
		wasReadSuccessful = readDatabaseVersion2(dataArray);
		break;

	default:
		wasReadSuccessful = false;
		break;
	}

	if (!wasReadSuccessful)
		return false;

	gLog << tr("IP2C database read in %1 ms. Entries read: %2").arg(time.elapsed()).arg(pTargetDatabase->numKnownEntries());
	return true;
}

void IP2CParser::parsingThreadFinished()
{
	bool bSuccessState = currentParsingThread->bSuccessState;
	gLog << tr("IP2C parsing thread has finished.");

	delete currentParsingThread;
	currentParsingThread = nullptr;

	emit parsingFinished(bSuccessState);
}

bool IP2CParser::readDatabase(const QString &filePath)
{
	bool bSuccess = doReadDatabase(filePath);
	emit parsingFinished(bSuccess);

	return bSuccess;
}

void IP2CParser::readDatabaseThreaded(const QString &filePath)
{
	if (currentParsingThread != nullptr)
		return;

	auto pParsingThread = new ParsingThread(this, filePath);
	connect(pParsingThread, SIGNAL(finished()), this, SLOT(parsingThreadFinished()));

	currentParsingThread = pParsingThread;

	pParsingThread->start();
}

bool IP2CParser::readDatabaseVersion2(const QByteArray &dataArray)
{
	QBuffer buffer;
	buffer.setData(dataArray);
	buffer.open(QIODevice::ReadOnly);
	QDataStream dstream(&buffer);
	dstream.setByteOrder(QDataStream::LittleEndian);
	DataStreamOperatorWrapper stream(&dstream);
	stream.skipRawData(6); // skip file tag and version number

	// We need to store the addresses in such hash table to make sure they
	// are ordered in proper, ascending order. Otherwise the whole library
	// will not work!
	QMap<unsigned, IP2C::IP2CData> hashTable;

	while (stream.hasRemaining())
	{
		// Base entry for each IP read from the file
		IP2C::IP2CData baseEntry;

		baseEntry.countryFullName = QString::fromUtf8(stream.readRawUntilByte('\0'));
		baseEntry.country = QString::fromUtf8(stream.readRawUntilByte('\0'));
		if (!stream.hasRemaining())
			return false;

		quint32 numOfIpBlocks = stream.readQUInt32();

		for (quint32 x = 0; x < numOfIpBlocks; ++x)
		{
			// Create new entry from the base.
			IP2C::IP2CData entry = baseEntry;

			entry.ipStart = stream.readQUInt32();
			if (!stream.hasRemaining())
				return false;
			entry.ipEnd = stream.readQUInt32();

			hashTable[entry.ipStart] = entry;
		}
	}

	pTargetDatabase->setDatabase(hashTable.values());

	return true;
}

////////////////////////////////////////////////////////////////////////////////

IP2CParser::ConstructorDestructorParserStateSetter::ConstructorDestructorParserStateSetter(IP2CParser *pParser)
{
	this->pParser = pParser;
	pParser->bIsParsing = true;
}

IP2CParser::ConstructorDestructorParserStateSetter::~ConstructorDestructorParserStateSetter()
{
	pParser->bIsParsing = false;
}

////////////////////////////////////////////////////////////////////////////////

void IP2CParser::ParsingThread::run()
{
	bSuccessState = pParser->doReadDatabase(filePath);
}
