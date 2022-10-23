//------------------------------------------------------------------------------
// rconprotocol.cpp
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
// Copyright (C) 2010 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
//------------------------------------------------------------------------------
#include "rconprotocol.h"
#include "serverapi/server.h"

#include <cassert>

DClass<RConProtocol>
{
public:
	bool connected;
	QList<Player> players;
	QHostAddress serverAddress;
	quint16 serverPort;
	QUdpSocket socket;

	void (RConProtocol::*disconnectFromServer)();
	void (RConProtocol::*sendCommand)(const QString &);
	void (RConProtocol::*sendPassword)(const QString &);
};

DPointeredNoCopy(RConProtocol)

RConProtocol::RConProtocol(ServerPtr server)
{
	d->connected = false;
	d->serverAddress = server->address();
	d->serverPort = server->port();

	d->socket.bind();

	set_disconnectFromServer(&RConProtocol::disconnectFromServer_default);
	set_sendCommand(&RConProtocol::sendCommand_default);
	set_sendPassword(&RConProtocol::sendPassword_default);
}

RConProtocol::~RConProtocol()
{
	d->socket.close();
}

POLYMORPHIC_DEFINE(void, RConProtocol, disconnectFromServer, (), ())
POLYMORPHIC_DEFINE(void, RConProtocol, sendCommand, (const QString &cmd), (cmd))
POLYMORPHIC_DEFINE(void, RConProtocol, sendPassword, (const QString &password), (password))

const QHostAddress &RConProtocol::address() const
{
	return d->serverAddress;
}

void RConProtocol::disconnectFromServer_default()
{
	assert(0 && "RConProtocol::disconnectFromServer() is not implemented");
}

bool RConProtocol::isConnected() const
{
	return d->connected;
}

quint16 RConProtocol::port() const
{
	return d->serverPort;
}

const QList<Player> &RConProtocol::players() const
{
	return d->players;
}

QList<Player> &RConProtocol::playersMutable()
{
	return d->players;
}

void RConProtocol::sendCommand_default(const QString &cmd)
{
	Q_UNUSED(cmd);
	assert(0 && "RConProtocol::sendCommand() is not implemented");
}

void RConProtocol::sendPassword_default(const QString &password)
{
	Q_UNUSED(password);
	assert(0 && "RConProtocol::sendPassword() is not implemented");
}

void RConProtocol::setConnected(bool b)
{
	d->connected = b;
}

QUdpSocket &RConProtocol::socket()
{
	return d->socket;
}
