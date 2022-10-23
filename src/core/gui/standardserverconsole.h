//------------------------------------------------------------------------------
// standardserverconsole.h
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
#ifndef __STANDARD_SERVER_CONSOLE_H_
#define __STANDARD_SERVER_CONSOLE_H_

#include "gui/widgets/serverconsole.h"

#include <QMainWindow>
#include <QProcess>
#include <QString>
#include <QStringList>

class Server;

/**
 * This is a wrapper for the standard input and output of a console app.  This
 * is mainly for use on Linux, but it is compiled on Windows.
 */
class StandardServerConsole : public QMainWindow
{
	Q_OBJECT

public:
	StandardServerConsole(const QIcon &icon, const QString &program, const QStringList &arguments);
	~StandardServerConsole() override;

private slots:
	void errorDataReady();
	void finish(int exitCode, QProcess::ExitStatus exitStatus);
	void outputDataReady();
	void writeToStandardInput(const QString &message);

private:
	ServerConsole *console;
	QProcess *process;
};

#endif
