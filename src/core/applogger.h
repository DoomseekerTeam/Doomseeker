//------------------------------------------------------------------------------
// applogger.h
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
// Copyright (C) 2019 Pol Marcet Sardà <polmarcetsarda@gmail.com>
//------------------------------------------------------------------------------
#ifndef __APP_LOGGER_H_
#define __APP_LOGGER_H_

#include "dptr.h"
#include "global.h"
#include "apprunner.h"

#include <QObject>

class QProcess;

/**
 * @brief Holds output data sent trough the stdout and stderr of a program.
 *
 */
struct OutputAppLogger
{
	QStringList stdoutput;
	QStringList stderror;
};

/**
 * @ingroup group_pluginapi
 * @brief Allows you to run another program and redirect its output.
 */
class MAIN_EXPORT AppLogger : public QObject
{
	Q_OBJECT

public:
	AppLogger(const CommandLineInfo& cmdInfo);
	~AppLogger();

	/**
	 * @brief Runs the program specified in the constructor for a set of time.
	 *
	 * @param time Time until the program is terminated (SIGTERM or equivalent).
	 *        If it's set to -1, the program will run indefinitely.
	 *        Time is in miliseconds.
	 *
	 * @return const OutputAppLogger& The output of the hole execution.
	 */
	const OutputAppLogger& runAndLogExecutableUntilTime(int time);

	/**
	 * @brief terminates (SIGTERM or equivalent) a given process.
	 */
	static void terminateProcess(QProcess &process);

private slots:
	void errorDataReady();
	void outputDataReady();

private:
	DPtr<AppLogger> d;
};

#endif
