//------------------------------------------------------------------------------
// srb2masterclient.h
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
#ifndef SRB2MASTERCLIENT_H
#define SRB2MASTERCLIENT_H

#include <QTcpSocket>
#include <QTimer>
#include <serverapi/masterclient.h>

class DataStreamOperatorWrapper;
class EnginePlugin;

namespace Srb2Master
{
struct Header;
struct ServerPayload;
}

QDataStream &operator<<(QDataStream &stream, const Srb2Master::Header &header);
QDataStream &operator>>(QDataStream &stream, Srb2Master::Header &header);

QDataStream &operator>>(QDataStream &stream, Srb2Master::ServerPayload &server);

class Srb2MasterClient : public MasterClient
{
	Q_OBJECT

public:
	Srb2MasterClient();
	const EnginePlugin *plugin() const override;
	void updateAddress();

public slots:
	void refreshStarts() override;

protected:
	QByteArray createServerListRequest() override;
	Response readMasterResponse(const QByteArray &data) override;

private:
	QTcpSocket socket;
	QTimer readTimer;

	void parseServerPayload(const QByteArray &payload);
	Srb2Master::Header readHeader();
	void sendChallenge();
	void timeoutRefreshEx() override;

private slots:
	void readResponse();
	void socketStateChanged(QAbstractSocket::SocketState state);
};

#endif
