//------------------------------------------------------------------------------
// udpsocketpool.h
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
// Copyright (C) 2017 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#ifndef id42cca172_73df_4d48_99c6_32b537985549
#define id42cca172_73df_4d48_99c6_32b537985549

#include "dptr.h"
#include <QHostAddress>
#include <QUdpSocket>

class UdpSocketPool : public QObject
{
	Q_OBJECT
	Q_DISABLE_COPY(UdpSocketPool);

public:
	UdpSocketPool(int sliceSize = 25);
	~UdpSocketPool();

	QUdpSocket *acquire(const QHostAddress &address, quint16 port);
	QUdpSocket *acquireMasterSocket();
	void releaseAll();

	bool hasPendingDatagrams() const;
	QByteArray readNextDatagram(QHostAddress *address = nullptr, quint16 *port = nullptr);

signals:
	void readyRead();

private:
	DPtr<UdpSocketPool> d;
};

#endif
