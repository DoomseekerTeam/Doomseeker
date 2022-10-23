//------------------------------------------------------------------------------
// huffmanudpsocket.cpp
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
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "huffmanudpsocket.h"

#include "huffmanqt.h"
#include <cassert>

class HuffmanUdpSocket::PrivData
{
public:
	QUdpSocket *socket;
};


HuffmanUdpSocket::HuffmanUdpSocket(QUdpSocket *socket)
{
	d = new PrivData();
	d->socket = socket;
}

HuffmanUdpSocket::~HuffmanUdpSocket()
{
	delete d;
}

bool HuffmanUdpSocket::hasPendingDatagrams() const
{
	assert(!isNull());
	return d->socket->hasPendingDatagrams();
}

bool HuffmanUdpSocket::isNull() const
{
	return d->socket == nullptr;
}

QByteArray HuffmanUdpSocket::readDatagram(QHostAddress *address, quint16 *port)
{
	assert(!isNull());
	qint64 size = d->socket->pendingDatagramSize();
	char *data = new char[size];
	d->socket->readDatagram(data, size, address, port);
	QByteArray decoded = HuffmanQt::decode(data, size);
	delete [] data;
	return decoded;
}

void HuffmanUdpSocket::setSocket(QUdpSocket *socket)
{
	d->socket = socket;
}

bool HuffmanUdpSocket::writeDatagram(const QByteArray &datagram,
	const QHostAddress &host, quint16 port)
{
	assert(!isNull());
	QByteArray encoded = HuffmanQt::encode(datagram);
	qint64 written = d->socket->writeDatagram(encoded, host, port);
	return written == datagram.size();
}

bool HuffmanUdpSocket::writeDatagram(const char *data, int size,
	const QHostAddress &host, quint16 port)
{
	return writeDatagram(QByteArray(data, size), host, port);
}
