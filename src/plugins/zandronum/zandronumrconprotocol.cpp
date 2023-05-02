//------------------------------------------------------------------------------
// zandronumrconprotocol.cpp
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
// Copyright (C) 2009 Braden "Blzut3" Obrzut <admin@maniacsvault.net>
// Copyright (C) 2014 "Zalewa" <zalewapl@gmail.com>
//------------------------------------------------------------------------------
#include "zandronumrconprotocol.h"

#include "huffman/huffman.h"
#include "zandronumserver.h"
#include <datastreamoperatorwrapper.h>
#include <QBuffer>
#include <QCryptographicHash>
#include <QDateTime>
#include <QMessageBox>

#define RCON_PROTOCOL_VERSION   3

ZandronumRConProtocol::ZandronumRConProtocol(ServerPtr server)
	: RConProtocol(server)
{
	set_disconnectFromServer(&ZandronumRConProtocol::disconnectFromServer);
	set_sendCommand(&ZandronumRConProtocol::sendCommand);
	set_sendPassword(&ZandronumRConProtocol::sendPassword);

	connectStage = Disconnected;
	connectionAttempts = 0;
	passwordAttempts = 0;
	authTime.invalidate();

	huffmanSocket.setSocket(&socket());
	connect(&socket(), SIGNAL(readyRead()), this, SLOT(readAllPendingDatagrams()));

	// Note: the original rcon utility did TIMEOUT/4.
	// Try to get at least 4 packets in before timing out,
	pingTimer.setInterval(2500);
	connect(&pingTimer, SIGNAL(timeout()), this, SLOT(sendPong()));
	connect(&pingTimer, SIGNAL(timeout()), this, SLOT(readAllPendingDatagrams()));

	timeoutTimer.setSingleShot(true);
	this->connect(&timeoutTimer, SIGNAL(timeout()), SLOT(packetTimeout()));
}

void ZandronumRConProtocol::connectToServer()
{
	const char beginConnection[2] = { CLRC_BEGINCONNECTION, RCON_PROTOCOL_VERSION };

	if (connectionAttempts < MAX_CONNECTIONT_ATTEMPTS)
	{
		++connectionAttempts;
		emit messageReceived(tr("Connection attempt ..."));
		setConnected(false);
		huffmanSocket.writeDatagram(beginConnection, 2, address(), port());
		timeoutTimer.start(3000);
	}
	else
	{
		setDisconnectedState();
		emit messageReceived(tr("Too many failed connection attempts. Aborting."));
	}
}

void ZandronumRConProtocol::disconnectFromServer()
{
	if (isConnected())
	{
		const char disconnectPacket[1] = { CLRC_DISCONNECT };
		huffmanSocket.writeDatagram(disconnectPacket, 1, address(), port());
	}
	setDisconnectedState();
	emit disconnected();
}

void ZandronumRConProtocol::sendCommand(const QString &cmd)
{
	char packet[4096];
	packet[0] = CLRC_COMMAND;
	packet[cmd.length() + 1] = 0;
	memcpy(packet + 1, cmd.toUtf8().constData(), cmd.length());
	huffmanSocket.writeDatagram(packet, 4096, address(), port());
}

void ZandronumRConProtocol::sendPassword(const QString &password)
{
	this->password = password;
	stepConnect();
}

void ZandronumRConProtocol::sendMemorizedPassword()
{
	if (passwordAttempts < MAX_PASSWORD_ATTEMPTS)
	{
		++passwordAttempts;
		emit messageReceived(tr("Authenticating ..."));
		// Calculate the MD5 of the salt + password
		QString hashPassword = salt + password;
		QCryptographicHash hash(QCryptographicHash::Md5);
		hash.addData(hashPassword.toUtf8());
		QByteArray md5 = hash.result().toHex();

		// Create the packet
		char passwordPacket[34];
		passwordPacket[0] = CLRC_PASSWORD;
		memcpy(passwordPacket + 1, md5.data(), md5.size());
		passwordPacket[33] = 0;

		huffmanSocket.writeDatagram(passwordPacket, 34, address(), port());
		timeoutTimer.start(3000);
	}
	else
	{
		setDisconnectedState();
		emit messageReceived(tr("Too many failed authentication attempts. Aborting."));
	}
}

void ZandronumRConProtocol::setDisconnectedState()
{
	pingTimer.stop();
	setConnected(false);
	authTime.invalidate();
	connectStage = Disconnected;
}

void ZandronumRConProtocol::setReconnectState()
{
	setDisconnectedState();
	connectStage = ConnectEstablishing;
}

void ZandronumRConProtocol::stepConnect()
{
	switch (connectStage)
	{
	case Disconnected:
		connectionAttempts = 0;
		passwordAttempts = 0;
		connectStage = ConnectEstablishing;
		stepConnect();
		break;
	case ConnectEstablishing:
		connectToServer();
		break;
	case ConnectPassword:
		if (!authTime.isValid())
		{
			sendMemorizedPassword();
		}
		else
		{
			int delta = qMax(0, AUTH_FLOOD_PREVENTION_PERIOD - static_cast<int>(authTime.elapsed()));
			if (delta > 0)
			{
				emit messageReceived(
					tr("Delaying for about %n seconds before next authentication attempt.",
					nullptr, qMax(1, delta / 1000)));
			}
			setReconnectState();
			QTimer::singleShot(delta, this, SLOT(stepConnect()));
		}
		break;
	default:
		break;
	}
}

