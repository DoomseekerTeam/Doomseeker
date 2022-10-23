//------------------------------------------------------------------------------
// Turok2Exmasterclient.cpp
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
// Copyright (C) 2017 "Edward Richardson" <Edward850@crantime.org>
//------------------------------------------------------------------------------
#include <QBuffer>
#include <QDataStream>

#include "datastreamoperatorwrapper.h"
#include "global.h"
#include "turok2exengineplugin.h"
#include "turok2exmasterclient.h"
#include "turok2exserver.h"

#define NETM_MSLIST 11

Turok2ExMasterClient::Turok2ExMasterClient()
	: MasterClient(),
	seedIP(0),
	seedPort(0)
{
}

QByteArray Turok2ExMasterClient::createServerListRequest()
{
	const int size = sizeof(int8_t) + sizeof(int32_t) + sizeof(int32_t) + sizeof(int16_t);
	char challenge[size];
	WRITEINT8(challenge, NETM_MSLIST);
	WRITEINT32(&challenge[1], GENERICKEY);
	WRITEINT32(&challenge[5], seedIP);
	WRITEINT16(&challenge[9], seedPort);
	return QByteArray (challenge, size);
}

const EnginePlugin *Turok2ExMasterClient::plugin() const
{
	return Turok2ExEnginePlugin::staticInstance();
}

MasterClient::Response Turok2ExMasterClient::readMasterResponse(const QByteArray &data)
{
	// Decompress the response.
	QBuffer ioBuffer;
	ioBuffer.setData(data);
	ioBuffer.open(QIODevice::ReadOnly);
	QDataStream inStream(&ioBuffer);
	inStream.setByteOrder(QDataStream::LittleEndian);
	DataStreamOperatorWrapper in(&inStream);

	while (in.remaining() >= 6)
	{
		// IP address is stored back to front
		quint8 ip4 = in.readQUInt8();
		quint8 ip3 = in.readQUInt8();
		quint8 ip2 = in.readQUInt8();
		quint8 ip1 = in.readQUInt8();

		quint16 port = in.readQUInt16();

		if (ip1 == 0 && ip2 == 0 && ip3 == 0 && ip4 == 0 && port == 0)
		{
			// end of list
			emit listUpdated();
			return RESPONSE_GOOD;
		}
		else
		{
			QString ip = QString("%1.%2.%3.%4").
				arg(ip1, 1, 10, QChar('0')).
				arg(ip2, 1, 10, QChar('0')).
				arg(ip3, 1, 10, QChar('0')).
				arg(ip4, 1, 10, QChar('0'));

			Turok2ExServer *server = new Turok2ExServer(QHostAddress(ip), port);
			registerNewServer(ServerPtr(server));

			seedIP = (ip1 << 24) | (ip2 << 16) | (ip3 << 8) | ip4;
			seedPort = port;
		}
	}

	// Still more servers to retrieve
	return RESPONSE_REPLY;
}

void Turok2ExMasterClient::refreshStarts()
{
	// Make sure we have an empty list.
	emptyServerList();
	seedIP = 0;
	seedPort = 0;

	MasterClient::refreshStarts();
}
