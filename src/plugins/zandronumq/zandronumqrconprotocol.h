//------------------------------------------------------------------------------
// zandronumqrconprotocol.h
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
#ifndef idd10651d2_c61d_498e_8ff6_88a782a8fa1c
#define idd10651d2_c61d_498e_8ff6_88a782a8fa1c

#include "huffmanudpsocket.h"
#include <QElapsedTimer>
#include <QTimer>
#include <serverapi/rconprotocol.h>

class ZandronumqRConProtocol : public RConProtocol
{
	Q_OBJECT

private:
	enum
	{
		SVRCU_PLAYERDATA = 0,
		SVRCU_ADMINCOUNT,
		SVRCU_MAP,

		SVRC_OLDPROTOCOL = 32,
		SVRC_BANNED,
		SVRC_SALT,
		SVRC_LOGGEDIN,
		SVRC_INVALIDPASSWORD,
		SVRC_MESSAGE,
		SVRC_UPDATE,

		CLRC_BEGINCONNECTION = 52,
		CLRC_PASSWORD,
		CLRC_COMMAND,
		CLRC_PONG,
		CLRC_DISCONNECT
	};

public:
	ZandronumqRConProtocol(ServerPtr server);
	void connectToServer();

public slots:
	void disconnectFromServer();
	void sendCommand(const QString &cmd);
	void sendPassword(const QString &password);
	void sendPong();

private:
	static const int MAX_CONNECTIONT_ATTEMPTS = 3;
	static const int MAX_PASSWORD_ATTEMPTS = 3;
	static const int AUTH_FLOOD_PREVENTION_PERIOD = 11 * 1000;

	enum ConnectStage
	{
		ConnectEstablishing,
		ConnectPassword,
		ConnectEstablished,
		Disconnected,
	};

	void processEstablishingPacket(QIODevice &ioDevice);
	void processPacket(QIODevice *ioDevice, bool initial = false, int maxUpdates = 1);
	void sendMemorizedPassword();
	void setDisconnectedState();
	/**
	 * @brief Reconnect state is similar to disconnect state, but
	 * doesn't clear failed attempts counters.
	 */
	void setReconnectState();

	QElapsedTimer authTime;
	ConnectStage connectStage;
	int connectionAttempts;
	int passwordAttempts;
	HuffmanUdpSocket huffmanSocket;
	QTimer pingTimer;
	QString hostName;
	QString password;
	QString salt;
	int serverProtocolVersion;
	QTimer timeoutTimer;

private slots:
	void readAllPendingDatagrams();
	void packetTimeout();
	void stepConnect();
};

#endif
