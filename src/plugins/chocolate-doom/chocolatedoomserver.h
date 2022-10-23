//------------------------------------------------------------------------------
// chocolatedoomserver.h
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

#ifndef __CHOCOLATEDOOMSERVER_H__
#define __CHOCOLATEDOOMSERVER_H__

#include "serverapi/server.h"

class EnginePlugin;

class ChocolateDoomServer : public Server
{
	Q_OBJECT

public:
	ChocolateDoomServer(const QHostAddress &address, unsigned short port);

	GameClientRunner *gameRunner() override;

	EnginePlugin *plugin() const override;

protected:
	QString version;
	unsigned int serverState;
	unsigned int game;
	unsigned int gameMission;
	QString description;

	QByteArray createSendRequest();
	Response readRequest(const QByteArray &data);

private:
	void interpretIwad(int mission, int gameMode);
};

#endif /* __CHOCOLATEDOOMSERVER_H__ */
