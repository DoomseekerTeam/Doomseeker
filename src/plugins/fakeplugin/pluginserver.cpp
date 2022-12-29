//------------------------------------------------------------------------------
// chocolatedoomserver.cpp
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
// Copyright (C) 2013 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "pluginserver.h"

#include "global.h"
#include "pluginengineplugin.h"
#include "serverapi/playerslist.h"

PluginServer::PluginServer(const QHostAddress &address, quint16 port)
	: Server(address, port)
{
	set_readRequest(&PluginServer::readRequest);
	set_createSendRequest(&PluginServer::createSendRequest);
}

EnginePlugin *PluginServer::plugin() const
{
	return PluginEnginePlugin::staticInstance();
}

QByteArray PluginServer::createSendRequest()
{
	return QByteArray("FAKF", 4);
}

Server::Response PluginServer::readRequest(const QByteArray &data)
{
	if (data.left(4) != "FAKF")
		return RESPONSE_BAD;

	QList<QByteArray> tokens = data.split(';');
	if (data.size() < 3)
		return RESPONSE_BAD;

	const QByteArray &status = tokens[2];

	this->setName(QString("Fake Server %1").arg(this->port()));

	if (status == "OK")
		return RESPONSE_GOOD;
	else if (status == "BAN")
		return RESPONSE_BANNED;
	else if (status == "WAIT")
		return RESPONSE_WAIT;
	else
		return RESPONSE_BAD;
}
