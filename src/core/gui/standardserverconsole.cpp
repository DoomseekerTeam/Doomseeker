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
#include "applogger.h"
#include <QMessageBox>

#ifndef Q_OS_WIN32
#include <QTextStream>
#include <signal.h>
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
	AppLogger::terminateProcess(*process);
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
