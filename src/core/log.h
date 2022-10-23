//------------------------------------------------------------------------------
// log.h
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
// Copyright (C) 2009 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef __LOG_H_
#define __LOG_H_

#define gLog Log::instance

#include "dptr.h"
#include "global.h"
#include <QMutex>
#include <QObject>

/**
 * @ingroup group_pluginapi
 * @brief Log manager.
 */
class MAIN_EXPORT Log : public QObject
{
	Q_OBJECT

public:
	/**
	 * @brief Global instance of the logger.
	 */
	static Log instance;

	Log();
	virtual ~Log() override;

	/**
	 * @brief Timestamps are in format [hh:mm:ss]. Enabled by default.
	 */
	bool areTimestampsEnabled() const;

	/**
	 * @brief Entire content of the log.
	 */
	const QString &content() const;

	/**
	 * @brief If true all new entries will also be printed to stderr.
	 *
	 * Otherwise entries are stored only in the logContent member.
	 * Default is true.
	 */
	bool isPrintingToStderr() const;
	void setPrintingToStderr(bool b);

	/**
	 * @brief If true all new entries will also be printed to stderr.
	 *
	 * Otherwise entries are stored only in the logContent member.
	 * Default is true.
	 *
	 * @deprecated Use isPrintingToStderr().
	 */
	bool isPrintingToStdout() const;
	/**
	 * @deprecated Use setPrintingToStderr().
	 */
	void setPrintingToStdout(bool b);

	void setTimestampsEnabled(bool b);

	/**
	 * @brief Executes addEntry(const QString&).
	 */
	Log &operator<<(const QString &string);

public slots:
	/**
	 * Prints the string to specified output and appends a '\\n' character
	 * to the end of that string. Additional formatting is also applied
	 * if certain flags are enabled.
	 * @see newEntry()
	 */
	void addEntry(const QString &string);

	/**
	 * Prints the string to specified output AS IT IS.
	 * @see addEntry()
	 * @see newEntry()
	 */
	void addUnformattedEntry(const QString &string);

	/**
	 * @brief Clears log content stored in the memory.
	 */
	void clearContent();

signals:
	/**
	 * Sends out an already formatted new entry. Please note that there
	 * are no additional newline character appended to the entry string.
	 * This is emitted by addEntry() and addUnformattedEntry() slots.
	 */
	void newEntry(const QString &entry);

private:
	DPtr<Log> d;
};

#endif
