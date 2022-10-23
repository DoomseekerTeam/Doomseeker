//------------------------------------------------------------------------------
// srb2masterclient.cpp
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
// Copyright (C) 2016 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "srb2masterclient.h"

#include "srb2engineplugin.h"
#include "srb2server.h"
#include <datastreamoperatorwrapper.h>
#include <global.h>

#include <cstring>
#include <QBuffer>
#include <QDataStream>
#include <QHostAddress>

using namespace Srb2Master;

struct Srb2Master::ServerPayload
{
	friend QDataStream &::operator>>(QDataStream &stream, ServerPayload &server);

	// This defines the current size in bytes that the operator >> requires.
	static constexpr char SIZE = 84;
	QHostAddress address;
	quint16 port;
	QString name;
	qint32 room;
	QString version;

	ServerPayload() : port(0), room(0)
	{
	}

	bool isValid() const
	{
		return !address.isNull() && port > 0;
	}

private:
	void rawIp(QByteArray raw)
	{
		for (int i = 0; i < raw.length(); ++i)
		{
			if (raw[i] != '.' && !QChar(raw[i]).isDigit())
			{
				raw = raw.left(i);
				break;
			}
		}
		this->address = QHostAddress(QString(raw));
	}

	void rawPort(QByteArray raw)
	{
		for (int i = 0; i < raw.length(); ++i)
		{
			if (!QChar(raw[i]).isDigit())
			{
				raw = raw.left(i);
				break;
			}
		}
		this->port = raw.toShort();
	}
};

QDataStream &operator>>(QDataStream &stream, ServerPayload &server)
{
	DataStreamOperatorWrapper out = DataStreamOperatorWrapper(&stream);

	out.readRaw(16); // Header full of zeros.
	server.rawIp(out.readRaw(16));
	server.rawPort(out.readRaw(8));
	server.name = Srb2::asciiOnly(out.readRaw(32));
	server.room = out.readQInt32();
	server.version = out.readRaw(8);

	return stream;
}

///////////////////////////////////////////////////////////////////////////

struct Srb2Master::Header
{
	enum Type
	{
		GET_SHORT_SERVER = 205,
		ASK_SERVER = 206, /// Reply to GET_SHORT_SERVER
		ASK_SERVER_ALT = -838860800 /// Little-Endian ASK_SERVER (lol?)
	};

	qint32 id;
	qint32 type;
	qint32 room;
	quint32 length;

	Header()
	{
		memset(this, 0, sizeof(Header));
		this->id = MAKEID('D', 'O', 'O', 'M');
	}

	bool isGetShortServerReply() const
	{
		return (type == ASK_SERVER || type == ASK_SERVER_ALT) && length >= ServerPayload::SIZE;
	}

	bool isValid() const
	{
		Header invalid;
		return memcmp(this, &invalid, sizeof(Header)) != 0;
	}
};

QDataStream &operator<<(QDataStream &stream, const Header &header)
{
	stream << header.id;
	stream << header.type;
	stream << header.room;
	stream << header.length;
	return stream;
}

QDataStream &operator>>(QDataStream &stream, Header &header)
{
	stream >> header.id;
	stream >> header.type;
	stream >> header.room;
	stream >> header.length;
	return stream;
}

///////////////////////////////////////////////////////////////////////////

Srb2MasterClient::Srb2MasterClient() : MasterClient()
{
	this->connect(&socket, SIGNAL(readyRead()), SLOT(readResponse()));
	this->connect(&socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
		SLOT(socketStateChanged(QAbstractSocket::SocketState)));
	this->connect(&readTimer, SIGNAL(timeout()), SLOT(readResponse()));

	readTimer.setInterval(1000);
}

QByteArray Srb2MasterClient::createServerListRequest()
{
	return QByteArray();
}

void Srb2MasterClient::refreshStarts()
{
	MasterClient::refreshStarts();
	socket.close();
	socket.connectToHost(address(), port());
}

const EnginePlugin *Srb2MasterClient::plugin() const
{
	return Srb2EnginePlugin::staticInstance();
}

void Srb2MasterClient::readResponse()
{
	while (socket.bytesAvailable() > 0)
	{
		Header header = readHeader();

		if (header.length != ServerPayload::SIZE)
		{
			socket.close();
			emit listUpdated();
			break;
		}

		QByteArray payload = socket.read(header.length);
		if (header.isGetShortServerReply() &&
			payload.length() == static_cast<qint32>(header.length))
		{
			parseServerPayload(payload);
		}
	}
}

Header Srb2MasterClient::readHeader()
{
	QByteArray data = socket.read(sizeof(Header));
	if (data.length() < static_cast<qint64>(sizeof(Header)))
		return Header();
	QDataStream stream(data);
	stream.setByteOrder(QDataStream::BigEndian);

	Header header;
	stream >> header;
	return header;
}

void Srb2MasterClient::parseServerPayload(const QByteArray &payload)
{
	QDataStream stream(payload);
	stream.setByteOrder(QDataStream::BigEndian);

	ServerPayload info;
	stream >> info;

	if (info.isValid())
	{
		auto server = new Srb2Server(info.address, info.port);
		server->setName(info.name);
		server->setGameVersion(info.version);
		registerNewServer(ServerPtr(server));
	}
}

MasterClient::Response Srb2MasterClient::readMasterResponse(const QByteArray &data)
{
	Q_UNUSED(data);
	return RESPONSE_GOOD;
}

void Srb2MasterClient::socketStateChanged(QAbstractSocket::SocketState state)
{
	if (state == QAbstractSocket::ConnectedState)
	{
		sendChallenge();
		readTimer.start();
	}
	else if (state == QAbstractSocket::UnconnectedState)
		readTimer.stop();
}

void Srb2MasterClient::sendChallenge()
{
	QBuffer buffer;
	buffer.open(QIODevice::WriteOnly);
	QDataStream out(&buffer);
	out.setByteOrder(QDataStream::BigEndian);

	Header header;
	header.type = Header::GET_SHORT_SERVER;
	out << header;

	socket.write(buffer.data());
	socket.flush();
}

void Srb2MasterClient::timeoutRefreshEx()
{
	socket.close();
}
