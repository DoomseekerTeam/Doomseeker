//------------------------------------------------------------------------------
// serverresponder.cpp
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
#include "serverresponder.h"

#include "responder/awaitingclient.h"
#include <QTimer>
#include <QUdpSocket>

class ServerResponder::PrivData
{
public:
	QList<AwaitingClient> awaitingClients;
	int responseFailChance;
	QUdpSocket *socket;
};
///////////////////////////////////////////////////////////////////////////////
ServerResponder::ServerResponder(QObject *parent)
	: QObject(parent)
{
	d = new PrivData();
	d->socket = new QUdpSocket();
	d->responseFailChance = 0;
	this->connect(d->socket, SIGNAL(readyRead()),
		SLOT(readPendingDatagrams()));
}

ServerResponder::~ServerResponder()
{
	delete d;
}

bool ServerResponder::bind(unsigned short port)
{
	return d->socket->bind(port);
}

unsigned short ServerResponder::port() const
{
	return d->socket->localPort();
}

void ServerResponder::readPendingDatagrams()
{
	while (d->socket->hasPendingDatagrams())
	{
		readPendingDatagram();
	}
}

void ServerResponder::readPendingDatagram()
{
	QHostAddress address;
	unsigned short port = 0;
	const int MAGIC_SIZE = 4;
	char buffer[MAGIC_SIZE];
	d->socket->readDatagram(buffer, MAGIC_SIZE, &address, &port);
	QByteArray packet = QByteArray(buffer, MAGIC_SIZE);
	if (packet == "FAKF")
	{
		if (shouldRespond())
		{
			d->awaitingClients.append(AwaitingClient(address, port));
			int msec = 10 + qrand() % 300;
			QTimer::singleShot(msec, this, SLOT(respond()));
		}
	}
}

void ServerResponder::respond()
{
	AwaitingClient client = d->awaitingClients.takeFirst();
	QByteArray response = QByteArray("FAKF");
	response += QString::number(port()).toUtf8();
	d->socket->writeDatagram(response, client.address, client.port);
}

void ServerResponder::setResponseFailChance(unsigned chance)
{
	d->responseFailChance = chance;
}

bool ServerResponder::shouldRespond() const
{
	if (d->responseFailChance == 0)
		return true;
	int roll = qrand() % MAX_CHANCE + 1;
	return roll > d->responseFailChance;
}
