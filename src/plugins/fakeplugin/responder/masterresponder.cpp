//------------------------------------------------------------------------------
// masterresponder.cpp
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
#include "masterresponder.h"

#include "responder/awaitingclient.h"
#include "responder/respondercfg.h"
#include "responder/serverresponder.h"
#include <log.h>
#include <QTimer>
#include <QUdpSocket>

class MasterResponder::PrivData
{
public:
	QList<AwaitingClient> awaitingClients;
	QList<ServerResponder *> serverResponders;
	QUdpSocket *socket;
};
///////////////////////////////////////////////////////////////////////////////
MasterResponder::MasterResponder(QObject *parent)
	: QObject(parent)
{
	d = new PrivData();
	d->socket = new QUdpSocket();
	this->connect(d->socket, SIGNAL(readyRead()),
		SLOT(readPendingDatagrams()));
}

MasterResponder::~MasterResponder()
{
	qDeleteAll(d->serverResponders);
	delete d->socket;
	delete d;
}

bool MasterResponder::areServerRespondersRunning()
{
	return !d->serverResponders.isEmpty();
}

bool MasterResponder::bind(unsigned short port)
{
	bool result = d->socket->bind(port);
	if (!result)
	{
		gLog << QString("FakePlugin, failed to bind master server on port %1."
				" Plugin will not be functional.").arg(port);
	}
	return result;
}

void MasterResponder::bindServer(ServerResponder *server, unsigned short port)
{
	if (server->bind(port))
		d->serverResponders.append(server);
	else
	{
		gLog << QString("FakePlugin, failed to bind server on port: %1").arg(port);
		delete server;
	}
}

QList<QByteArray> MasterResponder::buildResponsePackets()
{
	const unsigned short maxPortsListsLength = qMax((unsigned short)20,
		ResponderCfg::maxPacketSize());
	QList<QByteArray> portsLists;
	QString packet = "";
	for (const ServerResponder *server : d->serverResponders)
	{
		if (server->port() == 0)
			continue;
		QString currentPort = QString::number(server->port());
		QString tmp = packet + ";" + currentPort;
		if (tmp.toUtf8().length() > maxPortsListsLength)
		{
			portsLists.append(packet.toUtf8());
			packet = currentPort;
		}
		else
			packet = tmp;
	}
	portsLists.append(packet.toUtf8());
	// Now that we have the number of packets, we can prepend
	// that number to each packet.
	QList<QByteArray> result;
	for (const QByteArray &portsList : portsLists)
	{
		QString number = QString::number(portsLists.size()) + ";";
		result.append(number.toUtf8() + portsList);
	}
	return result;
}

void MasterResponder::readPendingDatagrams()
{
	while (d->socket->hasPendingDatagrams())
	{
		readPendingDatagram();
	}
}

void MasterResponder::readPendingDatagram()
{
	// This mocker awaits for "FAKE" magic number to arrive,
	// then sleeps for a moment to simulate network
	// behavior, and then responds.
	QHostAddress address;
	quint16 port = 0;
	const int MAGIC_SIZE = 4;
	char *buffer = new char[MAGIC_SIZE];
	d->socket->readDatagram(buffer, MAGIC_SIZE, &address, &port);
	QByteArray packet = QByteArray(buffer, MAGIC_SIZE);
	delete[] buffer;

	gLog << QString("FakePlugin, received master datagram. Magic = %1"
			", Client: '%2:%3'").arg(QString(packet))
		.arg(address.toString()).arg(port);
	if (packet == "FAKE")
	{
		d->awaitingClients.append(AwaitingClient(address, port));
		int msec = 10 + qrand() % 300;
		gLog << QString("FakePlugin, sending master response in: %1ms.").arg(msec);
		QTimer::singleShot(msec, this, SLOT(respond()));
	}
}

void MasterResponder::respond()
{
	if (!areServerRespondersRunning())
		startServerResponders();

	AwaitingClient client = d->awaitingClients.takeFirst();
	QList<QByteArray> packets = buildResponsePackets();
	gLog << QString("FakePlugin, sending master response now. "
			"Packets count = %1, Target client: '%2:%3'").arg(packets.size())
		.arg(client.address.toString()).arg(client.port);
	for (const QByteArray &packet : packets)
	{
		gLog << QString("FakePlugin, Sending master packet, size: %1").arg(packet.size());
		d->socket->writeDatagram(packet, client.address, client.port);
	}
}

void MasterResponder::startServerResponders()
{
	gLog << "FakePlugin, starting server responders.";
	unsigned short port = ResponderCfg::serverPortBase();
	for (int i = 0; i < ResponderCfg::numValidServers(); ++i)
	{
		ServerResponder *server = new ServerResponder();
		bindServer(server, port);
		++port;
	}
	for (int i = 0; i < ResponderCfg::numNotRespondingServers(); ++i)
	{
		ServerResponder *server = new ServerResponder();
		server->setResponseFailChance(ServerResponder::MAX_CHANCE);
		bindServer(server, port);
		++port;
	}
	gLog << QString("FakePlugin, started %1 servers").arg(d->serverResponders.size());
}
