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

#include "ip2c.h"
#include <QMutex>
#include <QThread>

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
class IP2CParser : public QObject
{
	Q_OBJECT

public:
	IP2CParser(IP2C *pTargetDatabase);

	/**
	 * @brief Retrieves the IP2C database this parser operates on.
	 */
	IP2C *ip2c() const
	{
		return pTargetDatabase;
	}

	/**
	 * @brief For multi-threading purposes. If this is true it is not
	 * recommended to delete this object nor the underlying IP2C database.
	 */
	bool isParsing() const
	{
		return bIsParsing;
	}

	bool readDatabase(const QString &filePath);
	void readDatabaseThreaded(const QString &filePath);

signals:
	/**
	 * @brief A signal emitted when parser finishes its job.
	 *
	 * It is not safe to delete IP2CParser object after readDatabase()
	 * call and before this signal is emitted.
	 */
	void parsingFinished(bool bSuccess);

protected:
	/**
	 * @brief Sets states for IP2C when being constructed and destructed.
	 *
	 * //TODO: A nicer name?
	 */
	class ConstructorDestructorParserStateSetter
	{
	public:
		ConstructorDestructorParserStateSetter(IP2CParser *pParser);
		~ConstructorDestructorParserStateSetter();

	private:
		IP2CParser *pParser;
	};

	class ParsingThread : public QThread
	{
	public:
		bool bSuccessState;
		QString filePath;
		IP2CParser *pParser;


		ParsingThread(IP2CParser *pParser, const QString &filePath)
		{
			bSuccessState = false;
			this->filePath = filePath;
			this->pParser = pParser;
		}

		void run() override;
	};

	/**
	 * Key value is the abbreviation of the country name.
	 */
	typedef QHash<QString, QList<IP2C::IP2CData> > Countries;
	typedef QHash<QString, QList<IP2C::IP2CData> >::iterator CountriesIt;
	typedef QHash<QString, QList<IP2C::IP2CData> >::const_iterator CountriesConstIt;

	bool bIsParsing;
	ParsingThread *currentParsingThread;

	/**
	 * @brief Database to which the IP2C parser will save the data it
	 * retrieves from IP2C file.
	 *
	 * Since IP2CParser is prepared to work in a separate thread it is
	 * not advised to delete the IP2C object before parsing is complete.
	 */
	IP2C *pTargetDatabase;

	QMutex thisLock;

	bool doReadDatabase(const QString &filePath);
	bool readDatabaseVersion2(const QByteArray &dataArray);

protected slots:
	void parsingThreadFinished();
};

#endif
