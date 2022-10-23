//------------------------------------------------------------------------------
// huffmanudpsocket.h
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
#ifndef id4301ca8a_c4db_48b7_860b_f1299bd1ff2a
#define id4301ca8a_c4db_48b7_860b_f1299bd1ff2a

#include <QByteArray>
#include <QHostAddress>
#include <QUdpSocket>

class HuffmanUdpSocket
{
public:
	HuffmanUdpSocket(QUdpSocket *socket = nullptr);
	~HuffmanUdpSocket();

	bool hasPendingDatagrams() const;
	bool isNull() const;
	QByteArray readDatagram(QHostAddress *address = nullptr, quint16 *port = nullptr);
	void setSocket(QUdpSocket *socket);
	bool writeDatagram(const QByteArray &datagram, const QHostAddress &host, quint16 port);
	bool writeDatagram(const char *data, int size, const QHostAddress &host, quint16 port);

private:
	class PrivData;
	PrivData *d;
};

#endif
