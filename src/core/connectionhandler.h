//------------------------------------------------------------------------------
// connectionhandler.h
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
// Copyright (C) 2012 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//                    "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------

#ifndef __CONNECTIONHANDLER_H__
#define __CONNECTIONHANDLER_H__

#include "dptr.h"
#include "serverapi/serverptr.h"
#include <QObject>

class CommandLineInfo;
class JoinError;
class Server;
class QUrl;
class QWidget;

/**
 * This is needed so we can register the ConnectionHandler with
 * QDesktopServices.
 */
class PluginUrlHandler : public QObject
{
	Q_OBJECT

public:
	static void registerAll();
	static void registerScheme(const QString &scheme);

public slots:
	void handleUrl(const QUrl &url);

private:
	static PluginUrlHandler *instance;
};

class ConnectionHandler : public QObject
{
	Q_OBJECT

public:
	ConnectionHandler(ServerPtr server, QWidget *parentWidget = nullptr);
	~ConnectionHandler() override;

	void run();

	static ConnectionHandler *connectByUrl(const QUrl &url);

signals:
	void finished(int response);

private:
	DPtr<ConnectionHandler> d;

	void finish(int response);
	void refreshToJoin();
	void runCommandLine(const CommandLineInfo &cli);

private slots:
	void buildJoinCommandLine();
	void checkResponse(const ServerPtr &server, int response);
	void onCommandLineBuildFinished();
};

#endif
