//------------------------------------------------------------------------------
// odamexmasterclient.cpp
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

#include "global.h"
#include "pluginengineplugin.h"
#include "pluginmasterclient.h"
#include "pluginserver.h"
#include <QTimer>
#include <serverapi/message.h>

class PluginMasterClient::PrivData
{
public:
	int expectedPackets;
	int gotPackets;
	QTimer timeoutTimer;
};
//////////////////////////////////////////////////////////////////////////////
const unsigned RESPONSE_TIMEOUT_MS = 1000;

PluginMasterClient::PluginMasterClient()
	: MasterClient()
{
	d = new PrivData();
	d->expectedPackets = 0;
	d->gotPackets = 0;

	this->connect(&d->timeoutTimer, SIGNAL(timeout()),
		SIGNAL(listUpdated()));

	// Master responder cannot be started in Plugin's init procedure
	// or the plugin will fail to load in VC++ debug mode with
	// "Access Violation" error. Fortunately, this constructor is executed
	// somewhere later, and is a safe place to start the responder.
	// Remember that responder must be started in a thread that can handle
	// Qt events.
	PluginEnginePlugin *plugin = (PluginEnginePlugin *) PluginEnginePlugin::staticInstance();
	if (!plugin->isMasterResponderInstantiated())
		plugin->startMasterResponder();
}

PluginMasterClient::~PluginMasterClient()
{
	delete d;
}

QByteArray PluginMasterClient::createServerListRequest()
{
	d->expectedPackets = 0;
	d->gotPackets = 0;
	return QByteArray("FAKE", 4);
}

const EnginePlugin *PluginMasterClient::plugin() const
{
	return PluginEnginePlugin::staticInstance();
}

MasterClient::Response PluginMasterClient::readMasterResponse(const QByteArray &data)
{
	QStringList ports = QString(data).split(";");
	// First element is amount of expected packets:
	d->expectedPackets = ports.takeFirst().toUInt();
	for (const QString &portEncoded : ports)
	{
		quint16 port = portEncoded.toUShort();
		if (port != 0)
		{
			PluginServer *server = new PluginServer(QHostAddress("127.0.0.1"), port);
			registerNewServer(ServerPtr(server));
		}
	}
	++d->gotPackets;
	if (d->gotPackets >= d->expectedPackets)
	{
		d->timeoutTimer.stop();
		emit listUpdated();
	}
	else
	{
		// Timeout will fire if some of the packets become lost.
		d->timeoutTimer.start(RESPONSE_TIMEOUT_MS);
	}
	return RESPONSE_GOOD;
}
