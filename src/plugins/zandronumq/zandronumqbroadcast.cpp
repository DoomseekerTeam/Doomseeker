//------------------------------------------------------------------------------
// zandronumqbroadcast.cpp
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
// Copyright (C) 2015 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "zandronumqbroadcast.h"

#include "zandronumqengineplugin.h"
#include "zandronumqserver.h"
#include <log.h>
#include <QElapsedTimer>
#include <QTimer>
#include <QUdpSocket>
#include <serverapi/server.h>

DClass<ZandronumqBroadcast>
{
public:
	class HostPort
	{
	public:
		QHostAddress host;
		unsigned short port;

		HostPort(const QHostAddress &host, unsigned short port)
		{
			this->host = host;
			this->port = port;
		}

		bool operator<(const HostPort &other) const
		{
			return host != other.host ? host.toString() < other.host.toString() : port < other.port;
		}
	};

	class LanServer
	{
	public:
		ServerPtr server;
		QElapsedTimer lastActivityTime;

		LanServer()
		{
			lastActivityTime.start();
		}

		LanServer(ServerPtr server)
		{
			this->server = server;
			lastActivityTime.start();
		}
	};

	static const int BROADCAST_LISTEN_PORT = 15102;
	static const int TERMINATE_OLD_AGE_MS = 10000;

	bool bindFailureLogged;
	QUdpSocket *socket;
	QMap<HostPort, LanServer> servers;
	QTimer activityTimer;
};
DPointeredNoCopy(ZandronumqBroadcast)

ZandronumqBroadcast::ZandronumqBroadcast()
{
	d->bindFailureLogged = false;
	d->socket = nullptr;
}

ZandronumqBroadcast::~ZandronumqBroadcast()
{
}

EnginePlugin *ZandronumqBroadcast::plugin() const
{
	return ZandronumqEnginePlugin::staticInstance();
}

void ZandronumqBroadcast::start()
{
	d->socket = new QUdpSocket(this);
	this->connect(d->socket, SIGNAL(readyRead()), SLOT(readAllPendingDatagrams()));
	bindSocket();

	this->connect(&d->activityTimer, SIGNAL(timeout()), SLOT(terminateOldServers()));
	this->connect(&d->activityTimer, SIGNAL(timeout()), SLOT(readAllPendingDatagrams()));
	d->activityTimer.start(PrivData<ZandronumqBroadcast>::TERMINATE_OLD_AGE_MS);
}

void ZandronumqBroadcast::bindSocket()
{
	const int port = PrivData<ZandronumqBroadcast>::BROADCAST_LISTEN_PORT;
	bool bound = d->socket->bind(QHostAddress::Any, port,
		QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
	if (bound)
	{
		gLog << tr("Listening to Q-Zandronum's LAN servers broadcasts on port %1.").arg(port);
	}
	else
	{
		if (!d->bindFailureLogged)
		{
			gLog << tr("Failed to bind Q-Zandronum's LAN broadcasts listening socket on port %1. "
					"Will keep retrying silently.").arg(port);
			d->bindFailureLogged = true;
		}
		QTimer::singleShot(10000, this, SLOT(bindSocket()));
	}
}

void ZandronumqBroadcast::readAllPendingDatagrams()
{
	while (d->socket->hasPendingDatagrams())
	{
		QByteArray datagram;
		datagram.resize(d->socket->pendingDatagramSize());
		QHostAddress sender;
		unsigned short senderPort;

		d->socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
		if (!isEnabled())
		{
			// just drop the packets
			continue;
		}

		ServerPtr server = ZandronumqEnginePlugin::staticInstance()->server(sender, senderPort);
		if (server->readRefreshQueryResponse(datagram) == Server::RESPONSE_GOOD)
		{
			PrivData<ZandronumqBroadcast>::HostPort addr(sender, senderPort);
			if (d->servers.contains(addr))
			{
				PrivData<ZandronumqBroadcast>::LanServer &lanserver = d->servers[addr];
				lanserver.lastActivityTime.start();
				emit serverDetected(lanserver.server, false);
			}
			else
			{
				// Rebuild server instance as we need to refresh it
				// anyway and we don't want whatever was intepreted
				// from broadcast to pollute the status.
				server = ZandronumqEnginePlugin::staticInstance()->server(sender, senderPort);
				d->servers[addr] = server;
				emit serverDetected(server, true);
			}
		}
	}
}

void ZandronumqBroadcast::terminateOldServers()
{
	QMutableMapIterator<PrivData<ZandronumqBroadcast>::HostPort,
		PrivData<ZandronumqBroadcast>::LanServer> it(d->servers);
	while (it.hasNext())
	{
		it.next();
		PrivData<ZandronumqBroadcast>::LanServer &server = it.value();
		if (server.lastActivityTime.elapsed() > PrivData<ZandronumqBroadcast>::TERMINATE_OLD_AGE_MS)
		{
			emit serverLost(server.server);
			it.remove();
		}
	}
}
