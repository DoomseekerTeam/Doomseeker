//------------------------------------------------------------------------------
// standardserverconsole.cpp
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
#include "standardserverconsole.h"

#include <QMessageBox>

#ifndef Q_OS_WIN32
#include <csignal>
#include <QTextStream>
#endif

StandardServerConsole::StandardServerConsole(const QIcon &icon, const QString &program, const QStringList &arguments)
{
	// Have the console delete itself
	setAttribute(Qt::WA_DeleteOnClose);

	// Set up the window.
	setWindowTitle("Server Console");
	setWindowIcon(icon);
	resize(640, 400);

	// Add our console widget
	console = new ServerConsole();
	setCentralWidget(console);

	// Start the process
	process = new QProcess();
	process->start(program, arguments);
	if (process->waitForStarted())
	{
		show();
		connect(console, SIGNAL(messageSent(const QString&)), this, SLOT(writeToStandardInput(const QString&)));
		connect(process, SIGNAL(readyReadStandardError()), this, SLOT(errorDataReady()));
		connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(outputDataReady()));
		connect(process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finish(int,QProcess::ExitStatus)));
	}
	else // Didn't start get rid of this console.
		close();
}

StandardServerConsole::~StandardServerConsole()
{
	#ifndef Q_OS_WIN32
	if (process->processId() != 0)
	{
		// On non-Windows systems try to find child processes and kill them.
		// Unfortunately it doesn't look like Qt can help us here (which kind of
		// makes sense considering it seems like there no real API to do it).
		// Ideally only one process started and we don't have to do this.
		auto ps = new QProcess();
		ps->start("ps", {"h", "--ppid", QString::number(process->processId()), "-o", "pid"});
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

	process->disconnect(this);
	process->terminate();
	process->waitForFinished(1000);
	process->close();
	process->waitForFinished();
	delete process;
}

void StandardServerConsole::errorDataReady()
{
	console->appendMessage(QString(process->readAllStandardError()));
}

void StandardServerConsole::finish(int exitCode, QProcess::ExitStatus exitStatus)
{
	if (exitStatus == QProcess::CrashExit && exitCode != 0)
		QMessageBox::critical(this, "Server crash", QString("The server terminated unexpectedly with exit code: %1").arg(exitCode));

	close();
}

void StandardServerConsole::outputDataReady()
{
	console->appendMessage(QString(process->readAllStandardOutput()));
}

void StandardServerConsole::writeToStandardInput(const QString &message)
{
	process->write((message + "\n").toUtf8());
}