void ZandronumRConProtocol::sendPong()
{
	// create a "PONG" packet
	const char pong[1] = { CLRC_PONG };
	huffmanSocket.writeDatagram(pong, 1, address(), port());
}

void ZandronumRConProtocol::readAllPendingDatagrams()
{
	while (socket().hasPendingDatagrams())
	{
		timeoutTimer.stop();
		QByteArray packet = huffmanSocket.readDatagram();
		QBuffer stream(&packet);
		stream.open(QIODevice::ReadOnly);
		switch (connectStage)
		{
		case ConnectEstablishing:
			processEstablishingPacket(stream);
			break;
		case ConnectPassword:
		case ConnectEstablished:
			processPacket(&stream);
			break;
		case Disconnected:
			break;
		}
	}
}

void ZandronumRConProtocol::packetTimeout()
{
	switch (connectStage)
	{
	case ConnectEstablishing:
		emit messageReceived(tr("Failed to establish connection."));
		break;
	case ConnectPassword:
		emit messageReceived(tr("Timeout on authentication."));
		break;
	default:
		return;
	}
	stepConnect();
}

void ZandronumRConProtocol::processEstablishingPacket(QIODevice &ioDevice)
{
	QDataStream dataStream(&ioDevice);
	dataStream.setByteOrder(QDataStream::LittleEndian);
	DataStreamOperatorWrapper in(&dataStream);

	qint8 code = in.readQInt8();
	switch (code)
	{
	case SVRC_BANNED:
		emit messageReceived(tr("You have been banned from this server."));
		setDisconnectedState();
		break;
	default:
	case SVRC_OLDPROTOCOL:
		emit messageReceived(tr("The protocol appears to be outdated."));
		setDisconnectedState();
		break;
	case SVRC_SALT:
		setConnected(true);
		salt = QString(in.readRawUntilByte('\0'));
		pingTimer.start();
		connectStage = ConnectPassword;
		stepConnect();
		break;
	}
}

void ZandronumRConProtocol::processPacket(QIODevice *ioDevice, bool initial, int maxUpdates)
{
	static const QRegExp colorCode(R"(\\c(\[[a-zA-Z0-9]*\]|[a-v+\-!*]))");

	// QIODevice is assumed to be already opened at this point.
	QDataStream dataStream(ioDevice);
	dataStream.setByteOrder(QDataStream::LittleEndian);
	DataStreamOperatorWrapper in(&dataStream);

	while (in.hasRemaining() && maxUpdates-- != 0)
	{
		// Determine how we get to the update.
		int update = 0;
		if (initial)
			update = SVRC_UPDATE;
		else
			update = in.readQUInt8();

		switch (update)
		{
		default:
			qDebug() << "Unknown update (" << update << ")";
			return;
		case SVRC_INVALIDPASSWORD:
			authTime.start();
			emit messageReceived(tr("Authentication failure."));
			emit invalidPassword();
			break;
		case SVRC_LOGGEDIN:
		{
			emit messageReceived(tr("Remote console connection established."));
			emit messageReceived(tr("-----")); // Just a delimiter.
			connectStage = ConnectEstablished;
			serverProtocolVersion = in.readQUInt8();
			hostName = in.readRawUntilByte('\0');
			emit serverNameChanged(hostName);

			int numUpdates = in.readQUInt8();

			processPacket(ioDevice, true, numUpdates);

			int numStrings = in.readQUInt8();
			while (numStrings-- > 0)
			{
				QString message = in.readRawUntilByte('\0');
				message.replace(colorCode, "\034\\1");
				emit messageReceived(message.trimmed());
			}
			break;
		}
		case SVRC_MESSAGE:
		{
			QString message = QDateTime::currentDateTime().toString("[hh:mm:ss ap] ") + in.readRawUntilByte('\0');
			message.replace(colorCode, "\034\\1");
			emit messageReceived(message);
			break;
		}
		case SVRC_UPDATE:
			int updateType = in.readQUInt8();
			switch (updateType)
			{
			default:
				qDebug() << "Uknown streamlined update (" << update << ")";
				return;
			case SVRCU_MAP:
			{
				QString map = in.readRawUntilByte('\0');
				break;
			}
			case SVRCU_ADMINCOUNT:
			{
				// Unused:
				// int admins = in.readQUint8();
				// !!! MAKE SURE to remove the line below if line above
				// is uncommented!
				in.skipRawData(1);
				break;
			}
			case SVRCU_PLAYERDATA:
			{
				int players = in.readQUInt8();
				this->playersMutable().clear();
				while (players-- > 0)
				{
					QString player = in.readRawUntilByte('\0');
					this->playersMutable().append(Player(player, 0, 0));
				}
				emit playerListUpdated();
				break;
			}
			}
			break;
		}
	}
}
