//------------------------------------------------------------------------------
// applogger.cpp
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

#include "serverapi/message.h"
#include "apprunner.h"
#include "applogger.h"
#include "log.h"

#include <QProcess>
#include <QSharedPointer>

#ifndef Q_OS_WIN32
#include <QTextStream>
#include <signal.h>
#endif

DClass<AppLogger>
{
	public:
		QSharedPointer<QProcess> process;
		OutputAppLogger output;
};

DPointered(AppLogger)

AppLogger::AppLogger(const CommandLineInfo& cmdInfo)
{
	d->process.reset(new QProcess());
	d->process->setProgram(cmdInfo.executable.absoluteFilePath());
	d->process->setArguments(cmdInfo.args);

	connect(d->process.data(), SIGNAL(readyReadStandardError()), this, SLOT(errorDataReady()));
	connect(d->process.data(), SIGNAL(readyReadStandardOutput()), this, SLOT(outputDataReady()));
}

AppLogger::~AppLogger()
{
	if (d->process->state() != QProcess::ProcessState::NotRunning)
		terminateProcess(*d->process.data());
}

const OutputAppLogger& AppLogger::runAndLogExecutableUntilTime(int timeout)
{
	d->process->start();
	if(d->process->waitForStarted())
	{
		if(!d->process->waitForFinished(timeout))
			terminateProcess(*d->process.data());
	}
	else
		gLog << tr("The file could not be started: %1").arg(d->process->program());
	return d->output;
}

void AppLogger::terminateProcess(QProcess &process)
{
	#ifndef Q_OS_WIN32
	if (process.processId() != 0)
	{
		// On non-Windows systems try to find child processes and kill them.
		// Unfortunately it doesn't look like Qt can help us here (which kind of
		// makes sense considering it seems like there no real API to do it).
		// Ideally only one process started and we don't have to do this.
		auto ps = new QProcess();
		ps->start("ps", {"h", "--ppid", QString::number(process.processId()), "-o", "pid"});
		ps->waitForFinished();
		QByteArray psOutput = ps->readAllStandardOutput();
		QTextStream stream(&psOutput);
		stream.skipWhiteSpace();
		while (!stream.atEnd())
		{
			unsigned int cpid;
			stream >> cpid;
			stream.skipWhiteSpace();
			kill(cpid, SIGTERM);
		}
		delete ps;
	}
	#endif

	process.disconnect(); // TODO Disconnect *Everything*??
	process.terminate();
	process.waitForFinished(1000);
	process.close();
	process.waitForFinished();
}

void AppLogger::errorDataReady()
{
	d->output.stderror.append(QString(d->process->readAllStandardError()));
}

void AppLogger::outputDataReady()
{
	d->output.stdoutput.append(QString(d->process->readAllStandardOutput()));
}
