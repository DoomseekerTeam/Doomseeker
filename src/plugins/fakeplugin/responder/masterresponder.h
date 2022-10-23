//------------------------------------------------------------------------------
// masterresponder.h
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
#ifndef PLUGIN_RESPONDER_MASTERRESPONDER_H
#define PLUGIN_RESPONDER_MASTERRESPONDER_H

#include <QByteArray>
#include <QList>
#include <QObject>

class ServerResponder;

class MasterResponder : public QObject
{
	Q_OBJECT

public:
	MasterResponder(QObject *parent = nullptr);
	~MasterResponder();

	bool bind(unsigned short port);

private:
	class PrivData;

	PrivData *d;

	bool areServerRespondersRunning();
	void bindServer(ServerResponder *server, unsigned short port);
	QList<QByteArray> buildResponsePackets();
	void startServerResponders();

private slots:
	void readPendingDatagrams();
	void readPendingDatagram();
	void respond();
};

#endif
