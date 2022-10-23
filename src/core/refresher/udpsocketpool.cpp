//------------------------------------------------------------------------------
// udpsocketpool.cpp
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
#include "udpsocketpool.h"

#include "refresher/hostport.h"
#include <QList>
#include <QUdpSocket>

DClass<UdpSocketPool>
{
public:
	class Asset
	{
	public:
		QUdpSocket *socket;
		QSet<HostPort> addresses;
		bool valid;

		Asset()
		{
			socket = new QUdpSocket();
			#ifdef Q_OS_OPENBSD
			valid = socket->bind(QHostAddress("0.0.0.0"));
			#else
			valid = socket->bind();
			#endif
		}

		~Asset()
		{
			delete socket;
		}

		void addAddress(const HostPort &hostPort)
		{
			addresses.insert(hostPort);
		}

		bool hasAddress(const HostPort &hostPort) const
		{
			return addresses.contains(hostPort);
		}

		int size() const
		{
			return addresses.size();
		}
	};

	QList<Asset *> pool;
	int sliceSize;
};
DPointeredNoCopy(UdpSocketPool)

UdpSocketPool::UdpSocketPool(int sliceSize)
{
	d->sliceSize = qMax(1, sliceSize);
}

UdpSocketPool::~UdpSocketPool()
{
	qDeleteAll(d->pool);
}

QUdpSocket *UdpSocketPool::acquire(const QHostAddress &address, quint16 port)
{
	HostPort hostPort(address, port);
	// Return if already acquired.
	for (PrivData<UdpSocketPool>::Asset *asset : d->pool)
	{
		if (asset->hasAddress(hostPort))
		{
			return asset->socket;
		}
	}
	// Acquire existing.
	for (PrivData<UdpSocketPool>::Asset *asset : d->pool)
	{
		if (asset->size() < d->sliceSize)
		{
			asset->addAddress(hostPort);
			return asset->socket;
		}
	}
	// Acquire new.
	auto asset = new PrivData<UdpSocketPool>::Asset();
	if (asset->valid)
	{
		this->connect(asset->socket, SIGNAL(readyRead()), SIGNAL(readyRead()));
		asset->addAddress(hostPort);
		d->pool << asset;
		return asset->socket;
	}
	else
	{
		delete asset;
		return nullptr;
	}
}

QUdpSocket *UdpSocketPool::acquireMasterSocket()
{
	return acquire(QHostAddress("0.0.0.0"), 0);
}

void UdpSocketPool::releaseAll()
{
	qDeleteAll(d->pool);
	d->pool.clear();
}

bool UdpSocketPool::hasPendingDatagrams() const
{
	for (PrivData<UdpSocketPool>::Asset *asset : d->pool)
	{
		if (asset->socket->hasPendingDatagrams())
			return true;
	}
	return false;
}

QByteArray UdpSocketPool::readNextDatagram(QHostAddress *address, quint16 *port)
{
	for (PrivData<UdpSocketPool>::Asset *asset : d->pool)
	{
		QUdpSocket *socket = asset->socket;
		if (socket->hasPendingDatagrams())
		{
			int size = socket->pendingDatagramSize();
			char *buffer = new char[size];
			socket->readDatagram(buffer, size, address, port);
			QByteArray data(buffer, size);
			delete [] buffer;
			return data;
		}
	}
	return QByteArray();
}
